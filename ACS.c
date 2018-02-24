/* ACS.c
 * Created: 9/26/2017 6:14:07 PM
 * Authors: Amer Akhtar, Ectis Velazquez
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avr035.h"
#include "lcd.h"
#include "ds18b20.h"
#include "led.h"

#define PH PORTA, 1
//#define SAL0 PORTA, 2
//#define SAL1 PORTA, 3
//#define SAL2 PORTA, 4
#define LIGHT_RECEIVER PORTA, 5
#define THERMOMETER PORTB, 0
#define HEATER PORTB, 1
#define COOLER PORTB, 2
#define PUMP PORTB, 3
#define WTR_LVL_LOW PORTB, 4
#define WTR_LVL_MED PORTB, 5
#define WTR_LVL_HIGH PORTB, 6
#define SAL4 PORTB, 7
#define LCD0 PORTC, 0
#define LCD1 PORTC, 1
#define LCD2 PORTC, 2
#define LCD3 PORTC, 3
#define LCD4 PORTC, 4
#define LCD5 PORTC, 5
#define LCD6 PORTC, 6
#define LCD7 PORTC, 7
#define ESP0 PORTD, 0
#define ESP1 PORTD, 1
#define FEEDER PORTD, 2

#define LED_STRIP PORTD, 4
#define LCD_RS PORTD, 5
#define LCD_RW PORTD, 6
#define LCD_E PORTD, 7

// Parameter globals.
char waterLock = 0;
double tempe = 0.00;
double lowTemp = 23.00;
double highTemp = 26.00;
double pH = 0.00;
int sal = 0;
int light = 0;
int lightThreshold = 100;
int waterLevel = 0;
char numWaterSensors = 2;
int waterLoop = 0;

char ADCBuff[8];
char tempBuff[5];
char pHBuff[5];
char salBuff[5];
char lightBuff[5];

// LCD globals.
unsigned char PB_Status = 0xFF, Change_Display = 0,
Change_Display1 = 1;

// LED globals.
unsigned int iLED = 0;
unsigned int jLED = 0;
unsigned int blinker = 0;

void readTemp(void);
void printTemp(void);
void readpH(void);
void readSal(void);
void readLight(void);
void printLight(void);
void readWaterLevel(void);
void printWaterLevel(void);
void printFeeder(void);
void printLCD(void);
void printLED1(int redLED, int greenLED, int blueLED, int cycle);
void printLED2(int redLED, int greenLED, int blueLED);
void printLED3(int redLED, int greenLED, int blueLED, int dirty);

int main(void)	{
	// Analog pre-scaler: 128 = 16 MHz / 125 kHz.
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	// AVcc, 8-bit.
	ADMUX |= (1 << REFS0) | (1 << ADLAR) | (1 << MUX0);
	// Enable ADC interrupts.
	ADCSRA |= (1 << ADIE);
	// Enable ADC.
	ADCSRA |= (1 << ADEN);
	// Data direction.
	DDRB |= 0x4E;
	DDRC |= 0xFF;
	DDRD |= 0xF4;
	// Enable internal pull-up for digital inputs.
	C_SETBIT(WTR_LVL_LOW);
	C_SETBIT(WTR_LVL_MED);
	C_SETBIT(WTR_LVL_HIGH);
	C_SETBIT(SAL4);
	// Disable JTAG.
	MCUCSR = (1<<JTD);
	MCUCSR = (1<<JTD);
	ADCSRA |= (1 << ADSC);

	init_devices();
	sei();

	while (1)	{	
		//if(waterLock == 0)	{
			readTemp();
			printTemp();
			readpH();
//			readSal();
			readLight();
			printLight();
			readWaterLevel();
			printWaterLevel();
//			printFeeder();
			printLCD();
			_delay_ms(1000);
		//}
		//else{
			//readWaterLevel();
			//printWaterLevel();
			//_delay_ms(1000);
		//}
	}
}

void readTemp()	{
	tempe = ds18b20_gettemp();
	dtostrf(tempe, 4, 1, tempBuff);
}

void printTemp()	{
	if(tempe < lowTemp)	{
		C_CLEARBIT(COOLER);
		C_SETBIT(HEATER);
		printLED1(150,0,0,1);
	}
	else if(tempe > highTemp)	{
		C_CLEARBIT(HEATER);
		C_SETBIT(COOLER);
		printLED1(0,0,150,1);
	}
	else{
		C_CLEARBIT(HEATER);
		C_CLEARBIT(COOLER);
		printLED1(0,150,0,0);
	}
}

void readpH()	{
	pH = (double)ADCBuff[1] * 60.0 / 1024;
	dtostrf(pH, 3, 1, pHBuff);
}

void readSal()	{
	sal = ADCBuff[2];
	sal = (sal / 255) * 999;
}

void readLight()	{
	light = ADCBuff[5];
	itoa(light, lightBuff, 10);
	strcat(lightBuff, "  ");
}
void printLight()	{
	if(light < lightThreshold)	{
		printLED3(0,150,0,1);
	}
	else{
		printLED3(0,150,0,0);
	}
}

void readWaterLevel()  {
	waterLevel = 0;
	if(!(PINB & (1 << PB4)))	{
		waterLevel = 1;
		if(!(PINB & (1 << PB5)))	{
			waterLevel = 2;
		}
	}
	switch(waterLevel) {
		case 0:
			waterLock = 1;
			break;
		case 1:
			waterLock = 0;
			break;
		case 2:
			waterLock = 0;
			break;
		default:
			waterLock = 1;
			break;
	}
}

void printWaterLevel()	{
	if(waterLock == 0)	{
		printLED2(150,150,150);
	}
	else{
		printLED2(5,5,5);
	}
}

void printFeeder()	{
	C_SETBIT(FEEDER);
	_delay_ms(1000);
	C_CLEARBIT(FEEDER);
}

void printLCD()	{
	if(Change_Display != Change_Display1)	{
		if(Change_Display == 0)	{
			LCD_DisplayString_F(1,1,"Climate");
			LCD_DisplayString_F(1,9,tempBuff);
			LCD_DisplayString_F(1,14,"C");
			LCD_DisplayString_F(2,1,"pH");
			LCD_DisplayString_F(2,4,pHBuff);
			LCD_DisplayString_F(2,8,"Sal");
			LCD_DisplayString_F(2,12,lightBuff);
			LCD_DisplayString_F(2,16,"t");
		}
		Change_Display1 = Change_Display;
	}
	Change_Display = ~Change_Display;
}

// Heat, cycle red. Cool, cycle blue. Otherwise, solid green.
void printLED1(int redLED, int greenLED, int blueLED, int cycle)	{
	if((iLED == 0) && (cycle == 1))	{
		for(jLED = 0; jLED < 10; jLED+=2)	{
			colors[jLED] = (rgb_color){ redLED, greenLED, blueLED };
		}
		for(jLED = 1; jLED < 10; jLED+=2)	{
			colors[jLED] = (rgb_color){ 0, 0, 0 };
		}
		iLED = 1;
	}
	else if((iLED == 1) && (cycle == 1))	{
		for(jLED = 0; jLED < 10; jLED+=2)	{
			colors[jLED] = (rgb_color){ 0, 0, 0 };
		}
		for(jLED = 1; jLED < 10; jLED+=2)	{
			colors[jLED] = (rgb_color){ redLED, greenLED, blueLED };
		}
		iLED = 0;	
	}
	else{
		for(jLED = 0; jLED < 10; jLED++)	{
			colors[jLED] = (rgb_color){ redLED, greenLED, blueLED };
		}
	}
	led_strip_write(colors, LED_COUNT);
}

// If water present, lights are solid white. Otherwise, gray.
void printLED2(int redLED, int greenLED, int blueLED)	{
	for(jLED = 10; jLED < 20; jLED++)	{
		colors[jLED] = (rgb_color){ redLED, greenLED, blueLED };
	}
	led_strip_write(colors, LED_COUNT);
}

// If clean, lights are solid green. If dirty, blink green.
void printLED3(int redLED, int greenLED, int blueLED, int dirty)	{
	if(dirty == 0)	{
		for(jLED = 20; jLED < LED_COUNT; jLED++)	{
			colors[jLED] = (rgb_color){ redLED, greenLED, blueLED };
		}
	}
	else	{
		if(blinker == 0)	{
			for(jLED = 20; jLED < LED_COUNT; jLED++)	{
				colors[jLED] = (rgb_color){ redLED, greenLED, blueLED };
			}
			blinker = 1;
		}
		else{
			for(jLED = 20; jLED < LED_COUNT; jLED++)	{
				colors[jLED] = (rgb_color){ 0, 0, 0 };
			}
			blinker = 0;
		}
	}
	led_strip_write(colors, LED_COUNT);
}

ISR(ADC_vect)	{
	switch(ADMUX)	{
		case 0x61:
			ADCBuff[1] = ADCH;
			ADMUX = 0x65;
			break;
		case 0x65:
			ADCBuff[5] = ADCH;
			ADMUX = 0x61;
			break;
		default:
			break;
	}
	ADCSRA |= (1 << ADSC);
}