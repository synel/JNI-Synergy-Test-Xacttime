#include "set_date.h"
#include "../public/public.h"



/*--------------------------------------------------------------------------*
@Function			: MenuSetDate - set date
@Include      		: "readcard.h"
@Description			: sdate：date value format:2009-06-05
				opt : protocol
				RTC_SET - set rtc date
				RTC_NOTSET - not set rtc date
@Return Value		:
				Success TREU
				Failure FALSE
				Date format Error ERR_FORMAT		
*---------------------------------------------------------------------------*/
int MenuSetDate(IN const char *str,int opt)
{
	struct timeval tv; //时间结构
	struct timezone tz; //时区结构
	char cmdbuf[128];
	struct tm *tdate;
	time_t curdate;
	int year = 0, mon = 0, day = 0, i = 0, j = 0;

	if (!str || strlen(str) != 10) {
		return ERR_FORMAT;
	}
	sscanf((char*) str, "%04d-%02d-%02d", &year, &mon, &day); //截取参数日期
	if (year < 2000 || year > 2050 || mon < 1 || mon > 12 || day < 1
			|| day > 31) {
		return ERR_FORMAT;
	}
	time(&curdate);
	tdate = localtime(&curdate); //获取当前时间
	tdate->tm_year = year - 1900;
	tdate->tm_mon = mon - 1;
	tdate->tm_mday = day;
	gettimeofday(&tv, &tz); //获取本机的时区信息
	tv.tv_sec = mktime(tdate);
	; //设置系统时间
	i = settimeofday(&tv, &tz);
	if (opt) {
#if defined _x86
#elif defined _2410
		sprintf(cmdbuf, "setdate -w %04d.%02d.%02d-%02d:%02d:%02d", year, mon,
				day, tdate->tm_hour, tdate->tm_min, tdate->tm_sec);
		j=system(cmdbuf);
#else
		sprintf(cmdbuf,"hwclock -w");
	    j = system(cmdbuf);
#endif

	}
	if(i<0||(j<0&&opt))
	{
		return FALSE;
	}

	return TRUE;
}

/*--------------------------------------------------------------------------*
@Function			: MenuSetTime - set time
@Include      		: "readcard.h"
@Description			: sdate：date value（09:06:05）
				opt : protocol
				1 - set rtc time
				0 - not set rtc time
@Return Value		:
				Success TREU
				Failure FALSE
				Date format Error ERR_FORMAT		
*---------------------------------------------------------------------------*/
int MenuSetTime(IN const char *str,IN int opt)
{
	char cmdbuf[128];
	struct tm *tdate;
	time_t curdate;
	int hour,min,sec,i=0,j=0;
	struct timeval tv;                                //时间结构
	struct timezone tz;                               //时区结构

	if(!str||strlen(str)!=8)
	{
		return ERR_FORMAT;
	}
	sscanf((char*)str,"%02d:%02d:%02d",&hour,&min,&sec);      //截取参数日期
	if((hour>=24)||(hour<0)||(min>=60)||(min<0)||(sec>=60)||(sec<0))
	{
		return ERR_FORMAT;
	}
  
	time (&curdate);                     
	tdate = localtime (&curdate);                    //获取当前时间
	tdate->tm_hour=hour;
	tdate->tm_min=min;
	tdate->tm_sec=sec;
	gettimeofday (&tv , &tz);                                //获取本机的时区信息
	tv.tv_sec=mktime(tdate);                                    ;//设置系统时间
	i=settimeofday(&tv,&tz);
	if(opt)
	{
#if defined _x86
#elif defined _2410
		sprintf(cmdbuf, "setdate -w %04d.%02d.%02d-%02d:%02d:%02d", tdate->tm_year+1900, tdate->tm_mon+1,tdate->tm_mday, hour, min,sec);
		j=system(cmdbuf);
#else
		sprintf(cmdbuf,"hwclock -w");
	    j = system(cmdbuf);
#endif
	}

	if((i<0)||(j<0&&opt))
	{
		return FALSE;
	}

	return TRUE;
}

