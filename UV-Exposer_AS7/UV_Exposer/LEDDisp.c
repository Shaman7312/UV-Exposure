/*
 * LEDDisp.c
 *
 * Created: 13.08.2017 21:52:39
 *  Author: Shaman
 */ 


#include <stdint.h>
#include <avr/interrupt.h>
#include "LEDDisp.h"
//#include "main.c"

const uint8_t NumToLedTable[20] = {0b10000001, 0b11001111, 0b10010010, 0b10000110, 0b11001100, 0b10100100, 0b10100000, 0b10001111, 0b10000000, 0b10000100,    //Цифры без точки.
0b11111111}; // Ничего

uint8_t LEDState[4];
uint8_t LEDCurrentDigits[4];  // 0 -> 9
uint8_t LEDCurrentDP[4];  // 0 || 1
uint8_t CurrentDigit = 0;

//#define secPresc 2
//uint16_t timerTics = 0;

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
	//TCCR0 &= ~(1<<CS00 | 1<<CS01);
}

void SetLeds(uint8_t * digits)
{
	for (int i = 0; i <4; i++)
	{
		if (digits[i]>=0 && digits[i]<=10)
		{
			LEDState[i] = NumToLedTable[digits[i]]+(LEDCurrentDP[i]<<7);//*128;
			LEDCurrentDigits[i] = digits[i];
		}
	}
}

void SetDigit(uint8_t num, uint8_t dig)
{
	LEDCurrentDigits[dig] = num;
	LEDState[dig] = NumToLedTable[num]+(LEDCurrentDP[num]<<7);//LEDCurrentDP[num]*128;
}

void ToggleDP(uint8_t dig)
{
	if (LEDCurrentDP[dig] != 0) 
		LEDCurrentDP[dig] = 0;
	else LEDCurrentDP[dig] = 1;
	//SetDigit(dig, LEDCurrentDigits[dig]);
}

void SetDP(uint8_t dig)
{
	LEDCurrentDP[dig] = 1;
	//SetDigit(dig, LEDCurrentDigits[dig]);
}

void ResetDP(uint8_t dig)
{
	LEDCurrentDP[dig] = 0;
	//SetDigit(dig, LEDCurrentDigits[dig]);
}

void ResetAllDP()
{
	LEDCurrentDP[0] = 0;
	LEDCurrentDP[1] = 0;
	LEDCurrentDP[2] = 0;
	LEDCurrentDP[3] = 0;
	//SetLeds(LEDCurrentDigits);

// 	ResetDP(0);
// 	ResetDP(1);
// 	ResetDP(2);
// 	ResetDP(3);
}

ISR(TIMER0_OVF_vect)
{
	//timerTics++;
	//if (timerTics >= secPresc)
	//{
		LedDigit0(0);
		LedDigit1(0);
		LedDigit2(0);
		LedDigit3(0);
		//_delay_us(100);
		CurrentDigit++;
		//CurrentDigit = 3;
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
				break;
			case 1:
				LedDigit1(1);
				break;
			case 2:
				LedDigit2(1);
				break;
			case 3:
				LedDigit3(1);
				break;
		}
		//timerTics = 0;
	//}
}