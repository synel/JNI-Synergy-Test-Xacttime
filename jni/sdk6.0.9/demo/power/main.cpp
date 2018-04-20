#include <stdio.h>  
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../libfunc.h"

//#define _2410
#define _2416

int main(int argc, char *argv[]){

	printf("Usage: powertest baud_table_index\r\n");
	printf("baud table: 2400,4800,9600,19200,38400,57600,115200\r\n");

	COMINFO cominfo[4];
	int  status;
	int energy;

	if(argc<2||atoi(argv[1])>6)	
	{
		printf("pls input baud rate\n");
		return -1;
	}

	cominfo[0].enable=0;
	cominfo[2].enable=0;//rs485
#if defined _2410
	cominfo[1].enable=1;//reader
	cominfo[1].baudrate=atoi(argv[1]);	//2400,4800,9600,19200,38400,57600,115200
	cominfo[1].workmode=1;
#elif defined _2416

	cominfo[1].enable=0;//gsm

	cominfo[3].type = 1; //0 = uart 1 = spi reader
	cominfo[3].enable=1;
	cominfo[3].baudrate=atoi(argv[1]);	//2400,4800,9600,19200,38400,57600,115200
	cominfo[3].workmode=1; //answer
#endif
	
	if(OpenCom(cominfo) == FALSE) // Initialize serial port
	{
		perror("com init error");
		return -1;
	}

	while(1)
	{
		if(ReadCom1()==FALSE)	continue;

		status = get_battery_power_status();

		switch(status){
		case 0:
			printf("no battery power\n");
			break;
		case 1:
			printf("battery charging\n");
			energy = get_battery_power_energy();
			printf("energy %d\n",energy);
			break;
		case 2:
			printf("battery charged\n");
			energy = get_battery_power_energy();
			printf("energy %d\n",energy);
			break;
		case 3:
			printf("battery discharging\n");
			energy = get_battery_power_energy();
			printf("energy %d\n",energy);
			break;
		case 4:
			printf("low voltage\n");
			break;
		case 5:
			printf("high voltage\n");
			break;
		default:
			printf("unknown\n");
			break;
		}
	}

#if defined _2410
	UnCom(1);// Close serial device
#elif defined _2416
	UnCom(3 + 10);
#endif
	return 0;
}
