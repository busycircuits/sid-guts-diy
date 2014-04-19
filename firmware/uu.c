/*
  'UU' - Small AVR utility lib

  Copyright (c) 2014 ALMCo Ltd

  Parts of code based in Wiring/Arduino.h;
  Copyright (c) 2005-2006 David A. Mellis

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA
*/

#include "uu.h"

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

const uint8_t PROGMEM _pin_table_PGM[] = {

#if  defined(__AVR_ATmega328P__) 
  0,
#endif
#if defined(__AVR_AT90USB1286__)
  (int)&PINA,
#endif
  (int)&PINB,
  (int)&PINC,
  (int)&PIND
  /*(int)&PORTE,*/
  /*(int)&PORTF*/
};

void 
uu_pin_mode (Pin pin, bool output)
{
  /* DDRD |= (1<<6); set pin 6 in PORTD to output  */
  /* DDRx &= ~(1<<6); set pin 6 in PORTD to input  */

  if (output)
    *PIN_TO_MODEREG(pin) |= PIN_TO_MASK(pin);
  else
    *PIN_TO_MODEREG(pin) &= ~PIN_TO_MASK(pin);

}

void 
uu_pin_digital_write (Pin pin, bool value)
{
  if (value)
    *PIN_TO_OUTREG(pin) |=  PIN_TO_MASK(pin);
  else
    *PIN_TO_OUTREG(pin) &=~ PIN_TO_MASK(pin);
}

bool 
uu_pin_digital_read (Pin pin)
{
  if ((*PIN_TO_PORTREG(pin) & PIN_TO_MASK(pin)) > 0)
    return HIGH;
  return LOW;
}

void 
uu_pin_shift_out(Pin dataPin, Pin clockPin, uint8_t bitOrder, uint8_t value)
{
  uint8_t mask;
  if (bitOrder == LSBFIRST) 
    {
      for (mask=0x01; mask; mask <<= 1) 
	{
	  uu_pin_digital_write(dataPin, value & mask);
	  uu_pin_digital_write(clockPin, HIGH);
	  uu_pin_digital_write(clockPin, LOW);
	}
    }
  else
    {
      for (mask=0x80; mask; mask >>= 1) 
	{
	  uu_pin_digital_write(dataPin, value & mask);
	  uu_pin_digital_write(clockPin, HIGH);
	  uu_pin_digital_write(clockPin, LOW);
	}
    }
}

uint8_t 
uu_pin_shift_in(Pin dataPin, Pin clockPin, uint8_t bitOrder)
{
  uint8_t mask, value=0;
  if (bitOrder == LSBFIRST) 
    {    
      for (mask=0x01; mask; mask <<= 1) 
	{
	  uu_pin_digital_write(clockPin, HIGH);
	  if (uu_pin_digital_read(dataPin)) value |= mask;
	  uu_pin_digital_write(clockPin, LOW);
	}
    }
  else
    {
      for (mask=0x80; mask; mask >>= 1) 
	{
	  uu_pin_digital_write(clockPin, HIGH);
	  if (uu_pin_digital_read(dataPin)) value |= mask;
	  uu_pin_digital_write(clockPin, LOW);
	}
    }
  return value;
}

void
uu_init(int flags)
{
  sei();
}
