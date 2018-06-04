/**
 * @chinese
 * @file   public.c
 * @author 刘训
 * @date   Wed Jul 13 09:55:52 2011
 *
 * @brief 通用模块接口
 * @endchinese
 *
 *
 * @english
 * @file   public.c
 * @author Liu Xun
 * @date   Wed Jul 13 09:55:52 2011
 *
 * @brief general module
 * @endenglish
 *
 *
 */
#include    <stdio.h>
#include    <sys/types.h>
#include    <signal.h>
#include    <sys/param.h>
#include    <sys/utsname.h>
#include    <sys/ioctl.h>
#include    <sys/socket.h>
#include    <net/if.h>
#include    <netinet/in.h>
#include    <net/if_arp.h>
#include    <arpa/inet.h>
#include    <unistd.h>
#include    "_precomp.h"
#include    "public.h"
#include    "stringchar.h"
#include    "inifile.h"
#include    "config.h"
#include    "ipconfig.h"
#include 	"debug.h"
#include "cardinfo.h"

char kstr[500][45];
#ifndef _ARM_A23
//卡处理规则文件
char *cardRuleFileName="/weds/kq42/cardrule.wds";
char tmp_file[128]="/tmp/update/~~.tmp",send_file_path[128]; /**文件传输时候使用的临时文件*/
#else
//卡处理规则文件
char *cardRuleFileName="/mnt/obb/weds/kq42/cardrule.wds";
char tmp_file[128]="/mnt/obb/tmp/update/~~.tmp",send_file_path[128]; /**文件传输时候使用的临时文件*/
#endif
//int user_data_len=992;
int communication_type = NET_SERVER,communication_over_time = 30,connect_success_flag=0;

/**
 *
 *
 * @param format
 * @param starter
 *
 * @return
 */
int  GetCount(char *format,char starter)
{
    char *P;
    int count=0;

    if(format == NULL)
        return ERROR;

    P = format;
    while( *format == starter)
    {
        format++;
    }

    count = format - P + 1;
    return count;
}


/**
 *读取终端机编号,取IP地址的后六位
 *
 * @param param 终端机编号
 *
 * @return 成功-TRUE,失败-FALSE
 */
int read_terminal_number(char *param)
{
    char *p1,*p2;
    int bh1=0,bh2=0,retval=0;
    char ipaddr[32],*p;

    memset(ipaddr,0,sizeof(ipaddr));
    retval = get_ip("eth0",ipaddr);
    if(retval == ERROR)
    {
        return FALSE;
    }

    p = strtok(ipaddr,".");
    if(p == NULL)
    {
        return FALSE;
    }
    p = strtok(NULL,".");
    if(p == NULL)
    {
        return FALSE;
    }
    p1=strtok(NULL,".");
    if(p1 == NULL)
    {
        return FALSE;
    }
    p2=strtok(NULL,".");
    if(p2 == NULL)
    {
        return FALSE;
    }

    if(p1)
    {
        bh1=atoi(p1);
    }
    if(p2)
    {
        bh2=atoi(p2);
    }
    memset(param,0,sizeof(param));
    sprintf(param,"%03d%03d",bh1,bh2);
    return TRUE;
}


/**
 * 读取终端机MAC地址信息
 *
 *
 * @return 成功-MAC,失败-空
 */
char * read_mac_config()
{
	/*
    FILE *file=NULL;
    static char mac[18];
    char buff[64],*ptr=NULL;

    file=popen("ifconfig","r");
    memset(mac,0,sizeof(mac));
    if(!file)
        return mac;
    while(fgets(buff,sizeof(buff),file))
    {
        ptr=strstr(buff,"HWaddr");
        if(ptr)
        {
            strncpy(mac,ptr+7,sizeof(mac)-1);
            break;
        }
    }
    pclose(file);
    cut_rn(mac);
    return mac;
    */
	FILE *file=NULL;
	char buff[64],*ptr=NULL,*ptr_mac=NULL;
	static char mac[18];
	int i = 0;
#ifndef _ARM_A23
	file=fopen("/weds/mac.sh","r");
#else
        file=fopen("/mnt/sdcard/weds/mac.sh","r");
#endif

	if(!file)
	{
		return NULL;
	}

	memset(buff,0,sizeof(buff));
	fgets(buff,sizeof(buff),file);
	memset(buff,0,sizeof(buff));
	fgets(buff,sizeof(buff),file);
	ptr=strstr(buff,"ether");
	if(ptr)
	{
		memset(mac,0,sizeof(mac));
		strncpy(mac,ptr+6,17);
		cut_rn(mac);
	}
	fclose(file);
	ptr_mac = (char*)mac;
	while(*ptr_mac)
	{
		if (*ptr_mac == ':')
		{
			i++;
		}

		ptr_mac++;
	}
	if((strlen(mac) != 17)||(i!=5))
		return NULL;
	return mac;
}

