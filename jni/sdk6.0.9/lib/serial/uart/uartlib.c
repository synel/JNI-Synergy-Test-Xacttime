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

#include "../serial.h"
#include "uartlib.h"

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
#if defined _2410
    sprintf(devpath,"/dev/ttyS%d",port);
#elif defined _2416
    sprintf(devpath,"/dev/ttySAC%d",port);
#endif

    /*设置 可读写、非阻塞方式*/
    serial_fd[port]=open(devpath, O_RDWR|O_NOCTTY|O_NONBLOCK);
    if (serial_fd[port]<0)
    {
        //plog ("open uart %d error!\r\n",port);
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
#if defined _x86
    tty_path = "/dev/ttyS";
#elif defined _2410
    tty_path = "/dev/ttyS";
#elif defined _2416
    if(uart_port>=30 && uart_port <40)
    {
        uart_port = uart_port - 30;
        tty_path = "/dev/ttyUSB";
    }
    else
    {
        tty_path = "/dev/ttySAC";
    }
#endif

    memset(devpath,0,sizeof(devpath));
    sprintf(devpath,"%s%d",tty_path,uart_port);
    printf("open uart %s\r\n",devpath);
    /*设置 可读写、非阻塞方式*/
    fd=open(devpath, O_RDWR|O_NOCTTY|O_NONBLOCK);
    if (fd < 0)
    {
        printf("open uart %s error!\r\n",devpath);
        return ERROR;
    }

    serial_fd[port] = fd;

    if(uart_clearattr(port)<0)
    {
        return ERROR;
    }
    if(uart_set_baudrate(port,baudrate)<0)
    {
        return ERROR;
    }
    if(uart_set_databit(port,databit)<0)
    {
        return ERROR;
    }
    if(uart_set_parity(port,parity)<0)
    {
        return ERROR;
    }
    if(uart_set_stopbit(port,stopbit)<0)
    {
        return ERROR;
    }

    return OK;
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
int uart_clearattr(int port)
{
    struct termios tio;

    if(tcgetattr(serial_fd[port],&tio)<0)
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

    if(tcsetattr(serial_fd[port],TCSANOW,&tio)<0)
    {
        return ERROR;
    }

    return OK;
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
int uart_set_baudrate(int port, int baudrate)
{
    struct termios tio;
    speed_t Bbaudrate;

    if(serial_fd[port]<0)
    {
        return ERROR;
    }

    if(tcgetattr(serial_fd[port],&tio)<0)
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

    if(tcsetattr(serial_fd[port],TCSANOW,&tio)<0)
    {
        return ERROR;
    }

    return OK;
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
int uart_set_databit(int port, int databit)
{
    struct termios tio;

    if(serial_fd[port]<0)
    {
        return ERROR;
    }

    if(tcgetattr(serial_fd[port],&tio)<0)
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

    if(tcsetattr(serial_fd[port],TCSANOW,&tio)<0)
    {
        return ERROR;
    }
    return OK;
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
int uart_set_parity(int port, char parity)
{
    struct termios tio;

    if(serial_fd[port]<0)
    {
        return ERROR;
    }

    if(tcgetattr(serial_fd[port],&tio)<0)
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

    if(tcsetattr(serial_fd[port],TCSANOW,&tio)<0)
    {
        return ERROR;
    }
    return OK;
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
int uart_set_stopbit(int port, char *stopbit)
{
    struct termios tio;

    if(serial_fd[port]<0)
    {
        return ERROR;
    }

    if(tcgetattr(serial_fd[port],&tio)<0)
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

    if(tcsetattr(serial_fd[port],TCSANOW,&tio)<0)
    {
        return ERROR;
    }
    return OK;
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
/*
int uart_recv_data(int port, char *data, int len)
{
    fd_set read_fd;
    struct timeval over_timer;
    int len_total=0,len_tmp=0;
    int retval=0,num=0;

    if(serial_fd[port]<=0)
    {
        return ERROR;
    }
    over_timer.tv_sec = 0;
    over_timer.tv_usec = 150000;//150000 //40000;//lxy 2013-3-13 text_card卡
    FD_ZERO(&read_fd);
    FD_SET(serial_fd[port],&read_fd);

   // while(1)
   // {
    	retval=select(serial_fd[port]+1, &read_fd, NULL, NULL, &over_timer);
        if(retval<0)
        {
            return ERROR;
        }
        else if(retval==0)
        {
            return len_total;
        }
        //over_timer.tv_sec = 0;
        //over_timer.tv_usec = 50000;//150000 //40000;//lxy 2013-3-13 text_card卡

        //if(FD_ISSET(serial_fd[port],&read_fd)>0)
        //{
            len_tmp=0;
            len_tmp=read(serial_fd[port],data+len_total,len-len_total);


            if(len_tmp<=0)
            {
                if(errno == EINTR)
                {
                    len_tmp = 0;
                    if(num++>10)
                    {
                        return ERROR;
                    }
                }
                else
                {
                    return ERROR;
                }

            }

            //append_file("./gprs.log.txt",(unsigned char*)data+len_total,len_tmp);
            //len_total=len_total+len_tmp;
        //}

       // if(len_total==len)
        //{
         //   return len_total;
        //}
    //}//end while
    //return len_total;
    return len_tmp;
}*/

int uart_recv_data(int port, char *data, int len)
{
    fd_set read_fd;
    struct timeval over_timer;
    int len_total=0,len_tmp=0;
    int retval=0,num=0;

    if(serial_fd[port]<=0)
    {
        return ERROR;
    }
    over_timer.tv_sec = 0;
    over_timer.tv_usec = 50000;//150000 //40000;//lxy 2013-3-13 text_card卡
//    FD_ZERO(&read_fd);
//    FD_SET(serial_fd[port],&read_fd);

    while(1)
    {
    	//add by aduo 2014.6.9
    	//<!--
        FD_ZERO(&read_fd);
        FD_SET(serial_fd[port],&read_fd);
        //-->

    	retval=select(serial_fd[port]+1, &read_fd, NULL, NULL, &over_timer);
        if(retval<0)
        {
            /*错误*/
            return ERROR;
        }
        else if(retval==0)
        {
            /*没有设备可以读取数据*/
            return len_total;
        }
//        over_timer.tv_sec = 0;
//        over_timer.tv_usec = 50000;//150000 //40000;//lxy 2013-3-13 text_card卡

        if(FD_ISSET(serial_fd[port],&read_fd)>0)
        {
            len_tmp=0;
            len_tmp=read(serial_fd[port],data+len_total,len-len_total);


            if(len_tmp<=0)
            {
                if(errno == EINTR)
                {
                    len_tmp = 0;
                    if(num++>10)
                    {
                        return ERROR;
                    }
                }
                else
                {
                    return ERROR;
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
int uart_send_data(int port, char *data, int len)
{
    struct timeval over_timer;
    fd_set write_fd;
    int len_total=0,len_tmp=0;
    int retval=0,num=0;

    if(serial_fd[port]<=0)
    {
        return ERROR;
    }
    over_timer.tv_sec=0;
    over_timer.tv_usec=40000;

    while(len>len_total)
    {
        FD_ZERO(&write_fd);
        FD_SET(serial_fd[port],&write_fd);

        retval=select(serial_fd[port]+1,NULL,&write_fd, NULL, &over_timer);
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

        if(FD_ISSET(serial_fd[port],&write_fd)>0)
        {
            len_tmp=write(serial_fd[port],data+len_total,len-len_total);
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
int uart_clear(int port)
{
    if(serial_fd[port]>0)
    {
        if(tcflush(serial_fd[port],TCIOFLUSH)<0)
        {
            return ERROR;
        }
        else
        {
            return OK;
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
int uart_close(int port)
{
    if(serial_fd[port]<=0)
    {
        return ERROR;
    }
    if(close(serial_fd[port])<0)
    {
        return ERROR;
    }
    return OK;
}
