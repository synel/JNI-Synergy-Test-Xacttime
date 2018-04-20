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
#include "./include/SynergyUtility.h"


#define hlp_printf printf
#define app_printf printf

struct timeval tv1,tv2;
char *templateLocation;
int fd ;
off_t target = 0x56000000;


void _tick_start()
{
	gettimeofday(&tv1, NULL);
}

int _tick_end()
{
	gettimeofday(&tv2, NULL);
	return (tv2.tv_sec-tv1.tv_sec)*1000000+(tv2.tv_usec-tv1.tv_usec);
}

void _tick_print(char* szPrompt, int nTime)
{
	app_printf("%s %d.%03ds\n",szPrompt,nTime/1000000,(nTime/1000)%1000);
}

int _memory_setup(uint32_t** gpio){
	size_t free =getpagesize();
	//Obtain handle to physical memory
	if ((fd = open ("/dev/mem", O_RDWR | O_SYNC) ) < 0) {
		printf("Unable to open /dev/mem: %s\n", strerror(errno));
		return -1;
	}
	//map a page of memory to gpio at offset 0x56000000
	*gpio = (uint32_t *)mmap(0, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, fd, target );
	if((int32_t)*gpio < 0) {
		printf("Mmap failed: %s\n", strerror(errno));
		return -1;
	}
#ifdef _2410
	*gpio = *gpio + 0X1C;//Offset of GPH Register base is $target adding 1 to base pointer adds 4 to address in 32 bit memory offset of GPH is 70 0x70/0x4 = 0x1C
#endif

	return 0;
}

int _memory_close(uint32_t** gpio){
	close(fd);
#ifdef _2410
	*gpio = *gpio - 0x1C;
#endif

	if(munmap(*gpio, getpagesize())<0){
		return -1;
	}
	else{
		return 0;
	}
}

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



