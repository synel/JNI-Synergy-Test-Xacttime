/**
 * @chinese
 * @file   uartlib.c
 * @author 刘训
 * @date   Thu Oct 13 10:00:00 2011
 *
 * @brief  提供串口基本操作方面的函数
 * @endchinese
 *
 * @english
 * @file   uartlib.c
 * @author Liu Xun
 * @date   Thu Oct 13 10:00:00 2011
 *
 * @brief  functions of serail operating
 * @endenglish
 */

#include <termios.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "usr_sem.h"
#include "uartlib.h"
#include "config.h"
#include "debug.h"
#include "serial_devices.h"
#include "device_protocol.h"
//#include "../../android/wedsa23.h"


void append_file(char *filename,unsigned char * data,int len)
{
    FILE *file;
    int num = 0;
    file = fopen(filename, "a");
    if (file == NULL) {
        perror(filename);
        return;
    }
    //num=fwrite("|",1,1,file);
    num = fwrite(data, 1, len, file);
    if (num != len) {
        perror("append_file");
        printf("%d,%d\n", len, num);
    }
    fclose(file);
}

/**
 * @chinese
 * 打开串口但不设置属性
 *
 * @param port 串口号
 *
 * @return 成功-文件描述符，失败-ERROR
 * @endchinese
 *
 * @english
 * open uart device but not set attribution
 *
 * @param port port to open
 *
 * @return Success-fd，Fail-ERROR
 * @endenglish
 */
int uart_open(int port)
{
    char devpath[56];
    int fd=0;

    memset(devpath,0,sizeof(devpath));
    sprintf(devpath,"/dev/ttyS%d",port);

    /*设置 可读写、非阻塞方式*/
    fd=open(devpath, O_RDWR|O_NOCTTY|O_NONBLOCK);
    if (fd<0)
    {
        plog ("open uart %d error!\r\n",port);
        return ERROR;
    }

    return fd;
}

/**
 * @chinese
 * 打开串口并设置属性
 *
 * @param port 串口号
 * @param baudrate 波特率
 * @param databit 数据位
 * @param stopbit 停止位
 * @param parity 奇偶校验
 * @note
在类UNIX系统中，每个串口都有一设备文件相对应(在/dev目录中)，但每个系统
又不相同，下面是一些系统中对应的串口及设备文件(最后一行为Windows系统)。

 * @verbatim
System         Port 1      Port 2
IRIX          /dev/ttyf1   /dev/ttyf2
HP-UX         /dev/tty1p0  /dev/tty2p0
Solaris/SunOS /dev/ttya    /dev/ttyb
Linux         /dev/ttyS0   /dev/ttyS1
Linux         /dev/ttyUSB0 /dev/ttyUSB1 (usb-serial converter)
Digital UNIX  /dev/tty01   /dev/tty02
(Windows       COM1        COM2)
 * @return 成功-文件描述符，失败-ERROR
 * @endchinese
 *
 * @english
 * open uart device and set attribution
 *
 * @param port port to open
 * @param baudrate baudrate
 * @param databit data bit
 * @param stopbit stop bit
 * @param parity parity
 *
 * @return Success-fd，Fail-ERROR
 * @endenglish
 */
