//copyright Rapahel Bourbousson 
#include "mbed.h"
#include "my_header.h"

int main() {
    
    setup_start();
    
    pc.printf("Hello world\n"); //debug

    setup_wifi();            //******** not used when debuging
    
    while(1) {
        start_depart();
        BUZZ_position();
        transmit_reaction_wifi();
        transmit_times_wifi();
        led_arrive();
        // wait_us(10);
    }
}