#include "gpio.h"
#include "../public/public.h"

int gpiofd=-1; 
int gpiostatus=0;    
GPIOINFO gpio_info[4];

/*--------------------------------------------------------------------------*
@Function		: OpenGpioBoard - open gpio device
@Include      	: "gpio.h"
@Description		: path£ºpath of gpio device file
			00-iuput,01-out
@Return Value	: Success TRUE, Failure FALSE.
@Create time		: 2009-08-07		
*---------------------------------------------------------------------------*/
int OpenGpioBoard(IN char *path)    // 0--open ; 1--close
{
#if defined _x86
#elif defined _2410
  int val_first=207,val_set=21589;   // 207=1100,1111 ; 21761=101,0101,0000,0001

  if(access(path,F_OK)!=0||(gpiofd = open(path, O_RDWR)) == -1) {
    perror("open GPIODEV error:");
    return FALSE;
  }

  if(ioctl(gpiofd, con_DAT, &val_set) != 0) //set GPIO control register to 21589
  {
    return FALSE;
  }

  gpiostatus=val_first; //GPIO current state
  if(ioctl(gpiofd, put_DAT, &gpiostatus) != 0) //set GPIO inital state 207(LCD on,input 0,others off)
  {
    return FALSE;
  }
#else
  if(access(path,F_OK)!=0){
	  return FALSE;
  }
  gpiofd = open(path, O_RDWR);
  if(gpiofd == -1) {
    perror("open GPIODEV error:");
    return FALSE;
  }
#endif

  return TRUE;
}

/*--------------------------------------------------------------------------*
@Function				:	CloseGpioBoard - close gpio
@Include      	: "gpio.h"
@Description		: release resource occupied by gpio device£»
@Return Value		: Success TRUE, Failure FALSE.
@Create time		: 2009-08-07		
*---------------------------------------------------------------------------*/
int CloseGpioBoard(void) 
{
  if(gpiofd >0 && close(gpiofd) == -1)
  {
	  perror("close gpio");
	return FALSE;
  }
return TRUE;
} 

/*--------------------------------------------------------------------------*
@Function          val_ok - open power-light
@Include      	gpio.h
@Description		open red light
			0-open	,1-close
@Return Value	Success TRUE, Failure FALSE.
@Create time		2009-08-07		
*---------------------------------------------------------------------------*/
int red_on(void)      
{
#if defined _x86
#elif defined _2410
	int val_ok=128;   // 128=1000,0000		~128=0111,1111
	if(gpiofd<=0)	return FALSE;
	gpiostatus &=~val_ok; // set the eighth bit to 0 (open light)
	if(ioctl(gpiofd, put_DAT, &gpiostatus) != 0) // set light on 
	{
		perror("ioctl put_DAT error 1");
		return FALSE;
	}
#else
	int pin,value;
	char buf[2] = {0};
	if(gpiofd<=0)	return FALSE;
	pin = 1;
	value = 1;

    buf[0] = pin;
    buf[1] = value;

    if (write(gpiofd, buf, 2) < 0)
    {
        //plog("gpio write error!\n");
        return FALSE;
    }
#endif
	return TRUE;
}
/*--------------------------------------------------------------------------*
@Function		: val_err - power-light off
@Include      	: "gpio.h"
@Description		: shutdonw red light£» //gph10
				0-open	,1-close
@Return Value	: Success TRUE, Failure FALSE.
@Create time		: 2009-08-07		
*---------------------------------------------------------------------------*/
int red_off(void) 
{
#if defined _x86
#elif defined _2410
	int val_err=128;
	if(gpiofd<=0)	return FALSE;
	gpiostatus=gpiostatus|val_err;    // set the eighth bit to 1 (close light)
	if(ioctl(gpiofd, put_DAT, &gpiostatus) != 0) 
	 {
		  perror("ioctl put_DAT error 2");
		return FALSE;
	 }
#else
	int pin,value;
	char buf[2] = {0};
	if(gpiofd<=0)	return FALSE;
	pin = 1;
	value = 0;

    buf[0] = pin;
    buf[1] = value;

    if (write(gpiofd, buf, 2) < 0)
    {
        //plog("gpio write error!\n");
        return FALSE;
    }
#endif
	return TRUE;
}

