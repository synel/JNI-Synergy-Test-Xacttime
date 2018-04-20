#include <stdio.h>  
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../libfunc.h"

//#define _2410
#define _2416

int main(int argc, char *argv[]){
	int return_code;

	printf("Usage: watchdogtest status\r\n");
	printf("status: 0 = close 255 = shutdown 1-254 = interval\r\n");

	if (argc < 2){
		return 0;
	}

	COMINFO cominfo[3];
	int i=0;
	if(argc==2)
	i=atoi(argv[1]);
	cominfo[0].enable=0;
	cominfo[2].enable=0;
#if defined _2410
	cominfo[1].enable=1;
	cominfo[1].baudrate=6;
	cominfo[1].workmode=1;
#elif defined _2416
	cominfo[1].enable=0;

	cominfo[3].type=1;
	cominfo[3].enable=1;
	cominfo[3].baudrate=6;
	cominfo[3].workmode=1;
#endif

	if(OpenCom(cominfo) == FALSE) // Initialize serial port
	{
		perror("com init error");
		return -1;
	}
	printf("set machine mode\r\n");
	return_code = Set_Machine_Mode(i);
	printf("return_code %d\r\n",return_code);
#if defined _2410
	UnCom(1);// Close serial device
#elif defined _2416
	UnCom(3 + 10);// Close serial device
#endif
	return 0;
}