int uart_open_and_setattr(int port, int baudrate, int databit, char *stopbit, char parity)
{
    char devpath[56],*tty_path = "/dev/ttyS";
    int fd=0;
    int uart_port=port;
#ifdef _ARM_2410
    tty_path = "/dev/s3c2410_serial";
#elif defined _DM365
    tty_path = "/dev/ttyS";
#elif defined _ARM_2416
    if(uart_port>=30 && uart_port <40)
    {
        uart_port = uart_port - 30;
        tty_path = "/dev/ttyUSB";
    }
    else
    {
        tty_path = "/dev/ttySAC";
    }
#elif defined _AM335X
    if(uart_port>=30 && uart_port <40)
    {
        uart_port = uart_port - 30;
        tty_path = "/dev/ttyUSB";
    }
    else
    {
        tty_path = "/dev/ttySAC";  
    }
#elif defined _ARM_A23
    if(uart_port>=30 && uart_port <40)
    {
        uart_port = uart_port - 30;
        tty_path = "/dev/ttyUSB";
    }
    else
    {
    	if(PLATFORM_TYPE == 2 || PLATFORM_TYPE == 3)
    	{
			tty_path = "/dev/wedscom";
		}
        else
		{
			tty_path = "/dev/ttyS";
		}
    }
#endif


    memset(devpath,0,sizeof(devpath));
    sprintf(devpath,"%s%d",tty_path,uart_port);
    /*设置 可读写、非阻塞方式*/
printf("uart path == %s\n", devpath);	
	
//	LOGI("tty path = %s\n", devpath);
	
    fd=open(devpath, O_RDWR|O_NOCTTY|O_NONBLOCK);

    if (fd<0)
    {
        plog ("open uart %s error!\r\n",devpath);
        return ERROR;
    }

    if(uart_clearattr(fd)<0)
    {
        return ERROR;
    }
    if(uart_set_baudrate(fd,baudrate)<0)
    {
        return ERROR;
    }
    if(uart_set_databit(fd,databit)<0)
    {
        return ERROR;
    }
    if(uart_set_parity(fd,parity)<0)
    {
        return ERROR;
    }
    if(uart_set_stopbit(fd,stopbit)<0)
    {
        return ERROR;
    }

    return fd;
}

/**
 * @chinese
 * 清除终端属性
 *
 * @param fd 文件描述符
 *
 * @return 成功-SUCCESS，失败-ERROR
 * @endchinese
 *
 * @english
 * clear attribution of terminal
 *
 * @param fd fd
 *
 * @return Success-SUCCESS，Fail-ERROR
 * @endenglish
 */
int uart_clearattr(int fd)
{
    struct termios tio;

    if(tcgetattr(fd,&tio)<0)
    {
        return ERROR;
    }

    bzero (&tio, sizeof(tio));
    cfmakeraw (&tio);

    tio.c_cflag |= CLOCAL | CREAD;
    tio.c_oflag = 0;
    tio.c_lflag |= 0;
    tio.c_oflag &= ~OPOST;
    tio.c_cc[VTIME] = 1;
    tio.c_cc[VMIN] = 1;

    if(tcsetattr(fd,TCSANOW,&tio)<0)
    {
        return ERROR;
    }

    return SUCCESS;
}

/**
 * @chinese
 * 设置波特率
 *
 * @param fd 文件描述符
 * @param baudrate 波特率
 *
 * @return 成功-SUCCESS，失败-ERROR
 * @endchinese
 *
 * @english
 * set baudrate of uart
 *
 * @param fd fd
 * @param baudrate baudrate
 *
 * @return Success-SUCCESS，Fail-ERROR
 * @endenglish
 */
int uart_set_baudrate(int fd, int baudrate)
{
    struct termios tio;
    speed_t Bbaudrate;

    if(fd<0)
    {
        return ERROR;
    }

    if(tcgetattr(fd,&tio)<0)
    {
        return ERROR;
    }

    switch(baudrate)
    {
    case 0:
        Bbaudrate=B0;
        break;
    case 50:
        Bbaudrate=B50;
        break;
    case 75:
        Bbaudrate=B75;
        break;
    case 110:
        Bbaudrate=B110;
        break;
    case 134:
        Bbaudrate=B134;
        break;
    case 150:
        Bbaudrate=B150;
        break;
    case 200:
        Bbaudrate=B200;
        break;
    case 300:
        Bbaudrate=B300;
        break;
    case 600:
        Bbaudrate=B600;
        break;
    case 1200:
        Bbaudrate=B1200;
        break;
    case 2400:
        Bbaudrate=B2400;
        break;
    case 4800:
        Bbaudrate=B4800;
        break;
    case 9600:
        Bbaudrate=B9600;
        break;
    case 19200:
        Bbaudrate=B19200;
        break;
    case 38400:
        Bbaudrate=B38400;
        break;
    case 57600:
        Bbaudrate=B57600;
        break;
    case 115200:
        Bbaudrate=B115200;
        break;
    default:
        Bbaudrate=B9600;
        break;
    }

#if 0
    //使用下列方式设置波特率，在PC 2.6.11内核上，会引起程序崩溃
    tio.c_cflag |= Bbaudrate;
#endif
    cfmakeraw(&tio);
    cfsetispeed(&tio,Bbaudrate);
    cfsetospeed(&tio,Bbaudrate);

    if(tcsetattr(fd,TCSANOW,&tio)<0)
    {
        return ERROR;
    }

    return SUCCESS;
}

