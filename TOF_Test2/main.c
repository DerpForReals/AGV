/* Use analog 7, PF7 for yellow */

#ifndef F_CPU
#define F_CPU 16000000
#endif
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// Aansluitingen:
// 8: SDI	-> PH5
// 7: SFTCLK	-> PH4
// 4: LCHCLK	-> PG5
#define SDI_BIT		PH5
#define DDR_SDI		DDRH
#define PORT_SDI	PORTH

#define SFTCLK_BIT	PH4
#define DDR_SFTCLK	DDRH
#define PORT_SFTCLK	PORTH

#define LCHCLK_BIT	PG5
#define DDR_LCHCLK	DDRG
#define PORT_LCHCLK	PORTG

#define INITIAL_TIMER_VALUE 3036

static unsigned int segmentcodes[] = {
	~0xFC, ~0x60, ~0xDA, ~0xF2,
	~0x66, ~0xB6, ~0xBE, ~0xE0,
	~0xFE, ~0xF6, ~0xEE, ~0x3E,
	~0x9C, ~0x7A, ~0x9E, ~0x8E };

void init_display (void)
{
	// Initialiseer de pinnen voor datain, shiftclk en latchclk als output
	DDR_SDI    |= (1 << SDI_BIT);
	DDR_SFTCLK |= (1 << SFTCLK_BIT);
	DDR_LCHCLK |= (1 << LCHCLK_BIT);

	// Maak shiftclk en latchclk laag
	PORT_SFTCLK &= ~(1 << SFTCLK_BIT);
	PORT_LCHCLK &= ~(1 << LCHCLK_BIT);
}

void send_data(char data)
{
	for (unsigned i = 0; i < 8; i++)
	// Herhaal voor alle bits in een char
	{
		// Bepaal de waarde van de bit die je naar het schuifregister
		// wil sturen
		int bit = data & 1;
		data >>= 1;

		// Maak de juiste pin hoog of laag op basis van de bepaalde waarde
		// van het bit
		if (bit)
		{
			PORT_SDI |= (1 << SDI_BIT);
		}
		else
		{
			PORT_SDI &= ~(1 << SDI_BIT);
		}

		// Toggle shiftclk (hoeveel tijd moest het signaal minimaal hoog zijn?)
		// Puls moet minimaal 13 ns hoog zijn. Een clk cycle op de Arduino duurt
		// 62 ns, dus signaal kan hoog en de volgende cycle weer omlaag
		PORT_SFTCLK |= (1 << SFTCLK_BIT);
		PORT_SFTCLK &= ~(1 << SFTCLK_BIT);
	}
}

void send_enable(int display_nummer)
{
	send_data(0x10 << display_nummer);
}

void display(char data, int display_nummer)
{
	send_data(data);
	send_enable(display_nummer);

	// Toggle latchclk (hoeveel tijd moest het signaal minimaal hoog zijn?)
	// Puls moet minimaal 13 ns hoog zijn. Een clk cycle op de Arduino duurt
	// 62 ns, dus signaal kan hoog en de volgende cycle weer omlaag
	PORT_LCHCLK |= (1 << LCHCLK_BIT);
	PORT_LCHCLK &= ~(1 << LCHCLK_BIT);
}

void display_getal(unsigned int getal)
{
	int deler = 1000; //origineel = 1000
	int niet_0_gezien = 0;
	for (int i = 3; i >= 0; i--)
	{
		int cijfer = (getal/deler)%10;
		if (cijfer || niet_0_gezien || deler == 1)
		{
			display(segmentcodes[cijfer], i);
			_delay_ms(1);   // 1 kHz
			niet_0_gezien = 1;
		}
		deler /= 10;
	}
}

static volatile int adc_value;
static volatile int distance;

ISR(ADC_vect)
{
	adc_value = ADCL +(ADCH<<8);

	distance = 29.988 * pow((adc_value*0.0048828125), -1.173); // 5/1024 = 0.00488
}

void init_adc(void)
{
	ADMUX = (0<<REFS1) | (1<<REFS0);
	ADMUX |= (1<<MUX2) | (1<<MUX1) | (1<<MUX0); // A7 gebruiken

	ADCSRA = (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
	ADCSRA |= (1<<ADEN);
	ADCSRA |= (1<<ADIE);
}

void start_adc(void)
{
	ADCSRA |= (1<<ADSC);
}

int adc_done(void)
{
	return (ADCSRA & (1<<ADSC)) == 0;
}

static volatile int show_value = 0;

ISR(TIMER3_OVF_vect)
{
	TCNT3 = INITIAL_TIMER_VALUE;
	start_adc();

	// Set to allow display update
	//show_value ^= 1;
}

void init_timer3(void)
{
	// 1 s = 16000000 cycles.
	// 16000000/(2^16) = 224, gebruik prescaler van 256
	// 16000000/256 = 62500. Overflow treedt op bij 65536
	// Om timing precies te krijgen, moet TCNT3 starten op 65536-62500 = 3036,
	// zodat te teller precies 62500 keer wordt verhoogd voordat de overflow
	// optreedt. Let op dat dit telkens moet gebeuren!
	TCNT3 = INITIAL_TIMER_VALUE;

	// Enable overflow interrupt
	TIMSK3 = (1<<TOIE3);
}

void start_timer3(void)
{
	// prescaler 256: CS = 0b100
	TCCR3B = (1<<CS32) | (0<<CS31) | (0<<CS30);
}

void init(void)
{
	init_display();
	init_adc();
	init_timer3();
	start_timer3();
	sei();
}

int main(void)
{
	init();


	//start_adc();
	//while (!adc_done()) { }

	while (1)
	{
		/*if (adc_done())
		{
		    start_adc();
			//adc_value = ADC;
		}*/
		if (1)
		{
			display_getal(distance);

			//show_value = 0;
		}
	}
}

