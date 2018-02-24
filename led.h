#ifndef LED_H_
#define LED_H_

#define LED_STRIP_PORT PORTD
#define LED_STRIP_DDR  DDRD
#define LED_STRIP_PIN  4
#define LED_COUNT 30

typedef struct rgb_color
{
	unsigned char red, green, blue;
} rgb_color;
rgb_color colors[LED_COUNT];

void __attribute__((noinline)) led_strip_write(rgb_color * colors, unsigned int count);

#endif /* LED_H_ */