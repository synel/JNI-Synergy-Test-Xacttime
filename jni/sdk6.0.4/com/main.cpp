#include<stdio.h>
#include<stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "../libfunc.h"

//#define _2410
#define _2416

int netback(const char* filename )
{
	printf("filename=%s\n",filename);
	return 0;
}
int endback( int wddaflag )
{
	MenuKey("ft");
	printf("wddaflag=%d\n",wddaflag);
	return 0;
}

int verify_manger(char *id,char *password,char *card)
{
	printf("manger=%s,%s,%s\n",id,password,card);
	return 0;
}

int main(int argc, char *argv[])
{
	int flag=1;
#if defined _2410
	COMINFO cominfo[3];
#elif defined _2416
	COMINFO cominfo[4];
#endif

	printf("Usage: comcommtest baud_table_index\r\n");
	printf("baud table: 2400,4800,9600,19200,38400,57600,115200\r\n");

	if(argc<2)	
	{
		printf("pls input baud rate\n");
		return -1;
	}
#if defined _2410
	cominfo[0].enable=0;
	cominfo[1].enable=0;//reader
	cominfo[2].enable=1;//rs485
	cominfo[2].baudrate=atoi(argv[1]);
	cominfo[2].workmode=0;
	cominfo[2].overtime=30;
#elif defined _2416
	cominfo[0].enable=0;
	cominfo[1].enable=0;//gsm
	cominfo[2].enable=1;//rs485
	cominfo[2].baudrate=atoi(argv[1]);
	cominfo[2].workmode=0; //recv 0xAA..0xA5
	cominfo[2].overtime=30;
	cominfo[3].enable=0;//reader
#endif
	CALBACK calback;
	NETINFO netinfo;


	calback.netback=netback;
	calback.endback=endback;
	calback.verify_manger=verify_manger;

	strcpy(netinfo.netovertime,"30");
	strcpy(netinfo.netport,"3350");
	strcpy(netinfo.netserverip,"192.168.0.95");
	netinfo.linetype=1;
	netinfo.line=0;
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
	SOUNDINFO soundinfo;

//	soundinfo.maxyl=80;
	soundinfo.sound_kq=1;
	InitVolume(&soundinfo);

	if(InitNet(&netinfo,&calback)==FALSE)  //initialize socket and listen
		printf("Init error\n");

	if(OpenCom(cominfo) == FALSE) // Initialize serial port
	{
		perror("com init error");
		return -1;
	}
	while(1)
	{
		flag=AcceptLinecom(); //wait connection
		if(flag ==1) 	//judge whether there is connection requestion
		{ 
			MenuKey("fw");
			printf("com line ok\n");	
		}	
		else if(flag==2) //connection ok
		{
			ComSend();//recv request and send response
		}
	}
	UnCom(2);
	return 0;
}