/*--------------------------------------------------------------------------*
@Function		: light_on - Green light on
@Include      	: "gpio.h"
@Parameters		: 0-open,1-close 
@Return Value	: Success TRUE, Failure FALSE.
@Create time		: 2009-08-07		
*---------------------------------------------------------------------------*/
int green_on(void)
 {
#if defined _x86
#elif defined _2410
	int lightoff = 64; //64=0100,0000	~64=1011,1111
	if (gpiofd <= 0) return FALSE;
	gpiostatus &=~lightoff;
	if(ioctl(gpiofd, put_DAT,&gpiostatus) != 0)
	{
		perror("ioctl put_DAT error 5");
		return FALSE;
	}
#else
	int pin, value;
	char buf[2] = { 0 };
	if (gpiofd <= 0) return FALSE;
	pin = 0;
	value = 1;

	buf[0] = pin;
	buf[1] = value;

	if (write(gpiofd, buf, 2) < 0) {
		//plog("gpio write error!\n");
		return FALSE;
	}
#endif
	return TRUE;
}
/*--------------------------------------------------------------------------*
@Function		: light_off - Green light off
@Include      	: "gpio.h"
@Description		: 0-open,1-close 
@Return Value	: Success TRUE, Failure FALSE.
@Create time		: 2009-08-07		
*---------------------------------------------------------------------------*/
int green_off(void)
{
#if defined _x86
#elif defined _2410
	int lightoff=64;
	if(gpiofd<=0)	return FALSE;
	gpiostatus=gpiostatus|lightoff;
	if(ioctl(gpiofd, put_DAT,&gpiostatus) != 0)  
	{
	      perror("ioctl put_DAT error 6");
		return FALSE;
	}
#else
	int pin, value;
	char buf[2] = { 0 };
	if(gpiofd<=0)	return FALSE;
	pin = 0;
	value = 0;

	buf[0] = pin;
	buf[1] = value;

	if (write(gpiofd, buf, 2) < 0) {
		//plog("gpio write error!\n");
		return FALSE;
	}
#endif
	return TRUE;
}

/*--------------------------------------------------------------------------*
@Function		: val_on - Display on
@Include      	: "gpio.h"
@Parameters		: 0-open,1-close 
@Return Value	: Success TRUE, Failure FALSE.
@Create time		: 2009-08-07		
*---------------------------------------------------------------------------*/
int lcd_on(void)     
{
#if defined _x86
#elif defined _2410
	int val_on=32;
	if(gpiofd<=0)	return FALSE;
	gpiostatus &=~val_on;
	if(ioctl(gpiofd, put_DAT, &gpiostatus) != 0) 
	 {
		 perror("ioctl put_DAT error 3");
		return FALSE;
	 }
#else
	int pin, value;
	char buf[2] = { 0 };
	if(gpiofd<=0)	return FALSE;
	pin = 2;
	value = 1;

	buf[0] = pin;
	buf[1] = value;

	if (write(gpiofd, buf, 2) < 0) {
		//plog("gpio write error!\n");
		return FALSE;
	}
#endif
	return TRUE;
}

/*--------------------------------------------------------------------------*
@Function		: val_off - Display off
@Include      	: "gpio.h"
@Parameters		: 0-open,1-close 
@Return Value	: Success TRUE, Failure FALSE.
@Create time		: 2009-08-07		
*---------------------------------------------------------------------------*/
int lcd_off(void)      
{
#if defined _x86
#elif defined _2410
	int val_off=32; // 32=0010,0000
	if(gpiofd<=0)	return FALSE;
	gpiostatus=gpiostatus|val_off;
	if(ioctl(gpiofd, put_DAT, &gpiostatus) != 0)  
	{
		 perror("ioctl put_DAT error 4");
		return FALSE;
	}
#elif defined _2416
	int pin, value;
	char buf[2] = { 0 };
	if(gpiofd<=0)	return FALSE;
	pin = 2;
	value = 0;

	buf[0] = pin;
	buf[1] = value;

	if (write(gpiofd, buf, 2) < 0) {
		//plog("gpio write error!\n");
		return FALSE;
	}
#endif
	return TRUE;
}

/*--------------------------------------------------------------------------*
@Function		: relay_on - relay on
@Include      	: "gpio.h"
@Parameters		: 0-open,1-close
@Return Value	: Success TRUE, Failure FALSE.
@Create time		: 2009-08-07		
*---------------------------------------------------------------------------*/
int relay_on(void)     
{
#if defined _x86
#elif defined _2410
	int val_on=8;   //8=0000,1000
	if(gpiofd<=0)	return FALSE;
	gpiostatus &=~val_on;
	if(ioctl(gpiofd, put_DAT, &gpiostatus) != 0) 
	 {
		 perror("ioctl put_DAT error 3");
		return FALSE;
	 }
#else
	int pin, value;
	char buf[2] = { 0 };
	if(gpiofd<=0)	return FALSE;
	pin = 7;
	value = 0;

	buf[0] = pin;
	buf[1] = value;

	if (write(gpiofd, buf, 2) < 0) {
		//plog("gpio write error!\n");
		return FALSE;
	}
#endif
	return TRUE;
}

