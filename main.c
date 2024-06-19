/*
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

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
void initsonars(void){
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
void initleds(void){
    DDRA |= (1<<PA3)| (1<<PA4) | (1<<PA5)| (1<<PA6)| (1<<PA7);
}

void initknoppen(void){
    DDRA &= ~(1<<PA0) & ~(1<<PA1) & ~(1<<PA2);
}
void initsensoren(void){
    DDRE &= (1<<PE3) & (1<<PE4) & (1<<PE5);
    DDRG &= (1<<PG5);
}
void rechtsboomled(int aan){
    if (aan) PORTA |= (1<<PA3);
    else {
           PORTA &= ~(1<<PA3);
    }
}
void initbuzzer(void){
        DDRG |= (1<<PG1); //pin 40
    }
void buzzer(int aan){
        if (aan) PORTG |= (1<<PG1);
        else {
               PORTG &= ~(1<<PG1);
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
            TCCR5A &= ~(1<<COM5B1); //achteruit uit
            TCCR5A |= (1<<COM5A1); //vooruit aan
    }
    else{
        TCCR5A &= ~(1<<COM5A1); //zet vooruit uit
        TCCR5A |= (1<<COM5B1); //zet achteruit aan
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
int detecteerboomlinks(void){
    //PIN 2 = PE4
    if ((PINE & (1<<PE4)) == 0) return(0);
    if ((PINE & (1<<PE4)) != 0) return(1);
}
int detecteerboomrechts(void){
    //PIN 3 = PE5
    if ((PINE & (1<<PE5)) == 0) return(0);
    if ((PINE & (1<<PE5)) != 0) return(1);
}
int detecteerlatlinks(void){
    //PIN 4 = PG5
    if ((PING & (1<<PG5)) == 0) return(0);
    if ((PING & (1<<PG5)) != 0) return(1);
}
int detecteerlatrechts(void){
    //PIN 5 = PE3
    if ((PINE & (1<<PE3)) == 0) return(0);
    if ((PINE & (1<<PE3)) != 0) return(1);
}
void initmotoren(void){
    //timer 1 de l richting:
    DDRB |= (1<<PB5) | (1<<PB6);
    TCCR1B |= (1<<CS12)|(1<<CS10); //CLOCK / 1024
    TCCR1A |= (1<<WGM12) |(1<<WGM10); //8 BIT FAST PWM MODE
    OCR1A = 127; //pin 11
    OCR1B = 127; //pin 12

    //timer 5    de r richting:
    DDRL |= (1<<PL3) | (1<<PL4);
    TCCR5B |= (1<<CS52)|(1<<CS50); //CLOCK / 1024
    TCCR5A |= (1<<WGM52) |(1<<WGM50); //8 BIT FAST PWM MODE
    OCR5A = 127; //pin 46
    OCR5B = 127; //pin 45

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
    OCR5A = snelheid;
    OCR5B = snelheid;
}
void rsnelheid(int snelheid){
    OCR1A = snelheid;
    OCR1B = snelheid;
}
int main(void)
{
    initLCD();
    initleds();
    initknoppen();
    initmotoren();
    initsensoren();
    initsonars();
    initbuzzer();
    rechtsboomled(1);
    linksboomled(1);
    followled(0);
    onled(0);
    noodstopled(0);
    updateLCDScreen(2, "afstand:", afstand, "mm" );
    updateLCDScreen(1, "state1:", 0, "" );
    lsnelheid(41);
    rsnelheid(40);
    buzzer(1);
    _delay_ms(100);
    buzzer(0);
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
    updateLCDScreen(2, "rechts:", leessonaruit(0), "mm" );
    _delay_ms(1000);
    updateLCDScreen(2, "links:", leessonaruit(1), "mm" );
    _delay_ms(1000);
    updateLCDScreen(2, "voor:", leessonaruit(2), "mm" );
    _delay_ms(1000);
    }
    return 0;
}
