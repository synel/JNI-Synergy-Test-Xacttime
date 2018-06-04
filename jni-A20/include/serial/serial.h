#ifndef __SERIAL_H
#define __SERIAL_H

/**
 * @chinese
 * @file   serial.c
 * @author 刘训
 * @date   Thu Oct 13 10:00:00 2011
 *
 * @brief  串口底层协议处理模块
 * 卡头作为服务器:接受到0XAB，发送0XAA
 * RS485/RS232作为客户端
 * @endchinese
 *
 * @english
 * @file   serial.c
 * @author Liu Xun
 * @date   Thu Oct 13 10:00:00 2011
 *
 * @brief  serial protocol handling module
 * @endenglish
 */

/*串口最大数量*/
#define MAXCOM 20
/*SPI设备起始位置*/
#define SPI_OFFSET 10

/* 在 SERVERADD 和 BROADCASTADD 之间的地址为终端机地址 */
#define SERVERADDR 0x00
#define BROADCASTADDR 0xFF

/*定义数据包错误信息*/
#define TIME_OUT ERROR-1
#define BEGINERROR ERROR-2
#define ENDERROR ERROR-3
#define BCCERROR ERROR-4
#define SUMERROR ERROR-5
#define LENGTHERROR ERROR-6
#define ADDRERROR ERROR-7
#define UNKNOWNERROR ERROR-8

/*串口传输数据的开始结束标志位*/
#define STX 0xAA
#define STY 0xAB
#define ETX 0xA5

/*一包数据最大长度*/
#define MAXLENGTH 259
/*有效数据最大长度*/
#define USERDATALEN 253

/*
 * 终端机与服务机通信数据包存诸结构简图：
 *                                   |←------ 有效数据 -------→|
 * +----------------------------------------------------------------------+
 * |ADD | LEN | INSTRUCTION |ITEMNUM |USER DATA(MAX 253 BYTES)| BCC | SUM |
 * +----------------------------------------------------------------------+
 *            |←-------------------- len --------------------→|
 * |←------------------ 255 字节(协议数据)---------------------→|
 * |←----------------------- 259 字节(原始数据包)--------------------------→|
 */
/*原始数据包*/
typedef char TPACKAGE[MAXLENGTH];
typedef struct __RECVBUFF{
    char package[MAXLENGTH+10];
    int len;
}RECVBUFF;
RECVBUFF recvbuff_fd[MAXCOM];//增加接收缓存区为每个接口

/*原始数据包中前四项数据的地址*/
enum {P_ADDRESS=0, P_LENGTH, P_INSTRUCTION, P_ITEMNUM};
extern int serial_fd[MAXCOM];
/*协议数据包*/
typedef struct __TDATA
{
    unsigned char address;     /**< 接收（服务器）/发送（终端）端地址*/
    unsigned char nbytes;      /**< user_data+itemnum+instruction 里数据的字节数 */
    unsigned char instruction; /**< 接收/发送的指令 */
    unsigned char itemnum;     /**< user_data 里面逻辑上独立的数据单位的个数 */
    unsigned char user_data[USERDATALEN + 1]; /**< 用户数据 */
} TDATA;

/*串口定义，SPI串口号+10*/
typedef enum __TCOM{COM1=0, COM2, COM3, COM4, COM5, COM6, COM7, COM8, SPI_COM=10} TCOM;

/*工作方式类型方式：终端方式或服务器方式 */
typedef enum __TWORK_MODE {TERMINAL, SERVER} TWORK_MODE;

/*定义发送数据包还是接收数据包*/
typedef enum __ACTION {RECV_PACKAGE, SEND_PACKAGE} ACTION;

int serial_package_processor(TCOM com, TDATA *tdata, ACTION action);
int _serial_package_processor_async( TCOM com, TDATA *tdata, ACTION action );
int serial_init(int port, int baudrate,  int workmode,  int address);
int serial_close(TCOM com);
void serial_set_work_mode(TCOM com,TWORK_MODE work_mode);
TWORK_MODE serial_get_work_mode(TCOM com);
void serial_set_address(TCOM com ,unsigned char address);
char serial_get_address(TCOM com);
int serial_recv_package(TCOM com, TPACKAGE package, char begin, char end);
int serial_send_package(TCOM com, TPACKAGE package, int len);
int serial_analyze_package(TPACKAGE package, TDATA *tdata);
int serial_make_package(TPACKAGE package, TDATA *tdata);
unsigned char serial_cal_bcc(unsigned char *data, short int nbytes);
unsigned char serial_cal_sum(unsigned char *data, short int nbytes);
int serial_recv_data(TCOM com, unsigned char *data, int len);
int serial_recv_data_all(TCOM com, unsigned char *data);
int serial_send_data(TCOM com, char *data, int len);
int serial_recv_onebyte(TCOM com, char * byte);
int serial_send_onebyte(TCOM com, char byte);
int serial_clear(TCOM com);
int is_uart(TCOM com);
int is_spi(TCOM com);
int get_uard_fd(TCOM com);
int check_uart_cache(TCOM com);


#endif
