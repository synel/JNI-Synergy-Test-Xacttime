/**
 * @chinese
 * @file   libserial.c
 * @author 刘训
 * @date   Wed Jul 13 09:52:45 2011
 *
 * @brief weds串口操作协议处理模块
 * @details
    串行通讯采用服务器呼叫/终端机应答方式，每次服务器呼叫某编号终端机， @n
 *  该终端机收到呼叫后应答，但广播指令不应答
 * @endchinese
 *
 * @english
 * @file   libserial.c
 * @author Liu Xun
 * @date   Wed Jul 13 09:52:45 2011
 *
 * @brief weds serail operation protocal module
 * @details
    serial communications using server-client modal.@n
    `client` often refer to a PC machine on which background software was installed@n
    `server` often refer to a terminal machine.
    `client` will call actively, and `server` will response to its calling.@n
     Every time,`client` will call some numbered `server`@n
     This `server` always response `client`, except some `broadcasting instructions`
 * @endenglish
 *
 *
 */
#include "usr_sem.h"
#include "libserial.h"
#include "config.h"
//#include "../spi/seed_spi_api.h"
#include "debug.h"

/**
 *设置超时触发值
 *
 * @param timer 超时的秒数
 * @param seconds 指向定时器的指针
 * @note
    这个函数设置时间超时的触发值：等侍 seconds 秒之后超时。 @n
    首先将定时器复位，重新设置触发值后，必须调用 @n
    start_timer 重新开始计时。在使用定时器之前，一定要首先 @n
    设置触发值，否则无法预知执行结果。但如果只限于用 @n
    time_used 测量时间而不用知道何时超时，可以不设置超时时间。 @n

 * @return 如果 seconds 小于 0.0 或 timer 为空，返回 0，否则返回 1。
 */
int set_time_out_trigger(TTimer *timer, double seconds) {

    if(seconds < 0.0 || timer == NULL)
        return 0;
    timer->status = STOP;
    timer->begin = timer->end = 0.0;
    timer->trigger_ticks = seconds * PRECISION;
    return 1;
} /* set_time_out_trigger */


/**
 *取得超时触发值,这个函数取得超时触发值。每次重新设置触发值时，必须用这个函数取得旧的触发值
 *
 * @param timer 指向定时器的指针
 *
 * @return 如果 timer 是空指针，返回 0.0，否则返回旧的超时触发值
 */
double get_time_out_trigger(TTimer *timer) {

    return timer == NULL ? 0.0 : timer->trigger_ticks / PRECISION;
} /* get_time_out_trigger */


/**
 *关闭计时
 *
 * @param timer 指向定时器的指针
 * @note
    关闭计时，把计时器状态设置为 STOP。与 end_timer 不同的 @n
    是，当 stop_timer 之后，is_time_out 总是返回 0，也就是 @n
    永远不会超时。stop_timer 之后，当 start_timer 时，由 @n
    start_timer 函数自动把计时器状态设置为 RUN。 @n

 * @return 如果成功返回 1；否则返回 0。
 */
int stop_timer(TTimer *timer) {

    if(timer == NULL)
        return 0;
    timer->status = STOP;
    return 1;
} /* stop_timer */


/**
 *开始计时
 *
 * @param timer 指向定时器的指针
 *
 * @return 如果成功返回 1；否则返回 0
 */
int start_timer(TTimer *timer) {

    struct timeval tv;

    if(timer == NULL || timer->status == RUN)
        return 0;

    gettimeofday(&tv, NULL);
    timer->end = 0.0;
    timer->begin = (double)tv.tv_sec * PRECISION + (double)tv.tv_usec;
    timer->status = RUN;
    return 1;
} /* start_timer */


/**
 *结束计时,结束计时。当计时器状态为 STOP 时返回 0
 *
 * @param timer 指向定时器的指针
 *
 * @return 如果成功返回 1；否则返回 0
 */
int end_timer(TTimer *timer) {

    struct timeval tv;

    if(timer == NULL || timer->status == STOP)
        return 0;
    gettimeofday(&tv, NULL);
    timer->end = (double)tv.tv_sec * PRECISION + (double)tv.tv_usec;
    return 1;
} /* end_timer */


/**
 *使用的时间
 *
 * @param timer 指向定时器的指针
 * @note
    计算从开始计时到结束计时使用的时间。如果需要计算这个时 @n
    间，那么在开始之后，必须首先结束计时，然后再用这个函数 @n
    计算所用的时间。如果只限于用这个函数测量时间而不用知道 @n
    何时超时，可以不设置超时时间

 * @return 如果成功返回使用的时间；否则返回 0.0
 */
double time_used(TTimer *timer) {

    if(timer == NULL || timer->end <= 0.0)
        return 0.0;

    return (timer->end - timer->begin) / PRECISION;
} /* time_used */


/**
 *判断是否超时的函数
 *
 * @param timer 指向定时器的指针
 * @note
    判断是否超时的函数，如果超时了，在下一次判断超时之前， @n
    必须重新开始计时。如果计时器处于 STOP 状态，返回零值， @n
    也就是永远不会超时

 * @return 如果超时了，返回非零值；否则返回零值
 */
