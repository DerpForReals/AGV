/*
timer rekensom van counts naar afstand E = 10^

kloksnelheid = 16E6 hz 1 cilus duurt 1/16E6 = 6.25E-8 sec
16 bits timer dus timer overflowt na 2^16 * 6.25E-8 sec = 4.096E-3 sec of na 4.096ms

geluidssnelheid bij 20 graden is 343,4 m/s  = 34340 cm/s

omdat de gemeten tijd ook de uitzendtijd meeneemt moeten we delen door twee

afstand in cm = 34340/2 * tijd in seconden => afstand in cm = 17170 * tijd in seconden

max meetafstand is 17170 * 4.096E-3 sec = 70.32832 cm

we meten helaas geen seconden maar clicks, één click is 6.25E-8 sec = 1.073125E-3 cm

counterwaarde * 0.001073125 = afstand in cm

als de counter overflowt is de meting out of range


kloksnelheid = 16E6 hz 1 cilus duurt 1/16E6 = 6.25E-8 sec
met een klokdeler van /8 = 5E-7 sec

1 click = 17170 cm/s * 5E-7 sec = 8.585E-3 cm

0.008585 cm
0.08585 mm

 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "LCD.h"


volatile uint16_t clicks;
volatile int flag = 0;
volatile int afstand;
volatile int fe =0;
volatile int re = 0;


void innitsensoren(void){
    /* ECHO PL0, ICP4 = PIN49
       TRIGGER PB2, PIN51
       MULTIPLEXER A PB0, PIN53 ->  pin 14 op de chip
       MULTIPLEXER B PB1, PIN52 -> pin 2 op de chip
    */



    DDRL &= ~(1<<PL0); //ICP4 als input
    DDRB |= (1<<PB1); //MUXA als output
    DDRB |= (1<<PB0); //MUXB als output
    DDRB |= (1<<PB2); //TRIGER als output
    //DDRH |= (1<<PH3); //S0 enable als output
    //DDRH |= (1<<PH4); //S1 enable als output

    PORTH &= ~(1<<PH3); PORTH &= ~(1<<PH4); //alle sensoren uit

    TCCR4B = (1<<CS11); // /8 klokdeler
	TCCR4B |= (1<<ICNC4); //enable input capture noise canceler
	TCCR4B |= (1<<ICES4); //change input capture edge select on rising edge
	TIMSK4 |= (1<<ICIE4) | (1<<TOIE4); //enable input capture interrupt & timer overflow
	TIFR4 = (1<<ICF4) | (1<<TOV4); //clear input capture flag & timer overflow flag
    TCNT4 = 0;
	flag = 0;
    sei();
}
int leessonaruit(int sensor){
    switch(sensor){
default:
    return 0;
    break;
    //als je een getal opgeeft wat niet gelinkt is aan een sensor geeft de functie 0 terug.
case 0:
    //AB = 00
    PORTB &= ~(1<<PB0);
    PORTB &= ~(1<<PB1);
    break;
case 1:
    //AB = 10
     PORTB |= (1<<PB0);
    PORTB &= ~(1<<PB1);
    break;
case 2:
    //AB =01
     PORTB &= ~(1<<PB0);
    PORTB |= (1<<PB1);
    break;
case 3:
    //AB = 11
    PORTB |= (1<<PB0);
    PORTB |= (1<<PB1);
    break;
    }
    PORTB |= (1<<PB2);
    _delay_us(2);
    PORTB &= ~(1<<PB2);
        flag = 0;
        TCNT4 = 0;
    	TCCR4B |= (1<<CS40); //start de timer
    	TIMSK4 |= (1<<ICIE4); //enable input capture interrupts
    	while(flag < 2){}
    	//clicks = fall_edge - rise_edge;
    	//updateLCDScreen(1, "RE:",re, "");
    	afstand = (fe *0.858)-20;
    	if (afstand < 0 ) afstand = 0;
       return afstand;
}
ISR(TIMER4_CAPT_vect){
	if (flag == 0)  //rising edge
	{
		re = ICR4; //save rise fall edge time to input capture register
		TCCR4B &= ~(1<<ICES4); //change input capture edge select on falling edge
		TCNT4 = 0;
	}
	if (flag == 1) //falling edge
	{
		fe = ICR4; //save fall edge time to input capture register
        TCCR4B |= (1<<ICES4); //change input capture edge select on rising edge
		TIMSK4 &= ~(1<<ICIE4); //disable input capture interrupt
		TCCR4B &= ~(1<<CS40); //stop de timer
		TCNT1 = 0;
	}
	flag ++; //increment flag
}
ISR(TIMER4_OVF_vect){
    flag = 3;
    fe = 0;
    re = 0;
    TCCR4B |= (1<<ICES4); //change input capture edge select on rising edge
    TIMSK4 &= ~(1<<ICIE4); //disable input capture interrupt
    TCCR4B &= ~(1<<CS40); //stop de timer
    TCNT1 = 0;
}

void setup(void){

    initLCD();
    innitsensoren();
    updateLCDScreen(1, "clicks:", clicks, "");
    updateLCDScreen(2, "afstand:", afstand, "mm" );
}
int batchmeting(int sensor, int aantal){
    if (aantal < 0) return 0;
    if (aantal > 100) return 0;
    int som;
        for(int i=0; i<aantal;++i){
            som = som + leessonaruit(sensor);
        }
        som = som / aantal;
        return som;
}
int main(void)
{
    setup();
    while(1){
        _delay_ms(1000);
        updateLCDScreen(1, "afstand:",leessonaruit(0), "mm");
        updateLCDScreen(2, "sensor:", 0, "" );
        _delay_ms(1000);
        updateLCDScreen(1, "afstand:",leessonaruit(1), "mm");
        updateLCDScreen(2, "sensor:", 1, "" );
    }
    return 0;
}
