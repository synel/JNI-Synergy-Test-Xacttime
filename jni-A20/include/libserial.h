/**
 * @file   libserial.h
 * @author 刘训
 * @date   Wed Jul 13 09:55:16 2011
 *
 * @brief
 *
 */
#ifndef _LIBSERIAL_H
#define _LIBSERIAL_H 1
#include <sys/time.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <termios.h>            /* tcgetattr, tcsetattr */
#include <stdio.h>              /* perror, printf, puts, fprintf, fputs */
#include <unistd.h>             /* read, write, close */
#include <fcntl.h>              /* open */
#include <sys/signal.h>
#include <sys/types.h>
#include <string.h>             /* bzero, memcpy */
#include <limits.h>             /* CHAR_MAX */
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ioctl.h>

typedef enum _TStatus {STOP = 0x01, RUN = 0x02} TStatus;
typedef struct _TTimer {
    double begin, end;
    TStatus status;  /**< 取值 STOP 或者 RUN */
    double trigger_ticks; /**< 超时触发值 */
} TTimer;

/* 用户数据缓冲的最大字节数*/

typedef enum _TWordLength {
    _5,   /**< 5 位数据 */
    _6,   /**< 6 位数据 */
    _7,   /**< 7 位数据 */
    _8    /**< 8 位数据 */
} TWordLength;

typedef enum _TParity {
    _odd,
    _even,
    _none
} TParity;

typedef enum _TStopBits {
    _1,
    _1_5,
    _2
} TStopBits;


//typedef enum _TStatus {STOP = 0x01, RUN = 0x02} TStatus;
enum {PRECISION = 1000000};

double get_time_out_trigger(TTimer *timer);
int stop_timer(TTimer *timer);
int end_timer(TTimer *timer);
double time_used(TTimer *timer);

enum {_USERDATALEN = 253};
typedef enum _TError {SUCCESS = 0x7F00, TIME_OUT, FAILURE, NOTMYADDRESS,
                      BEGINERROR, ENDERROR, BCCERROR, SUMERROR, LENGTHERROR,
                      UNKNOWNERROR
                     } TError;
typedef enum _TCom {COM1=0, COM2, COM3, COM4, COM5, COM6, COM7, COM8,SPI_COM=10} TCom;

/**
* @chinese
* @brief  串口通讯数据包定义
* @endchinese
*
* @english
* @brief  define serial communication data package
* @endenglish
*
*/
typedef struct __TData {
    unsigned char address;     /**< 接收（服务器）/发送（终端）端地址*/
    unsigned char nbytes;      /**< user_data 里数据的字节数 */
   unsigned char instruction; /**< 接收/发送的指令 */
  unsigned  char itemnum;     /**< user_data 里面逻辑上独立的数据单位的个数 */
  unsigned  char user_data[_USERDATALEN + 1]; /**< 用户数据 */
} _TData;

int start_com_io(TCom com, long baud_rate);
void end_com_io(TCom com);
void ComPort_Clear(int ComPort);

int _set_address(TCom com, unsigned char address);
char _get_address(TCom com);
TError _put_data(TCom com,_TData *data);
int _get_data(TCom com,_TData *data);
int set_time_out_trigger(TTimer *timer, double trigger_seconds);
int start_timer(TTimer *timer);
int is_time_out(TTimer *timer);



enum {MAXLENGTH = 259};

/* data packet */
typedef char TPackage[MAXLENGTH];

unsigned char cal_bcc(void *data, short int nbytes);
unsigned char cal_sum(void *data, short int nbytes);
TError put_byte(TCom com, unsigned char byte);
TError get_byte(TCom com, unsigned char *byte);
TError do_put_data(TCom com, const unsigned char  *data, short int nbytes);
TError do_get_data(TCom com, void *data, short int nbytes);

typedef int     LONG;
//typedef unsigned short WORD;
//typedef unsigned int DWORD;
//typedef unsigned char BYTE;
typedef char    TCHAR;

typedef int     INT32;
typedef short   INT16;
//typedef char    INT8;
typedef unsigned int UNIT32;
typedef unsigned short UINT16;
typedef unsigned char UINT8;

enum {MAXCOM = 20};
INT32    fd[MAXCOM];
extern unsigned char _begin_putch[MAXCOM], _begin_getch[MAXCOM]; /* default : terminal mode */

/* serial.c */
INT32 OpenComPort (INT32 ComPort, INT32 baudrate, INT32 databit,
                   const char *stopbit, char parity);
void CloseComPort (INT32 ComPort);
INT32 check_uart_received(INT32 ComPort);
INT32 ReadComPort (INT32 ComPort, void *data, INT32 datalength);
INT32 ReadComPort2 (INT32 ComPort, void *data, INT32 datalength);

INT32 WriteComPort (INT32 ComPort, const UINT8 * data, INT32 datalength);
INT32 SendFile (INT32 ComPort, const char *pathname, INT32 echo);



/* 串行通讯采用服务器呼叫/终端机应答方式，每次服务器呼叫某编号终端机，
 * 该终端机收到呼叫后应答，但广播指令不应答
 */

/*-------------------------------------*
 * 字符        数值     定义
 * TER_STX	   0xAA   呼叫通讯开始
 * TER_STY	   0xAB	  应答通讯开始
 * TER_ETX	   0xA5	  通讯结束
 * ADD		          接收地址
 * LEN		          数据包大小=N+1
 * BCC		          数据异或校验
 * SUM		          数据和校验
 *------------------------------------*/
  /* 呼叫通讯开始 应答通讯开始 通讯结束*/
enum {_STX = 0xAA, _STY = 0xAB, _ETX = 0xA5};

/*-------------------------------------*
 * 服务器地址ADD＝0，终端机地址ADD＝1~254，ADD=255为广播地址，不应答
 * BCC= ADD^LEN^DATA[0]^DATA[1]^…^DATA[N]  	异或校验
 * SUM= ADD+LEN+DATA[0]+DATA[1]+…+DATA[N]		和校验（高字节略去）
 *------------------------------------*/
/* 在 SERVERADD 和 BROADCASTADD 之间的地址为终端机地址 */
enum {_SERVERADDR = 0x00, _BROADCASTADDR = 0xFF};


/*
 * 终端机与服务机通信数据包存诸结构简图：
 *                                   |←----- 一项数据 -----→|
 * +----------------------------------------------------------------------+
 * |ADD | LEN | INSTRUCTION |ITEMNUM |USER DATA(MAX 253 BYTES)| BCC | SUM |
 * +----------------------------------------------------------------------+
 *            |←-------- 255 字节（TOTALLENGTH）-----------→|
 */
enum {_ADDRESS, _LENGTH, _INSTRUCTION, _ITEMNUM};

/* 工作方式类型方式：终端方式或服务器方式 */
typedef enum _TWorkMode {_TERMINAL, _SERVER} TWorkMode;
void _set_work_mode(TCom com,TWorkMode work_mode) ;
TWorkMode _get_work_mode(TCom com);

int _get_uart_state(TCom ComPort);
#endif
