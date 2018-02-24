#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "ds18b20.h"

uint8_t ds18b20_reset() {
	uint8_t i;

	// Low.
	DS18B20_PORT &= ~ (1<<DS18B20_DQ);
	DS18B20_DDR |= (1<<DS18B20_DQ);
	_delay_us(480);

	// Release line.
	DS18B20_DDR &= ~(1<<DS18B20_DQ);
	_delay_us(60);

	// Get value.
	i = (DS18B20_PIN & (1<<DS18B20_DQ));
	_delay_us(420);

	return i;
}

void ds18b20_writebit(uint8_t bit)	{
	// Low.
	DS18B20_PORT &= ~ (1<<DS18B20_DQ);
	DS18B20_DDR |= (1<<DS18B20_DQ);
	_delay_us(1);

	if(bit)
		DS18B20_DDR &= ~(1<<DS18B20_DQ);

	// Release line.
	_delay_us(60);
	DS18B20_DDR &= ~(1<<DS18B20_DQ);
}

uint8_t ds18b20_readbit(void)	{
	uint8_t bit=0;

	// Low.
	DS18B20_PORT &= ~ (1<<DS18B20_DQ);
	DS18B20_DDR |= (1<<DS18B20_DQ);
	_delay_us(1);

	// Release line.
	DS18B20_DDR &= ~(1<<DS18B20_DQ);
	_delay_us(14);

	// Read value.
	if(DS18B20_PIN & (1<<DS18B20_DQ))
		bit=1;

	//wait 45uS and return read value
	_delay_us(45);
	return bit;
}

void ds18b20_writebyte(uint8_t byte)	{
	uint8_t i=8;
	while(i--){
		ds18b20_writebit(byte&1);
		byte >>= 1;
	}
}

uint8_t ds18b20_readbyte(void)	{
	uint8_t i=8, n=0;

	while(i--){
		n >>= 1;
		n |= (ds18b20_readbit()<<7);
	}
	return n;
}

double ds18b20_gettemp() {
	uint8_t temperature_l;
	uint8_t temperature_h;
	double retd = 0;

	#if DS18B20_STOPINTERRUPTONREAD == 1
	cli();
	#endif

	ds18b20_reset();
	ds18b20_writebyte(DS18B20_CMD_SKIPROM);
	ds18b20_writebyte(DS18B20_CMD_CONVERTTEMP);

	// Wait until conversion is complete.
	while(!ds18b20_readbit());

	ds18b20_reset();
	ds18b20_writebyte(DS18B20_CMD_SKIPROM);
	ds18b20_writebyte(DS18B20_CMD_RSCRATCHPAD);

	// Read two bytes from scratchpad.
	temperature_l = ds18b20_readbyte();
	temperature_h = ds18b20_readbyte();

	#if DS18B20_STOPINTERRUPTONREAD == 1
	sei();
	#endif

	// Convert value.
	retd = ( ( temperature_h << 8 ) + temperature_l ) * 0.0625;

	return retd;
}

