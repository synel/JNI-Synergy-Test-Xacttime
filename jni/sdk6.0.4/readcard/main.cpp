#include <stdio.h>  
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../libfunc.h"

//#define _2410
#define _2416

int main(int argc, char *argv[]){

	printf("Usage: readertest baud_table_index\r\n");
	printf("baud table: 2400,4800,9600,19200,38400,57600,115200\r\n");

	char buf[128],cardsno[128],ver[32],*Pwg=NULL;
	int cardtype,wgtype=0,tmp=0,i=0,len=0;
	unsigned long  int data=0;

	COMINFO cominfo[4];

	struct card_INFO cardinfo;

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

	cominfo[3].type = 1; //0 = uart 1 = spi
	cominfo[3].enable=1;
	cominfo[3].baudrate=atoi(argv[1]);	//2400,4800,9600,19200,38400,57600,115200
	cominfo[3].workmode=1; //answer
#endif
	
	cardinfo.track=2;

	if(OpenCom(cominfo) == FALSE) // Initialize serial port
	{
		perror("com init error");
		return -1;
	}

	memset(ver,0,sizeof(ver));
	//printf("getver \n");
	//get keyboardVer 

	if(get_keyboardVer(ver)==TRUE)
	{
		printf("ver %s\n",ver);
	}

	InitCard(&cardinfo);

	printf("pls put  card \n");

	while(1)
	{
		if(ReadCom1()==FALSE)	continue;

		memset(buf,0,sizeof(buf));
/*

*/
		cardtype=ReadCard(buf);
//		if(cardtype==0X00)	continue;
		//printf("buf:%s\n",buf);
		if(cardtype!=0)
		//printf("return value: %d\n",cardtype);
		//printf("return value: %2X\n",cardtype);
		switch(cardtype)
		{
			case -1:printf("read magetic failure\n");break;
			case -2:printf("read barcode decode failure\n");break;
			case -3:printf("read barcode failure\n");break;
			case -4:printf("read magetic decode failure\n");break;
			case -5:printf("unknown error\n");break;
			case -6:printf("read EM decode failure\n");break;
			case -7:printf("read EM failure\n");break;
			case -8:printf("read MF1 decode failure\n");break;
			case -9:printf("read MF1 failure\n");break;
			case -10:printf("read HID decode failure\n");break;
			case -11:printf("read HID failure\n");break;
			case 0X9C: //HID
					len=buf[0];
					wgtype=buf[1];
					Pwg=buf+2;
					printf("wgtype %d;len=%d\n",wgtype,len);
					switch(wgtype)
					{
					case 26:	//wg26 26bit; 4 byte	
						data=0;
						for(i=0;i<len-1;i++){
							data=(data<<8)|Pwg[i];
							//printf("%X,%X\n",data,Pwg[i]);
						}
						data=data>>(sizeof(data)*8-wgtype);
						data=(data&0x1FFFFFE)>>1;//get wg valid bit ;(clear odd bit/even bit) you can calc these
						
						printf("%02X,%02X,%02X,%02X\n",Pwg[0],Pwg[1],Pwg[2],Pwg[3]);
						printf("cardno %05ld\n",data&0XFFFF);
						break;
						/*
						example:
						B4,C1,07,80
						----bit---->
						10110100 11000001 00000111 10000000
						-----get 26bit ---->
						10110100 11000001 00000111 10
						10 11010011 00000100 00011110
						*/
					case 34://wg34,you can add it	
						break;
					}
					break;
			case 0x97://voltage wiegand
				printf("wiegand \r\n");
				len=buf[0];
									wgtype=buf[1];
									Pwg=buf+2;
									printf("wgtype %d;len=%d\n",wgtype,len);
									switch(wgtype)
									{
									case 26:	//wg26 26bit; 4 byte
										data=0;
										for(i=0;i<len-1;i++){
											data=(data<<8)|Pwg[i];
											//printf("%X,%X\n",data,Pwg[i]);
										}
										data=data>>(sizeof(data)*8-wgtype);
										data=(data&0x1FFFFFE)>>1;//get wg valid bit ;(clear odd bit/even bit) you can calc these

										printf("%02X,%02X,%02X,%02X\n",Pwg[0],Pwg[1],Pwg[2],Pwg[3]);
										printf("cardno %05ld\n",data&0XFFFF);
										break;
										/*
										example:
										B4,C1,07,80
										----bit---->
										10110100 11000001 00000111 10000000
										-----get 26bit ---->
										10110100 11000001 00000111 10
										10 11010011 00000100 00011110
										*/
									case 34://wg34,you can add it
										break;
									}
									break;
			case 0x98:break;//power mode
			//case 0x91:printf("card no.:%s\n",buf);
			case 0:break;
			default:printf("other card :%s\n",buf);
			break;
		}

		if(cardtype==0X93){
			memset(cardsno,0,sizeof(cardsno));
			card_linearhandle(buf,cardsno);
			printf("card no.:%s\n",cardsno);
			memset(cardsno,0,sizeof(cardsno));
			card_synelhandle(buf,cardsno);
			printf("card no.:%s\n",cardsno);
		}

		//usleep(200000);
	}

#if defined _2410
	UnCom(1);// Close serial device
#elif defined _2416
	UnCom(3 + 10);
#endif
	return 0;
}