int is_time_out(TTimer *timer) {

    struct timeval tv;
    double clicks;
    gettimeofday(&tv, NULL);
    clicks = (double)tv.tv_sec * PRECISION + (double)tv.tv_usec - timer->begin;

    if(timer == NULL || timer->status == STOP)
    {
        return 0;
    }
    if(clicks >= timer->trigger_ticks || clicks < 0)
    {
        timer->status = STOP;
        return  1 ;
    }
    return 0;
} /* is_time_out */


#define TIMEOUT_SEC(buflen,baud) (buflen * 20 / baud + 2)
#define TIMEOUT_USEC 0

#define BUFFER_LEN	1024    /* sendfile() */

INT32    fd[MAXCOM];
static struct termios termios_old[MAXCOM], termios_new[MAXCOM];
static fd_set   fs_read, fs_write;
static struct timeval tv_timeout;


static void     SetBaudrate (INT32 ComPort, INT32 baudrate);
static void     SetDataBit (INT32 ComPort, INT32 databit);
static INT32    BAUDRATE (INT32 baudrate);
static INT32    SetPortAttr (INT32 ComPort, INT32 baudrate, INT32 databit,
                             const char *stopbit, char parity);
static void     SetStopBit (INT32 ComPort, const char *stopbit);
static void     SetParityCheck (INT32 ComPort, char parity);
/**
 *打开串口操作
 *
 * @param ComPort 串口号
 * @param baudrate 波特率
 * @param databit 数据位
 * @param stopbit 停止位
 * @param parity 校验位
 *
 * @return 成功时返回串口设备文件描述符，否则，返回-1并给出相应提示信息
 * @note
在类UNIX系统中，每个串口都有一设备文件相对应(在/dev目录中)，但每个系统
又不相同，下面是一些系统中对应的串口及设备文件(最后一行为Windows系统)。

 * @note
System         Port 1      Port 2
IRIX          /dev/ttyf1   /dev/ttyf2
HP-UX         /dev/tty1p0  /dev/tty2p0
Solaris/SunOS /dev/ttya    /dev/ttyb
Linux         /dev/ttyS0   /dev/ttyS1
Linux         /dev/ttyUSB0 /dev/ttyUSB1 (usb-serial converter)
Digital UNIX  /dev/tty01   /dev/tty02
(Windows       COM1        COM2)


参数为串口号，比如，open_serial_device(1)是打开系统中第1个串口设备。 */
INT32 OpenComPort (INT32 ComPort, INT32 baudrate, INT32 databit,
                   const char *stopbit, char parity)
{
    char           *pComPort;
    INT32           retval;
    switch (ComPort) {
    case 0:
        pComPort = "/dev/ttyS0";
        break;
    case 1:
        pComPort = "/dev/ttyS1";//"/dev/s3c2410_serial1";
        break;
    case 2:
        pComPort = "/dev/ttyS2";
        break;
    case 3:
        pComPort = "/dev/ttyS3";
        break;
    case 4:
        pComPort = "/dev/ttyS4";
        break;
    case 5:
        pComPort = "/dev/ttyS5";
        break;
    case 6:
        pComPort = "/dev/ttyS6";
        break;
    case 7:
        pComPort = "/dev/ttyS7";
        break;
    case 8:
        pComPort = "/dev/davinci_spi_eeprom";
        fd[ComPort] = open(pComPort, O_RDONLY|O_NONBLOCK);
        if (fd[ComPort] < 0)
        {
	    plog("file open %s error \n", pComPort);
            return -1;
        }
        return fd[ComPort];
    default:
        pComPort = "/dev/ttyS0";
        break;
    }
    plog("init com:%s,%d\n",pComPort,baudrate);
    fd[ComPort] = open (pComPort, O_RDWR |  O_NOCTTY | O_NONBLOCK); //| O_SYNC)
    if (-1 == fd[ComPort]) {
        fprintf (stderr, "cannot open port %s: %s\n", pComPort,strerror(errno));
        return (-1);
    }
    tcgetattr (fd[ComPort], &termios_old[ComPort]); /* save old termios value */
    /* 0 on success, -1 on failure */
    retval = SetPortAttr (ComPort, baudrate, databit, stopbit, parity);

    if (-1 == retval) {
        fprintf (stderr, "\nport %s cannot set baudrate at %d: %s\n", pComPort,
                 baudrate, strerror(errno));
    }
    return (retval);
}

/**
 * @chinese
 * 关闭串口
 *
 * @param ComPort 此处为传递进来的串口号
 * @endchinese
 *
 * @english
 * close serial port
 *
 * @param ComPort serial port number
 * @endenglish
 *
 */
void CloseComPort (INT32 ComPort)
{

    if(fd[ComPort] == 0)
        return ;
    tcsetattr (fd[ComPort], TCSADRAIN, &termios_old[ComPort]);
    close (fd[ComPort]);
}
/**
 *清空串口缓存数据
 *
 * @param ComPort 此处为传递进来的串口号
 */
void ComPort_Clear(int ComPort)
{
    if(fd[ComPort] >0)
        tcflush(fd[ComPort],TCIOFLUSH);
}
/**
 *检测串口是否有数据到来
 *
 * @param ComPort 此处为传递进来的串口号
 *
 * @return 成功-1,失败-0
 */