/**
 * @chinese
 * 设置数据位
 *
 * @param fd 文件描述符
 * @param databit 数据位
 *
 * @return 成功-SUCCESS，失败-ERROR
 * @endchinese
 *
 * @english
 * set databit of uart
 *
 * @param fd fd
 * @param databit databit
 *
 * @return Success-SUCCESS，Fail-ERROR
 * @endenglish
 */
int uart_set_databit(int fd, int databit)
{
    struct termios tio;

    if(fd<0)
    {
        return ERROR;
    }

    if(tcgetattr(fd,&tio)<0)
    {
        return ERROR;
    }

    tio.c_cflag &= ~CSIZE;
    switch (databit)
    {
    case 8:
        tio.c_cflag |= CS8;
        break;
    case 7:
        tio.c_cflag |= CS7;
        break;
    case 6:
        tio.c_cflag |= CS6;
        break;
    case 5:
        tio.c_cflag |= CS5;
        break;
    default:
        tio.c_cflag |= CS8;
        break;
    }

    if(tcsetattr(fd,TCSANOW,&tio)<0)
    {
        return ERROR;
    }
    return SUCCESS;
}

/**
 * @chinese
 * 设置奇偶校验
 *
 * @param fd 文件描述符
 * @param parity 奇偶校验字符['N':不进行校验 'E':偶校验 'O':奇校验]
 *
 * @return 成功-SUCCESS，失败-ERROR
 * @endchinese
 *
 * @english
 * set parity of uart
 *
 * @param fd fd
 * @param parity parity['N':no parity 'E':even 'O':odd]
 *
 * @return Success-SUCCESS，Fail-ERROR
 * @endenglish
 */
int uart_set_parity(int fd, char parity)
{
    struct termios tio;

    if(fd<0)
    {
        return ERROR;
    }

    if(tcgetattr(fd,&tio)<0)
    {
        return ERROR;
    }

    switch (parity)
    {
    /*无校验*/
    case 'N':
        tio.c_cflag &= ~PARENB;
        break;
        /*偶校验*/
    case 'E':
        tio.c_cflag |= PARENB;
        tio.c_cflag &= ~PARODD;
        break;
        /*奇校验*/
    case 'O':
        tio.c_cflag |= PARENB;
        tio.c_cflag |= ~PARODD;
        break;
        /*无校验*/
    default:
        tio.c_cflag &= ~PARENB;
        break;
    }

    if(tcsetattr(fd,TCSANOW,&tio)<0)
    {
        return ERROR;
    }
    return SUCCESS;
}

/**
 * @chinese
 * 设置停止位
 *
 * @param fd 文件描述符
 * @param stopbit 停止位字符
 *
 * @return 成功-SUCCESS，失败-ERROR
 * @endchinese
 *
 * @english
 * set stopbit of uart
 *
 * @param fd fd
 * @param stopbit stopbit
 *
 * @return Success-SUCCESS，Fail-ERROR
 * @endenglish
 */
int uart_set_stopbit(int fd, char *stopbit)
{
    struct termios tio;

    if(fd<0)
    {
        return ERROR;
    }

    if(tcgetattr(fd,&tio)<0)
    {
        return ERROR;
    }

    if(0 == strcmp(stopbit, "1"))
    {
        tio.c_cflag &= ~CSTOPB;
    }
    else if (0 == strcmp (stopbit, "1.5"))
    {
        tio.c_cflag &= ~CSTOPB;
    }
    else if (0 == strcmp (stopbit, "2"))
    {
        tio.c_cflag |= CSTOPB;
    }
    else
    {
        tio.c_cflag &= ~CSTOPB; /* 1 stop bit */
    }

    if(tcsetattr(fd,TCSANOW,&tio)<0)
    {
        return ERROR;
    }
    return SUCCESS;
}

