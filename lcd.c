/*
 * lcd.c
 * Created: 10/5/2017 11:18:29 PM
 *  Author: Amer
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "LCD.h"

void port_init(void)	{
	DDRC |= 0xFF;
	PORTC |= 0x00;
	DDRD |= 0xF4;
	PORTD |= 0x00;
}

void init_devices(void)	{
	cli();
	port_init();
	LCD_init();
	MCUCR = 0x00;
	GICR = 0x00;
	TIMSK = 0x00;
}

void LCD_init(void)	{
	delay_ms(100);
	// Eight data lines.
	LCD_WriteCommand(0x38);
	// Cursor Setting.
	LCD_WriteCommand(0x06);
	// Display on.
	LCD_WriteCommand(0x0F);
	// Clear LCD memory.
	LCD_WriteCommand(0x01);
	delay_ms(10);
}

void LCD_WriteCommand(unsigned char Command)	{
	SET_LCD_CMD;
	PORTC = Command;
	ENABLE_LCD;
	asm("nop");
	asm("nop");
	DISABLE_LCD;
	delay_ms(1);
}

void LCD_WriteData(unsigned char Data)	{
	SET_LCD_DATA;
	PORTC = Data;
	ENABLE_LCD;
	asm("nop");
	asm("nop");
	DISABLE_LCD;
	delay_ms(1);
}

void LCD_DisplayString_F(char row, char column,
char *string)	{
	LCD_Cursor(row, column);
	while(*string)
	LCD_WriteData(*string++);
}

void LCD_Cursor(char row, char column)	{
	switch(row)	{
		case 1: LCD_WriteCommand(0x80 + column - 1); break;
		case 2: LCD_WriteCommand(0xc0 + column - 1); break;
		default: break;
	}
}

// LCD delay: 16 MHz crystal.
void delay_ms(int miliSec)	{
	int i,j;

	for(i = 0; i < miliSec; i++)
	for(j = 0; j < 1550; j++)	{
		asm("nop");
		asm("nop");
	}
}