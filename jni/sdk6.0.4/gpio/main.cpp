//#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../libfunc.h"

//#define _2410
#define _2416

int main(int argc, char *argv[])
{
	char buf[12];
	 printf("Usage: gpiotest\n");
	//init
#if defined _2410
	if(OpenGpioBoard("/dev/GPIO2_5") == FALSE) // Open GPIO device and initialize it 
	{
		return -1;
	}
#elif defined _2416
	if(OpenGpioBoard("/dev/gpio_2416") == FALSE) // Open GPIO device and initialize it
	{
		return -1;
	}
#endif
	Gpio_Sub((const char*)"G,S,2");
	Gpio_Sub((const char*)"R,B,2");
	Gpio_Sub((const char*)"0,S,2");


/*
	GetIostate();
	while(1)
	{
		fgets(buf,sizeof(buf),stdin);
		if(GpioOpt(atoi(buf)) == FALSE) // Specific operation to GPIO
		{
			printf("error");
		}
	}
*/
	while(1)
	{
		LedFlashing(); // led on at time you specified
	}

	if(CloseGpioBoard() == FALSE) // Close GPIO device
	{
		return -1;
	}
	return 0;
}