/**
 *可打印字符串转换为字节数据
 *如："C8329BFD0E01" --> {0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01}
 * @param pSrc 源字符串指针
 * @param pDst 目标数据指针
 * @param nSrcLength 源字符串长度
 *
 * @return 目标数据长度
 */
int gsmString2Bytes(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
    int i=0;
    for ( i = 0; i < nSrcLength; i += 2)
    {
        // 输出高4位
        if ((*pSrc >= '0') && (*pSrc <= '9'))
        {
            *pDst = (*pSrc - '0') << 4;
        }
        else
        {
            *pDst = (*pSrc - 'A' + 10) << 4;
        }
        pSrc++;
        // 输出低4位
        if ((*pSrc>='0') && (*pSrc<='9'))
        {
            *pDst |= *pSrc - '0';
        }
        else
        {
            *pDst |= *pSrc - 'A' + 10;
        }
        pSrc++;
        pDst++;
    }

    return (nSrcLength / 2);
}

/**
 * @chinese
 * 字节数据转换为可打印字符串
 *如：{0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01} --> "C8329BFD0E01"
 * @param pSrc 源数据指针
 * @param pDst  目标字符串指针
 * @param nSrcLength 源数据长度
 *
 * @return 目标字符串长度
 * @endchinese
 *
 * @english
 * change byte data string to printable string.
 * for example：{0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01} --> "C8329BFD0E01"
 * @param pSrc pointer which point to source string
 * @param pDst  pointer which point to  resulting string.
 * @param nSrcLength the length of source data
 *
 * @return length of the resulting string

 * @endenglish
 *
 */
int gsmBytes2String(unsigned char* pSrc, char* pDst, int nSrcLength)
{
    const char tab[]="0123456789ABCDEF";	// 0x0-0xf的字符查找表
    int i=0;

    for (i = 0; i < nSrcLength; i++)
    {
        *pDst++ = tab[*pSrc >> 4];	// 输出高4位
        *pDst++ = tab[*pSrc & 0x0f];	// 输出低4位
        pSrc++;
    }
    *pDst = '\0';

    return (nSrcLength * 2);
}

/**
 *
 *格式化日期时间,将日期/时间按照format参数格式化成相应字符串由result返回
 * @param format 日期时间格式
 * @param result 日期时间格式化字符串
 * @param the_time 当前时间
 *
 * @return Success TRUE .Failure FALSE
 */
int FormatDateTime(const char * format,char *result,time_t the_time)
{
    //time_t the_time;
    struct tm * tm_time;
    char LastToken,Starter,Token;
    int count=0,Digits=0;
    int year=2009, month=11, day=19, hour=11, min=21, sec=10;

    if(format==NULL)	return FALSE;
    //time(&the_time);
    tm_time=localtime(&the_time);

    year=tm_time->tm_year+1900;
    month=tm_time->tm_mon+1;
    day=tm_time->tm_mday;
    hour=tm_time->tm_hour;
    min=tm_time->tm_min;
    sec=tm_time->tm_sec;

    LastToken = ' ';
    while (isascii(*format)&&strlen(format))
    {

        Starter = *format;
        format = format+1;
        Token = Starter;
        if (Token>'a'&& Token<'z') Token=Token - 32;
        if (Token>'A'&& Token<'Z')
        {
            if (Token == 'M' && LastToken == 'H') Token = 'N';
            LastToken = Token;
        }
        count=GetCount((char*)format,Starter);
        format+=(count-1);
        result+=Digits;
        switch(Token){
        case 'Y':
            if(count<=2)
            {
                sprintf(result,"%02d",year%100);
            }
            else
            {
                sprintf(result,"%04d",year);
            }
            break;
                case 'M':
            switch(count){
            case 1:
            case 2:
                sprintf(result,"%02d",month);
                break;
            case 3:
                //AppendString(ShortMonthNames[Month]);
                break;
            default:
                //AppendString(LongMonthNames[Month]);
                break;
            }
            break;
			case 'D':
            switch(count){
            case 1:
            case 2:
                sprintf(result,"%02d",day);
                break;
            case 3:
                //AppendString(ShortMonthNames[Month]);
                break;
            default:
                //AppendString(LongMonthNames[Month]);
                break;
            }
            break;
			case 'H':
            switch(count){
            case 1:
            case 2:
                sprintf(result,"%02d",hour);
                break;
            case 3:
                //AppendString(ShortMonthNames[Month]);
                break;
            default:
                //AppendString(LongMonthNames[Month]);
                break;
            }
            break;
			case 'N':
            sprintf(result,"%02d",min);
            break;
			case 'S':
            sprintf(result,"%02d",sec);
            break;
			case '-':
			case '/':
			case ':':
			case ' ':
            while(count--)
            {
                sprintf(result,"%c",Token);
                Digits=strlen(result);
                result+=Digits;
            }
            break;
			default:
            while(count--)
            {
                sprintf(result,"%c",Token);
                Digits=strlen(result);
                result+=Digits;
            }
            break;
        }
        Digits=strlen(result);
    }//while
    return TRUE;
}


