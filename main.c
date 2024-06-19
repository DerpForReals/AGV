/*
STM Met Aangevulde Cases + Geüpdatet functies.
18/6/24
 */


// Included Libraries:
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "LCD.h"




// Standaard Set Variables:
int aan_Onthouden = 0; // Onthouden of knop is ingedrukt of niet
int noodstop_Onthouden; // Om te onthouden of de noodstop is ingedrukt

int draaienL_Onthouden; // Deze waardes zodat je weer van noodstop naar draaien kan
int draaienR_Onthouden;

int heeftZiel = 1; // De AGV heeft een ziel en heeft ook senses

int boomR_Gezien = 0; // Om bij te houden dat Rechter boom is gedetecteerd
int boomL_Gezien = 0; // Om bij te houden dat Linker boom is gedetecteerd

int bomenR_Geteld = 0; // Bomen bijhouden rechts
int bomenL_Geteld = 0; // Bomen bijhouden links

int rijstrook = 1; // Begint bij rijstrook 1

volatile uint16_t clicks;
volatile int flag = 0;
volatile int afstand;
volatile int fe = 0;
volatile int re = 0;



// Init Functies:
void init(void){
    initleds();
    initbuzzer();
    initknoppen();
    initmotoren();
    initsensoren();

    //initLCD();
    initsonars();
    /*updateLCDScreen(1, "clicks:", clicks, "");
    updateLCDScreen(2, "afstand:", afstand, "mm" ); */
}

void initleds(void){
    DDRA |= (1<<PA3)| (1<<PA4) | (1<<PA5)| (1<<PA6)| (1<<PA7);
    // BoomRechts; BoomLinks; Follow; AanLed; NoodstopAan
}

void initbuzzer(void){
    DDRL |= (1<<PL2); //pin 37
}

void initknoppen(void){
    DDRA &= ~(1<<PA0) & ~(1<<PA1) & ~(1<<PA2);
    // Follow; Aan/Uit; Noodstop 1 en 2 + Bumper
}

void initsensoren(void){
    DDRE &= (1<<PE3) & (1<<PE4) & (1<<PE5);
    DDRG &= (1<<PG5);
}

