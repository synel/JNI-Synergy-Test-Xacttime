#include <stdio.h>  
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../libfunc.h"


//#define _2410
#define _2416
//write card sectors

char new_key[16]={"111111111111"},old_key[16]={"FFFFFFFFFFFF"};
char keymode=1; //1 Akey ,2 Bkey
int write_sectors()
{
	char cardsno[128],tmpsno[32];
	unsigned char buf[32];
	
	//read/write sectors
	memset(buf,0,sizeof(buf));
	strcpy((char *)buf,"abcdefghijklmnop");
	while(1)
	{

		if(ReadCom1()==FALSE)	continue;	//
		memset(cardsno,0,sizeof(cardsno));
		if(ReadCard(cardsno)==0)	continue;
		sprintf(tmpsno,"%08X",strtoul(cardsno,NULL,10));
		//cardnum,sector,block,data,keymode,key
		if(mifs_write(tmpsno,1,0,buf,keymode,old_key)==FALSE)
		printf("write error\n");
		else printf("write ok\n");
		break;
		
	}
	return 0;
}
//read card sectors
int read_sectors()
{
	int i,len=0;
	char cardsno[128],tmpsno[32];
	unsigned char buf[32];

	while(1)
	{
		if(ReadCom1()==FALSE)	continue;	//
		memset(cardsno,0,sizeof(cardsno));
		if(ReadCard(cardsno)==0)	continue;
		sprintf(tmpsno,"%08X",strtoul(cardsno,NULL,10));
		//len=MF1Read(tmpsno,buf);
		printf("tmpsno=%s,cardnso=%s\n",tmpsno,cardsno);
		//cardnum,sector,block,data,keymode(1-Akey,2-Bkey),key
		if(mifs_read(tmpsno,1,0,buf,keymode,old_key)==FALSE)continue;
		//if(mifs_read(cardsno,1,0,buf,keymode,old_key)==FALSE)continue;
		for(i=0;i<16;i++)
		{
			printf("%02X ",buf[i]);
		}
		printf("\n");
		break;
	}
	printf("read ok\n");
	//read/write sectors end
	return 0;
}


int chang_key()
{

	int i,len=0;
	char cardsno[128],tmpsno[32];
	unsigned char buf[32];
	while(1)
	{
		if(ReadCom1()==FALSE)	continue;	//
		memset(cardsno,0,sizeof(cardsno));
		if(ReadCard(cardsno)==0)	continue;
		sprintf(tmpsno,"%08X",strtoul(cardsno,NULL,10));
		printf("tmpsno=%s,cardnso=%s\n",tmpsno,cardsno);

		//len=MF1Read(tmpsno,buf);
		//cardnum,sector,block,data,keymode(1-Akey,2-Bkey),key
		//block 3 is key-block
		//use old_key read data block
		if(mifs_read(tmpsno,1,0,buf,keymode,old_key)==FALSE)continue;
		printf("old_key read data:	");
		for(i=0;i<16;i++)
		{
			printf("%02X ",buf[i]);
		}
		printf("\n");
		//use old_key read key-block 
		if(mifs_read(tmpsno,1,3,buf,keymode,old_key)==FALSE)continue;
		printf("old_key read key_block:	");
		for(i=0;i<16;i++)
		{
			printf("%02X ",buf[i]);
		}
		printf("\n");
		//change key data
		if(keymode==1)
		gsmString2Bytes(new_key,buf,12);
		else gsmString2Bytes(new_key,buf+10,12);
		printf("new_key key_data:	");
		for(i=0;i<16;i++)
		{
			printf("%02X ",buf[i]);
		}
		printf("\n");
		if(mifs_write(tmpsno,1,3,buf,keymode,old_key)==FALSE)
		{
			printf("write key block error\n");
		}
		//use new_key read data block
		if(mifs_read(tmpsno,1,0,buf,keymode,new_key)==FALSE)continue;
		printf("new_key read block_data:");
		for(i=0;i<16;i++)
		{
			printf("%02X ",buf[i]);
		}
		printf("\n");
		
		//use new_key read key-block 
		if(mifs_read(tmpsno,1,3,buf,keymode,new_key)==FALSE)continue;
		printf("new_key read key_block:	");
		for(i=0;i<16;i++)
		{
			printf("%02X ",buf[i]);
		}
		printf("\n");
		//change key data to old_key
		if(keymode==1)
		gsmString2Bytes(old_key,buf,12);
		else gsmString2Bytes(old_key,buf+10,12);
		printf("old_key key_data:	");
		for(i=0;i<16;i++)
		{
			printf("%02X ",buf[i]);
		}
		printf("\n");
		if(mifs_write(tmpsno,1,3,buf,keymode,new_key)==FALSE)
		{
			printf("write key block error\n");
		}

		if(mifs_read(tmpsno,1,0,buf,keymode,old_key)==FALSE)continue;
		printf("old_key read data:	");
		for(i=0;i<16;i++)
		{
			printf("%02X ",buf[i]);
		}
		printf("\n");
		break;
	}
	printf("read ok\n");
	return 0;
	
}

int main(int argc, char *argv[]){
	
	COMINFO cominfo[3];
	struct card_INFO cardinfo;
	int mode;

	printf("Usage: rwreadertest baud_table_index mode\r\n");
	printf("baud table: 2400,4800,9600,19200,38400,57600,115200\r\n");
	printf("mode: 0 = read 1 = write 2 = change key\r\n");

	if(argc<3||atoi(argv[1])>6)	
	{
		printf("pls input baud rate\n");
		printf("./main 6 0 --read\n");
		printf("./main 6 1 --write\n");
		printf("./main 6 2 --charnge key\n");
		return -1;
	}
	mode=atoi(argv[2]);
	cominfo[0].enable=0;
	cominfo[2].enable=0;//rs485
#if defined _2410
	cominfo[1].enable=1;//reader
	cominfo[1].baudrate=atoi(argv[1]);	//2400,4800,9600,19200,38400,57600,115200
	cominfo[1].workmode=1;
#elif defined _2416
	cominfo[1].enable=0; //gsm

	cominfo[3].type=1;//spi
	cominfo[3].enable=1;
	cominfo[3].baudrate=atoi(argv[1]);	//2400,4800,9600,19200,38400,57600,115200
	cominfo[3].workmode=1;

#endif
	//read sectors

	if(OpenCom(cominfo) == FALSE) // Initialize serial port
	{
		perror("com init error");
		return -1;
	}
	InitCard(&cardinfo);
	switch(mode)
	{
		case 1:
			printf("write data\n");
			write_sectors();
			break;
		case 0:
			printf("read data\n");
			read_sectors();
			break;
		case 2:
			printf("change key");
			chang_key();
			break;
	}
#if defined _2410
	UnCom(1);// Close serial device
#elif defined _2416
	UnCom(3 + 10);// Close serial device
#endif
	return 0;
}