/**
 *
 *获得Linux内核信息
 *
 * @return 成功-Linux内核信息,失败-空
 */
char *getkernelinfo()
{
    //	FILE *	fp;
    static char version[128];
    //
    //	memset(version,0,sizeof(version));
    //	fp=fopen("/proc/version","r");
    //	if(!fp)	return version;
    //	if(fgets(version,sizeof(version),fp)==NULL)
    //	{
    //	    fclose(fp);
    //	    return version;
    //	}
    //	cut(version);
    //	fclose(fp);
    struct utsname name;

    uname(&name);
    memset(version,0,sizeof(version));
    strcpy(version,name.release);
    //     if(uname(&name) < 0)
    //     {
    //         plog("ERROR\n");
    //         return version;
    //     }else
    //     {
    //         plog("sysname=\t%s\n",name.sysname);
    //         plog("nodename=\t%s\n",name.nodename);
    //         plog("release=\t%s\n",name.release);
    //         plog("version=\t%s\n",name.version);
    //         plog("machine=\t%s\n",name.machine);
    //     }
    return version;
}


/**
 *获得终端机CPU信息
 *
 *
 * @return 成功-CPU信息,失败-空
 */
char *getcpuinfo()
{
    FILE *	fp;
    char buf[256],*p;
    static char cpuinfo[128];

    memset(cpuinfo,0,sizeof(cpuinfo));
    memset(buf,0,sizeof(buf));
    fp=fopen("/proc/cpuinfo","r");
    if(!fp)	return cpuinfo;
    if(fgets(buf,sizeof(buf),fp)==NULL)
    {
        fclose(fp);
        return cpuinfo;
    }
    cut_rn(buf);
    p=strstr(buf,":");
    p+=1;
    strncpy(cpuinfo,p,26);
    fclose(fp);
    return cpuinfo;
}


/**
 *获得终端机内存大小信息
 *
 *
 * @return 成功-终端机内存大小,失败-空
 */
char *getmeminfo()
{
    FILE *	fp;
    char buf[256],*p;
    static char meminfo[128];

    memset(meminfo,0,sizeof(meminfo));
    memset(buf,0,sizeof(buf));
    fp=fopen("/proc/meminfo","r");
    if(!fp)	return meminfo;
    //	if(fgets(buf,sizeof(buf),fp)==NULL)
    //    {
    //        fclose(fp);
    //        return meminfo;
    //    }
    //    memset(buf,0,sizeof(buf));
    if(fgets(buf,sizeof(buf),fp)==NULL)
    {
        fclose(fp);
        return meminfo;
    }
    cut_rn(buf);
    p = strtok(buf," ");
    p = strtok(NULL," ");
    sprintf(meminfo,"%dM",atoi(p)/1000);
    fclose(fp);
    return meminfo;
}
//获取进程id号
int pid_get(char *current_process)
{
    int pid_id = -1;
    char save_id[8];
    FILE *fp;    
    char command_process[255] = {0};
#ifndef _ARM_A23
#ifdef _A20_UBUNTU

    strcat(command_process, "ps -ef| grep ");
    strcat(command_process, current_process);
    strcat(command_process, " | grep -v grep | awk '{print $2}'");
#else

    strcat(command_process, "ps | grep ");
    strcat(command_process, current_process);
    strcat(command_process, " | grep -v grep | awk '{print $1}'");
#endif
#else
    return get_pid_by_name(current_process,NULL);
    strcat(command_process, "su -c \"busybox ps\" | su -c \"busybox grep\" ");
    strcat(command_process, current_process);
    strcat(command_process, " | su -c \"busybox grep -v grep\" | su -c \"busybox awk '{print $1}'\"");
#endif
//    printf("command is '%s'\n",command_process);

    fp = popen(command_process, "r");
    if(fp == NULL)
    {
        return pid_id;
    }
    fgets(save_id, sizeof(save_id), fp);

    pclose(fp);
    pid_id = atoi(save_id);
    return pid_id;
}
//杀死进程
int killp(int pid)
{
//    char buf[256];
    int retval = 0;

    if(pid<=0)
    {
        return ERROR;
    }
    retval = kill(pid,SIGKILL);
    if(retval == 0)
    {
        return 1;
    }
    return ERROR;
#if 0
    memset(buf,0,sizeof(buf));
    sprintf(buf,"kill -SIGINT %d",pid);
    if(system(buf)<0)
    {
        plog("kill pid error!");
        return -1;
    }
    return 1;
#endif
}