INT32 check_uart_received(INT32 ComPort)
{
    INT32  retval = 0;

    if(fd[ComPort] <=0 )
        return 0;
    FD_ZERO (&fs_read);
    FD_SET (fd[ComPort], &fs_read);
    tv_timeout.tv_sec = 0;//TIMEOUT_SEC (datalength, GetBaudrate ());
    tv_timeout.tv_usec = 0;//TIMEOUT_USEC;
    //    usr_sem_wait();
    retval = select (fd[ComPort] + 1, &fs_read, NULL, NULL, &tv_timeout);
    //plog("retval:%d,%d\n",retval,fd[ComPort]);
    if(retval <=0)
        return 0;
    return 1;
}
/**
 *通过串口读取数据
 *
 * @param ComPort 此处为传递进来的串口号
 * @param data 读取到的串口数据
 * @param datalength 读取串口数据长度
 *
 * @return 成功-读取到的串口数据长度，失败-错误代码
 */
INT32 ReadComPort (INT32 ComPort, void *data, INT32 datalength)
{
    INT32  retval = 0;
    INT32 len=0;

    FD_ZERO (&fs_read);
    FD_SET (fd[ComPort], &fs_read);
    tv_timeout.tv_sec = 0;//TIMEOUT_SEC (datalength, GetBaudrate ());
    tv_timeout.tv_usec = 150000;//TIMEOUT_USEC;
    usr_sem_wait();
    retval = select (fd[ComPort] + 1, &fs_read, NULL, NULL, &tv_timeout);
    if (retval)
    {
        len = read (fd[ComPort], data, datalength);
        usr_sem_post();
        return len;
    }
    else
    {
        usr_sem_post();
        return (-1);
    }
}
/**
 *读取串口数据，暂时未用
 *
 * @param ComPort 此处为传递进来的串口号
 * @param data
 * @param datalength
 *
 * @return
 */
int ReadComPort2(INT32 ComPort, void *data, INT32 datalength) {

    int recvbytes, n;
    n = recvbytes = 0;

    if(ioctl(fd[ComPort], FIONREAD, &recvbytes) != 0) {
        perror("ioctl");
        return -1;
    }

    if(recvbytes <= 0 || recvbytes < datalength)
        return -1;
    //    plog("I want %d bytes. There are %d bytes available\n",     datalength, recvbytes);


    if(recvbytes >= datalength)
        n = read(fd[ComPort], data, datalength);
    return n;
} /* ReadComPort2 */

/**
 *写入串口数据
 *
 * @param ComPort 此处为传递进来的串口号
 * @param data 写入的串口数据
 * @param datalength 写入的串口数据长度
 *
 * @return 成功-写入数据长度,失败-错误代码
 */
INT32 WriteComPort (INT32 ComPort, const UINT8 * data, INT32 datalength)
{
    INT32       retval, len = 0, total_len = 0;
    FD_ZERO (&fs_write);
    FD_SET (fd[ComPort], &fs_write);
    tv_timeout.tv_sec = 0;//TIMEOUT_SEC (datalength, GetBaudrate ());
    tv_timeout.tv_usec = 150000;//TIMEOUT_USEC;
    // tcflush (fd[ComPort], TCOFLUSH);
    return write (fd[ComPort],data, datalength );
    for (total_len = 0, len = 0; total_len < datalength;)
    {
        retval = select (fd[ComPort] + 1, NULL, &fs_write, NULL, &tv_timeout);
        if (retval)
        {
            len = write (fd[ComPort], &data[total_len], datalength - total_len);
            if (len > 0)
            {
                total_len += len;
                tcflush (fd[ComPort], TCOFLUSH);
            }
        }
        else
        {
            tcflush (fd[ComPort], TCOFLUSH);
            break;
        }
    }
    return (total_len);
}
/**
 *
 *设置串口波特率
 * @param ComPort 此处为传递进来的串口号
 * @param baudrate 波特率值
 */
static void SetBaudrate (INT32 ComPort, INT32 baudrate)
{
    termios_new[ComPort].c_cflag |= BAUDRATE (baudrate);
}
/**
 *
 *设置串口数据位
 * @param ComPort 此处为传递进来的串口号
 * @param databit 串口数据位
 */
static void SetDataBit (INT32 ComPort, INT32 databit)
{
    termios_new[ComPort].c_cflag &= ~CSIZE;
    switch (databit) {
    case 8:
        termios_new[ComPort].c_cflag |= CS8;
        break;
    case 7:
        termios_new[ComPort].c_cflag |= CS7;
        break;
    case 6:
        termios_new[ComPort].c_cflag |= CS6;
        break;
    case 5:
        termios_new[ComPort].c_cflag |= CS5;
        break;
    default:
        termios_new[ComPort].c_cflag |= CS8;
        break;
    }
}
/**
 *设置串口停止位
 *
 * @param ComPort 此处为传递进来的串口号
 * @param stopbit 串口停止位
 */