/*--------------------------------------------------------------------------*
@Function		: relay_off - relay off
@Include      	: "gpio.h"
@Parameters		: 0-open,1-close 
@Return Value	: Success TRUE, Failure FALSE.
@Create time		: 2009-08-07		
*---------------------------------------------------------------------------*/
int relay_off(void)      
{
#if defined _x86
#elif defined _2410
	int val_off=8; // 8=0000,1000
	if(gpiofd<=0)	return FALSE;
	gpiostatus=gpiostatus|val_off;
	if(ioctl(gpiofd, put_DAT, &gpiostatus) != 0)  
	{
		 perror("ioctl put_DAT error 4");
		return FALSE;
	}
#else
	int pin, value;
	char buf[2] = { 0 };
	if(gpiofd<=0)	return FALSE;
	pin = 7;
	value = 1;

	buf[0] = pin;
	buf[1] = value;

	if (write(gpiofd, buf, 2) < 0) {
		//plog("gpio write error!\n");
		return FALSE;
	}
#endif
	return TRUE;
}
/*--------------------------------------------------------------------------*
@Function		: video_on - Camera power on
@Include      	: "gpio.h"
@Description		: 0-close,1-open 
@Return Value	: Success TRUE, Failure FALSE.
@Create time		: 2009-08-07		
*---------------------------------------------------------------------------*/
int  video_on(void)
{
#if defined _x86
#elif defined _2410
	int val_video=4; // 0000,1000
	if(gpiofd<=0)	return FALSE;
	gpiostatus=gpiostatus|val_video;
	if(ioctl(gpiofd, put_DAT,&gpiostatus) != 0)
   	{
	      perror("ioctl put_DAT error 7");
		return FALSE;
   	}
#else
	int pin, value;
	char buf[2] = { 0 };
	if(gpiofd<=0)	return FALSE;
	pin = 6;
	value = 1;

	buf[0] = pin;
	buf[1] = value;

	if (write(gpiofd, buf, 2) < 0) {
		//plog("gpio write error!\n");
		return FALSE;
	}
#endif
	return TRUE;
}

/*--------------------------------------------------------------------------*
@Function		: video_off - Camera power off 
@Include      	: "gpio.h"
@Description		: 0-close,1-open
@Return Value	: Success TRUE, Failure FALSE.
@Create time		: 2009-08-07		
*---------------------------------------------------------------------------*/
int video_off(void)
{
#if defined _x86
#elif defined _2410
	int status,val_video=4;
	if(gpiofd<=0)	return FALSE;
	status=gpiostatus &=~val_video;
	if(ioctl(gpiofd, put_DAT,&gpiostatus) != 0)
   	{
	      perror("ioctl put_DAT error 8");
		return FALSE;
   	}
#else
	int pin, value;
	char buf[2] = { 0 };
	if(gpiofd<=0)	return FALSE;
	pin = 6;
	value = 0;

	buf[0] = pin;
	buf[1] = value;

	if (write(gpiofd, buf, 2) < 0) {
		//plog("gpio write error!\n");
		return FALSE;
	}
#endif
	return TRUE;
}

/*--------------------------------------------------------------------------*
@Function		: GpioOpt - gpio operation 
@Include      	: "gpio.h"
@Description		: opt- type£¨1-8£©
@Return Value	: Success TRUE, Failure FALSE.
@Create time		: 2009-08-07		
*---------------------------------------------------------------------------*/
int GpioOpt(IN int opt)
{
	switch(opt){
		case 1:
			if(red_on()==FALSE)		return FALSE;
			break;
		case 2:
			if(red_off()==FALSE)	return FALSE;
			break;
		case 3:
			if(green_on()==FALSE)	return FALSE;
			break;
		case 4:
			if(green_off()==FALSE)	return FALSE;
			break;
		case 5:
			if(lcd_on()==FALSE)		return FALSE;
			break;
		case 6:
			if(lcd_off()==FALSE)	return FALSE;
			break;
		case 7:
			if(relay_on()==FALSE)	return FALSE;
			break;
		case 8:
			if(relay_off()==FALSE)	return FALSE;
			break;			
		case 9:
			if(video_on()==FALSE)	return FALSE;
			break;
		case 10:
			if(video_off()==FALSE)	return FALSE;
			break;			
	}
	return TRUE;
}

