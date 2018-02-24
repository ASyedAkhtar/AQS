#ifndef LCD_H_
#define LCD_H_

#include "lcd.h"

void init_devices(void);
void LCD_init(void);
void LCD_WriteCommand (unsigned char CMD);
void LCD_WriteData (unsigned char Data);
void LCD_DisplayString_F(char row, char column,
char *string);
void LCD_Cursor(char row, char column);
void delay_ms(int miliSec);
#define ENABLE_LCD PORTD |= 0x80
#define DISABLE_LCD PORTD &= ~0x80
#define SET_LCD_DATA PORTD |= 0x20
#define SET_LCD_CMD PORTD &= ~0x20
#define pushButton1_PRESSED !(PINB & 0x02)
#define pushButton1_OPEN (PINB & 0x02)

#endif /* LCD_H_ */