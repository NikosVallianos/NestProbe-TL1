﻿/********************************************************************************

  uart.c: Uart interface module of NestProbe TL1 firmware.
 
  Copyright (C) 2017 Nikos Vallianos <nikos@wildlifesense.com>
  
  This file is part of NestProbe TL1 firmware.
  
  NestProbe TL1 firmware is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  any later version.
  
  NestProbe TL1 firmware is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  in the file COPYING along with this program.  If not,
  see <http://www.gnu.org/licenses/>.

  ********************************************************************************/
#include <avr/io.h>
#include <util/delay.h>			// Needed for diesable delay.
#include "uart.h"


void uartEnable(void) {
	PRR0 &= ~(1<<PRUSART0);
	UCSR0B = (1 << RXEN0)|(1 << TXEN0);						// Enable USART transmitter/receiver.
	UBRR0 = 0;												// (8MHz) 103:9600, 3:250k, 1:500k, 0:1M
	UCSR0A |= (1<<U2X0);
	UCSR0A &= ~(1<<UDRE0);
	//UCSR0C |= (1<<USBS0)|(1 << UCSZ01)|(1 << UCSZ00);		// 8 data bits, 2 stop bits
	UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}

/*
 * uartDisable: Stop the UART module so it's at low-power mode.
 */
void uartDisable(void) {
	_delay_us(25);						// Needed to avoid problems if uart disabled before a sleep.
	//Debug: See what the status is here.
	UCSR0B &= ~(1<<RXEN0|1<<TXEN0|1<<RXCIE0);			// Disable UART transmitter, receiver, and RX complete interrupt.
	PRR0 |= (1<<PRUSART0);
	DDRD &= ~((1<<DDRD0)|(1<<DDRD1));						// Set pins PD0 and PD1 to input with pull-up.
	PORTD |= ((1<<PORTD0)|(1<<PORTD1));						// RX0 is PD0, TX0 is PD1
}

/*
 * uartInterruptEnable: If UART module is on, enable UART RX complete interrupt (USART0_RX_vect)
 */
void uartRxInterruptEnable(void) {
	if( UCSR0B & 1<<RXEN0 ) {	// Only if UART0 module is on
		UCSR0B |= 1<<RXCIE0;
	}
}

/*
 * uartInterruptDisable: Disable uart RX complete interrupt
 */
void uartRxInterruptDisable(void) {
	UCSR0B &= 1<<RXCIE0;
}

// Send a byte over usart.
void uartSendByte(uint8_t data) {
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = data;
}


uint8_t uartReceiveByte(void) {
	/* Wait for data to be received */
	while ( !(UCSR0A & (1<<RXC0)) )
	;
	/* Get and return received data from buffer */
	return UDR0;
}

/*
 * Send a 16-bit unsigned word over the uart line, MSB first.
 */
void uartSendWord(uint16_t data) {
	uartSendByte((uint8_t) (data>>8));
	uartSendByte((uint8_t) data);	
}


/*
 * Send a string of up to 255 characters over USART
 */
void uartSendString(const char myString[]) {
    for (uint8_t i=0; myString[i]; i++)
        uartSendByte(myString[i]);
}


/*
 *	Send a two-byte integer as characters to the uart bus.
 */
void uartPrintWord(uint16_t word) {
	uartSendByte('0' + (word / 10000));                 /* Ten-thousands */
	uartSendByte('0' + ((word / 1000) % 10));               /* Thousands */
	uartSendByte('0' + ((word / 100) % 10));                 /* Hundreds */
	uartSendByte('0' + ((word / 10) % 10));                      /* Tens */
	uartSendByte('0' + (word % 10));                             /* Ones */
}

/*
 * Sends a byte to uart as a series of '0' and '1' characters.
 */
void uartPrintBinaryByte(uint8_t byte) {
	uint8_t bit;
	for (bit = 8; bit > 0; bit--) {
		if (bit_is_set(byte, bit-1))
		uartSendByte('1');
		else
		uartSendByte('0');
	}
}

/*
 * Sends a 16-bit word to uart as a series of '0' and '1' characters.
 */
void uartPrintBinaryWord(uint16_t word) {
	// Prints out a word as a series of 1's and 0's
	uartPrintBinaryByte((uint8_t)(word>>8));
	uartPrintBinaryByte((uint8_t)word);
}
