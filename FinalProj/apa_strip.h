/* apa_strip.h | Created: 11/20/2021 5:56:26 PM | Author: Kevin */

#define NUM_LEDS 60

typedef struct {
	uint8_t Brightness[NUM_LEDS];
	uint8_t			 R[NUM_LEDS];
	uint8_t			 G[NUM_LEDS];
	uint8_t			 B[NUM_LEDS];
} strip;

void SPI_LEDtransmit(uint8_t digital_data);		// Transmits 8 bits or 2 bytes on SPI0 using SPDR0 Register

void strip_init(strip* LEDSTRIP, int length);	// Initializes the LEDSTRIP Object

void LED_StartSignal(void);						// Start Transmission Signal

void LED_EndSignal(void);						// End Transmission Signal

void LED_ByteSend(uint8_t brightness, uint8_t R, uint8_t G, uint8_t B);									// Sends bytes needed for a LED with brightness and RGB Values

void StripCLR(strip* LEDSTRIP, int length);																// Clears LEDSTRIP

void store_LED(strip* LEDSTRIP, int i, uint8_t Brightness, uint8_t R, uint8_t G, uint8_t B);			// Set the ith index on LEDSTRIP

void LED_Display(strip* LEDSTRIP, int numLEDS);															// Displays Stored Info to LEDSTRIP

void LED_snake(strip* LEDSTRIP, int tail, int head, int numLEDS);										// Displays LED Snake on LEDSTRIP

void LED_Bounce_Pattern(int st, int en, int s_len);														// Bouncing Pattern (UTIL: LED_snake)