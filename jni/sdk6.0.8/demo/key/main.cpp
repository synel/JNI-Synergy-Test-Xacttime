#include <stdio.h>  
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../libfunc.h"

//#define _2410
#define _2416

int main(int argc, char *argv[]){

	printf("Usage: keyboardtest baud_table_index keymode\r\n");
	printf("baud table: 0 = 2400,1 = 4800, 2 = 9600,3 = 19200,4 = 38400,5 = 57600,6 = 115200\r\n");
	printf("keymode: 0 = number 1 = alpha 2 = alpha-number \r\n");

	int key=-1,mode=-1;
	char *p=NULL;

	COMINFO cominfo[4];

	KEYINFO keyinfo;
	SOUNDINFO soundinfo;

	if(argc<3)	return -1;

	cominfo[0].enable=0;//
	cominfo[2].enable=0;//rs485
#if defined _2410
	cominfo[1].enable=1;
	cominfo[1].baudrate=atoi(argv[1]);	//2400,4800,9600,19200,38400,57600,115200
	cominfo[1].workmode=1;
#elif defined _2416

	cominfo[1].enable=0;//gsm

	cominfo[3].type = 1; //0 = uart 1 = spi
	cominfo[3].enable=1;
	cominfo[3].baudrate=atoi(argv[1]);	//2400,4800,9600,19200,38400,57600,115200
	cominfo[3].workmode=1; //answer
#endif

	keyinfo.enable=1;
	keyinfo.mode=atoi(argv[2]);
	keyinfo.timeout=2;

	soundinfo.sound_kq=1;
	InitVolume(&soundinfo);

	if(OpenCom(cominfo) == FALSE) // Initialize serial port
	{
		perror("com init error");
		return -1;
	}

	InitKeyBoard(&keyinfo);
	printf("pls press key \n");
	while(1)
	{
		if(ReadCom1()==FALSE)	continue;

		key=ThreadKeyBoard();
		if(key==-1)	continue;
		printf("key=%d\n",key);	

		switch(key)
		{
			case 11:
				mode=GetKeyMode();
				printf("mode=%d\n",mode);
				break;
			case 12:
				SetKeyMode(1);
				break;
			default:
				p=ReadKey(key);
				if (p){
				printf("%s\n",p);
				}
			break;
		}

	}
#if defined _2410
	UnCom(1);// Close serial device
#elif defined _2416
	UnCom(3 + 10);// Close serial device
#endif
	return 0;
}
