
#ifndef __DEVICE_H
#define __DEVICE_H

#include "serial.h"


/**
 * @chinese
 * @file   device.c
 * @author 刘训
 * @date   Thu Oct 13 10:00:00 2011
 *
 * @brief  设备通信协议处理模块
 * @endchinese
 *
 * @english
 * @file   device.c
 * @author Liu Xun
 * @date   Thu Oct 13 10:00:00 2011
 *
 * @brief  protocol between device handling module
 * @endenglish
 */

#define FEEDBACK			0X00                                /**< 反馈信息 */
#define FEEDBACK_SUCCESS	0X00                                /**< 成功 */
#define FEEDBACK_ERROR		0X01                                /**< 失败 */

#define TRANSIT			0x31                                    /**< 中转*/

#define PSAMCMD			0x32                                    // psam

#define GET_VERSION		0X33                                    /**< 获取单片机版本号*/
#define UPDATEUARTCARD	0X3A                                    /**<更新串口卡头*/
#define BACKLIGHT_T		0X40                                    /**< 设置键盘背光时间*/
#define LOGOBACKLIGHT	0X41                                    /**< 设置LOGO 背光时间*/
#define LOGOBACKLIGHT_V	0X42                                    /**< 设置LOGO 背光时间,竖屏版本*/


#define REBOOT_P 0xFE                                           /**< 重启 */

#define LIGHT_CMD	0x0D                                        /**< 短按开关机键，mcu发出背光控制指令，应用层关闭或打开背光*/
#define REBOOT_CMD	0x0E                                        /**< 重启: 按住关机键3s，mcu发送重启终端指令，并开启5s重启计时 */
#define CONTROL_P	0X0F /**< 控制终端机操作 关机*/    // 无外接适配器
#define CONTROL_PL	0X1F                                        /// 电池电压过低关机

#define RK3288_GET_IO_STAT	0x70			// 主动获取IO当前状态, 但返回的应答使用 0x8E指令码

#define MF1_READ_KEY		0X80                                /**< read block (with key) */
#define MF1_READ_UNKEY		0X82                                /**< read block (without key) */
#define MF1_WERITE_KEY		0X81                                /**< write block (with key) */
#define MF1_WERITE_UNKEY	0X83                                /**< write block (without key) */

#define WIEGAND_CARD	0X88                                    /**< wiegand card*/
#define RK3288_GPIO_OUT	0x8D

#define RK3288_GPIO_IN		0x8E
#define WATCHDOG		0X8F                                    /**< shutdown machine*/

#define LIGHT_SENSE	0X8A                                    /**< 光感*/
#define HUMAN_SENSE	0X8B                                    /**< 人体感应*/


#define SEARCH_CARD_IC	0x61                                    /* 主动寻卡+复位卡头*/
#define SEARCH_CARD		0x62                                    /* 主动寻CPU卡号+返回复位信息*/
#define WIEGANDCARD		0x70                                    /**< wigand card */
#define FP_DEV			0x71                                    /**< finger prints */

#define KEYBOARD		0X90                                    /**< read keypad value */
#define READMAGNETIC	0X91                                    /**< read magic-card */
#define READBARCODE		0X92                                    /**< read bar-card */
#define READEMCARD		0X93                                    /**< read EM No. */
#define MF1_S50_CARDNO	0X94                                    /**< read S50 MF1 No. */
#define CPU_CARDNO		0X95                                    /**< cpu card. */

#define WIEGANDINPUT 0X97 /**< wiegand input */                 // 以维根的方式获取的HID 20170703

#define MF1_S70_CARDNO	0X9A                                    /**< read S70 MF1 No. */
#define ID_CARDNO		0X9B                                    /**< 身份证 No. */
#define READHIDCARD		0X9c                                    /**< read HID card No.*/

#define READPF 0x9D                                             // iclass card 20170703

#define READCARD_BLOCK		0X80                                /**< read block*/
#define BATTERY_INFO		0X8C                                /**< Battery info */
#define READCARD_SUCCESS	0X00                                /**< successive operation */
#define SPI_TEST			0XB0
#define SERIAL_TEST			0XB1

#define RK3288_MAXCNT		6		// 缓存数据数量 暂定最多支持 6

int device_recv_RK3288_buf_data(TDATA	*otdata );

int device_recv_data( int port, char *value );
int rk3288_device_recv_data( int port, TDATA	*otdata );


int device_send_data( int port, char address, char instruction, char itemnum, char * data, int data_len );


int mf1_recv_data( int port, char sector, char block, char *data, char mode, char *key );


int mf1_send_data( int port, char sector, char block, char *data, char mode, char *key );


int machine_reboot( int port );


int mf1_recv_data_bk( int port, char sector, char block, char *data, char mode, char *key, int outtime );


int mf1_recv_data_bk( int port, char sector, char block, char *data, char mode, char *key, int outtime );


extern int tongdao_itemnum;
extern char	card_number [];
extern int	track_number;


typedef struct __RK3288_DATABUF
{
	TDATA tdata;
	struct __RK3288_DATABUF *next;
}_RK3288_DATABUF;

extern _RK3288_DATABUF *RK3288_HEAD;

extern unsigned int RK3288cnt;	// 缓存数据数量 暂定最多支持 6

int rk3288_psam_apdu_data( unsigned char* data, int bytes, unsigned char *obuf );

extern int RK3288_51port;

void freeRK3288Buf();


#endif

