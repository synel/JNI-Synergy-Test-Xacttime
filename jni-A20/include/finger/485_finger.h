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
#include <sched.h>
#include "libserial.h"
#include "public.h"
#include "_fp.h"
#include "arm9_finger.h"
#include "stringchar.h"

typedef struct __ZW485_INFO
{
  int fd;
  unsigned char DevId; //
  unsigned char HostId;
  int isconnect; // 1-connect ,
  int setfingerok;

}_ZW485_INFO;

extern _ZW485_INFO ZW485_INFO;


#define COMM_CRYPT_KEY				0x9BD2FC17
#define FPREADER_COUNT			4	// 4个读头
union
{
	unsigned char vRandKeyArray[4];
	unsigned int vRandKey;
} vCryptKey;

typedef struct
{
	BYTE byID;
	BOOL bTouchUse;
} READER_INFO;


#define RETRY_SEARCHING_CONTROLLER	10
#define RETRY_SEARCHING_READER		1	// 2

#define RCOM_RES_CMD				1
#define RCOM_RES_ACK				2
#define RCOM_RES_NAK				3

#define RCOM_ACK_TIMEOUT_SHORT		100
#define RCOM_ACK_TIMEOUT_MEDIUM		300
#define RCOM_ACK_TIMEOUT_LONG		1000
#define RCOM_ACK_TIMEOUT_VERYLONG	3000

#define RCOM_A30C_ID				105		// 主机地址
#define RCOM_A30R_INVALID_ID		255

#define CMD_FINISH_MARK		(-1UL)
#define CMD_FINISH_MARK1	(-2UL)


enum
{
	RCMD_AR1003_PREPARE = 100,
	RCMD_AR1003_ROLLCALL,
	RCMD_AR1003_CONNECT,
	RCMD_AR1003_ALIVE,
	RCMD_AR1003_DISCONNECT,
	RCMD_AR1003_FP_SLAVE_SEND,
	RCMD_AR1003_FP_TOUCH_SET,
	RCMD_AR1003_DEV_ID_SET,
	RCMD_AR1003_FIND_DEV,
	RCMD_AR1003_DEV_ID_CHANGE,
	RCMD_AR1003_FIRMWARE_UPDATE,
	RCMD_AR1003_TAMPER_REMOVE,
	RCMD_AR1003_INIT_SET,
	RCMD_AR1003_INIT_GET,
	RCMD_AR1003_CLEAR_DEV_ID,
	RCMD_AR1003_TEST,
	RCMD_AR1003_DISABLE_DEVICE,
	RCMD_AR1003_REBOOT_DEVICE,
};

typedef struct
{
	BYTE	Head1;
	BYTE	Head2;
	BYTE	destMID;
	BYTE	Code;
	DWORD	dwData;
	BYTE	srcMID;
	BYTE	ChkSum;
} RCOMPKT;

//_RCOMPKT RCOMPKT;

#define RCOMPKTSIZE					(sizeof(RCOMPKT)-2)

#define MAX_USART_FIFO_SIZE			2048

typedef struct
{
	unsigned short	FFOutOffset;
	unsigned short	FFInOffset;
	BYTE	FIFO[MAX_USART_FIFO_SIZE];
}_UsartRxFIFO;

_UsartRxFIFO UsartRxFIFO;//[FPREADER_COUNT];

unsigned char g_TempBuffer[1404];

#define STX1 0x55
#define STX2 0xAA
#define STX3 0x5A
#define STX4 0xA5

#define rcom_sendcmd(byMID, byCode, dwData)		rcom_sendpacket(STX1, STX2, byMID, byCode, dwData)
#define rcom_sendresp(byMID, byCode, dwData) 	rcom_sendpacket(STX2, STX1, byMID, byCode, dwData)

#define KEY_BUF_MODINC(Offset)						((Offset + 1) % MAX_USART_FIFO_SIZE)
#define BLOCK_SIZE 1020

enum
{
	COM_UI_VERIFY_FP = 2,
	COM_UI_VERIFY_CARD,
};
void rcom_updateReaderList(BOOL bForce);

int init_ComFingerPrinter(int com); //初始化串口指纹仪
long Com_FP_One2NMatch( ); //一比N比对指纹

#endif
