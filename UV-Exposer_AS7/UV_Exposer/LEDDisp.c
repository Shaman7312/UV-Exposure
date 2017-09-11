/*
 * LEDDisp.c
 *
 * Created: 13.08.2017 21:52:39
 *  Author: Shaman
 */ 


#include <stdint.h>
#include <avr/interrupt.h>
#include "LEDDisp.h"

uint8_t LEDState[4];
uint8_t LEDCurrentDigits[4];  // 0 -> 9
uint8_t LEDCurrentDP[4];  // 0 || 1
uint8_t CurrentDigit = 0;

void InitLEDDisplay()
{
	//////////////////////////////    PINS Initialization
	DDRB |= (1<<PINB2);
	DDRC |= (1<<PINC0 | 1<<PINC1 | 1<<PINC2 | 1<<PINC3 | 1<<PINC4 | 1<<PINC5);
	DDRD |= (1<<PIND0 | 1<<PIND1 | 1<<PIND5 | 1<<PIND6 | 1<<PIND7);

	LedDigit3(0);
	LedDigit2(0);
	LedDigit1(0);
	LedDigit0(0);
	//////////////////////////////    Timer0 (Dynamic indication 120Hz) Initialization
	TIMSK |= (1<<TOIE0);
	TCCR0 |= (1<<CS02);
	TCCR0 &= ~(1<<CS00 | 1<<CS01);
}

void SetLeds(uint8_t * digits)
{
	for (int i = 0; i <4; i++)
	{
		if (digits[i]>=0 && digits[i]<10)
		{
			LEDState[i] = NumToLedTable[digits[i]]+abs(((int)LEDCurrentDP[i]-1))*128;
			LEDCurrentDigits[i] = digits[i];
		}
	}
}

void SetDigit(uint8_t dig, uint8_t num)
{
	LEDCurrentDigits[num] = dig;
	LEDState[num] = NumToLedTable[dig]+LEDCurrentDP[dig]*128;
}

void ToggleDP(uint8_t dig)
{
	if (LEDCurrentDP[dig] == 1) 
		LEDCurrentDP[dig] = 0;
	else LEDCurrentDP[dig] = 1;
	SetDigit(dig, LEDCurrentDigits[dig]);
}

void SetDP(uint8_t dig)
{
	LEDCurrentDP[dig] = 1;
	SetDigit(dig, LEDCurrentDigits[dig]);
}

void ResetDP(uint8_t dig)
{
	LEDCurrentDP[dig] = 0;
	SetDigit(dig, LEDCurrentDigits[dig]);
}

ISR(TIMER0_OVF_vect)
{
	LedDigit0(0);
	LedDigit1(0);
	LedDigit2(0);
	LedDigit3(0);
	CurrentDigit++;
	if (CurrentDigit>3) CurrentDigit = 0;
	LedA((LEDState[CurrentDigit]&0b01000000)>>6)
	LedB((LEDState[CurrentDigit]&0b00100000)>>5)
	LedC((LEDState[CurrentDigit]&0b00010000)>>4)
	LedD((LEDState[CurrentDigit]&0b00001000)>>3)
	LedE((LEDState[CurrentDigit]&0b00000100)>>2)
	LedF((LEDState[CurrentDigit]&0b00000010)>>1)
	LedG((LEDState[CurrentDigit]&0b00000001))
	LedDP((LEDState[CurrentDigit]&0b10000000)>>7)
	switch (CurrentDigit)
	{
		case 0:
			LedDigit0(1);
		case 1:
			LedDigit1(1);
		case 2:
			LedDigit2(1);
		case 3:
			LedDigit3(1);
	}
}