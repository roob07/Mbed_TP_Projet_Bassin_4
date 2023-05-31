//copyright Rapahel Bourbousson
#include "mbed.h"
#include "Sortie.h"

#define nombre_de_ligne_max 3
#define nombre_de_tour_max 30

/****************fonctions****************/
void setup_start();  //setup interrupt fonctions and serial baud rate
void setup_wifi();   //function to setup wifi connection with all parameters       
void reception_ESP();//send bytes received from ESP8266 to pc(terminal)
void read_terminal();//read the terminal to setup competition parametres         
void setup_lignes(); //save the parametres obtained from read_terminal()
void set_lignes();   //used in setup_ligne
void empty_chaine_recus();//empty chaine_recus array
void BP_Init_ON();        //interrupt fonction when BP_Init is pressed (start timer to read time pressed)
void BP_Init_OFF();       //interrupt fonction when BP_Init is released (read timer) reset or startup depending on time pressed
void Bassin_Reset();      //reset and send interrupt on RAZ     
void Bassin_Startup_position();//launch startup procedure       
void Bassin_Start_Timer();     //Start the timer and BUZZ start signal
void start_depart();           //activate buzzer and turn on LED depart when starting 
void BUZZ_position();          //activate buzzer to give swimmers position
void transmit_reaction_wifi(); //transmit reaction timer via wifi once a player jump
void transmit_times_wifi();    //transmit the time of a swimmer via wifi every time a trigger is hit
volatile int temps_ms_to_MSC(volatile int temps_en_ms); //convertie temps from ms to MMSSCC 
void led_arrive();              //fonction to control led arriver

void Int_Carte_A_L1();         //Interrupt fonction when AL1 trig (count time and turn) ***** to complete (and also the rest)
void Int_Carte_B_L1();

void Int_Carte_A_L2();
void Int_Carte_B_L2();

void Int_Carte_A_L3();
void Int_Carte_B_L3();

/****************global variables****************/
/**setup_ligne and reception_ESP**/
volatile char Octet_recus_ESP;          //char to memorise received byte
volatile char chaine_recus[8];          //array to store char received from ESP8266
volatile int chaine_recus_itterator=0;  //itterate throught the array
volatile char SEXE;

int longueur_nage_choisie=0;            //memorise the length to swim
char nage_choisi=' ';                  //memorise the type of swim

volatile int Bassin_temps_tour_ligne [nombre_de_ligne_max] [nombre_de_tour_max];
volatile int Bassin_n_tour_ligne [nombre_de_ligne_max] = {0,0,0};
volatile int tab_classement[3] = {0,0,0};
volatile int itt_classement_ligne=0;
volatile bool flag_depart_lancer = false;   //flag used to buzz once(1) and turn on LED depart when starting
volatile bool flag_pos_buzz = false;        //flag used to buzz twice(2) when positioning
volatile bool flag_transmit_reac [3] = {false,false,false}; //flag used to trigger event to transmit a reaction time via wifi 
volatile bool flag_transmit_time [3] = {false,false,false}; //flag used to trigger event to transmit a time via wifi 
volatile bool flag_a_finit [3] = {false,false,false};       //flag used to marl when a player has finished

void setup_start(){
    pc.baud(115200);            //configure baud rate 
    wifi_emmiter.baud(115200);  //for terminal and ESP8266

    pc.attach(&read_terminal);
    wifi_emmiter.attach(&reception_ESP,Serial::RxIrq);

    BP_Starter.rise(&Bassin_Start_Timer);

    BP_Init.rise(&BP_Init_ON);  //attach interrupt fonction to BP
    BP_Init.fall(&BP_Init_OFF);

    Carte_A_ligne_1.rise(&Int_Carte_A_L1);
    Carte_B_ligne_1.rise(&Int_Carte_B_L1);

    Carte_A_ligne_2.rise(&Int_Carte_A_L2);
    // Carte_B_ligne_2.rise(&Int_Carte_B_L2);
}