void printftime( char *fun )
{
        static struct timeval	oldtime;
        struct timeval			newtime;
        gettimeofday( &newtime, NULL );

        if( fun )
        {
                printf( " %s--- %ld\n", fun, ( newtime.tv_sec - oldtime.tv_sec ) * 1000000 + newtime.tv_usec - oldtime.tv_usec );
                memcpy( &oldtime, &newtime, sizeof( oldtime ) );
        }else
        {
                memcpy( &oldtime, &newtime, sizeof( oldtime ) );
        }
}

unsigned short int crc16Table[256] = {
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
	0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
	0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
	0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
	0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
	0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
	0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
	0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
	0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
	0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
	0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
	0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
	0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
	0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
	0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
	0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
	0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
	0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
	0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
	0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
	0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
	0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
	0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
	0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0 };
/*
 *   ----------CRC-16/XMODEM
 *
 * in--需要进行crc运算的数组指针
 * len--进行crc运算的数据指针数组长度
 * append--取0表示算出的crc只占一个字节；取1表示算出的crc占俩字节
 * crc--crc初始值0x0818
*/
unsigned short int crc16_check(const void *in, unsigned int len, int append, unsigned short int crc)
{
	unsigned int i = 0;
	unsigned short int CRC=0;
	unsigned char * para = (unsigned char *)in;

	CRC=crc;
	for (i = 0; i < len; i++)
  	{
    	CRC = (CRC << 8) ^ crc16Table[(CRC >> 8) ^ *para++];
	}

	if (append)
	{
		*para++ = (CRC >> 8) & 0x00FF;
		*para = CRC & 0x00FF;
	}

  	return CRC;
}
// SHA1 算法

void creat_w(unsigned char input[64],unsigned long w[80])
{
   int i,j;unsigned long temp,temp1;
   for(i=0;i<16;i++)
   {
		j=4*i;
		w[i]=((long)input[j])<<24 |((long)input[1+j])<<16|((long)input[2+j])<<8|((long)input[3+j])<<0;

   }
   for(i=16;i<80;i++)
   {
		w[i]=w[i-16]^w[i-14]^w[i-8]^w[i-3];
		temp=w[i]<<1;
		temp1=w[i]>>31;
		w[i]=temp|temp1;

   }
}

char ms_len(long a,char intput[64])
{
	unsigned long temp3,p1;  int i,j;
	temp3=0;
	p1=~(~temp3<<8);
	for(i=0;i<4;i++)
	{
		j=8*i;
		intput[63-i]=(char)((a&(p1<<j))>>j);
	}
	return '0';
}

int SHA1(char *iData, char *oData)
{
	unsigned long H0=0x67452301,H1=0xefcdab89,H2=0x98badcfe,H3=0x10325476,H4=0xc3d2e1f0;
	unsigned long A,B,C,D,E,temp,temp1,temp2,temp3,k,f;int i,flag;unsigned long w[80];
	unsigned char input[64]; long x;int n;

	if (iData == NULL || oData == NULL)
	{
		return -1;
	}

//	printf("input message:\n");
//	scanf("%s",input);
	sprintf((char *)input, "%s", iData);
	n=strlen((char *)input);
	if(n<57)
	{
		 x=n*8;
		 ms_len(x,(char*)input);
		 if(n==56)
			 for(i=n;i<60;i++)
			 input[i]=0;
		 else
		{
		 input[n]=128;
		 for(i=n+1;i<60;i++)
		 input[i]=0;
		}
	}

	creat_w(input,w);
//	printf("\n");
	A=H0;B=H1;C=H2;D=H3;E=H4;
	for(i=0;i<80;i++)
	{
	   flag=i/20;
	   switch(flag)
		{
			case 0: k=0x5a827999;f=(B&C)|(~B&D);break;
			case 1: k=0x6ed9eba1;f=B^C^D;break;
			case 2: k=0x8f1bbcdc;f=(B&C)|(B&D)|(C&D);break;
			case 3: k=0xca62c1d6;f=B^C^D;break;
		}
	   /*printf("%lx,%lx\n",k,f); */
	   temp1=A<<5;
	   temp2=A>>27;
	   temp3=temp1|temp2;
	   temp=temp3+f+E+w[i]+k;
	   E=D;
	   D=C;

	   temp1=B<<30;
	   temp2=B>>2;
	   C=temp1|temp2;
	   B=A;
	   A=temp;
	   //printf("%lx,%lx,%lx,%lx,%lx\n",A,B,C,D,E);//输出编码过程
	}
	H0=H0+A;
	H1=H1+B;
	H2=H2+C;
	H3=H3+D;
	H4=H4+E;

//	printf("\noutput hash value:\n");

//	printf("%08lx%08lx%08lx%08lx%08lx",H0,H1,H2,H3,H4);

	sprintf(oData, "%08lx%08lx%08lx%08lx%08lx",H0,H1,H2,H3,H4);

	return 1;

}