/*--------------------------------------------------------------------------*
@Function		: Gpio_Sub - 
@Include      	: "gpio.h"
@Description		: lenno led£¨1 or 4£©
			// 0-red 1-green 2-relay
@Return Value	: Success TRUE, Failure FALSE.
*---------------------------------------------------------------------------*/
int Gpio_Sub(const char *buf)
{
   int num=0,len=0;//ledno=0,
   char tmp[128];


   if(strlen(buf)==0)	return FALSE;
   len=strcspn(buf,",");
   if(len>sizeof(tmp))	return FALSE;
   memset(tmp,0,sizeof(tmp));
   strncpy(tmp,buf,len);
   if(strcmp(tmp,"R")==0)	
    {
	num=0;
	memset(&gpio_info[num],0,sizeof(gpio_info[num]));
	gpio_info[num].ledno=1;
    }
   else if(strcmp(tmp,"G")==0)	
    {
	num=1;
	memset(&gpio_info[num],0,sizeof(gpio_info[num]));
	gpio_info[num].ledno=3;
    }
   else if(strcmp(tmp,"0")==0)	
    {
	num=2;
	memset(&gpio_info[num],0,sizeof(gpio_info[num]));
	gpio_info[num].ledno=7;
    }

   //opt type
   buf+=(len+1);
   len=strcspn(buf,",");
   if(len>sizeof(tmp))	return -1;
   memset(tmp,0,sizeof(tmp));
   strncpy(tmp,buf,len);
   gpio_info[num].opt_type=tmp[0];
   //opt duration
   buf+=(len+1);
   len=strcspn(buf,",");
   if(len>sizeof(tmp))	return -1;
   memset(tmp,0,sizeof(tmp));
   strncpy(tmp,buf,len);
   gpio_info[num].duration=atoi(tmp);

	return TRUE;
}



/*--------------------------------------------------------------------------*
@Function		: LedFlashing - flash led
@Include      	: "gpio.h"
@Description		: lenno led£¨1 or 4£©
			bgtime start time format: 2009-06-08 08:02:06
			duration :Duration £¨s£©
@Return Value	: Success TRUE, Failure FALSE.
@Create time		: 2009-08-07		1 2 3 4 7 8
*---------------------------------------------------------------------------*/
int LedFlashing(void)
{
   time_t  curtime;
   int i=0,ledno=0;
// 0-red 1-green 2-relay

   for(i=0;i<4;i++)
    {
	time(&curtime);
	ledno=gpio_info[i].ledno;
	if(ledno==0)	continue;
	switch(gpio_info[i].opt_type){
		case 'b':	//blink
		case 'B':	//blink
		case 'p':
		case 'P':
			if(gpio_info[i].enable==0)
			{
				if(abs(curtime-gpio_info[i].s_time)>=gpio_info[i].duration)
				{				
				      if(GpioOpt(ledno)==FALSE)	return FALSE;
					time(&gpio_info[i].r_time);
					gpio_info[i].enable=1;
				}
			}
			if(gpio_info[i].enable)
			{
				if(abs(curtime-gpio_info[i].r_time)>=gpio_info[i].duration)
				{
				   if(GpioOpt(ledno+1)==FALSE)	return FALSE;
				   time(&gpio_info[i].s_time);
				   gpio_info[i].enable=0;
				}
			}
			//printf("set:%c,%d,%d\n",gpio_info[i].opt_type,ledno,i);
		break;
		case 'S':
		case 's':	//turn on 
			if(gpio_info[i].enable==0)
			{
				if(GpioOpt(ledno)==FALSE)	return FALSE;
				time(&gpio_info[i].r_time);
				gpio_info[i].enable=1;
			}
			if(gpio_info[i].enable){
				if(abs(curtime-gpio_info[i].r_time)>=gpio_info[i].duration)
				{
					if(GpioOpt(ledno+1)==FALSE)	return FALSE;
					memset(&gpio_info[i],0,sizeof(gpio_info[i]));
				}
			}
			//printf("set:%c,%d\n",gpio_info[i].opt_type,ledno);
		break;
		case 'R':
		case 'r':	//turn off/reset
			ledno+=1;
			if(ledno==9)
			{
				if(GetIostate()==TRUE)	ledno=9;
				else ledno=8;
			}
			if(GpioOpt(ledno)==FALSE)	return FALSE;
			//printf("relay:%c,%d\n",gpio_info[i].opt_type,ledno);
			memset(&gpio_info[i],0,sizeof(gpio_info[i]));
		break;
		default:
		break;
	   }//switch
	}
	return TRUE;
}

/*--------------------------------------------------------------------------*
@Function		: GetIostate - get I/O status
@Include      	: "gpio.h"
@Description		: void
@Return Value	: Success TRUE, Failure FALSE.
@Create time		: 2009-08-07		
*---------------------------------------------------------------------------*/
int GetIostate(void)
{
#if defined _x86
	if(gpiofd<=0)	return FALSE;
#elif defined _2410
	int status;
	if(gpiofd<=0)	return FALSE;
	if(ioctl(gpiofd, get_DAT,&status) != 0)
   	{
	      perror("ioctl put_DAT error 8");
		return FALSE;
   	}

	if((status & 16))   
		return TRUE;
	else
		return FALSE;
#else
	int pin, value;
	char buf[2] = { 0 };
	if(gpiofd<=0)	return FALSE;
	pin = 0;

	buf[0] = pin;

	if (read(gpiofd, buf, 2) < 0) {
		//plog("gpio write error!\n");
		return FALSE;
	}
	value = buf[1];
#endif
	return TRUE;
}