void setup_wifi(){
    wifi_emmiter.printf("AT\r\n");          //AT  OK
    wait(0.2);
    wifi_emmiter.printf("ATE0\r\n");        //NOT Echo 
    wait(0.2);
    wifi_emmiter.printf("AT+CWMODE=1\r\n"); //STA Station
    wait(0.2);                              //connect to router
    wifi_emmiter.printf("AT+CWJAP=%cNETGEAR70%c,%croyalapple578%c\r\n",34,34,34,34);
    wait(8);
    wifi_emmiter.printf("AT+CIPSTA=%c192.168.70.2%c\r\n",34,34);    //change IP adresse IP fixe
    wait(1);
    wifi_emmiter.printf("AT+PING=%c192.168.70.3%c\r\n",34,34);//ping target
    wait(1);                                                  //start UDP transmition
    wifi_emmiter.printf("AT+CIPSTART=%cUDP%c,%c192.168.70.3%c,50000\r\n",34,34,34,34);
      //no longer needed left here as debug tool
       wait(1);
       wifi_emmiter.printf("AT+CIPSEND=7\r\n");//send lenth 7 byte message  //debug //no longer needed
       wait(1);
       wifi_emmiter.printf("BONJOUR");         //message of size 7 byte
    
}

void reception_ESP(){
    Octet_recus_ESP=wifi_emmiter.getc();//memorise byte received from ESP8266
    pc.putc(Octet_recus_ESP);           //then send it to pc(terminal)
}

void read_terminal(){
    chaine_recus[chaine_recus_itterator]=pc.getc();     //memorise byte received from pc
    if (chaine_recus[chaine_recus_itterator]==0x0D) {   //when receive CR
        setup_lignes();
        chaine_recus_itterator=0;
        empty_chaine_recus();         //empty the chaine array
    }
    else{
        chaine_recus_itterator++;       //add +1 to itteration
        if (chaine_recus_itterator>7) { //if superior to array size
            chaine_recus_itterator=0;
        }
    }
}

void setup_lignes(){
    switch (chaine_recus[0]) {
        case 'S':
            set_lignes();
            pc.printf("setup done %dm %c %c\n",(longueur_nage_choisie*50),chaine_recus[2],chaine_recus[3]); //debug
            break;
        
    }
}

void set_lignes(){
    switch (chaine_recus[1]) {
        case '1':
            longueur_nage_choisie=1;
            break;
        case '2':
            longueur_nage_choisie=2;
            break;
        case '3':
            longueur_nage_choisie=4;
            break;
        case '4':
            longueur_nage_choisie=8;
            break;
        case '5':
            longueur_nage_choisie=16;
            break;
        case '6':
            longueur_nage_choisie=30;
            break;
    }
            
    switch (chaine_recus[2]) {
        case 'B':
            nage_choisi='B';
            break;
        case 'C':
            nage_choisi='C';
            break;
        case 'P':
            nage_choisi='P';
            break;
        case 'Q':
            nage_choisi='Q';
            break;
        case 'D':
            nage_choisi='D';
            break;
    }
    switch (chaine_recus[3]) {
        case 'H':
            SEXE='H';
            break;
        case 'F':
            SEXE='F';
            break;
    }
}

void empty_chaine_recus(){
    /****************put every value to empty****************/
    for (volatile int i=0; i<8; i++) {
        chaine_recus[i]=' ';
    }
}

void BP_Init_ON(){
    temps_Init.reset();
    temps_Init.start();
}

void BP_Init_OFF(){
    temps_Init.stop();
    if (temps_Init.read_ms()<=1000) {
        temps_Init.reset();
        Bassin_Reset();
    }
    else {
        temps_Init.reset();
        Bassin_Startup_position();
    }
}