//void init_daemon()
//{
//    pid_t pid;
//    int i;
//    if((pid=fork())==-1)
//        exit(1);
//    if(pid>0)
//        exit(0);
//    setsid();
//    if((pid=fork())==-1)
//        exit(1);
//    if(pid>0)
//        exit(0);
//    for(i=0;i<NOFILE;++i)
//        close(i);
//    umask(0);
//    return;
//}



void sync_fclose(FILE *file)
{
//fdatasync(fileno(file));
fclose(file);
//#ifdef _AM335X
//sync();
//#endif
}

/*
 */
int sdk_sync()
{
    sync();
    return 0;
}

// 20160122 Lee Add


//char llog[1024];


//void _printf(char *format, ...)
//{
//    return;
//}


//// in -- 要打印的数据
//// iLen -- in 的长度, 最大支持1024
//// isPad -- 字节间的填充' '字符,>= 1 填充空格
//// SorR -- 发送接收标志0-正常模式，'>'-发送数据 '<'-接收数据
//// Msg -- 提示信息

//void dbgShowHexData( unsigned char *in, unsigned int iLen, unsigned char isPad, unsigned char SorR, char *Msg)
//{
//        unsigned int i = 0;

//        if((int)iLen <0) // 防止溢出
//        {
//                return;
//        }

//        iLen = iLen > 1024 ? 1024: iLen;
//        if(Msg != NULL)
//        {
//                PRINTF( "\n%s(len = %d)", Msg, iLen);
//        }

//        if( SorR == '>' )
//        {
//                PRINTF( "\n-->" );
////		PRINTF( "\nSEND: " );
//        }
//        else if(SorR == '<')
//        {
//                PRINTF( "\n<--" );
////		PRINTF( "\nRECV: " );

//        }else
//        {
//                PRINTF( "\n" );
////		PRINTF( "\nDATA: " );
//        }

//        if (isPad > 0)
//        {
//                for( i = 0; i < iLen; i++ )
//                {
//                        PRINTF( "%02X ", in[i]);
//                }
//        }
//        else
//        {
//                for( i = 0; i < iLen; i++ )
//                {
//                        PRINTF( "%02X", in[i]);
//                }
//        }

//        PRINTF( "\n" );

//        return;
//}



//void chkdbgLog()
//{
//        char		mBuffer[20];
//        int 		Ret, size;
//        struct tm* pModifytime = NULL;	//=localtime(&(ptr->st_mtime));  文件内容最后被修改时间
//        struct stat st;
//        unsigned int Max = 1024 * 1024;

//    Ret = stat("./dbgLog.txt", &st);
//        if (Ret>= 0)
//        {
//                memset(mBuffer, 0, sizeof(mBuffer));
//                pModifytime = localtime(&(st.st_mtime));
//                strftime(mBuffer, 20, "%Y-%m-%d %H:%M:%S", pModifytime);
//                size = st.st_size;

//                if (size > Max) // 文件大小超过 1M
//                {
//                        remove("./dbgLog.txt");
//                        sprintf(llog, "[%s:%s:%d] [Del dbgLog.txt, and Creat New...]",__FILE__, __func__, __LINE__);
//                        dbgLog(llog, NULL, 0, 0);
//                }
//        }

//        return;

//}


//void dbgLog( char *iStrData, unsigned char *iHexData, unsigned int dataLen, char flag )
//{
//        FILE		*fp = NULL;
//        time_t		sect;
//        struct tm	*tt = NULL;
//        char		sTime[24];
//        char		oBuf[256];
//        sect   = time( NULL );
//        tt	   = localtime( &sect );
//        memset( sTime, 0, sizeof( sTime ) );
//        memset( oBuf, 0, sizeof( oBuf ) );

//        strftime( sTime, 22, "%m-%d %H:%M:%S", tt );

//        fp = fopen( "./dbgLog.txt", "a+" );
//        if( fp == NULL )
//        {
//                return;
//        }

//        if( iStrData != NULL )
//        {
//                if( flag == 's' )
//                {
//                        fprintf( fp, "[%s] -->%s\n", sTime, iStrData );
//                }else if( flag == 'r' )
//                {
//                        fprintf( fp, "[%s] <--%s\n", sTime, iStrData );
//                }else
//                {
//                        fprintf( fp, "[%s] %s\n", sTime, iStrData );
//                }
//        }

//        if( iHexData != NULL )
//        {
////		CPublic::HexToTHex( iHexData, oBuf, dataLen, ' ' );

//                if( flag == 's' )
//                {
//                        fprintf( fp, "[%s] -->%s\n", sTime, oBuf );
//                }else if( flag == 'r' )
//                {
//                        fprintf( fp, "[%s] <--%s\n", sTime, oBuf );
//                }else
//                {
//                        fprintf( fp, "[%s] %s\n", sTime, oBuf );
//                }
//        }

//        fclose( fp );

//        return;
//}

