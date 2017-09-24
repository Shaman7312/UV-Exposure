/*
 * UV_Exposer.c
 *
 * Created: 13.08.2017 21:47:10
 * Author : Shaman
 */ 
		
//#define MUTEMODE		//No sound mode

#define F_CPU 16000000

#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "LEDDisp.h"



#define RTC_SW_PRESCALER 124  //Real Time Clock Software Prescaler
#define RTS_SW_25TH_PRESCALER 5
#define BUTTON_TIMEOUT 26

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
uint8_t minutes;
uint8_t seconds;
uint8_t power;
uint8_t menuMode = 0;  // 0 - blink min; 1 - blink sec; 2 - set power; 3 - start exposing, blink middle DP; 4 - end beeping, blink "done"
uint8_t blinkState = 1; // Состояние моргающего текста: 0 - не видно, 1 - видно
uint8_t LedData[4] = {10, 10, 10, 10};
//static uint8_t RebootFlag = 0;
uint8_t lastMinutes;
uint8_t lastSeconds;
uint8_t lastPower;

uint8_t buttonTimeOutCounter = 0;
uint8_t TFthCounter = 0; // 25th counter


void Second25thElapsed()
{
	if (buttonTimeOutCounter>0)
	{
		buttonTimeOutCounter++;
		if (buttonTimeOutCounter >= BUTTON_TIMEOUT)
		{
			#ifndef MUTEMODE
			SoundTone(1975, 50);
			SoundTone(1864, 50);
			SoundTone(1720, 50);
			_delay_ms(50);
			SoundTone(1046, 50);
			SoundTone(932, 50);
			SoundTone(1046, 50);
			#endif
			wdt_enable(1);
			while(1)
			{};
		}
	}
	TFthCounter++;
	if (TFthCounter == 25)
		TFthCounter = 0;

	if (menuMode == 0)
	{
		if (TFthCounter < 5)
		{
			LedData[0] = 10;
			LedData[1] = 10;
			LedData[2] = seconds/10;
			LedData[3] = seconds%10;
		} else
		{
			LedData[0] = minutes/10;
			LedData[1] = minutes%10;
			LedData[2] = seconds/10;
			LedData[3] = seconds%10;
		}
		SetLeds(LedData);
	}
	else if (menuMode == 1)
	{
		if (TFthCounter < 5)
		{
			LedData[0] = minutes/10;
			LedData[1] = minutes%10;
			LedData[2] = 10;
			LedData[3] = 10;
		} else
		{
			LedData[0] = minutes/10;
			LedData[1] = minutes%10;
			LedData[2] = seconds/10;
			LedData[3] = seconds%10;
		}
		SetLeds(LedData);
	} else if (menuMode == 2)
	{
		LedData[0] = 10;
		if (power == 100)
		LedData[1] = 1;
		else
		LedData[1] = 10;
		if (power >= 10)
		LedData[2] = (power/10)%10;
		else
		LedData[2] = 10;
		LedData[3] = power%10;
		SetLeds(LedData);
	}
}

void SecondElapsedCallBack()
{
	if (menuMode == 3)
	{
		if (seconds != 0)
		{
			seconds=seconds-1;
		}
		else if (minutes != 0)
		{
			minutes=minutes-1;
			seconds = 59;
		} else
		{
			StopPWM();
			#ifndef MUTEMODE
			EndSound();
			#endif
			blinkState = 0;
			menuMode = 4;
			//SecondElapsedCallBack();
			//return;
		}
		ToggleDP(1);
		if (minutes >= 10)
		LedData[0] = minutes/10;
		else 
		LedData[0] = 10;
		LedData[1] = minutes%10;
		LedData[2] = seconds/10;
		LedData[3] = seconds%10;
	}
	if (menuMode == 4)
	{
		ResetAllDP();
		LedData[0] = 0+10*blinkState;
		LedData[1] = 0+10*blinkState;
		LedData[2] = 0+10*blinkState;
		LedData[3] = 0+10*blinkState;
		if (blinkState != 0) blinkState = 0;
		else blinkState = 1;
	}
	//ToggleDP(0);
	SetLeds(LedData);
}