static void SetStopBit (INT32 ComPort, const char *stopbit)
{
    if (0 == strcmp (stopbit, "1")) {
        termios_new[ComPort].c_cflag &= ~CSTOPB; /* 1 stop bit */
    }
    else if (0 == strcmp (stopbit, "1.5")) {
        termios_new[ComPort].c_cflag &= ~CSTOPB; /* 1.5 stop bits */
    }
    else if (0 == strcmp (stopbit, "2")) {
        termios_new[ComPort].c_cflag |= CSTOPB;  /* 2 stop bits */
    }
    else {
        termios_new[ComPort].c_cflag &= ~CSTOPB; /* 1 stop bit */
    }
}
/**
 *设置串口奇偶效验
 *
 * @param ComPort 此处为传递进来的串口号
 * @param parity 奇偶效验位
 */
static void SetParityCheck (INT32 ComPort, char parity)
{
    switch (parity) {
    case 'N':                  /* no parity check */
        termios_new[ComPort].c_cflag &= ~PARENB;
        break;
    case 'E':                  /* even */
        termios_new[ComPort].c_cflag |= PARENB;
        termios_new[ComPort].c_cflag &= ~PARODD;
        break;
    case 'O':                  /* odd */
        termios_new[ComPort].c_cflag |= PARENB;
        termios_new[ComPort].c_cflag |= ~PARODD;
        break;
    default:                   /* no parity check */
        termios_new[ComPort].c_cflag &= ~PARENB;
        break;
    }
}
/**
 *设置串口各种参数
 *
 * @param ComPort 此处为传递进来的串口号
 * @param baudrate 波特率
 * @param databit 数据位
 * @param stopbit 停止位
 * @param parity 校验位
 *
 * @return 成功时返回0，否则返回-1并给出相应提示信息
  * @note
 *         该程序中一些设置优先考虑最常用的，如波特率为115200，数据位为8位
 */
static INT32 SetPortAttr (INT32 ComPort, INT32 baudrate,
                          INT32 databit, const char *stopbit, char parity)
{
    bzero (&termios_new[ComPort], sizeof (termios_new));
    cfmakeraw (&termios_new[ComPort]);
    SetBaudrate (ComPort, baudrate);
    termios_new[ComPort].c_cflag |= CLOCAL | CREAD;
    SetDataBit (ComPort, databit);
    SetParityCheck (ComPort, parity);
    SetStopBit (ComPort, stopbit);
    termios_new[ComPort].c_oflag = 0;
    termios_new[ComPort].c_lflag |= 0;
    termios_new[ComPort].c_oflag &= ~OPOST;
    termios_new[ComPort].c_cc[VTIME] = 1;        /* 1/10 second. */
    termios_new[ComPort].c_cc[VMIN] = 1;
    tcflush (fd[ComPort], TCIFLUSH);
    return (tcsetattr (fd[ComPort], TCSANOW, &termios_new[ComPort]));
}
/**
 *波特率转换
 *
 * @param baudrate 波特率值
 *
 * @return 成功-转换后的波特率代码，失败-B9600
 */
static INT32 BAUDRATE (INT32 baudrate)
{
    //plog("baudrate %d\n",baudrate);
    switch (baudrate) {
    case 0:
        return (B0);
    case 50:
        return (B50);
    case 75:
        return (B75);
    case 110:
        return (B110);
    case 134:
        return (B134);
    case 150:
        return (B150);
    case 200:
        return (B200);
    case 300:
        return (B300);
    case 600:
        return (B600);
    case 1200:
        return (B1200);
    case 2400:
        return (B2400);
    case 4800:
        return (B4800);
    case 9600:
        return (B9600);
    case 19200:
        return (B19200);
    case 38400:
        return (B38400);
    case 57600:
        return (B57600);
    case 115200:
        return (B115200);
    default:
        return (B9600);
    }
}



/**
 *
 *计算异或校验
 * @param data 指向数据块的指针
 * @param nbytes 数据块中数据的字节数
 * @note
    计算公式：BCC = DATA(0)^DATA(1)^DATA(2)^...^DATA(nbytes - 1)；

 * @return 返回计算出的异或校验值，如果 data 为空指针或 nbytes 不大于 0，返回 -1
 */
unsigned char cal_bcc(void *data, short int nbytes) {

    short int i = 0;
    unsigned char bcc = 0, *p = (unsigned char *)data;

    if(data == NULL || nbytes <= 0)
        return 0;

    while(i < nbytes)
        bcc = (unsigned char)(bcc ^ (p[i++] & 0xFF));

    return bcc;
} /* cal_bcc */


/**
 *计算和校验
 *
 * @param data 指向数据块的指针
 * @param nbytes 数据块中数据的字节数
 * @note
    SUM = LOWBATE(DATA(0)+DATA(1)+DATA(2)+...+DATA(nbytes - 1))； @n
    其中，LOWBATE 表示的计算是取数值的最低字节

 * @return 返回计算出的和校验值如果 data 为空指针或 nbytes 不大于 0，返回 -1
 */
unsigned char cal_sum(void *data, short int nbytes) {

    short int i = 0;
    unsigned char *p = (unsigned char *)data;
    unsigned long int sum = 0UL;

    if(data == NULL || nbytes <= 0)
        return 0;

    while(i < nbytes)
        sum += (p[i++] & 0xFF);

    return (unsigned char)(sum & (unsigned long int)0xFF);     /* Only get the minimum bytes */
} /* cal_sum */