void initmotoren(void){
    //timer 1 is de L richting:
    DDRB |= (1<<PB5) | (1<<PB6);
    TCCR1B |= (1<<CS12)|(1<<CS10); //CLOCK / 1024
    TCCR1A |= (1<<WGM12) |(1<<WGM10); //8 BIT FAST PWM MODE
    OCR1A = 127; //pin 11
    OCR1B = 127; //pin 12

    //timer 4 is de R richting:
    DDRH |= (1<<PH3) | (1<<PH4);
    TCCR4B |= (1<<CS42)|(1<<CS40); //CLOCK / 1024
    TCCR4A |= (1<<WGM42) |(1<<WGM40); //8 BIT FAST PWM MODE
    OCR4A = 127; //pin6
    OCR4B = 127; //pin 7

    rrichting(0);
    lrichting(0);
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


// Reken Functies:
int draaiing(void){
    // Berekening om te kijken of rijstrook getal even of oneven is, zodat de AGV weet welke kant die op moet draaien.

    int uitwerking;

    uitwerking = rijstrook%2;

    return uitwerking;
}

int latVerschil(void){
    // Berekening om te kijken of de AGV te ver van zijn route afwijkt

    int verschil;

    verschil = leessonaruit(1) - leessonaruit(2);

    return verschil;
}



// Led Functies:
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


// Buzzer Functie:
void buzzer(int aan){
    if (aan) PORTL |= (1<<PL2);
    else {
            PORTL &= ~(1<<PL2);}
}


// Knop Functies:
int leestnoodstopuit(void){
        if ((PINA & (1<<PA2)) == 0){
            _delay_ms(20);
            return(0);
            }
        if ((PINA & (1<<PA2)) != 0){
            _delay_ms(20);
            return(1);
            }
}

int leestaanknopuit(void){
        if ((PINA & (1<<PA0)) == 0){
            _delay_ms(20);
            return(0);
            }
        if ((PINA & (1<<PA0)) != 0){
            _delay_ms(20);
            return(1);
            }
}

int leestfollowknopuit(void){
        if ((PINA & (1<<PA1)) == 0){
            _delay_ms(20);
            return(0);
            }
        if ((PINA & (1<<PA1)) != 0){
            _delay_ms(20);
            return(1);
            }
}



// Sensoren Functies:
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


// Sonars Uitlezen:
int leessonaruit(int sensor){
    switch(sensor){
default:
    return 0;
    break;
    //als je een getal opgeeft wat niet gelinkt is aan een sensor geeft de functie 0 terug.
case 0: // Voorkant
    //AB = 00
    PORTB &= ~(1<<PB0);
    PORTB &= ~(1<<PB1);
    break;
case 1: // Links
    //AB = 10
     PORTB |= (1<<PB0);
    PORTB &= ~(1<<PB1);
    break;
case 2: // Rechts
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



// Rijd Functies:
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


// Snelheid = 21 voor Aan
void lsnelheid(int snelheid){
    OCR4A = snelheid;
    OCR4B = snelheid;
}

// Snelheid = 20 voor Aan
void rsnelheid(int snelheid){
    OCR1A = snelheid;
    OCR1B = snelheid;
}



// Interrupts:
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




// Main Functie
int main(void){
    init();
    enum states {Rechtdoorrijden, DraaienR, DraaienL, Volgen, Noodstop, Uit, BoomSignaleren, RouteCorrigeren};
    int state;
    state = Uit;
    while(1){
    switch (state){
        case Noodstop:
            onled(0);
            noodstopled(1);


            // Snelheid uitzetten
            rsnelheid(0);
            lsnelheid(0);


            rrichting(0);
            lrichting(0);

            if(leestnoodstopuit() && (noodstop_Onthouden == 1))
            {
                noodstop_Onthouden = 0;
            }

            if(leestaanknopuit() && (noodstop_Onthouden == 0))
            {
                if(draaienL_Onthouden == 1){
                    state = DraaienL;
                    break;
                }
                else if(draaienR_Onthouden == 1){
                    state = DraaienR;
                    break;
                }
                else{
                    state = Rechtdoorrijden;
                    break;
                }
            }

            break;

        case Rechtdoorrijden:
            onled(1);

            // Laten rijden
            rsnelheid(20);
            lsnelheid(21);

            // 1 zodat die vooruit rijdt
            rrichting(1);
            lrichting(1);



            if(leestnoodstopuit() || (leessonaruit(0) < 25)){
                noodstop_Onthouden = 1;
                state = Noodstop;
                break;
            }
            if((leestaanknopuit()) && (aan_Onthouden == 1)){
                aan_Onthouden = 0;
                state = Uit;
                break;
            }

            if((detecteerboomlinks()) && (!boomL_Gezien)){
                boomL_Gezien = 1;
                state = BoomSignaleren;
                break;
            }
            else if((!detecteerboomlinks()) && (boomL_Gezien)){
                boomL_Gezien = 0;
                break;
            }

            if((detecteerboomrechts()) && (!boomR_Gezien)){
                boomR_Gezien = 1;
                state = BoomSignaleren;
                break;
            }
            else if((!detecteerboomrechts()) && (boomR_Gezien)){
                boomR_Gezien = 0;
                break;
            }

            if(((!detecteerlatlinks()) && (!detecteerlatrechts())) && (rijstrook > 4))
            {
                state = Uit;
                break;
            }

            if((!detecteerlatlinks()) && (draaiing() == 0)){
                draaienL_Onthouden = 1;
                state = DraaienL;
                break;
            }

            if((!detecteerlatrechts()) && (draaiing() == 1)){
                draaienR_Onthouden = 1;
                state = DraaienR;
                break;
            }

            if((latVerschil() > 10) || (latVerschil() < -10))
            {
                state = RouteCorrigeren;
                break;
            }

            break;

        case Uit:
            onled(0);


            PORTB |= (1<<PB6); //rechter motor uit
            PORTB |= (1<<PB7);  //linker motor uit
            DDRL &= ~(1<<PL1);//1A L
            DDRL &= ~(1<<PL2);//2A L
            lrichting(0);
            rrichting(0);

            /*
            if(leestnoodstopuit()){
                noodstop_Onthouden = 1;
                state = Noodstop;
                break;
            }
            */

            if((leestaanknopuit()) && (aan_Onthouden == 0)){
                aan_Onthouden = 1;
                state = Rechtdoorrijden;
                break;
            }

            if(leestfollowknopuit()){
                state = Volgen;
                break;
            }

            break;

        case DraaienR:

            // Laten draaien
            rsnelheid(10);
            lsnelheid(21);

            // uit state als sonar rechts minder dan 50 mm
            // Dan rechtdoor
            if(leessonaruit(2) < 30){
                draaienR_Onthouden = 0;
                rijstrook += 1;
                state = Rechtdoorrijden;
                break;
            }

            if(leestnoodstopuit()){
                noodstop_Onthouden = 1;
                state = Noodstop;
                break;
            }
            break;

        case DraaienL:

            // Laten rijden
            rsnelheid(20);
            lsnelheid(10);

            // uit state als sonar links minder dan 50 mm
            // Daarna weer rechtdoor rijden
            if(leessonaruit(1) < 30){
                draaienL_Onthouden = 0;
                rijstrook += 1;
                state = Rechtdoorrijden;
                break;
            }

            if(leestnoodstopuit()){
                noodstop_Onthouden = 1;
                state = Noodstop;
                break;
            }
            break;

        case Volgen:
            followled(1);

            // In deze toestand rijdt hij niet, omdat het (nog) niet geprogrammeerd is
            rsnelheid(0);
            lsnelheid(0);

            if(leestaanknopuit()){
                state = Uit;
                break;
            }

            if(leestnoodstopuit()){
                noodstop_Onthouden = 1;
                state = Noodstop;
                break;
            }
            break;

        case BoomSignaleren:


            // Snelheid uitzetten
            rsnelheid(0);
            lsnelheid(0);

            rrichting(0);
            lrichting(0);


            if(boomL_Gezien){
                bomenL_Geteld += 1;
                linksboomled(1);
            }
            else if(boomR_Gezien){
                bomenR_Geteld += 1;
                rechtsboomled(1);
            }

            // Buzzer gaat 1 seconde aan en noodstop kan gedetecteerd worden
            buzzer(1);

            for(int i = 0; i < 11; i++){
                if(leestnoodstopuit()){
                    buzzer(0);
                    noodstop_Onthouden = 1;
                    state = Noodstop;
                    break;
                }
                _delay_ms(100);
            }

            buzzer(0);


            // Even de led aan laten
            for(int i = 0; i < 31; i++){
                if(leestnoodstopuit()){
                    linksboomled(0);
                    rechtsboomled(0);
                    noodstop_Onthouden = 1;
                    state = Noodstop;
                    break;
                }
                _delay_ms(100);
            }


            linksboomled(0);
            rechtsboomled(0);

            if(leestnoodstopuit()){
                noodstop_Onthouden = 1;
                state = Noodstop;
                break;
            }
            else{
                state = Rechtdoorrijden;
                break;
            }
            break;
    }
    }
    return 0;
}

// Voor debugging:
/*
while(1){
        _delay_ms(1000);
        updateLCDScreen(1, "afstand:",leessonaruit(0), "mm");
        updateLCDScreen(2, "sensor:", 0, "" );
        _delay_ms(1000);
        updateLCDScreen(1, "afstand:",leessonaruit(1), "mm");
        updateLCDScreen(2, "sensor:", 1, "" );
    }
*/
