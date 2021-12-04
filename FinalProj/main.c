/* FinalProj.c | Created: 11/19/2021 1:05:24 PM | Author : kej16104 */

#define F_CPU 16000000UL	// Tells the Clock Freq (fclk) to the Compiler. 16 MHz
#include   "uart.h"
#include   "apa_strip.h"

#include <avr/io.h>			// Defines pins, ports etc.
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define BTN (!(PINB & (1 << PINB7))) /* macro to check if button 1 is pressed */
volatile int mode = 0;

strip LEDSTRIP;							// strip object to store info on LED STRIP

#define NUM_LEDS	60
#define buffsize	15
#define size_signal 37
// Globals								// Signal:  [Start Bit] [Address 0x00] [All signals start with 0xFF] [16 bit HEX signal]	[End Of Signal]		
volatile int RECV_Data[size_signal];	// Ex PWR BTN: [ 0 1 ] [ 0 0 0 0 0 0 0 0] [ 1 1 1 1 1 1 1 1 ] [ 0 1 1 0 0 0 1 0 1 0 0 1 1 1 0 1 ] [ 1 1 0 ]
volatile int pointer   = 0;
volatile int recieving = 1;

int O2I_delay = 10;
int RAN_delay = 10;
int s_len     =  4;

//SPI Port Definitions
#define SPI_DDR		DDRB
#define SPI_PORT	PORTB
#define SPI_SS		2	// Unused No SS for LEDSTRIP SPI Connection
#define SPI_MOSI	3
#define SPI_MISO	4	// Unused Slave doesn't return data
#define SPI_SCK		5
void SPI_MasterInit(void) {
	SPI_DDR = (1<<SPI_SS) | (1<<SPI_MOSI) | (1<<SPI_SCK);	/* Set SS, MOSI and SCK output, all others input */
	SPCR0 = (1<<SPE) | (1<<MSTR) | (0<<SPR1) | (0<<SPR0);	/* Enable SPI, Master, set clock rate fclk/4 = 4 MHz */
}

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
*/

void InitTimer1(void) {	// ISR every 100us | 200 * 8 / (1.6 * (10^7) ) thus 200 - 1 = 199 || 100 us = 0.1 ms
	TCCR1B |=   (1<<WGM12);			// Turn on clear-on-match with OCR1A
	OCR1A   =        65535;			// Max 16 bit timer
	TCCR1B |=    (1<<CS11);			// fast PWM with TOP = OCR1A, prescalar = 8
}

ISR(INT0_vect) {				// ISR on Falling Edge
	if (recieving) {
		// Collect Time Spent
		int timespent = TCNT1;		// each counter value is 0.5
		
		if (timespent < 3400) RECV_Data[pointer] = 0;		// If less than 1700 us || Bit 0
		else				  RECV_Data[pointer] = 1;		// Bit is 1
		pointer++;											// Increment position of RECV_Data
		if (pointer == size_signal - 1) {
			recieving = pointer = 0;						// Done Receiving
		}
		TCNT1 = 0;											// Reset Timer Counter
	}
}
void InitINT0(void) {	  // Init the interrupt pin
	DDRD  &= ~(1<<DDRD2); // INT1 is an input
	EIMSK |=  (1<<INT0);  // EXT interrupt enable
	EICRA |=  (1<<ISC01); // Falling Edge INT0 
}

/*
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
*/

unsigned int getMode(void) {
	int st = 33;									// Smallest value starts at index 33
	unsigned int modeVal = 0;
	for (int i = 0; i < 16; i++) {					// 16 bit signal
		if (RECV_Data[st] == 1) modeVal |= (1<<i);	// Converts Binary list to Value
		st--;
	}
	return modeVal;
}

void blinkLED(void) {
	for (int i = 0; i < 5; i++) {
		PORTD = (0<<PIND7);
		_delay_ms(150);
		PORTD = (1<<PIND7);
		_delay_ms(150);
	}
}

void dispRECV(void) {
	printf("\n Start Signal \n");
	for (int i = 18; i < 34; i++) {
		printf("%d ", RECV_Data[i]);
	}
	printf("\n END Signal \n");
}

void runMode(void) {
	unsigned int modeVal = getMode();
	switch (modeVal) {
		case 0x629D:	// PWR BTN
			printf("POWER ON/OFF\n");
			if (mode) {
				StripCLR(&LEDSTRIP, NUM_LEDS);
				mode = 0;
			}
			else mode = 1;
			break;
		case 0x22DD:	// A
			mode = 1;
			printf("A\n");
			break;
		case 0x02FD:	// B
			mode = 2;
			printf("B\n");
			break;
		case 0xC23D:	// C
			mode = 3;
			printf("C\n");
			break;
		case 0x9867:	// UP
			switch (mode) {
				case 1:		// Snake Mode | Increase Snake
				if (s_len < 10) s_len++;
					printf("UP %d\n", s_len);
					break;
				case 2:		// O2I   Mode | Decrease Delay
					if (O2I_delay > 5) O2I_delay -= 2;
					printf("UP %d\n", O2I_delay);
					break;
				case 3:		// Rand  Mode | Decrease Delay
					if (RAN_delay > 4) RAN_delay -= 4;
					printf("UP %d\n", RAN_delay);
					break;	
			}
			break;
		case 0x30CF:	// LEFT
			printf("LEFT\n");
			break;
		case 0x18E7:	// CIRCLE
			printf("CIRCLE\n");
			break;
		case 0x7A85:	// RIGHT
			printf("RIGHT\n");
			break;
		case 0x38C7:	// DOWN
			switch (mode) {
				case 1:		// Snake Mode | Decrease Snake
					if (s_len > 1) s_len--;
					printf("DOWN %d\n", s_len);
					break;
				case 2:		// O2I   Mode | Increase Delay
					if (O2I_delay < 20) O2I_delay += 2;
					printf("DOWN %d\n", O2I_delay);
					break;
				case 3:		// Rand  Mode | Increase Delay
					if (RAN_delay < 60) RAN_delay += 4;
					printf("DOWN %d\n", RAN_delay);
					break;
			}
			break;
		default:	// Error Case
			printf("ERROR OCCURED\n");
			for (int i = 0; i < size_signal; i++) {
				RECV_Data[i] = 0;
			}
			break;
	}
}

int main(void) {
	uart_init(0);
	//InitButton();

	InitINT0();
	InitTimer1();
	SPI_MasterInit();

	sei();					// global interrupts enabled 
	UCSR0B |= (1<<RXCIE0);	// Interrupts initialized
	// LED to show Power is being Supplied
	DDRD  = (1<<PIND7);		
	PORTD = (1<<PIND7);

	strip_init(&LEDSTRIP, NUM_LEDS);		// Initialize all values of OBJ to 0
	StripCLR(&LEDSTRIP, NUM_LEDS);			// Clears the LEDSTRIP display

	while (1) {
		if (!recieving) {
			dispRECV();
			runMode();
			// RESET
			recieving = 1;
			pointer   = 0;
			
		}
		switch (mode) {
			case 0:
				break;
			case 1:
				LED_Bounce_Pattern(&LEDSTRIP, 0, 59, s_len, NUM_LEDS);
				break;
			case 2:
				LED_Out2InPattern(&LEDSTRIP, NUM_LEDS, O2I_delay);
				break;
			case 3:
				LED_Rand_Pattern(&LEDSTRIP,NUM_LEDS, RAN_delay);
				break;
			default:
				break;
		}
	}
}