/**
 *发送一个字节
 *
 * @param com 此处为传递进来的串口号
 * @param byte 一个字节
 * @note
    向串口发送一个字节。在指定时间内没有发送一个的字节是为超时，返回

 * @return
    如果在指定的时间内发送了一个字节返回 SUCCESS，否则返回 @n
    FAILURE。枚举常量定义在 <tt> serial.h </tt> 中
 */
TError put_byte(TCom com, unsigned char byte) {

    TError error;
    //plog("put_byte %02X\n",byte);
    error = WriteComPort((INT32)com, &byte, 1) == 1 ? SUCCESS : FAILURE;
    //plog("put_byte %02X\n",byte);
    return error;
} /* put_byte */


/**
 *接收一个字节
 *
 * @param com 此处为传递进来的串口号
 * @param byte 指向一个字节的指针
 * @note
    从串口接收一个字节。在指定时间内没有接收一个的字节是为超时，返回

 * @return
    如果在指定的时间内接收了一个字节返回 SUCCESS， @n
    否则返回FAILURE;如果 byte 为空，返回 FAILURE。枚举常量定义在 <tt> serial.h </tt>中
 */
TError get_byte(TCom com, unsigned char *byte) {

    TError error;
    int r;
    if(com >= SPI_COM)
    {
        //      error = (API_spi_read(fd[com],(char *)byte ,1) == 1)? SUCCESS : FAILURE;
    }
    else{
        error = ((r = ReadComPort((INT32)com, byte, 1)) == 1) ? SUCCESS : FAILURE;
    }
    return error;
} /* get_byte */

/**
 *发送数据
 *
 * @param com 此处为传递进来的串口号
 * @param data 将要发送的数据
 * @param nbytes 数据的字节数
 * @note
    向串口发送一个数据块。在指定时间内没有发送出指定的字节数是为超时，返回而不管这时数据块是否被发送完毕

 * @return
    所数据块发送完毕返回成功；失败返回 FAILURE 或 TIME_OUT。 @n
    枚举常量定义在 <tt> serial.h</tt> 中
 */
TError do_put_data(TCom com, const unsigned char  *data, short int nbytes) {

    int sended = 0, n;

    assert(data != NULL && nbytes > 0);
    //plog("do_put_data\n");
    while(sended != nbytes) {
        n = WriteComPort((INT32)com, data + sended,
                         nbytes - sended);
        //plog("put_data  %d\n",n);
        if(n == -1)
            return TIME_OUT;
        sended += n;
    }

    return SUCCESS;
} /* do_put_data */


/**
 *接收数据
 *
 * @param com 此处为传递进来的串口号
 * @param data 存放的将要接收的数据的地方
 * @param nbytes 将接收的数据的字节数
 *
 * @return
    读取了指定字节的数据返回 SUCCESS,失败返回 FAILURE 或TIME_OUT. @n
    data 是空指针或nbytes不大于0 @n
    FAILURE枚举常量定义在 <tt> serial.h </tt>中 @n
 */
TError do_get_data(TCom com, void *data, short int nbytes) {

    int received = 0, n=0;

    assert(data != NULL && nbytes > 0);
    if(com >= SPI_COM)
    {
        while(received != nbytes) {
            //n = API_spi_read(fd[com],((char *)data + received),1);
            //           n = ReadComPort((INT32)com, (void *)((char *)data + received),1);
            if(n == -1)
                return TIME_OUT;
            received += 1;
        }
    }
    else{
        //plog("do_get_data %d\n",nbytes);
        while(received != nbytes) {
            n = ReadComPort((INT32)com, (void *)((char *)data + received),
                            nbytes - received);
            // plog("get %d\n",n);
            if(n == -1)
                return TIME_OUT;
            received += n;
        }
    }
    return SUCCESS;
} /* do_get_data */


/**
 * @chinese
 *打开串口
 *
 * @param com 指定串口。串口常量的定义在 <tt> serial.h </tt> 中，
 * @param baud_rate 指定波特率的索引。
 * @note
    数据位、停止位和奇偶校验由函数自动设置。 @n
    它所做的工作是打开串口、设置好定时器和设置通信地址，为串口通讯做好准备工作 @n

 * @return 1 is success and 0 is failure
 * @endchinese
 *
 * @english
 * open serail port
 *
 * @param com specified serial port @see libserial.h
 * @param baud_rate specify baudrate's index
 * @note
    data bit,stop bit, parity bit will be set automatically bu function.@n
   Tthis function just open serial port,setting timer and communication address,preparing for serial communication.@n

 * @return 1 is success and 0 is failure
 * @endenglish
 *
 */
