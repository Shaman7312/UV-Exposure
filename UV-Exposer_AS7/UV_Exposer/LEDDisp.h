/*
 * LEDDisp.h
 *
 * Created: 13.08.2017 21:52:54
 *  Author: Shaman
 */ 


#ifndef LEDDISP_H_
#define LEDDISP_H_

#define LedA(__SET) if(__SET==0){PORTC&=~(1<<PINC1);}else if (__SET==1){PORTC|=(1<<PINC1);}
#define LedB(__SET) if(__SET==0){PORTC&=~(1<<PINC5);}else if (__SET==1){PORTC|=(1<<PINC5);}
#define LedC(__SET) if(__SET==0){PORTD&=~(1<<PIND1);}else if (__SET==1){PORTD|=(1<<PIND1);}
#define LedD(__SET) if(__SET==0){PORTC&=~(1<<PINC2);}else if (__SET==1){PORTC|=(1<<PINC2);}
#define LedE(__SET) if(__SET==0){PORTC&=~(1<<PINC0);}else if (__SET==1){PORTC|=(1<<PINC0);}
#define LedF(__SET) if(__SET==0){PORTC&=~(1<<PINC3);}else if (__SET==1){PORTC|=(1<<PINC3);}
#define LedG(__SET) if(__SET==0){PORTD&=~(1<<PIND6);}else if (__SET==1){PORTD|=(1<<PIND6);}
#define LedDP(__SET) if(__SET==0){PORTD&=~(1<<PIND7);}else if (__SET==1){PORTD|=(1<<PIND7);}

#define LedDigit3(__SET) if(__SET==0){PORTB&=~(1<<PINB2);}else if (__SET==1){PORTB|=(1<<PINB2);}
#define LedDigit2(__SET) if(__SET==0){PORTD&=~(1<<PIND0);}else if (__SET==1){PORTD|=(1<<PIND0);}
#define LedDigit1(__SET) if(__SET==0){PORTC&=~(1<<PINC4);}else if (__SET==1){PORTC|=(1<<PINC4);}
#define LedDigit0(__SET) if(__SET==0){PORTD&=~(1<<PIND5);}else if (__SET==1){PORTD|=(1<<PIND5);}




static uint8_t NumToLedTable[20] = {0b10000001, 0b11001111, 0b10010010, 0b10000110, 0b11001100, 0b10100100, 0b10100000, 0b10001111, 0b10000000, 0b10000100,    //Цифры без точки.
									0b11111111}; // Ничего


void InitLEDDisplay();
void SetLeds(uint8_t * digits);
void SetDigit(uint8_t dig, uint8_t num);
void ToggleDP(uint8_t dig);
void SetDP(uint8_t dig);
void ResetDP(uint8_t dig);


#endif /* LEDDISP_H_ */