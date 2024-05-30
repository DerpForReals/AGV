/*
v1
L293D

BLAUW = 1,2EN OCR1A = PIN11
GROEN = 1A PL1 = PIN48
GEEL = 2A  PL2 = PIN47


GEEL = 3,4EN OCR1B = PIN12
ORANGJE = 3A PB3 = PIN50
ROOD = 4A    PB2 = PIN51

 */

#include <avr/io.h>


void Lrichting(int richting){
    if (richting){
            PORTL |= (1<<PL1); //1A H
            PORTL &= ~(1<<PL2);//2A L
    }
    else{
            PORTL &= ~(1<<PL1);//1A L
            PORTL |= (1<<PL2); //2A H
    }
}
void Rrichting(int richting){
    if (richting){
            PORTB |= (1<<PB3); //3A H
            PORTB &= ~(1<<PB2);//4A L
    }
    else{
            PORTB &= ~(1<<PB3);//3A L
            PORTB |= (1<<PB2); //4A H
    }
}
void startup(void){
DDRB |= (1<<PB4); //led 4 OC2A noodstop
DDRB |= (1<<PB5); //led 3 OC1A
DDRB |= (1<<PB6); //led 2 OC1B rechter motor
DDRB |= (1<<PB7); //led 1 OC0A/OC1C linker motor

DDRF &= ~(1<<PF1); //knop 1 noodstop
DDRF &= ~(1<<PF2); //knop 2 startknop
DDRF &= ~(1<<PF3); //knop 3 stopknop

PORTF |= (1<<PF1); //interne pullup aan
PORTF |= (1<<PF2); //interne pullup aan
PORTF |= (1<<PF3); //interne pullup aan

PORTB |= (1<<PB4) | (1<<PB5) | (1<<PB6) |(1<<PB7);

DDRL |= (1<<PL3) | (1<<PL4);
TCCR5B |= (1<<CS52)|(1<<CS50); //CLOCK / 1024
TCCR5A |= (1<<WGM52) |(1<<WGM50); //8 BIT FAST PWM MODE
OCR5A = 127; //pin46 Rpwm
OCR5B = 127; //pin45 Lpwm

DDRL |= (1<<PL1) | (1<<PL2);//PL1 = PIN48, PL2 = PIN47
DDRB |= (1<<PB2) | (1<<PB3); //PB2 = PIN51, PB3 = PIN50
}
int main(void){
    startup();
    enum states { Start, Rechtdoorrijden, Noodstop,Stop};
    int state;
    state = Start;
    while(1){
    switch (state){
        case Noodstop:
            PORTB |= (1<<PB6); //rechter motor uit
            PORTB |= (1<<PB7);  //linker motor uit
            PORTB &= ~(1<<PB4); //noodstop led aan
            PORTL &= ~(1<<PL1);//1A L
            PORTL &= ~(1<<PL2);//2A L

            PORTB &= ~(1<<PB3);//3A L
            PORTB &= ~(1<<PB2);//4A L

            break;
        case Rechtdoorrijden:
            PORTB &= ~(1<<PB6); //rechter motor aan
            PORTB &= ~(1<<PB7);  //linker motor aan
            Lrichting(1);
            Rrichting(1);
            if(!(PINF&(1<<PF1))){
                    state = Noodstop;
                    break;
            }
            if(!(PINF&(1<<PF3))){
                    state = Stop;
                    break;
            }
            break;
        case Start:
            PORTB &= ~(1<<PB5);
            if(!(PINF&(1<<PF2))){
                    state = Rechtdoorrijden;
                    break;
            }
            if(!(PINF&(1<<PF1))){
                    state = Noodstop;
                    break;
            }
            break;
        case Stop:
            PORTB |= (1<<PB6); //rechter motor uit
            PORTB |= (1<<PB7);  //linker motor uit
            DDRL &= ~(1<<PL1);//1A L
            DDRL &= ~(1<<PL2);//2A L
            Lrichting(0);
            Rrichting(0);
            if(!(PINF&(1<<PF1))){
                    state = Noodstop;
                    break;
            }
            if(!(PINF&(1<<PF2))){
                    state = Rechtdoorrijden;
                    break;
            }
            break;
    }
    }
    return 0;
}