/***********************A MODIFIER***********************/ 
void Bassin_Reset(){
    temps_nage.stop();      //
    temps_nage.reset();
    temps_Init.stop();
    temps_Init.reset();
    for (int i=0;i<nombre_de_ligne_max;i++){
        Bassin_n_tour_ligne[i]=0;
    }
    flag_depart_lancer=false;
    for (int i=0;i<3;i++){
        flag_transmit_time[i]=false;
    }
    
    for (int i=0;i<3;i++){
        flag_a_finit[i]=false;
    }
    
    for (int i=0;i<3;i++){
        tab_classement[i]=false;
    }

    itt_classement_ligne=0;
    
    RAZ=1;
    wait_us(1000);
    RAZ=0;
    pc.printf("RESET\n");
}
/***********************A MODIFIER***********************/
void Bassin_Startup_position(){
    flag_pos_buzz=true;
    // pc.printf("Position\n"); //debug
}

void Bassin_Start_Timer(){
    temps_nage.start();
    flag_depart_lancer = true;
}

void start_depart(){
    if(flag_depart_lancer){
        flag_depart_lancer=false;
        
        Pin_depart_ligne_1=1;
        Pin_depart_ligne_2=1;
        Pin_depart_ligne_3=1;
        wait_us(1000);
        Pin_depart_ligne_1=0;
        Pin_depart_ligne_2=0;
        Pin_depart_ligne_3=0;

        BUZZER=1;
        wait_ms(500);
        BUZZER=0;
    }
}

void BUZZ_position(){
    if(flag_pos_buzz){
        flag_pos_buzz=false;
        BUZZER = 1;
        wait_ms(3000);      //nageurs aproach the plot de depart
        BUZZER = 0;

        wait_ms(1000);
        BUZZER = 1;
        wait_ms(1000);      //nageurs get on the plot de depart
        BUZZER = 0;
    }
}

void transmit_reaction_wifi(){
    if (flag_transmit_reac[0]) {
        // pc.printf("R1%03.f\n",(((float)Bassin_temps_tour_ligne[0][0])/10)); //debug
        wifi_emmiter.printf("AT+CIPSEND=5\r\n");
        wait(1);
        wifi_emmiter.printf("R1%03.f\n",(((float)Bassin_temps_tour_ligne[0][0])/10));
        flag_transmit_reac[0]=false;
    }
    
    if (flag_transmit_reac[1]) {
        // pc.printf("R2%03.f\n",(((float)Bassin_temps_tour_ligne[1][0])/10)); //debug
        wifi_emmiter.printf("AT+CIPSEND=5\r\n");
        wait(1);
        wifi_emmiter.printf("R2%03.f\n",(((float)Bassin_temps_tour_ligne[1][0])/10));
        flag_transmit_reac[1]=false;
    }

    if (flag_transmit_reac[2]) {
        // pc.printf("R2%03.f\n",(((float)Bassin_temps_tour_ligne[2][0])/10)); //debug
        // wifi_emmiter.printf("AT+CIPSEND=5\r\n");
        // wait(1);
        // wifi_emmiter.printf("R2%03.f\n",(((float)Bassin_temps_tour_ligne[2][0])/10));
        flag_transmit_reac[2]=false;
    }
}