int main(void)
{
	DDRB |= (1<<PINB0 | 1<<PINB1);
	DDRD &= ~(1<<PIND2 | 1<<PIND3 | 1<<PIND4);		// Входы энкодера
	PORTB &= ~(1<<PINB1);
	//DDRD &= ~(1<<PIND2 | 1<<PIND3 | 1<<PIND4);   

	MCUCR |= (1<<ISC10 | 1<<ISC01);
	GICR |= (1<<INT1 | 1<<INT0);
	InitLEDDisplay();
	SetLeds(LedData);
	SetDP(1);
	sei();
	
	InitPWM();
	InitRTC();

	
	
	
	_delay_ms(100);

	EEAR = 0;
	EECR |= (1<<EERE);
	minutes = EEDR;
	if (minutes>99) minutes = 0;
	lastMinutes = minutes;
	EEAR = 1;
	EECR |= (1<<EERE);
	seconds = EEDR;
	if (seconds>59) seconds = 0;
	lastSeconds = seconds;
	EEAR = 2;
	EECR |= (1<<EERE);
	power = EEDR;
	if (power>100) power = 100;
	lastPower = power;
	//cli();
	//uint8_t debLeds[4] = {4,5,6,7};
	//SetLeds(debLeds);

	//#ifdef DEBUG
	//LedDebug();
	//#endif DEBUG
    while (1) 
    {
		
    }
}
////////////////////////////////////        Timer 1
void InitPWM()
{
	TCCR1A |= (1<<WGM11 | 1<<COM1A1);
	//TCCR1A &= ~(1<<FOC1A | 1<<FOC1B);
	TCCR1B |= (1<<WGM13 | 1<<WGM12);
	//OCR1A = 65535;
	ICR1 = 65535;
}

void StartPWM()
{
	TCNT1 = 0;
	TCCR1A |= (1<<COM1A1);
	TCCR1B |= (1<<CS10);
}

void StopPWM()
{
	
	TCCR1B &= ~(1<<CS12 | 1<<CS11 | 1<<CS10);
	TCCR1A &= ~(1<<COM1A1);
	//TCNT1 = 65535;
	PORTB &= ~(1<<PINB1);
}

void PWMSetDutyCycle(uint16_t dc)
{
	StopPWM();

	//ICR1 = dc;
	
	OCR1A = dc;
	//OCR1B = dc;
	//StartPWM();
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
	if (Timer2CTCCounter%RTS_SW_25TH_PRESCALER == 0)
	{
		Second25thElapsed();
	}
	
	if (Timer2CTCCounter >= RTC_SW_PRESCALER)
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
		if (minutes>99) minutes = 0;
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
		if (minutes==0) minutes = 99;
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
		EncoderMinusCallback();
	else
		EncoderPlusCallback();
}
///////////////////////////////////////     Button
void ButtonPressedCallback()
{
// 	uint8_t lol[4] = {10,1,0,1};
// 	SetLeds(lol);
	buttonTimeOutCounter = 1;
}

void ButtonUnpressedCallback()
{
	buttonTimeOutCounter = 0;
	if (menuMode == 0)
	{
		menuMode++;
		if (lastMinutes != minutes)
		{
			while (EECR & (1 << EEWE));
			EEAR = 0;
			EEDR = minutes;
			EECR |= (1<<EEMWE);
			EECR |= (1<<EEWE);
		}
	}
	else if (menuMode == 1)
	{
		ResetAllDP();
		menuMode++;
		if (lastSeconds != seconds)
		{
			while (EECR & (1 << EEWE));
			EEAR = 1;
			EEDR = seconds;
			EECR |= (1<<EEMWE);
			EECR |= (1<<EEWE);
		}
	}
	else if (menuMode == 2)
	{
		if (lastPower != power)
		{
			while (EECR & (1 << EEWE));
			EEAR = 2;
			EEDR = power;
			EECR |= (1<<EEMWE);
			EECR |= (1<<EEWE);
		}
		menuMode++;
		#ifndef MUTEMODE
		StartSound();
		#endif
		RestartRTC();
		PWMSetDutyCycle((uint16_t)655.35*power);
		StartPWM();
	}
	else if (menuMode == 4)
	{
		wdt_enable(1);
		while(1);
	}
}


ISR(INT1_vect)
{
	if ((PIND & (1<<PIND3)))
	ButtonUnpressedCallback();
	else
	ButtonPressedCallback();
}

////////////////////////////////////////////////

void delay5usX(uint16_t times)
{
	while (times--)
		_delay_us(5);
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
	uint8_t soundPinState = 0;
	for (uint32_t i = 0; i<duration*2000.0*freq/1000000.0 ; i++)
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
		delay5usX(100000.0/(freq));
		
	}
	PORTB &= ~(1<<PINB0);
	//soundPinState = 0;
}