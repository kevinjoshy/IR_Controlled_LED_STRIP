/* FinalProj.c | Created: 11/19/2021 1:05:24 PM | Author : kej16104 */

#define F_CPU 16000000UL // Tells the Clock Freq to the Compiler.
#include   "uart.h"
#include   "apa_strip.h"

#include <avr/io.h>      // Defines pins, ports etc.
#include <util/delay.h>  // Functions to waste time
#include <avr/interrupt.h>
#include <stdlib.h>
#include <string.h>

#define BTN (!(PINB & (1 << PINB7))) /* macro to check if button 1 is pressed */
volatile int mode = 0;

#define NUM_LEDS 60
#define buffsize 15

//SPI Port Definitions
#define SPI_DDR		DDRB
#define SPI_PORT	PORTB
#define SPI_SS		2	// Unused
#define SPI_MOSI	3
#define SPI_MISO	4	// Unused
#define SPI_SCK		5
void SPI_MasterInit(void) {
	SPI_DDR = (1<<SPI_SS) | (1<<SPI_MOSI) | (1<<SPI_SCK);	/* Set SS, MOSI and SCK output, all others input */
	SPCR0 = (1<<SPE) | (1<<MSTR) | (1<<SPR1) | (1<<SPR0);	/* Enable SPI, Master, set clock rate fck/128 */
}

// Globals
volatile int Ain, ms = 30, micros, FLAG = 0, Rising = 0, gather = 0;
volatile float Voltage;
char VoltageBuffer[6];
int cnt = 0;

/*
void InitADC0(void) {
	DDRC &= ~(1<<DDRC0); // PC0 = ADC0 is set as input
	// ADLAR set to 0: right adjusted to 10 bit resolution // MUX3:0 set to 0110: input voltage at ADC0
	ADMUX = (0<<MUX1) | (0<<MUX2) | (0<<MUX3) | (0<<ADLAR);
	// ADEN set to 1 : enables the ADC circuitry // ADPS2:0 set to 111 : prescalar set to 128 (104us per conversion)
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
	// Start A to D conversion
	ADCSRA |= (1<<ADSC);
}

ISR(TIMER0_COMPA_vect) {	// Trigger every 1 ms (timer ISR)
	if (ms < 30) ms++;
}
void InitTimer0(void) {		// ISR every 1ms
	TCCR0A |=  (1<<WGM01);	// Turn on clear-on-match with OCR0A
	OCR0A   = 249;			// Set the compare register to 250 ticks
	TIMSK0  = (1<<OCIE0A);	// Enable Timer 0 Compare A ISR
	TCCR0B  =   3;			// Set Prescalar to divide by 64 & Timer 0 starts
}

ISR(TIMER3_COMPA_vect) { // Trigger every 500us (timer ISR)
	micros++;
}
void InitTimer3(void) {		// ISR every 500us | 1000 * 8 / (1.6 * (10^7) ) thus 1000 - 1 = 999
	TCCR3B |=  (1<<WGM32);	// Turn on clear-on-match with OCR3A
	OCR3A   =		  999;
	TIMSK3  = (1<<OCIE3A);	// Enable Timer 1 Compare A ISR
	TCCR3B |=           2;
}

void GETvoltage(void) {	// Returns Voltage and Stores into VoltageBuffer
	ADCSRA |= (1<<ADSC);
	loop_until_bit_is_clear(ADCSRA, ADSC); // while (!(ADCSRA & (1<<ADSC) == 0));
	Ain = ADC; // Typecast the volatile integer into floating type // data, divide by maximum 10-bit value, and // multiply by 5V for normalization
	Voltage = (float)Ain/1024.00 * 5.00;
	dtostrf(Voltage, 3, 2, VoltageBuffer);
}

ISR(INT0_vect) {		 // interrupt button

}
void InitINT0(void) {	 // Init the interrupt BTN
	DDRD &= ~(1<<DDRD2); // INT1 is an input
	EIMSK |= (1<<INT0);	 // EXT interrupt enable
	EICRA |= (1<<ISC01); // Rising / Falling Edge
}
*/

ISR(PCINT0_vect) {	// interrupt button
	if (BTN) {		// BTN Pressed Case
		mode++;
		if (mode == 3) mode = 0;
	}
}
void InitButton(void) {		//  Initialize the interrupt BTN
	DDRB &= ~(1<<DDRB7);    //       DDRB7 is an input
	PCMSK0 |= (1<<PCINT7);  //     enable B7 interrupt
	PCICR  |= (1<<PCIE0);   // PCICR is the register for PCIE which allows interrupts for the PCINT
}

int main(void) {
	uart_init(1);
	InitButton();
    //InitADC0();
	//InitTimer0();
	//InitINT0();
	SPI_MasterInit();

	sei();
	UCSR0B |= (1<<RXCIE0);	//Interrupts INIT
	
	DDRD  = (1<<PIND7);		// LED to show Power is being Supplied
	PORTD = (1<<PIND7);
	
	strip LEDSTRIP;
	strip_init(&LEDSTRIP, NUM_LEDS);
	StripCLR(&LEDSTRIP, NUM_LEDS);
	
	int firstt = 1;
	while (1) {
		if (mode == 0) {
			if (firstt) {
				StripCLR(&LEDSTRIP, NUM_LEDS);
				firstt = 0;
			}
			LED_Bounce_Pattern(&LEDSTRIP, 0, 59, 4, NUM_LEDS);
		}
		else if (mode == 1)		  LED_Out2InPattern(&LEDSTRIP, NUM_LEDS);
		else						LED_Rand_Pattern(&LEDSTRIP,NUM_LEDS);
		if (mode) firstt = 1;
	}
	/*
	int arr[buffsize];
    while (0) {
		if (cnt == buffsize) {
			for(int i = 0; i < buffsize; i++) {
				printf("%d ", arr[i]);
			}
			printf("\n");
			printf("Size = %d\n", sizeof(arr) / sizeof(arr[0]));
			Rising = 0;
			cnt = 0;
		}
		if (FLAG) {
			if (Rising) {	// Rising Edge Case
				arr[cnt] = 1;
			}
			else {			// Falling Edge Case
				arr[cnt] = 0;
			}
			cnt++;
			FLAG = 0;
		}
	}
	*/
}