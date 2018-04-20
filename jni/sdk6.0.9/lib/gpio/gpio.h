#ifndef __GPIO_H
#define __GPIO_H
#include "../_precomp.h"
#include "../version.h"

#define get_DAT 0x4800
#define put_DAT 0x4801
#define con_DAT 0x4802
#define get_GPFDAT 0x4800
#define put_GPFDAT 0x4801


/****************************
pin define:

7	128	red-led
6	64	green-led
5	32	lcd-backlight
4	16	input-point
3	8	relay
2	4	camera
1	2	NC
0	1	NC
*****************************/
 typedef struct _Gpio_INFO
 {
	int ledno;
	char opt_type;
	int duration;
	int enable;
	time_t s_time,r_time;
 }GPIOINFO;

 int OpenGpioBoard(IN char *path);
 int GpioOpt(IN int opt);
 int Gpio_Sub(const char *buf);
 int LedFlashing(void);
 int CloseGpioBoard(void);
 int GetIostate(void);

 int red_on(void);
 int red_off(void);
 int green_on(void);
 int green_off(void);
 int lcd_on(void);     
 int lcd_off(void);     
 int relay_on(void);   
 int relay_off(void);   
 int video_on(void);
 int video_off(void);

#endif 