void transmit_times_wifi(){
    if (flag_transmit_time[0]) {
        // pc.printf("I1%06.f\n",(float)temps_ms_to_MSC(Bassin_temps_tour_ligne[0][(Bassin_n_tour_ligne[0]-1)])); //debug
        wifi_emmiter.printf("AT+CIPSEND=8\r\n");
        wait(1);
        wifi_emmiter.printf("I1%06.f\n",(float)temps_ms_to_MSC(Bassin_temps_tour_ligne[0][(Bassin_n_tour_ligne[0]-1)]));
        flag_transmit_time[0]=false;
        
        if(((Bassin_n_tour_ligne[0])>=(longueur_nage_choisie+1))&&(flag_a_finit[0]==false)){      //if finished --> stop
            flag_a_finit[0]=true;                   //flag as to not have the same player twice 
            pc.printf("stop \n");
            tab_classement[itt_classement_ligne]=1;                              // ******* a débugger ******************
            itt_classement_ligne++;                                                     //increment place
            // pc.printf("%d est premier\n",tab_classement[0]); //debug
            // pc.printf("%d est deuxieme\n",tab_classement[1]);
        }
    }
    
    if (flag_transmit_time[1]) {
        // pc.printf("I2%06.f\n",(float)temps_ms_to_MSC(Bassin_temps_tour_ligne[1][(Bassin_n_tour_ligne[1]-1)])); //debug
        wifi_emmiter.printf("AT+CIPSEND=8\r\n");
        wait(1);
        wifi_emmiter.printf("I2%06.f\n",(float)temps_ms_to_MSC(Bassin_temps_tour_ligne[1][(Bassin_n_tour_ligne[1]-1)]));
        flag_transmit_time[1]=false;
        if(((Bassin_n_tour_ligne[1])>=(longueur_nage_choisie+1))&&(flag_a_finit[1]==false)){      //if finished --> stop
            flag_a_finit[1]=true;                   //flag as to not have the same player twice 
            pc.printf("stop \n");
            tab_classement[itt_classement_ligne]=2;                              // ******* a débugger ******************
            itt_classement_ligne++;                                                     //increment place
            // pc.printf("%d est premier\n",tab_classement[0]); //debug
            // pc.printf("%d est deuxieme\n",tab_classement[1]);
        }
    }

    if (flag_transmit_time[2]) {
        pc.printf("I3%06.f",(float)temps_ms_to_MSC(Bassin_temps_tour_ligne[2][(Bassin_n_tour_ligne[2]-1)])); //debug
        // wifi_emmiter.printf("AT+CIPSEND=8\r\n");
        // wait(1);
        // wifi_emmiter.printf("I3%06.f",(float)temps_ms_to_MSC(Bassin_temps_tour_ligne[2][Bassin_n_tour_ligne[2]]));
        flag_transmit_time[2]=false;
    }
}

volatile int temps_ms_to_MSC(volatile int temps_en_ms){
    volatile int mnt=0,sec=0,cent=0,temps_MSC;
    cent = (temps_en_ms%1000)/10;
    sec = (temps_en_ms/1000)%60;
    mnt = (temps_en_ms/60000)%100;

    temps_MSC=cent+sec*100+mnt*10000;
    return temps_MSC;
}


void led_arrive(){          
    switch (tab_classement[0]) {    //if premier
        case 0:
            Pin_led_arrive_0_1=0;
            Pin_led_arrive_1_1=0;
            Pin_led_arrive_0_2=0;
            Pin_led_arrive_1_2=0;
            break;
        case 1:
            Pin_led_arrive_0_1=0;
            Pin_led_arrive_1_1=1;
            break;
        case 2:
            Pin_led_arrive_0_2=0;
            Pin_led_arrive_1_2=1;
            break;
    }

    switch (tab_classement[1]) {    //if deuxieme
        case 1:
            Pin_led_arrive_0_1=1;
            Pin_led_arrive_1_1=0;
            break;
        case 2:
            Pin_led_arrive_0_2=1;
            Pin_led_arrive_1_2=0;
            break;
    }
}



void Int_Carte_A_L1(){
    // pc.printf("int_AL1\n");  
    Bassin_temps_tour_ligne [0] [Bassin_n_tour_ligne[0]] = temps_nage.read_ms();    //read timer store it into Bassin_temps array [ligne] [tour]

    if(((Bassin_temps_tour_ligne[0][0])<=300)&&(Bassin_n_tour_ligne[0]==0)) {       //if faux depart
        //                                  ********************faire clignoter led arrivée make a new fonction 
        flag_transmit_reac[0] = true; 
    }
    else if(((Bassin_temps_tour_ligne[0][0])>300)&&(Bassin_n_tour_ligne[0]==0)) {   //if bon depart
        flag_transmit_reac[0] = true;
    }

    else if((Bassin_n_tour_ligne[0]>=1)&&(flag_a_finit[0]==false)) {                      //une fois le saut effectuer
        flag_transmit_time[0]=true;
    }

    Bassin_n_tour_ligne[0]++;                                                       //increment the number of tour 
}

