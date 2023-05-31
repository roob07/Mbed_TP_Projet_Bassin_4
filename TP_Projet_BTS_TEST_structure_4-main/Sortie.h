//copyright Rapahel Bourbousson
#include "mbed.h"

Serial pc(USBTX, USBRX); // tx, rx (terminal)
Serial wifi_emmiter(p28,p27); //configure pin wifi ESP8266

InterruptIn BP_Starter (p5,PullDown);   //(JUMP) Start Timer and Buzz
InterruptIn BP_Init (p6,PullDown);      //(Initialisation) Reset(short) or Start launch(long) depending on press time ***

InterruptIn Carte_A_ligne_1 (p12, PullDown);    //interrupt for Carte_Plot at ligne 1 
InterruptIn Carte_B_ligne_1 (p13, PullDown);    //interrupt for Carte_extremite at ligne 1

InterruptIn Carte_A_ligne_2 (p17, PullDown);    //same for ligne 2
InterruptIn Carte_B_ligne_2 (p18, PullDown);

InterruptIn Carte_A_ligne_3 (p23, PullDown);    //same for ligne 3
InterruptIn Carte_B_ligne_3 (p24, PullDown);

DigitalOut wifi_reset(p29,1);//pin to reset wifi module with 0
DigitalOut RAZ(p20);         //pin to send reset pulse to every Carte (and verify proper fonction)
DigitalOut BUZZER(p30,0);    //pin to controle buzzer

DigitalOut Pin_depart_ligne_1(p7,0);    //pin to signal depart to carte 1 (allume led depart) ******* changer par un bus ? *******
DigitalOut Pin_depart_ligne_2(p14,0);   //same
DigitalOut Pin_depart_ligne_3(p19,0);

DigitalOut Pin_led_arrive_0_1(p11,0);
DigitalOut Pin_led_arrive_1_1(p8,0);
DigitalOut Pin_led_arrive_0_2(p16,0);
DigitalOut Pin_led_arrive_1_2(p15,0);

Timer temps_nage;
Timer temps_Init;
