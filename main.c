/*
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
void initleds(void){
    PORTA |= (1<<PA3)| (1<<PA4) | (1<<PA5)| (1<<PA6)| (1<<PA7);
}
void initknoppen(void){
    PORTA &= ~(1<<PA0) & ~(1<<PA1) & ~(1<<PA2);
}
void rechtsboomled(int aan){
    if (aan) PORTA |= (1<<PA3);
    else {
           PORTA &= ~(1<<PA3);
    }
}
void linksboomled(int aan){
    if (aan) PORTA |= (1<<PA4);
    else {
           PORTA &= ~(1<<PA4);
    }
}
void followled(int aan){
    if (aan) PORTA |= (1<<PA5);
    else {
           PORTA &= ~(1<<PA5);
    }
}
void onled(int aan){
    if (aan) PORTA |= (1<<PA6);
    else {
           PORTA &= ~(1<<PA6);
    }
}
void noodstopled(int aan){
    if (aan) PORTA |= (1<<PA7);
    else {
           PORTA &= ~(1<<PA7);
    }
}
void rrichting(int richting){
    if (richting){
        TCCR1A &= ~(1<<COM1A1); //zet vooruit uit
        TCCR1A |= (1<<COM1B1); //zet achteruit aan
    }
    else{
        TCCR1A &= ~(1<<COM1B1); //achteruit uit
        TCCR1A |= (1<<COM1A1); //vooruit aan
    }
}
void lrichting(int richting){
    if (richting){
            TCCR4A &= ~(1<<COM4B1); //achteruit uit
            TCCR4A |= (1<<COM4A1); //vooruit aan
    }
    else{
        TCCR4A &= ~(1<<COM4A1); //zet vooruit uit
        TCCR4A |= (1<<COM4B1); //zet achteruit aan
    }
}
int leestnoodstopuit(void){
        if ((PINA & (1<<PA2)) == 0) return(0);
        if ((PINA & (1<<PA2)) != 0) return(1);
}
int leestaanknopuit(void){
        if ((PINA & (1<<PA0)) == 0) return(0);
        if ((PINA & (1<<PA0)) != 0) return(1);
}
int leestfollowknopuit(void){
        if ((PINA & (1<<PA1)) == 0) return(0);
        if ((PINA & (1<<PA1)) != 0) return(1);
}
void initmotoren(void){
    //timer 1 de l richting:
    DDRB |= (1<<PB5) | (1<<PB6);
    TCCR1B |= (1<<CS12)|(1<<CS10); //CLOCK / 1024
    TCCR1A |= (1<<WGM12) |(1<<WGM10); //8 BIT FAST PWM MODE
    OCR1A = 127; //pin 11
    OCR1B = 127; //pin 12

    //timer 4 de r richting:
    DDRH |= (1<<PH3) | (1<<PH4);
    TCCR4B |= (1<<CS42)|(1<<CS40); //CLOCK / 1024
    TCCR4A |= (1<<WGM42) |(1<<WGM40); //8 BIT FAST PWM MODE
    OCR4A = 127; //pin6
    OCR4B = 127; //pin 7

    rrichting(0);
    lrichting(0);

    //limit switches init
    //sei();
    EIMSK |= (1<<INT4) | (1<<INT5);
    DDRE &= ~(1<<PE4) & ~(1<<PE5); //pin 2 en 3 als current sink
    EICRB |= (1<<ISC41) | (1<<ISC40) | (1<<ISC50) | (1<<ISC51); //rising edge op knoppen

}
ISR(INT4_vect){
    //x as links
    //rrichting(0);

}
ISR(INT5_vect){
    //x as rechts
    //rrichting(1);
}
void lsnelheid(int snelheid){
    OCR4A = snelheid;
    OCR4B = snelheid;
}
void rsnelheid(int snelheid){
    OCR1A = snelheid;
    OCR1B = snelheid;
}
int main(void)
{
    initleds();
    initknoppen();
    initmotoren();

    rechtsboomled(1);
    linksboomled(1);
    followled(0);
    onled(0);
    noodstopled(0);
    while(1){
        if(leestnoodstopuit()){
            noodstopled(1);
        }
        else{
            noodstopled(0);
        }
        if(leestfollowknopuit()){
            followled(1);
        }
        else{
            followled(0);
        }
        if(leestaanknopuit()){
            onled(1);
        }
        else{
            onled(0);
    }
    }
    return 0;
}
