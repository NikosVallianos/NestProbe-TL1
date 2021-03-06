﻿/********************************************************************************

  spi.c: SPI module of NestProbe TL1 firmware.
 
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
#include <avr/interrupt.h>
#include "spi.h"

// #################### PORTS & PINS FOR SPI0. Don't change unless porting to another mcu ####################
//	MISO0 is PB4
//	SCK0 is PB5
//	MOSI0 is PB3
//	SS0 is PB2

// Initialize pins for spi communication
void spiEnable(void) {
	DDRB &= ~(1<<DDRB4);		// MISO is INPUT
	PORTB &= ~(1<<PORTB4);		// No pull (tri-state for inputs?)
	
	DDRB |= (1<<DDRB3);			// MOSI output
	PORTB &= ~(1<<PORTB3);		// Low

	DDRB |= (1<<DDRB5);			// SCK out
	PORTB &= ~(1<<PORTB5);		// Low

    PRR0 &= ~(1<<PRSPI0);								// Clock SPI0 clock, powers the module
	
	SPCR0 = (1<<SPE) | (1<<MSTR) | (0<<SPR1) | (0<<SPR0);		// Enable SPI, set to Master, set prescaler.
}

/*
 * spiDisable: Disable the SPI module and set it for lowest power consumption.
 * TODO: Add some fault detection: SPSR0.WCOL0
 */
void spiDisable(void) {
	SPCR0 = 0;											// Disable SPI.
	PRR0 |= (1<<PRSPI0);								// Cut off SPI0 clock, shuts the module	
	DDRB  &= ~(1<<DDRB3|1<<DDRB4|1<<DDRB5);				// MOSI0, SCK0, MISO0 pins are input
	PORTB |=  (1<<PORTB3|1<<PORTB4|1<<PORTB5);			// Pull-up enabled, reduces consumption.
}

// When a serial transfer is complete, the SPIF flag is set.
uint8_t spiTradeByte(uint8_t byte) {
	SPDR0 = byte;										
	loop_until_bit_is_set(SPSR0, SPIF);			
	return SPDR0;
}



// Shift an array out and retain returned data.
void spiExchangeArray(uint8_t * dataout, uint8_t * datain, uint8_t len) {
	for (uint8_t i = 0; i < len; i++) {
		datain[i] = spiTradeByte(dataout[i]);
	}
}



// Shift an array out and discard returned data.
void spiTransmitArray(uint8_t * dataout, uint8_t len) {
	for (uint8_t i = 0; i < len; i++) {
		spiTradeByte(dataout[i]);
	}
}

// Shift an array in while sending scrap data
void spiReceiveArray(uint8_t * datain, uint8_t len) {
	for (uint8_t i=0; i<len; i++) {
		datain[i] = spiTradeByte(0x00);
	}
}