static const unsigned char	crc8Table[] = {
        0,	 94,  188, 226, 97,  63,  221, 131, 194, 156, 126, 32,	163, 253, 31,  65,
        157, 195, 33,  127, 252, 162, 64,  30,	95,  1,   227, 189, 62,  96,  130, 220,
        35,  125, 159, 193, 66,  28,  254, 160, 225, 191, 93,  3,	128, 222, 60,  98,
        190, 224, 2,   92,	223, 129, 99,  61,	124, 34,  192, 158, 29,  67,  161, 255,
        70,  24,  250, 164, 39,  121, 155, 197, 132, 218, 56,  102, 229, 187, 89,  7,
        219, 133, 103, 57,	186, 228, 6,   88,	25,  71,  165, 251, 120, 38,  196, 154,
        101, 59,  217, 135, 4,	 90,  184, 230, 167, 249, 27,  69,	198, 152, 122, 36,
        248, 166, 68,  26,	153, 199, 37,  123, 58,  100, 134, 216, 91,  5,   231, 185,
        140, 210, 48,  110, 237, 179, 81,  15,	78,  16,  242, 172, 47,  113, 147, 205,
        17,  79,  173, 243, 112, 46,  204, 146, 211, 141, 111, 49,	178, 236, 14,  80,
        175, 241, 19,  77,	206, 144, 114, 44,	109, 51,  209, 143, 12,  82,  176, 238,
        50,  108, 142, 208, 83,  13,  239, 177, 240, 174, 76,  18,	145, 207, 45,  115,
        202, 148, 118, 40,	171, 245, 23,  73,	8,	 86,  180, 234, 105, 55,  213, 139,
        87,  9,   235, 181, 54,  104, 138, 212, 149, 203, 41,  119, 244, 170, 72,  22,
        233, 183, 85,  11,	136, 214, 52,  106, 43,  117, 151, 201, 74,  20,  246, 168,
        116, 42,  200, 150, 21,  75,  169, 247, 182, 232, 10,  84,	215, 137, 107, 53
};


//static unsigned short int crc16Table[256] = {
//        0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
//        0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
//        0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
//        0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
//        0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
//        0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
//        0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
//        0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
//        0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
//        0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
//        0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
//        0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
//        0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
//        0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
//        0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
//        0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
//        0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
//        0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
//        0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
//        0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
//        0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
//        0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
//        0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
//        0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
//        0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
//        0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
//        0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
//        0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
//        0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
//        0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
//        0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
//        0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
//};


//static unsigned int crc32Table[256] = {
//        0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L, 0x706af48fL, 0xe963a535L, 0x9e6495a3L,
//        0x0edb8832L, 0x79dcb8a4L, 0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L, 0x90bf1d91L,
//        0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL, 0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L,
//        0x136c9856L, 0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L, 0xfa0f3d63L, 0x8d080df5L,
//        0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L, 0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
//        0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L, 0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L,
//        0x26d930acL, 0x51de003aL, 0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L, 0xb8bda50fL,
//        0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L, 0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL,
//        0x76dc4190L, 0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL, 0x9fbfe4a5L, 0xe8b8d433L,
//        0x7807c9a2L, 0x0f00f934L, 0x9609a88eL, 0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
//        0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL, 0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L,
//        0x65b0d9c6L, 0x12b7e950L, 0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L, 0xfbd44c65L,
//        0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L, 0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL,
//        0x4369e96aL, 0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L, 0xaa0a4c5fL, 0xdd0d7cc9L,
//        0x5005713cL, 0x270241aaL, 0xbe0b1010L, 0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
//        0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L, 0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL,
//        0xedb88320L, 0x9abfb3b6L, 0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L, 0x73dc1683L,
//        0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L, 0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L,
//        0xf00f9344L, 0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL, 0x196c3671L, 0x6e6b06e7L,
//        0xfed41b76L, 0x89d32be0L, 0x10da7a5aL, 0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
//        0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L, 0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL,
//        0xd80d2bdaL, 0xaf0a1b4cL, 0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL, 0x4669be79L,
//        0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L, 0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL,
//        0xc5ba3bbeL, 0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L, 0x2cd99e8bL, 0x5bdeae1dL,
//        0x9b64c2b0L, 0xec63f226L, 0x756aa39cL, 0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
//        0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL, 0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L,
//        0x86d3d2d4L, 0xf1d4e242L, 0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L, 0x18b74777L,
//        0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL, 0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L,
//        0xa00ae278L, 0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L, 0x4969474dL, 0x3e6e77dbL,
//        0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L, 0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
//        0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L, 0xcdd70693L, 0x54de5729L, 0x23d967bfL,
//        0xb3667a2eL, 0xc4614ab8L, 0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL, 0x2d02ef8dL
//};