/**
 * @chinese
 * 通过串口读取数据
 *
 * @param fd 已打开的串口
 * @param data 存放接收到的数据
 * @param len 要读取的数据长度
 *
 * @return 成功-读取到的串口数据长度, 0-无数据，失败-ERROR
 * @endchinese
 *
 * @english
 * read data from uart
 *
 * @param fd com opened
 * @param data save data received from uart
 * @param len lenth of data to receive from uart
 *
 * @return Success-data lenth of received，0-no data, Fail-ERROR
 * @endenglish
 */
int uart_recv_data(int fd, char *data, int len)
{
    fd_set read_fd;
    struct timeval over_timer;
    int len_total=0,len_tmp=0;
    int retval=0,num=0, i=0;
    if(fd<=0)
    {
        return ERROR;
    }
    over_timer.tv_sec = 0;
    over_timer.tv_usec = 5000;//150000 //40000;//lxy 2013-3-13 text_card卡
    FD_ZERO(&read_fd);
    FD_SET(fd,&read_fd);

    while(1)
    {

        retval=select(fd+1, &read_fd, NULL, NULL, &over_timer);
       // printf("%s0,fd=%d,%d\n",__func__,fd,retval);
        if(retval<0)
        {
            /*错误*/
            return len_total;
        }
        else if(retval==0)
        {
            /*没有设备可以读取数据*/
            return len_total;
        }
//        over_timer.tv_sec = 0;
//        over_timer.tv_usec = 4000;//150000 //40000;//lxy 2013-3-13 text_card卡

        if(FD_ISSET(fd,&read_fd)>0)
        {
            len_tmp=0;
            len_tmp=read(fd,data+len_total,len-len_total);
//printf("%s1,%d %d,%d\n",__func__,len_tmp,len,len_total);

            if(len_tmp<=0)
            {
                if(errno == EINTR)
                {
                    len_tmp = 0;
                    if(num++>10)
                    {
                        //printf("%s2\n",__func__);
                        return len_total;
                        //return ERROR;
                    }
                }
                else
                {
                   // printf("%s3 %d\n",__func__,len_total);
                   if(len_total>0)
                        return len_total;
                    else
                        return ERROR;
                   // return len_total;
                    //return ERROR;
                }

            }
            //append_file("./gprs.log.txt",(unsigned char*)data+len_total,len_tmp);
            len_total=len_total+len_tmp;
        }

        /*此段代码为必需代码，当串口数据长度大于len时，防止进入死循环*/

        if(len_total==len)
        {
            return len_total;
        }
        over_timer.tv_sec = 0;
        over_timer.tv_usec = 5000;
        FD_ZERO(&read_fd);
        FD_SET(fd,&read_fd);
    }//end while
	
    return len_total;
}

/**
 * @chinese
 * 发送数据
 *
 * @param fd 已打开的串口
 * @param data 要发送的数据
 * @param len 数据长度
 *
 * @return 成功-已发送长度，失败-ERROR
 * @endchinese
 *
 * @english
 * send data
 *
 * @param fd uart opened
 * @param data data to send
 * @param len lenth of data
 *
 * @return Sucess-lenth of data sent,fail-ERROR
 * @endenglish
 */
