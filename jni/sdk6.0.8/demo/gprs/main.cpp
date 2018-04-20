#include<stdio.h>
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
	printf("wddaflag=%d\n",wddaflag);
	MenuKey("ft");
	return 0;
}

int verify_manger(char *id,char *password,char *card)
{
	printf("manger=%s,%s,%s\n",id,password,card);
	return 0;
}

void writets(char *command,const char *command2,char *msg)
{
	printf("%s,%s,%s\n",command,command2,msg);
}

int main(int argc, char *argv[])
{
	int netflag=1;
	unsigned char data[2048];
#if defined _2410
	int port=2,rate=115200,overtime=30,protocol=0;
#elif defined _2416
	int port=1,rate=115200,overtime=30,protocol=0;
#endif

	CALBACK calback;
	NETINFO netinfo;

	calback.netback=netback;
	calback.endback=endback;
	calback.verify_manger=verify_manger;
	calback.write_ts=writets;

	strcpy(netinfo.netovertime,"30");
	strcpy(netinfo.netport,"3350");
	strcpy(netinfo.netserverip,"219.146.0.130");
	strcpy(netinfo.dailiip,"222.173.219.54");
	//netinfo.dailisocket=23350;
	netinfo.linetype=3;
	netinfo.line=0;

	if(InitNet(&netinfo,&calback)==FALSE)
		printf("Init error\n");

	InitGprs(port, rate, overtime, protocol,"222.173.219.54",5003); // Initialize gprs
	while(1)
	{
		GprsJianche();	//gprs connection check
	     if(netflag) 		//waiting for connection
	      {
			if(AcceptLinezd() != -1)		netflag=0;
	      }

		if(netflag==0)
		{
			if(Net()==-1)	netflag=1;
		}
	}

  return 0;
}
