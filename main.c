/*
STM Met Aangevulde Cases + Geüpdatet functies.
18/6/24
 */


// Included Libraries:
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>




// Standaard Set Variables:
int aan_Onthouden = 0; // Onthouden of knop is ingedrukt of niet

int boomR_Gezien = 0; // Om bij te houden dat Rechter boom is gedetecteerd
int boomL_Gezien = 0; // Om bij te houden dat Linker boom is gedetecteerd

int bomenR_Geteld = 0; // Bomen bijhouden rechts
int bomenL_Geteld = 0; // Bomen bijhouden links

int rijstrook = 1; // Begint bij rijstrook 1



// Init Functies:
void init(void){
    initleds();
    initknoppen();
    initmotoren();
    initsensoren();
}

void initleds(void){
    DDRA |= (1<<PA3)| (1<<PA4) | (1<<PA5)| (1<<PA6)| (1<<PA7);
    // BoomRechts; BoomLinks; Follow; AanLed; NoodstopAan
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



// Knop Functies:
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






// Main Functie
int main(void){
    init();
    enum states {Rechtdoorrijden, Draaien, Volgen, Noodstop, Uit, BoomSignaleren};
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


            break;

        case Rechtdoorrijden:
            onled(1);

            // Laten rijden
            rsnelheid(20);
            lsnelheid(21);

            // 1 zodat die vooruit rijdt
            rrichting(1);
            lrichting(1);


            if(leestnoodstopuit()){
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

            if((!detecteerlatlinks()) && (!detecteerlatrechts())){
                state = Draaien;
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
            if(leestnoodstopuit()){
                    state = Noodstop;
                    break;
            }
            if((leestaanknopuit()) && (aan_Onthouden == 0)){
                    aan_Onthouden = 1;
                    state = Rechtdoorrijden;
                    break;
            }
            break;

        case Draaien:

            break;

        case Volgen:
            followled(1);

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

            // buzzer aan, moet nog

            // Even de led aan laten
            for(int i = 0; i < 7; i++){
                if(leestnoodstopuit()){
                    state = Noodstop;
                    break;
            }
                _delay_ms(500);
            }


            linksboomled(0);
            rechtsboomled(0);

            if(leestnoodstopuit()){
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
