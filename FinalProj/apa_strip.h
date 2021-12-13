/* apa_strip.h | Created: 11/20/2021 5:56:26 PM | Author: Kevin Joshy */

// 1m APA strip has 60 addressable LEDs
#define NUM_LEDS 60

// Data Structure for LED STRIP information
typedef struct {
	uint8_t Brightness[NUM_LEDS];
	uint8_t			 R[NUM_LEDS];
	uint8_t			 G[NUM_LEDS];
	uint8_t			 B[NUM_LEDS];
} strip;

// Transmits 8 bits or 2 bytes on SPI0 using SPDR0 Register
void SPI_LEDtransmit(uint8_t digital_data);

// Initializes the LEDSTRIP Object
void strip_init(strip* LEDSTRIP, int length);

// Start Transmission Signal
void LED_StartSignal(void);

// End Transmission Signal
void LED_EndSignal(void);

// Sends bytes needed for a LED with brightness and RGB Values
void LED_ByteSend(uint8_t brightness, uint8_t R, uint8_t G, uint8_t B);

// Clears LEDSTRIP
void StripCLR(strip* LEDSTRIP, int length);

// Set the ith index on LEDSTRIP
void store_LED(strip* LEDSTRIP, int i, uint8_t Brightness, uint8_t R, uint8_t G, uint8_t B);

// Displays Stored Info to LEDSTRIP
void LED_Display(strip* LEDSTRIP, int numLEDS);

// Displays LED Snake on LEDSTRIP
void LED_snake(strip* LEDSTRIP, int tail, int head, int numLEDS);

// Returns a Random Pixel Value
uint8_t rand_Pixel(void);

// Bouncing Pattern (UTIL: LED_snake)
void LED_Bounce_Pattern(strip* LEDSTRIP, int st, int en, int s_len, int strip_len);

// Rand Color Pattern
void LED_Rand_Pattern(strip* LEDSTRIP, int strip_len, int delay);

// Out to in pattern
void LED_Out2InPattern(strip* LEDSTRIP, int strip_len, int delay);