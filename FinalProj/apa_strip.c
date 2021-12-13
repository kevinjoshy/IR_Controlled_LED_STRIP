/* apa_strip.c | Created: 11/19/2021 1:05:24 PM | Author: Kevin Joshy */

#define F_CPU 16000000UL // Tells the Clock Freq to the Compiler.
#include <avr/io.h>      // Defines pins, ports etc.
#include  <stdio.h>
#include <stdlib.h>
#include "apa_strip.h"

// Stop Pattern Flag 
extern int cancelpattern;

#define __DELAY_BACKWARD_COMPATIBLE__
#include <util/delay.h>

// Transmits 8 bits on SPI0 using SPDR0 Register
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
	if ((i >= 0) && (i < NUM_LEDS)) {
		LEDSTRIP->Brightness[i] = Brightness;
		LEDSTRIP->R[i] = R;
		LEDSTRIP->G[i] = G;
		LEDSTRIP->B[i] = B;
	}
	else printf("Invalid Index\n");
}

// Displays Stored Info to LEDSTRIP
void LED_Display(strip* LEDSTRIP, int numLEDS) {
	int disp = 0;
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
	uint8_t Brightness = 0xFF;
	uint8_t R = rand_Pixel();
	uint8_t G = rand_Pixel();
	uint8_t B = rand_Pixel();

	for (int i = 0; i < NUM_LEDS; i++) {
		if (i >= tail && i < head) {
			store_LED(LEDSTRIP, i, Brightness, R, G, B);	// Store the color
			
		}
		else {
			store_LED(LEDSTRIP, i, Brightness, 0, 0, 0);	// Clears LED[i]
		}
	}

	LED_Display(LEDSTRIP, NUM_LEDS);
}

// Returns a Random Pixel Value
uint8_t rand_Pixel(void) {
	uint8_t Pixel = rand() % (0xFF + 1);
	return Pixel;
}

// Creates Bouncing pattern between st and en (LED index 0 -> (NUM_LEDS - 1)). s-len::Snake Length
void LED_Bounce_Pattern(strip* LEDSTRIP, int st, int en, int s_len, int strip_len) {
	uint8_t Brightness = 0xFF;
	uint8_t R = rand_Pixel();
	uint8_t G = rand_Pixel();
	uint8_t B = rand_Pixel();
	int path_len = en - st;
	int delay = 10;
	// Get snake moving forward
	for (int i = 0; i < path_len; i++) {
		if (cancelpattern) return;
		// Populating Snake
		for (int j = 0; j < s_len; j++) {
			if ((j+i+st) <= en) {
				store_LED(LEDSTRIP, i+j+st, Brightness, R, G, B);	// Start at st and goes to that en
			}
		}
		LED_Display(LEDSTRIP, strip_len);
		_delay_ms(delay);
		// End of Snake
		if ((i + st) <= (en - s_len)) {
			store_LED(LEDSTRIP, i+st, Brightness, 0, 0, 0);
		}
	}
	// Get snake moving backward
	for (int i = 0; (-1)*i < path_len; i--) {
		if (cancelpattern) return;
		// Populating Snake
		for (int j = 0; (-1)*j < s_len; j--) {
			if (i+j+en >= st) {
				store_LED(LEDSTRIP, i+j+en, Brightness, R, G, B);	// Start at en and goes to that st
			}
		}
		LED_Display(LEDSTRIP, strip_len);
		_delay_ms(delay);
		// End of Snake
		if ((i + en) >= (st + s_len)) {
			store_LED(LEDSTRIP, i+en, Brightness, 0, 0, 0);
		}
	}
	
}

// Creates Completely Random Pattern on LEDSTRIP
void LED_Rand_Pattern(strip* LEDSTRIP, int strip_len, int delay) {
	for (int i = 0; i < strip_len; i++) {
		LEDSTRIP->Brightness[i] = rand_Pixel();
		LEDSTRIP->R[i]			= rand_Pixel();
		LEDSTRIP->G[i]			= rand_Pixel();
		LEDSTRIP->B[i]			= rand_Pixel();
	}
	LED_Display(LEDSTRIP, strip_len);
	_delay_ms(delay);
}

// Out to in pattern
void LED_Out2InPattern(strip* LEDSTRIP, int strip_len, int delay) {
	int left  =  0;
	int right = 59;
	uint8_t Brightness = 0xFF;
	uint8_t R = rand_Pixel();
	uint8_t G = rand_Pixel();
	uint8_t B = rand_Pixel();
	while (1) {	// Out 2 In
		if (cancelpattern) return;
		store_LED(LEDSTRIP, left, Brightness, R,  G,  B);
		store_LED(LEDSTRIP, right, Brightness,  R,  G,  B);
		left++;
		right--;
		LED_Display(LEDSTRIP, strip_len);
		if (left == 29) break;
		_delay_ms(delay);
	}
	left  = 29;
	right = 30;
	R = rand_Pixel();
	G = rand_Pixel();
	B = rand_Pixel();
	while (1) {	// In 2 Out
		if (cancelpattern) return;
		store_LED(LEDSTRIP, left, Brightness, R,  G,  B);
		store_LED(LEDSTRIP, right, Brightness,  R,  G,  B);
		left --;
		right++;
		LED_Display(LEDSTRIP, strip_len);
		if (left == 0) break;
		_delay_ms(delay);
	}
}