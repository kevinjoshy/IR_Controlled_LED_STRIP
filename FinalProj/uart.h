#include <stdio.h>
#include <stdbool.h>

/* Perform UART startup initialization. */
void	uart_init(bool use_tx_interrupts);

/* Send one character to the UART. */
int	uart_putchar(char c, FILE *stream);

/* Size of internal line buffer used by uart_getchar(). */
#define RX_BUFSIZE 80

/*
 * Receive one character from the UART.  The actual reception is
 * line-buffered, and one character is returned from the buffer at
 * each invocation.
 */
int	uart_getchar(FILE *stream);