int start_com_io(TCom com, long baud_rate) {

    int wl;
    char sb[4] = {0};
    char od;
    //long int baud;

    switch(_8) {
    case _5: case _6: case _7: case _8:
                wl = _8 + 5;
        break;
    default:
        wl = 8;
    }

    switch(_1) {
    case _1:
        sb[0] = '1';
        break;
    case _1_5:
        sb[0] = '1';
        sb[1] = '.';
        sb[2] = '5';
        break;
    case _2:
        sb[0] = '2';
        break;
    default:
        sb[0] = '1';
        break;
    }

    switch(_none) {
    case _odd:
        od = 'O';
        break;
    case _even:
        od = 'E';
        break;
    case _none:
        od = 'N';
        break;
    default:
        od = 'N';
    }
    //baud = baud_rate_table[baud_rate].baud_rate;
    if(OpenComPort((INT32)com, baud_rate, wl, sb, od) == -1)
        return 0;
    return 1;
} /* start_com_io */


/**
 *关闭串口,结束通讯，关闭串口和销毁计时器
 *
 * @param com 串口号
 */
void end_com_io(TCom com) {

    CloseComPort((INT32)com);
} /* end_com_io */


/*------------------------------------------------------------------------
 * File description£º
 * File name - protocol.c
 *
 * Function list £º
 *        _set_address                - set communication address
 *        _get_address                - get communication address
 *        _set_work_mode              - set work mode
 *        _get_work_mode              - get work mode
 *        _get_package                - get data packet
 *        _get_data                   - get data
 *        _put_data                   - send data
 *        _make_package               - make packet
 *        _analyze_package            - analyze data packet
 *-----------------------------------------------------------------------*/

/* communication between server and terminal £¬source files of terminal */


/** 终端机发送/接收数据的最大数（字节数）*/
enum {_MAXBUFLEN = MAXLENGTH};
/** 自己的通信地址，用来判断这个出据包是不是发送给自己的*/
static char _address[MAXCOM];

/*服务器的通信地址（未用到，不用问为什么）*/
/*static char serveraddress;*/

/*--------------------------------------------------------------------------
 * 工作方式为终端方式和服务器方式。开始字符为 _STY 和 _STX。
 * _begin_putch：发送时使用的开始字符；
 * _begin_getch：接收时使用的开始字符；
 *-------------------------------------------------------------------------*/

unsigned char _begin_putch[MAXCOM], _begin_getch[MAXCOM]; /* default : terminal mode */
static TError _get_package(TCom com,TPackage package, char begin, char end);
static TError _make_package(TPackage package, _TData *data, short int *total);
static int _analyze_package(TPackage package, _TData *data);


/**
 * @chinese
 *设置通信地址
 *
 * @param com 串口号
 * @param address 通信地址
 * @note
    通信地址被规定了最小值和最大值，设置通信地址。通信地址用来标志一个接收者

 * @return 如果设置成功返回非零值，否则返回零值
 * @endchinese
 *
 * @english
 * set communication address
 *
 * @param com serial port number
 * @param address communication address
 * @note
    communication address was specified minimum value and maximum value.@n
    setting communication address to present a `receiver`.

 * @return If setting successfully,return non-zero,else return 0.
 * @endenglish
 *
 */
int _set_address(TCom com ,unsigned char address) {

    return _address[com] = (address == _SERVERADDR || address == _BROADCASTADDR ? 0 : address);
} /* _set_address */

/**
 * @chinese
 *取得通信地址,取得设置好的通信地址
 *
 * @param com 串口号

 * @return 返回通信地址
 * @endchinese
 *
 * @english
 * get address of communication
 *
 * @param com serial port number

 * @return communication address
 * @endenglish
 *
 *
 */
char _get_address(TCom com) {

    return _address[com];
} /* _get_address */


/**
 * @chinese
 *设置工作方式
 *
 * @param com 串口号
 * @param work_mode 工作方式，取值为_TERMINAL 或 _SERVER
 * @endchinese
 *
 * @english
 * set working mode
 *
 * @param com serial port number
 * @param work_mode working mode,value maybe `_TERMINAL` or ` _SERVER`
 * @endenglish
 *
 */
void _set_work_mode(TCom com,TWorkMode work_mode) {

    switch(work_mode) {
    case _TERMINAL:
        _begin_putch[com] = _STY;
        _begin_getch[com] = _STX;
        break;
    case _SERVER:
        _begin_putch[com] = _STX;
        _begin_getch[com] = _STY;
        break;
    default:
        _begin_putch[com] = _STY; /* default : terminal mode */
        _begin_getch[com] = _STX;
        break;
    }
} /* _set_work_mode */


/**
 *取得工作方式
 *
 * @param com 串口号
 *
 * @return 当前工作方式
 */
TWorkMode _get_work_mode(TCom com) {
    return (_begin_putch[com] == _STX && _begin_getch[com] == _STY) ? _SERVER : _TERMINAL;
} /* _get_work_mode */