void Int_Carte_B_L1(){
    // pc.printf("int_BL1\n");
    Bassin_temps_tour_ligne [0] [Bassin_n_tour_ligne[0]] = temps_nage.read_ms();    //read timer store it into Bassin_temps array [ligne] [tour]

    if((Bassin_n_tour_ligne[0]>=1)&&(flag_a_finit[0]==false)) {                      //une fois le saut effectuer
        flag_transmit_time[0]=true;
    }

    Bassin_n_tour_ligne[0]++;                                                       //increment the number of tour 
}






//******************** A modifier une fois les test effectuer ********************//
void Int_Carte_A_L2(){
    Bassin_temps_tour_ligne [1] [Bassin_n_tour_ligne[1]] = temps_nage.read_ms();    //read timer store it into Bassin_temps array [ligne] [tour]

    if(((Bassin_temps_tour_ligne[1][0])<=300)&&(Bassin_n_tour_ligne[1]==0)) {       //if faux depart
        //                                  ********************faire clignoter led arrivée make a new fonction 
        flag_transmit_reac[1] = true; 
    }
    else if(((Bassin_temps_tour_ligne[1][0])>300)&&(Bassin_n_tour_ligne[1]==0)) {   //if bon depart
        flag_transmit_reac[1] = true;
    }

    else if((Bassin_n_tour_ligne[1]>=1)&&(flag_a_finit[1]==false)) {                      //une fois le saut effectuer
        flag_transmit_time[1]=true;
    }

    Bassin_n_tour_ligne[1]++;                                                       //increment the number of tour 
}

// void Int_Carte_B_L2(){
//     Bassin_temps_tour_ligne [1] [Bassin_n_tour_ligne[1]] = temps_nage.read_ms();
//     Bassin_n_tour_ligne[1]++;
// }



















/********* plus besoin (test) **********/
// void temps_centieme(volatile int cent){
//     volatile int centieme=0;
//     centieme=(cent%1000)/10;
//     return centieme;
// }
// void temps_seconde(volatile int sec){
//     volatile int seconde=0;
//     seconde=(sec/1000)%60;
//     return seconde;
// }
// void temps_minute(volatile int mnt){
//     volatile int minute=0;
//     minute=(mnt/60000)%10
//     return minute;
// }





/*  //paris
    pc.printf("Position\n");
    BUZZER=1; 
    wait_ms(220);
    BUZZER=0;
    wait_ms(220);
    BUZZER=1; 
    wait_ms(220);
    BUZZER=0;
    wait_ms(100);

    BUZZER=1; 
    wait_ms(120);
    BUZZER=0;
    wait_ms(120);
    BUZZER=1; 
    wait_ms(120);
    BUZZER=0;
    wait_ms(120);
    BUZZER=1; 
    wait_ms(120);
    BUZZER=0;
    wait_ms(200);

    BUZZER=1; 
    wait_ms(120);
    BUZZER=0;
    wait_ms(120);
    BUZZER=1; 
    wait_ms(120);
    BUZZER=0;
    wait_ms(120);
    BUZZER=1; 
    wait_ms(120);
    BUZZER=0;
    wait_ms(120);
    BUZZER=1; 
    wait_ms(120);
    BUZZER=0;
    wait_ms(250);

    BUZZER=1; 
    wait_ms(120);
    BUZZER=0;
    wait_ms(120);
    BUZZER=1; 
    wait_ms(120);
    BUZZER=0;
*/