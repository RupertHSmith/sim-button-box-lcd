/*  Author: Steve Gunn
 * Licence: This work is licensed under the Creative Commons Attribution License.
 *          View this license at http://creativecommons.org/about/licenses/
 * 
 *
 * Feb 2015: Slightly modified for LaFortuna board, KPZ  
 *
 *   | PIN | Function      | Connected to                                 |
 *   |-----+---------------+----------------------------------------------|
 *   | PD2 | RX of USART 1 | to ORANGE wire  -->  TX on FTDI C232HM cable |
 *   | PD3 | TX of USART 1 | to YELLOW wire  -->  RX on FTDI C232HM cable |
 *   | G   | Signal Ground | to BLACK wire   -->  ground on C232HM cable  |
 *
 * Configure your serial terminal for:
 *    |-----------+------|
 *    | Baud rate | 9600 |
 *    |-----------+------|
 *    | Data bits |    8 |
 *    |-----------+------|
 *    | Stop bits |    1 |
 *    |-----------+------|
 * 
 * Linux:  
 *   ls -lh /dev/serial/by-id
 *   picocom /dev/ttyUSB0
 */

#ifndef DEBUG_H
#define DEBUG_H


#define __ASSERT_USE_STDERR
#include <assert.h>
#include <stdio.h>
#include <avr/io.h>

#define DEBUG_BAUD  9600

int uputchar1(char c, FILE *stream)
{
//	if (c == '\n') uputchar1('\r', stream);
//	while (!(UCSR1A & _BV(UDRE1)));
//	UDR1 = c;
//	return c;
}

int ugetchar1(FILE *stream)
{
//	while(!(UCSR1A & _BV(RXC1)));
//	return UDR1;
}

void init_debug_uart1(void)
{
	/* Configure UART1 baud rate, one start bit, 8-bit, no parity and one stop bit */
//	UBRR1H = (F_CPU/(DEBUG_BAUD*16L)-1) >> 8;
//	UBRR1L = (F_CPU/(DEBUG_BAUD*16L)-1);
//	UCSR1B = _BV(RXEN1) | _BV(TXEN1);
//	UCSR1C = _BV(UCSZ10) | _BV(UCSZ11);

	/* Setup new streams for input and output */
//	static FILE uout = FDEV_SETUP_STREAM(uputchar1, NULL, _FDEV_SETUP_WRITE);
//	static FILE uin = FDEV_SETUP_STREAM(NULL, ugetchar1, _FDEV_SETUP_READ);

	/* Redirect all standard streams to UART1 */
//	stdout = &uout;
//	stderr = &uout;
//	stdin = &uin;
}


#endif /* DEBUG_H */

