/*
 * SynergyUtility.c
 *
 *  Created on: Nov 5, 2014
 *      Author: chaol
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/time.h>
#include "gpio/gpio.h"
#include "SynergyUtility.h"


#define hlp_printf printf
#define app_printf printf

struct timeval tv1,tv2;
char *templateLocation;
int fd ;

void _tick_start()
{
	gettimeofday(&tv1, NULL);
}
/*
int _tick_end()
{
	gettimeofday(&tv2, NULL);
	return (tv2.tv_sec-tv1.tv_sec)*1000000+(tv2.tv_usec-tv1.tv_usec);
}

void _tick_print(char* szPrompt, int nTime)
{
	app_printf("%s %d.%03ds\n",szPrompt,nTime/1000000,(nTime/1000)%1000);
}
*/

char* _ultostr(unsigned long num, char *str, int base)
{
	//hlp_printf("Convert unsigned long number to string..base %d.\n",base);
	unsigned long temp = num;
	unsigned int digit;
	if ( NULL == str || 1 > base ){
		return NULL;
	}
	//Calculate number of digits for the string representation of the number
	do {
		temp /= base;
		str += 1;     //move str_ptr to one digit space right
	} while ( temp > 0 );
	*str = '\0';
	//Now move backwards to fill the digits
	do {
		digit = num-base*(num/base);
		if ( digit < 10 ){
			*(--str) = '0' + digit;
		}
		else {
			*(--str) = 'A'-10 + digit;
		}
		num = num/base;
	} while (num > 0);
	return str;
}

void _initGpio() {
    int ret = init_gpio_devices(0L,0);
        if (ret != 1)
        {
                hlp_printf("GPIO Init Fail\n");
                return;
        }
        else
        {
                hlp_printf("GPIO Init Success\n");
        }

        hlp_printf("init gpio %d values..\n",GREEN_LED);
        init_gpio_value(GREEN_LED, 0, 0);
        hlp_printf("init gpio %d values..\n",RED_LED);
        init_gpio_value(RED_LED, 1, 0);
        init_gpio_value(LCD, 2, 1);
        init_gpio_value(SIDE_USB_POWER, 3, 1);
        init_gpio_value(CAMERA, 4, 1);
        init_gpio_value(GPRS_POWER_SUPPLY, 5, 0);
        init_gpio_value(GPRS_POWER, 6, 1);
        init_gpio_value(OUT_1, 7, 0);
        init_gpio_value(USB_POWER, 9, 1);
        init_gpio_value(BOTTOM_USB_POWER, 10, 1);
        init_gpio_value(SD_POWER, 11, 1);
        init_gpio_value(FP_LIGHT, 13, 1);
        init_gpio_value(INPUT_1, 0, 0);
        hlp_printf("init gpio values done!\n");
}