//int CRC8_CHK( unsigned char *srcData, unsigned int Len, unsigned int addFlag )
//{
//        unsigned int	i	   = 0;
//        unsigned char	crc8   = 0;
//        unsigned char	p;

//        if( srcData == NULL )
//        {
//                return 0;
//        }

//        for( i = 0; i < Len; i++ )
//        {
//                p	   = srcData[i];
//                crc8   = crc8Table[crc8 ^ p];
//        }

//        // 生成crc8校验,追加到末尾
//        if( addFlag )
//        {
//                srcData[Len] = crc8;
//                return 0;
//        }

//        // 校验crc8
//        if( crc8 == srcData[Len] )
//        {
//                //printf("crc8 ok... %02X \n", crc8);
//                return 0;
//        }else
//        {
//                //printf("crc8 err... %02X \n", crc8);
//                return -1;
//        }

//        //return crc8;
//}


int CalcCRC8( unsigned char *buf, int flag )
{
        unsigned int		i	   = 0;
        unsigned char		crc8   = 0X35; //20110805 初始值调整为0X35 ,原为0
        unsigned char		p;

        if( buf == NULL )
        {
                return ERR_PARAM;
        }

        for( i = 0; i < 15; i++ )
        {
                p	   = buf[i];
                crc8   = crc8Table[crc8 ^ p];
        }

        // 生成crc8校验,追加到末尾
        if( flag )
        {
                buf[15] = crc8;
                return ERR_OK;
        }

        // 校验crc8
        if( crc8 == buf[15] )
        {
                return ERR_OK;
        }else
        {
                return ERR_FALSE;
        }

        return 0;
}

/*
 *
 *----------CRC-16/XMODEM
 */
unsigned short int CalcCRC16( unsigned char *in, unsigned int len, int append )
{
        unsigned int		i;
        unsigned short int	CRC				   = 0x0818; // 初始值
        unsigned char		* para = (unsigned char *)in;

        for( i = 0; i < len; i++ )
        {
                CRC = ( CRC << 8 ) ^ crc16Table[( CRC >> 8 ) ^ *para++];
        }

        if( append )
        {
                *para++	   = ( CRC >> 8 ) & 0x00FF;
                *para	   = CRC & 0x00FF;
        }

        return CRC;
}



//////////////////////////////////////////////////////////////////////////
// CRC16 MODBUS 效验
// 输入参数: pDataIn: 数据地址
//           iLenIn: 数据长度
// 输出参数: pCRCOut: 2字节校验值


unsigned short CheckCRCModBus(unsigned char * pDataIn, int iLenIn)
{
    unsigned short wHi = 0;
   unsigned  short wLo = 0;
   unsigned  short wCRC;
   unsigned short wCheck=0;
    int i,j=0;
    wCRC = 0xFFFF;

    for ( i = 0; i < iLenIn; i++)
    {
        wCRC=*pDataIn^wCRC;
        for(j=0;j<8;j++)
        {

            wCheck=wCRC&0x0001;
            wCRC=wCRC>>1;
            if(wCheck)
                wCRC=wCRC^0xa001;
        }
        pDataIn++;
    }

    wHi = wCRC / 256;
    wLo = wCRC % 256;
    wCRC = (wHi << 8) | wLo;

    //*pCRCOut = wCRC;
    return wCRC;
}




//int chkOverTimeEvent(int Flag, unsigned long long *lstMicroSec, unsigned int *lstSec, unsigned int offTime)
//{
//        unsigned long long curMicroSec = 0;
//        unsigned long curSec = 0;

//        switch (Flag)
//        {
//                case 0:	// 精度为 s
//                        curSec = time(NULL);
//                        if (curSec < *lstSec) // 说明已经对钟(时间调回过去), 看做超时
//                        {
//                                *lstSec = curSec;
//                                return 0;
//                        }

////			if (curSec > *lstSec + offTime)	// 超时
//                        if (curSec >= *lstSec + offTime)	// must =, 否则误差在 1 s
//                        {
//                                return 1;
//                        }

//                        break;

//                case 1:	// 精度为 ms
//                        curMicroSec = GetTickCount();
//                        if (curMicroSec < *lstMicroSec) // 说明已经对钟(时间调回过去), 看做超时
//                        {
//                                *lstMicroSec = curMicroSec;
//                                return 0;
//                        }

//                        if (curMicroSec > *lstMicroSec + offTime)	// 超时
//                        {
//                                return 1;
//                        }

//                        break;

//                default:
//                        break;

//        }

//        return 0;

//}


/*
 hash 函数
 */
unsigned long FNVHash1(char * data,int len)
{
    int i=0;
    unsigned long p = 16777619;
    unsigned long hash = (unsigned long)2166136261L;
    for(i=0;i<len;i++)
        hash = (hash ^ data[i]) * p;
    hash += hash << 13;
    hash ^= hash >> 7;
    hash += hash << 3;
    hash ^= hash >> 17;
    hash += hash << 5;
    return hash;
}