/*--------------------------------------------------------------------------*
@Function			: get_date - get date/time
@Include      		: "readcard.h"
@Description			: str：return date/time（2009-06-05 08:06:02）
				opt : protocol
				1 - read rtc date/time
				0 - do not read rtc date/time
@Return Value		: Success TREU, Failure FALSE.
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int GetDate(OUT char *str,IN int opt)
{
	struct tm *tm, p;
	time_t timep;

	if(opt == RTC_DISABLE)
	{	
		time(&timep);
		tm=localtime(&timep);
	  	sprintf(str,"%04d-%02d-%02d %01d %02d:%02d:%02d",
				tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,
				tm->tm_wday,tm->tm_hour, tm->tm_min, tm->tm_sec);
	}
	else if(opt == RTC_ENABLE)
	{
		if(read_rtc(&p) == FALSE)
		{
			perror("read rtc error\n");	
			return FALSE;
		}
	  	sprintf(str,"%04d-%02d-%02d %02d:%02d:%02d",
				p.tm_year+1900,p.tm_mon+1,p.tm_mday,
				 p.tm_hour, p.tm_min, p.tm_sec);
	}
	return TRUE; 
}

/*--------------------------------------------------------------------------*
@Function			: write_rtc - write rtc time
@Include      		: "readcard.h"
@Description			: ptr: pointer of struct tm
				this function is no longer in use
@Return Value		: Success TREU, Failure FALSE.
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int write_rtc(struct tm *ptr)
{
#if defined _x86
#elif defined _2410
	struct rtc_time tm;
	int rtc_fd;

	rtc_fd = open("/dev/misc/rtc",O_RDWR);
	if (rtc_fd>0)
	{
		if(ptr)
			memcpy(&tm,ptr,sizeof(tm));
		if(ioctl(rtc_fd,RTC_ALM_SET,&tm)<0)
		{
			close(rtc_fd);
			return FALSE;
		}
		return TRUE;
	}
#else
    char *command="hwclock -w";
    system(command);
    return TRUE;
#endif
	return FALSE;
}

/*--------------------------------------------------------------------------*
@Function			: read_rtc - read rtc time
@Include      		: "readcard.h"
@Description			: ptr: pointer of struct tm
@Return Value		: Success TREU, Failure FALSE.
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int  read_rtc(struct tm *ptr)
{
#if defined _x86
#elif defined _2410
	struct rtc_time tm;
	int rtc_fd;

	rtc_fd = open("/dev/misc/rtc",O_RDWR);
	if (rtc_fd>0)
	{
		ioctl(rtc_fd,RTC_RD_TIME,&tm);
		close(rtc_fd);
		if(ptr)
		{
			memcpy(ptr,&tm,sizeof(tm));
		}
		return TRUE;
	}
#else
	FILE *stream;
	char *command = "hwclock -r";

	char buf[100];
	char year[5]={0};
	char mon[4]={0};
	char day[3]={0};
	char hour[3]={0};
	char minute[3]={0};
	char second[3]={0};
	char week[2]={0};
	char week_str[5];
	char time_str[20];

	stream = popen(command, "r");

	if (stream == NULL ) {
		//        plog("error!\n");
		return FALSE;
	}

	while (fgets(buf, 100, stream)) {
		break;
	}

	sscanf(buf, "%3s %3s %2s %8s %4s", week_str, mon, day, time_str, year);

	if (strncmp(week_str, "Sun", 3) == 0) {
			memcpy(week, "0", 1);
		} else if (strncmp(week_str, "Mon", 3) == 0) {
			memcpy(week, "1", 1);
		} else if (strncmp(week_str, "Tue", 3) == 0) {
			memcpy(week, "2", 1);
		} else if (strncmp(week_str, "Wed", 3) == 0) {
			memcpy(week, "3", 1);
		} else if (strncmp(week_str, "Thu", 3) == 0) {
			memcpy(week, "4", 1);
		} else if (strncmp(week_str, "Fri", 3) == 0) {
			memcpy(week, "5", 1);
		} else if (strncmp(week_str, "Sat", 3) == 0) {
			memcpy(week, "6", 1);
		}

	if (strncmp(mon, "Jan", 3) == 0) {
		memset(mon, 0, sizeof(mon));
		memcpy(mon, "01", 2);
	} else if (strncmp(mon, "Feb", 3) == 0) {
		memset(mon, 0, sizeof(mon));
		memcpy(mon, "02", 2);
	} else if (strncmp(mon, "Mar", 3) == 0) {
		memset(mon, 0, sizeof(mon));
		memcpy(mon, "03", 2);
	} else if (strncmp(mon, "Apr", 3) == 0) {
		memset(mon, 0, sizeof(mon));
		memcpy(mon, "04", 2);
	} else if (strncmp(mon, "May", 3) == 0) {
		memset(mon, 0, sizeof(mon));
		memcpy(mon, "05", 2);
	} else if (strncmp(mon, "Jun", 3) == 0) {
		memset(mon, 0, sizeof(mon));
		memcpy(mon, "06", 2);
	} else if (strncmp(mon, "Jul", 3) == 0) {
		memset(mon, 0, sizeof(mon));
		memcpy(mon, "07", 2);
	} else if (strncmp(mon, "Aug", 3) == 0) {
		memset(mon, 0, sizeof(mon));
		memcpy(mon, "08", 2);
	} else if (strncmp(mon, "Sep", 3) == 0) {
		memset(mon, 0, sizeof(mon));
		memcpy(mon, "09", 2);
	} else if (strncmp(mon, "Oct", 3) == 0) {
		memset(mon, 0, sizeof(mon));
		memcpy(mon, "10", 2);
	} else if (strncmp(mon, "Nov", 3) == 0) {
		memset(mon, 0, sizeof(mon));
		memcpy(mon, "11", 2);
	} else if (strncmp(mon, "Dec", 3) == 0) {
		memset(mon, 0, sizeof(mon));
		memcpy(mon, "12", 2);
	}

	sscanf(time_str,"%2s:%2s:%2s",hour,minute,second);
	printf("hour : %s\n",hour);
	printf("minute : %s\n",minute);
	printf("second : %s\n",second);

	ptr->tm_year = atoi(year) - 1900;
	ptr->tm_mon = atoi(mon) - 1; //0..11
	ptr->tm_mday = atoi(day); //1..31
    ptr->tm_hour = atoi(hour); //0..23
    ptr->tm_min = atoi(minute); //0..59
    ptr->tm_sec = atoi(second); //0..59
    ptr->tm_wday = atoi(week); //0..6

	pclose(stream);
	return TRUE;
#endif
	return FALSE;
}

/*--------------------------------------------------------------------------*
@Function			:syncclock - The clock efficacy
@Include      		: "readcard.h"
@Description			: clock_tz: calibration value
@Return Value		: Success TREU, Failure FALSE.
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int Sysclock(IN int clock_tz)
{
        int fd;
        //open device file
#if defined _x86
#elif defined _2410
        fd = open("/dev/CLK_ADJUST",O_RDWR);
        if(fd < 0)
        {
                perror("device open fail\n");
                return FALSE;
        }

         if(ioctl(fd,WRITESJ,&clock_tz)!=0)
          {
           perror("writesj error\n");
		return FALSE;
          }
	close(fd);
#else
    int x,y = 128;

	if(access("/dev/rtc0",F_OK) == -1 || clock_tz == 0)
	{
	   return FALSE;
	}

    fd = open("/dev/rtc0",O_RDWR);
    if(fd < 0)
    {
            perror("device open fail\n");
            return FALSE;
    }

    x = clock_tz;

      if (x < 0)
      {
          if (x <= -17)
              y = 63;
          else
              y = round(abs(x + 0.26) * 61 / 16.08) + 2;
      }
      else if (x > 0)
      {
          if (x >= 17)
              y = 64;
          else
              y = round ((16.87 - x) * 63 / 16.61) + 64;
      }
      else
          y = 128;;    //#x==0

      y=(y<<4)|(y>>4);
      y=((y<<2)&0xcc)|((y>>2)&0x33);
      y=((y<<1)&0xaa)|((y>>1)&0x55);

      if(ioctl(fd,SETADJTIME,&y)!=0)
      {
          perror("writesj error\n");
          close(fd);
          return FALSE;
      }
      close(fd);
#endif
        return TRUE;
}

//int main()
//{
//	set_date(NULL, "08:02:03",1);
//}