int uart_send_data(int fd, char *data, int len)
{
    struct timeval over_timer;
    fd_set write_fd;
    int len_total=0,len_tmp=0;
    int retval=0,num=0, i=0;

    if(fd<=0)
    {
        return ERROR;
    }
    FD_ZERO(&write_fd);
    FD_SET(fd,&write_fd);
    over_timer.tv_sec=0;
    over_timer.tv_usec=40000;

#if 0
printf("-->> ");
for(i=0;i<len;i++)
{
	printf("%02X ", data[i]);
	if(i > 0 && i%32 == 0) printf("\n");
}
printf("\n");
#endif

    while(len>len_total)
    {


        retval=select(fd+1,NULL,&write_fd, NULL, &over_timer);
        if(retval<0)
        {
            if(len_total>0)
            {
                return len_total;
            }
            else
            {
                return ERROR;
            }
        }
        else if(retval==0)
        {
            return ERROR;
        }

        if(FD_ISSET(fd,&write_fd)>0)
        {
            len_tmp=write(fd,data+len_total,len-len_total);
            if(len_tmp<=0)
            {
                if(errno == EINTR) /*电信3G测试过程中发现过EINTR表示在写的时候出现了中断错误，容易阻塞*/
                {
                    len_tmp = 0;
                    if(num++>10)
                    {
                        return ERROR;
                    }
                }
                else
                    return ERROR;
            }

            //            if(len_tmp <0)
            //            {
            //                if(len_total>0)
            //                {
            //                    return len_total;
            //                }
            //                else
            //                {
            //                    return ERROR;
            //                }
            //            }
            len_total=len_total+len_tmp;
        }
        FD_ZERO(&write_fd);
        FD_SET(fd,&write_fd);
        over_timer.tv_sec=0;
        over_timer.tv_usec=5000;
    }
    return len_total;
}

/**
 * @chinese
 * 清空串口缓存数据
 *
 * @param fd 串口文件描述符
 *
 * @return 成功-SUCCESS，失败-ERROR
 * @endchinese
 *
 * @english
 * clear uart port
 *
 * @param fd fd of uart
 *
 * @return Success-SUCCESS，Fail-ERROR
 * @endenglish
 *
 */
int uart_clear(int fd)
{
    if(fd>0)
    {
        if(tcflush(fd,TCIOFLUSH)<0)
        {
            return ERROR;
        }
        else
        {
            return SUCCESS;
        }
    }
    else
    {
        return ERROR;
    }
}

/**
 * @chinese
 * 关闭串口
 *
 * @param fd 串口文件描述符
 *
 * @return 成功-SUCCESS，失败-ERROR
 * @endchinese
 *
 * @english
 * close uart port
 *
 * @param fd fd of uart
 *
 * @return Success-SUCCESS，Fail-ERROR
 * @endenglish
 *
 */
int uart_close(int fd)
{
    if(fd<=0)
    {
        return ERROR;
    }
    if(close(fd)<0)
    {
        return ERROR;
    }
    return SUCCESS;
}



int UartWrite( int comfd, unsigned char *iBuf, int iLen )
{
	unsigned char	* addr = NULL;
	int				len;
	int				num=0, count=0;

	struct timeval tout;
	fd_set write_fd,read_fd;

	if(comfd <= 0 || !iBuf || iLen<=0) return -1;

	FD_ZERO(&write_fd);
	FD_SET(comfd,&write_fd);

	while( iLen != num ){
		tout.tv_sec  = 0;
		tout.tv_usec = 50000;
		if( select( comfd + 1, NULL, &write_fd, NULL, &tout ) <= 0 ){
			return -1;
		}
		addr	   = iBuf + num;
		count  = write( comfd, addr, iLen - num );
		if( count < 0 ){
			return -1;
		}
		num += count;
	}

//	dbgShowHexData(iBuf, iLen, 1, '>', "SEND");

	return num;

}

// oTime: 超时时间 单位 ms
int UartRead( int comfd, unsigned char *oBuf, int oLen, unsigned int oTime)
{
	unsigned char	* addr = NULL;
	int				len, i;
	int				num=0, count=0;
	unsigned char    buf[2048];

	struct timeval tout;
	fd_set write_fd,read_fd;

	if(comfd <= 0 || !oBuf || oLen < 0) return -1;

	FD_ZERO(&read_fd);
	FD_SET(comfd,&read_fd);

	while( oLen != num ){
		tout.tv_sec  = 0;
		tout.tv_usec = oTime * 1000 ;
		if( select( comfd + 1, &read_fd, NULL, NULL, &tout ) <= 0 ){
			goto END;
		}
		addr	   = oBuf + num;
		count  = read( comfd, addr, oLen - num );
		if( count < 0 ){
			goto END;
		}
		num += count;
	}
END:

	return num;

}




