static  int IsDigit(char a[])
{


    int size,i;
    size = strlen(a);
    if(size == 0)
    {
        return 0;
    }
    for(i = 0; i< size; i++)
    {
        if(a[i] <'0' || a[i] > '9')
        {
            return 0;
        }
    }
    return 1;
}
int get_pid_by_name(char *name,char *state)
{

    DIR * dp;
    char * dir = "/proc";
    pid_t pid;
    char szFileName [_POSIX_PATH_MAX],pname[128],cstate[32];

    struct dirent * dirp;
    FILE *fp;;
    if( (dp = opendir(dir))== NULL)
    {
        return 0;
    }
    while ((dirp = readdir(dp)) != NULL )
    {

        char data[30];
        sprintf(data, "%s", dirp->d_name);
        if((IsDigit(data)))
        {
            pid = atoi(dirp->d_name);
            sprintf (szFileName, "/proc/%d/stat",  pid);
            if (-1 == access (szFileName, R_OK)) {
                continue;
            }
            if ((fp = fopen (szFileName, "r")) == NULL) {
                continue;
            } /** IF_NULL **/
            memset(cstate,0,sizeof(cstate));
            //921 (mplayer) R 815 815 618 64256 928 4194304 8643 0 3 0 92943 3516 0 0 20 0 1 0 117363 27459584 1943 4294967295 32768 9597476 3196787728 3196781996 68303000
            fscanf(fp,"%*s (%[^)]) %[^ ]",pname,cstate);
            fclose(fp);
            if(strcmp(name,pname)==0)
            {
                if(state)
                    strcpy(state,cstate);
                closedir(dp);
                return pid;
            }
            // printf("pname=%s\n",pname);

        }
    }
    closedir(dp);
    return -1;
}


int RelativeTimer(struct timespec refertime, long int microff)
{
	long int v1 = 0, v2 = 0;
	struct timespec curtime = {0, 0}; 

    clock_gettime(CLOCK_MONOTONIC, &curtime);   
	v1 = refertime.tv_sec*1000 + refertime.tv_nsec/1000000;
	v2 = curtime.tv_sec*1000 + curtime.tv_nsec/1000000;
	if((v2 - v1) > microff)
	{
		return 1;
	}

	return 0;
}

// 返回ms时间差值
int DifferentTimer(struct timespec refertime)
{
	long int v1, v2;
	struct timespec curtime = {0, 0}; 

    clock_gettime(CLOCK_MONOTONIC, &curtime);        

	v1 = refertime.tv_sec*1000 + refertime.tv_nsec/1000000;
	v2 = curtime.tv_sec*1000 + curtime.tv_nsec/1000000;

	return (v2 - v1);
}


void LeeLog( char *iStrData, unsigned char *iHexData, unsigned int dataLen, char flag )
{
	FILE		*fp = NULL;
	time_t		sect;
	struct tm	*tt = NULL;
	char		sTime[24];
	char		oBuf[256];
	sect   = time( NULL );
	tt	   = localtime( &sect );
	memset( sTime, 0, sizeof( sTime ) );
	memset( oBuf, 0, sizeof( oBuf ) );

	strftime( sTime, 22, "%m-%d %H:%M:%S", tt );

	fp = fopen( "/weds/leeLog.txt", "a+" );
	if( fp == NULL )
	{
		return;
	}

	if( iStrData != NULL )
	{
		if( flag == 's' )
		{
			fprintf( fp, "[%s] -->%s\n", sTime, iStrData );
		}else if( flag == 'r' )
		{
			fprintf( fp, "[%s] <--%s\n", sTime, iStrData );
		}else
		{
			fprintf( fp, "[%s] %s\n", sTime, iStrData );
		}
	}

	if( iHexData != NULL )
	{
		HexToTHex( iHexData, oBuf, dataLen, 0 );

		if( flag == 's' )
		{
			fprintf( fp, "[%s] -->%s\n", sTime, oBuf );
		}else if( flag == 'r' )
		{
			fprintf( fp, "[%s] <--%s\n", sTime, oBuf );
		}else
		{
			fprintf( fp, "[%s] %s\n", sTime, oBuf );
		}
	}

	fclose( fp );

	return;
}


/*
 获取文件的MD5值
 */

int MD5_file(char *path,int size,char *Str_md5)
{
    char *data;
    unsigned char md5[16];
    FILE *file;
    int ret;
    int i;
    data=malloc(size);
    if(data==NULL)
    {
        return -1;
    }
    file=fopen(path,"r");
    if(!file){
        free(data);
        return -1;
    }
    ret=fread(data,size,1,file);
    if(ret!=1){
        free(data);
        return -1;
    }
    JMD5_Convert(data,size,md5);
    hex2string(md5,Str_md5,16);
    free(data);
    Str_md5[32]=0;
    return 0;

}