/**
 * @chinese
 *接收数据包
 * @param com 串口好
 * @param package 用来存放数据包
 * @param begin 表示数据开始的字节
 * @param end 表示数据结束的字节
 * @note
    从串口接收一个数据包；在指定时间内没有接收到指定的字节数是为超时

 * @return
    如果没有接收到 begin，返回 BEGINERROR；如果接收到的地址不是
    自己的地址，返回NOTMYADDRESS；如果没有接收到表示长度的字节，
    返回 LENGTHERROR；如果在接收到指定数目的字节后没有发现 end，
    返回 ENDERROR。
    返回值如下所示：
        超时 接收了指定字节数的字节   返回值      备注
        是       是                   无定义    不可能出现这种情况
        否       是                   SUCCESS  有可能已经接收到有
        是       否                   TIME_OUT 用的数据，怎么处理（失败还是成功）看高层函数的决策
        否       否                   FAILURE
        data 是空指针                 FAILURE
       枚举常量定义在 @see libserial.h 文件
 * @endchinese
 *
 *
 * @english
 * get package from serial port
 * @param com serial port number
 * @param package a package for storing data
 * @param begin begin byte of data
 * @param end  end byte of data
 * @note
    timeout: while during a specified time internal, not receiving a data package from a serial port.

 * @return
    If not received `begin`, return BEGINERROR;
    If received address is not itself address, return NOTMYADDRESS;
    If not received a byte represent the length of data, return LENGTHERROR;
    If not received `end` while after received the length of data,return ENDERROR.

    return value as following:
 *<table>
 <tr>
 <th>timeout</th><th>receiving-specified-length-bytes</th><th>return-value</th><th>remark</th>
 </tr>
 <tr>
 <td>Yes</td><td>Yes</td><td>Not defined</td><td>can't happend</td>
 </tr>
 <tr>
 <td>No</td><td>Yes</td><td>SUCCESS</td><td>probably receiving the data</td>
 </tr>
 <tr>
 <td>Yes</td><td>No</td><td>TIME_OUT</td><td>Using data, how to process(failing or successfully) dependent on higher function's decision</td>
 </tr>
 <tr>
 <td>No</td><td>No</td><td>FAILURE</td><td></td>
 </tr>
 <tr>
 <td>data pointer is NULL</td><td></td><td>FAILURE</td><td></td>
 </tr>
 * </table>
  @see libserial.h file has enum defination.
 * @endenglish
 *
 *
 */
static TError _get_package(TCom com,TPackage package, char begin, char end) {

    short int nbytes;
    unsigned char ch = 0, *p;
    int i=0;

    if(package == NULL)
        return FAILURE;
    lable:
    p = (unsigned char *)package;

    while(1)
    {
        if(get_byte(com,&ch) != SUCCESS)   /* 接收 begin */
            return TIME_OUT;
        //plog("\n\n");
        //plog("begin=%02X\n",ch);
        if(ch != begin)
        {
            if(com >= SPI_COM)
                return TIME_OUT;
            continue;
        }
        if(get_byte(com,&ch) != SUCCESS)   /* 根据协议，这个字节是地址 */
            return TIME_OUT;
        plog("add=%02X,%02X\n",ch,_get_address(com));
        /* 以终端方式工作时，检查是不是本机的地址；以服务器方式作时，不检查地址 */
        if(_get_work_mode(com) == _TERMINAL && ch != _get_address(com))
            return TIME_OUT;
        else break;
    }
    *p++ = ch;                     /* 把地址字节放入 package 的第一个字节 */
    if(get_byte(com,&ch) != SUCCESS)   /* 根据协议，这个字节是数据的字节数 */
        return TIME_OUT;
    plog("bytes=%02X\n",ch);
    *p++ = ch;                     /* 把数据字节数放入 package 的第二个字节 */
    nbytes = (short int)ch + 2;    /* 加 2 是为了接收 BCC 和 SUM 校验位 */

    if(do_get_data(com,p, nbytes) == TIME_OUT) /* 接收数据 */
        return TIME_OUT;
    for(i=0;i<nbytes;i++)
	plog("%02X ",*p++);
    plog("\n");
    if(get_byte(com,&ch) != SUCCESS)  /* 根据协议现在取得的字节值应是 end */
        return TIME_OUT;
    plog("end=%02X\n",ch);
    if(ch != end)
        goto lable;
    plog("1111\n");
    return SUCCESS;
} /* _get_package */


/**
 *发送数据
 *
 * @param com 串口号
 * @param data 指向存贮数据的结构的指针，如果结构里面 nbytes 域 为零，表明没有数据， @n
    itemnu 被自动置 0,忽略 use_data。这个函数首先根据参数建立一个数据包，然后发送出去。
 *
 * @return  若没有发送成功返回 TIME_OUT，否则返回 SUCCESS。如果 data为空指针，返回 FAILURE
 */
TError _put_data(TCom com,_TData *data) {

    TPackage package;
    short int total;
    unsigned char *ptr;
    //    int i;

    ptr=(unsigned char *)data;
    //plog("%02X,%02X,%02X,%02X\n",*ptr++,*ptr++,*ptr++,*ptr++);
    if(data ==NULL || data->nbytes > 253)
        return FAILURE;
    _make_package(package, data, &total);
    //for(i=0;i<total;i++)
    //plog("%02X ",package[i]);
    //plog("\n");
    if(put_byte(com,_begin_putch[com]) == TIME_OUT)  /* 开始通信 */
        return TIME_OUT;
    if(do_put_data(com,(unsigned char *)package, total) == TIME_OUT)
        return TIME_OUT;
    if(put_byte(com,_ETX) != SUCCESS)     /* 结束通信 */
        return TIME_OUT;
    //plog("send ok\n");
    return SUCCESS;
} /* _put_data */


