/*
 * UV_Exposer.c
 *
 * Created: 13.08.2017 21:47:10
 * Author : Shaman
 */ 
#define F_CPU 16000000

#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "LEDDisp.h"



#define RTC_SW_PRESCALER 125  //Real Time Clock Software Prescaler

void InitPWM();
void StartPWM();
void StopPWM();
void PWMSetDutyCycle(uint16_t dc);
void InitRTC();
void RestartRTC();
void SecondElapsedCallBack();
void EncoderPlusCallback();
void EncoderMinusCallback();
void ButtonPressedCallback();

void StartSound();
void EndSound();
void SoundTone(uint16_t freq, uint16_t duration);

uint16_t Timer2CTCCounter = 0;
uint8_t minutes = 0;
uint8_t seconds = 0;
uint8_t power = 100;
uint8_t menuMode = 0;  // 0 - blink min; 1 - blink sec; 2 - set power; 3 - start exposing, blink middle DP; 4 - end beeping, blink "done"
uint8_t blinkState = 1; // Состояние моргающего текста: 0 - не видно, 1 - видно
uint8_t LedData[4] = {0, 0, 0, 0};

void SecondElapsedCallBack()
{
	if (menuMode == 0)
	{
		if (blinkState == 1)
		{
			blinkState = 0;
			LedData[0] = 10;
			LedData[1] = 10;
			LedData[2] = seconds/10;
			LedData[3] = seconds%10;
		} else
		{
			blinkState = 1;
			LedData[0] = minutes/10;
			LedData[1] = minutes%10;
			LedData[2] = seconds/10;
			LedData[3] = seconds%10;
		}
	} else if (menuMode == 1)
	{
		if (blinkState == 1)
		{
			blinkState = 0;
			LedData[0] = minutes/10;
			LedData[1] = minutes%10;
			LedData[2] = 10;
			LedData[3] = 10;
		} else
		{
			blinkState = 1;
			LedData[0] = minutes/10;
			LedData[1] = minutes%10;
			LedData[2] = seconds/10;
			LedData[3] = seconds%10;
		}
	} else if (menuMode == 2)
	{
		LedData[0] = 10;
		if (power == 100)
		LedData[1] = 1;
		else 
		LedData[1] = 10;
		if (power > 10)
		LedData[2] = (power/10)%10;
		else
		LedData[2] = 10;
		LedData[3] = power%10;
	}
	else if (menuMode == 3)
	{
		
		LedData[0] = minutes/10;
		LedData[1] = minutes%10;
		LedData[2] = seconds/10;
		LedData[3] = seconds%10;
		ToggleDP(1);
		if (seconds > 0)
			seconds--;
		else if (minutes > 0)
		{
			minutes--;
			seconds = 59;
		} else
		{
		StopPWM();
		EndSound();
		menuMode = 4;
		}
	}
	else if (menuMode == 4)
	{
		LedData[0] = 0+10*blinkState;
		LedData[1] = 0+10*blinkState;
		LedData[2] = 0+10*blinkState;
		LedData[3] = 0+10*blinkState;
		if (blinkState == 0) blinkState++;
		else blinkState=0;
	}

	SetLeds(LedData);
}

int main(void)
{
	DDRB |= (1<<PINB0 | 1<<PINB1);
	DDRD &= ~(1<<PIND2 | 1<<PIND3 | 1<<PIND4);
	PORTB &= ~(1<<PINB1);
	DDRD &= ~(1<<PIND2 | 1<<PIND3 | 1<<PIND4);   // Входы энкодера

	MCUCR |= (1<<ISC11 | 1<<ISC01);
	GICR |= (1<<INT1 | 1<<INT0);


    /* Replace with your application code */
	sei();
	InitLEDDisplay();
	InitPWM();
	InitRTC();

	SetLeds(LedData);
	SetDP(1);
	
    while (1) 
    {
		
    }
}
////////////////////////////////////        Timer 1
void InitPWM()
{
	TCCR1A |= (1<<WGM11 | 1<<WGM10 | 1<<COM1A1);
	TCCR1B |= (1<<WGM13 | 1<<WGM12);

}

