/* apa_strip.c | Created: 11/19/2021 1:05:24 PM | Author : kej16104 */ 

#define F_CPU 16000000UL // Tells the Clock Freq to the Compiler.
#include <avr/io.h>      // Defines pins, ports etc.
#include  <stdio.h>
#include "apa_strip.h"
#include <string.h>		 //#include <stdlib.h>

// Transmits 8 bits or 2 bytes on SPI0 using SPDR0 Register
void SPI_LEDtransmit(uint8_t digital_data) {
	SPDR0 = digital_data;			// Send Data
	while(!(SPSR0 & (1<<SPIF)));	// Wait for transmission complete
}

// Initializes the LEDSTRIP Object
void strip_init(strip* LEDSTRIP, int length) {
	for (int i = 0; i < length; i++) {
		LEDSTRIP->Brightness[i] = 0;
		LEDSTRIP->R[i]			= 0;
		LEDSTRIP->G[i]			= 0;
		LEDSTRIP->B[i]			= 0;
	}
}

// Start Transmission Signal
void LED_StartSignal(void) {
	SPI_LEDtransmit(0);
	SPI_LEDtransmit(0);
	SPI_LEDtransmit(0);
	SPI_LEDtransmit(0);
}
// End Transmission Signal
void LED_EndSignal(void) {
	SPI_LEDtransmit(0xFF);
	SPI_LEDtransmit(0xFF);
	SPI_LEDtransmit(0xFF);
	SPI_LEDtransmit(0xFF);
}
// Sends 4 bytes of info for one LED
void LED_ByteSend(uint8_t brightness, uint8_t R, uint8_t G, uint8_t B) {
	SPI_LEDtransmit(brightness);	//brightness [111] [11111]Brightness value [10000] = 16/31 Driving Current
	SPI_LEDtransmit(B);				//blue [Strip Order is (BGR) not (RGB)]
	SPI_LEDtransmit(G);				//green
	SPI_LEDtransmit(R);				//red	
}
// Clears LEDSTRIP
void StripCLR(strip* LEDSTRIP, int length) {
	LED_StartSignal();
	strip_init(LEDSTRIP, length);
	for (int i = 0; i < length; i++) {
		LED_ByteSend(0xFF,0,0,0);
	}
	LED_EndSignal();
}
// Store the ith index on LEDSTRIP
void store_LED(strip* LEDSTRIP, int i, uint8_t Brightness, uint8_t R, uint8_t G, uint8_t B) {
	LEDSTRIP->Brightness[i] = Brightness;
	LEDSTRIP->R[i] = R;
	LEDSTRIP->G[i] = G;
	LEDSTRIP->B[i] = B;
}
// Displays Stored Info to LEDSTRIP
void LED_Display(strip* LEDSTRIP, int numLEDS) {
	int disp;
	int notblank = 0;
	LED_StartSignal();
	for (int i = 0; i < numLEDS; i++) {
		disp = LEDSTRIP->Brightness[i];
		if (disp) {
			LED_ByteSend(disp, LEDSTRIP->R[i], LEDSTRIP->G[i], LEDSTRIP->B[i]);
			notblank = 1;
		}
		else {
			LED_ByteSend(0xFF, 0, 0, 0);
		}
	}
	if (notblank) LED_EndSignal();
}
// Displays LED Snake on LEDSTRIP
void LED_snake(strip* LEDSTRIP, int tail, int head, int numLEDS) {
	int Brightness = 1;
	int R = 1;
	int G = 0;
	int B = 0;
	StripCLR(LEDSTRIP, numLEDS);
	LED_StartSignal();

	for (int i = 0; i < NUM_LEDS; i++) {
		if (i >= tail && i < head) {
			LED_ByteSend(0xFF,1,0,0);	// Pattern
			store_LED(LEDSTRIP, i, Brightness, R, G, B);
		}
		else {
			LED_ByteSend(0xFF,0,0,0);	// Clears LED[i]
		}
	}

	LED_EndSignal();
}
/*
void LED_Bounce_Pattern(int st, int en, int s_len) {
	for (int j = 0; j < s_len; j++) {
		
	}
}*/