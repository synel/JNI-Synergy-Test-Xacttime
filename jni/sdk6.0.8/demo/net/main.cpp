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
	int netflag=0;
	char buf[128];
	CALBACK calback;
	NETINFO netinfo;

//	if(argc<2)	return 0;

	calback.netback=netback;
	calback.endback=endback;
	calback.verify_manger=verify_manger;
	calback.write_ts=writets;

	strcpy(netinfo.netovertime,"30");
	strcpy(netinfo.netport,"3350");
	strcpy(netinfo.netserverip,"219.146.0.130");
	strcpy(netinfo.dailiip,"222.173.219.54");
	netinfo.dailisocket=23350;
	netinfo.linetype=0;
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


	soundinfo.sound_kq=1;
	InitVolume(&soundinfo);

	if(InitNet(&netinfo,&calback)==FALSE)
		printf("Init error\n");

	printf("begin net\n");
	while(1)
	{
		netflag=AcceptLinezd();
		if(netflag ==1) 	//judge whether there is connection requestion
		{ 
//			MenuKey("fw");
			printf("net line\n");	
		}	
		else if(netflag==2)
		{
			Net();
		}

//		if(GetDomain("www.baidu.com",buf)==TRUE)	break;
	}
	printf("ip=%s\n",buf);
	UninitNet();
	  return 0;
}