void StartPWM()
{
	TCCR1B |= (1<<CS12 | 1<<CS10);
}

void StopPWM()
{
	TCCR1B &= ~(1<<CS12 | 1<<CS10);
}

void PWMSetDutyCycle(uint16_t dc)
{
	StopPWM();
	PORTB &= ~(1<<PINB1);
	OCR1A = dc;
	StartPWM();
}
/////////////////////////////////////        Timer 2
void InitRTC()
{
	TCNT2 = 0;
	Timer2CTCCounter = 0;
	TCCR2 |= (1<<WGM21);
	OCR2 = 125;
	TIMSK |= (1<<OCIE2);
	TCCR2 |= (1<<CS22 | 1<<CS21 | 1<<CS20);
}

void RestartRTC()
{
	TCCR2 &= ~(1<<CS22 | 1<<CS21 | 1<<CS20);
	
	InitRTC();
}

ISR(TIMER2_COMP_vect)
{
	Timer2CTCCounter++;
	if (Timer2CTCCounter == RTC_SW_PRESCALER)
	{
		Timer2CTCCounter = 0;
		SecondElapsedCallBack();
	}
}
//////////////////////////////////////       Encoder
void EncoderPlusCallback()
{
	if (menuMode == 0)
	{
		minutes++;
		if (minutes>59) minutes = 0;
	}
	else if (menuMode == 1)
	{
		seconds++;
		if (seconds>59) seconds = 0;
	}
	else if (menuMode == 2)
	{
		power++;
		if (power>100) power = 0;
	}
}

void EncoderMinusCallback()
{
	if (menuMode == 0)
	{
		if (minutes==0) minutes = 59;
		else minutes--;
	}
	else if (menuMode == 1)
	{
		if (seconds==0) seconds = 59;
		else seconds--;
	}
	else if (menuMode == 2)
	{
		if (power==0) power = 100;
		else power--;
	}
}

ISR(INT0_vect)
{
	if (PIND&(1<<PIND4))
		EncoderPlusCallback();
	else
		EncoderMinusCallback();
}
///////////////////////////////////////     Button
void ButtonPressedCallback()
{
	if (menuMode == 0)
		menuMode++;
	else if (menuMode == 1)
		menuMode++;
	else if (menuMode == 2)
	{
		menuMode++;
		StartSound();
		RestartRTC();
		PWMSetDutyCycle((uint16_t)655.35*power);
		StartPWM();
	}
}

ISR(INT1_vect)
{
	ButtonPressedCallback();
}

void StartSound()
{
	SoundTone(2000, 55);
	_delay_us(15);
	SoundTone(2000, 60);
	_delay_us(10);
	SoundTone(3100, 55);
	SoundTone(2900, 70);
	SoundTone(3100, 65);
	_delay_us(5);
	SoundTone(2000, 60);
}

void EndSound()
{
	SoundTone(2000, 70);
	SoundTone(2100, 110);
	SoundTone(3100, 80);
	_delay_us(20);
	SoundTone(2800, 50);
}

void SoundTone(uint16_t freq, uint16_t duration) 
{
	// 1 us = F_CPU/1000000
	// 2 kHz = each 500 us
	static uint8_t soundPinState = 0;
	for (int i = 0; i<duration*1000/(1000000/freq) ; i++)
	{
		if (soundPinState == 0)
		{
			PORTB |= (1<<PINB0);
			soundPinState = 1;
		} else
		{
			PORTB &= ~(1<<PINB0);
			soundPinState = 0;
		}
		_delay_us(1000000/freq);
		
	}
	PORTB &= ~(1<<PINB0);
	soundPinState = 0;
}