/**
 * @chinese
 * 接收数据
 *
 * @param com 串口号
 * @param data 指向将要存放已接收数据的结构的指针。 @n
    接收完毕时，对数据包进行分析，并根据分析结果对参数中的各参数赋值
 *
 * @return 如果没有正确接收返回代表接收错误的错误代码； @n
            如果分析后发现错误，返回代表分析错误的错误代码； @n
            其它情况下返回SUCCESS。如果 data 为空指针，返回 FAILURE
 * @endchinese
 *
 * @english
 * Get data from serial port
 *
 * @param com serial port
 * @param data a pointer which point to  a palace storing getting data. @n
    when finishing getting data,analyze data package,and assign value to parameter according to analyzing results.
 *
 * @return If not getting successfully,return error code which representing getting error; @n
            If find error while analyzing data,return error code which representing analyzing error; @n
            Other conditions will return SUCCESS.If data is NULL pointer,return FAILURE
 * @endenglish
 *
 */
int _get_data(TCom com,_TData *data) {

    TError error;
    TPackage package;

    if(data == NULL)
        return FAILURE;
    if((error = _get_package(com,package, _begin_getch[com], _ETX)) != SUCCESS)
        return error;

    return _analyze_package(package, data);
} /* _get_data */


/**
 *制作数据包
 *
 * @param package  指向的数据包用指定的数据进行填充
 * @param data 指向存放数据的结构的指针，如果结构里面 nbytes 域 为零， @n
        表明没有数据，itemnu 被自动置 0,忽略 use_data
 * @param total 用来返回总的有效的字节数
 * @note
    它只是对 package 指向的数据包用指定的数据进行填充

 * @return 总是执行成功，返回 SUCCESS。注意：它不检查任何参数的合法性
 */
static TError _make_package(TPackage package, _TData *data, short int *total) {

    unsigned char *p = (unsigned char *)package;

    p[_ADDRESS] = data->address;
    p[_INSTRUCTION] = data->instruction; // 数据块的第三个字节为指令
    if(data->nbytes == 0) {         /* nbytes 为 0，表示没有数据  */
        p[_ITEMNUM] = data->itemnum = 0;
        memset(p + _ITEMNUM + 1, 0, _MAXBUFLEN - 4);
        p[_LENGTH] = 2; /* 2 为指令占用的字节数和数据项数占用字节数之和 */
    }
    else {
        p[_ITEMNUM] = data->itemnum;
        memcpy(p + _ITEMNUM + 1, data->user_data, data->nbytes);
        p[_LENGTH] = data->nbytes + 2; /* 2 为指令占用的字节数和数据项数占用字节数之和 */
    }

    p[p[_LENGTH] + 2] = cal_bcc(p, p[_LENGTH] + 2);
    p[p[_LENGTH] + 3] = cal_sum(p, p[_LENGTH] + 2);
    /* 2：数据包头部有两个字节：address 和 length；
     * 2：指令和数据项数占用字节数之和；
     * 2：BCC、SUM 共占两个字节 */
    *total = 2+ 2 + data->nbytes + 2;
    return SUCCESS;
} /* _make_package */

/**
 *分析数据包
 *
 * @param package 接收到的网络数据包
 * @param data 指向将要存放已接收数据的结构的指针
 * @note
    它首先对 package 指向的数据包的尾部处理，执行下面的操作： @n
            1。计算出 bcc 校验，与 package 中的 bcc 对比，看是否相等  @n
            2。计算出 sum 校验，与 package 中的 sum 对比，看是否相等 @n
            然后，如果上面二步执行正确，对参数指向的变量赋以相应的值。 @n
            注意，这个函数不对参数的合法性做任何检查
 * @return 返回值有以下几种情况：
            bcc 校验正确  sum 校验正确  返回值
               是             是       SUCCESS @n
               是             否       SUMERROR @n
               否             是       BCCERROR @n
               否             否       BCCERROR | SUMERROR @n
               常量的定义请参考 <tt> serial.h </tt>
 */
static int  _analyze_package(TPackage package, _TData *data) {

    //TError error = SUCCESS;
    int error=SUCCESS;
    unsigned char *p = (unsigned char *)package;

    if(p[p[_LENGTH] + 2] != cal_bcc(p, p[_LENGTH] + 2))
        error |= BCCERROR;
    if(p[p[_LENGTH] + 3] != cal_sum(p, p[_LENGTH] + 2))
        error |= SUMERROR;
    if(error != SUCCESS)
        return error;
    /* 向 data 指向的结构中的各个域赋以相应的值 */
    data->instruction = p[_INSTRUCTION];
    data->address = p[_ADDRESS];
    data->nbytes = p[_LENGTH] - 2;
    data->itemnum = p[_ITEMNUM];
    memcpy(data->user_data, p + 4, p[_LENGTH] - 2);
    return SUCCESS;
} /* _analyze_package */

/**
 * @chinese
 *获得串口状态
 *
 * @param ComPort 串口号
 *
 * @return 串口状态
 * @endchinese
 *
 * @english
 * get state of uart
 *
 * @param ComPort uart number
 *
 * @return the state of uart
 * @endenglish
 *
 */
int _get_uart_state(TCom ComPort)
{
    return fd[ComPort]>0;
}
