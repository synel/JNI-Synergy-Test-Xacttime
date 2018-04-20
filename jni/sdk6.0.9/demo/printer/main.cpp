#include<stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "../libfunc.h"

//#define _2410
#define _2416

int main(int argc, char *argv[])
{
#if defined _2410
	COMINFO cominfo[3];
#elif defined _2416
	COMINFO cominfo[4];
#endif

#if defined _2410
	cominfo[0].enable=1;	//serial 0 enable
	cominfo[2].enable=0;	//serial 2 disable
	cominfo[1].enable=0;	//serial 1 disable
	cominfo[0].baudrate=1;	//set serial 0 baud rate 4800 (2400,4800,9600,19200,38400,57600,115200)
	cominfo[0].workmode=1;	
#elif defined _2416
	cominfo[0].enable=1;	//serial 0 enable
	cominfo[0].baudrate=1;	//set serial 0 baud rate 4800 (2400,4800,9600,19200,38400,57600,115200)
	cominfo[0].workmode=1;
	cominfo[1].enable=0;	//serial 1 disable
	cominfo[2].enable=0;	//serial 2 disable
	cominfo[3].enable=0;
#endif

	if(OpenCom(cominfo) == FALSE) // Initialize serial port
	{
		perror("com init error");
		return -1;
	}


	PrintData("this test section!!");   //print data
	sleep(2);
	UnPrintCom();
	return 0;
}
