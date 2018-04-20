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

enum {_USERDATALEN = 253};
enum {MAXLENGTH = 259};

typedef enum _TError {SUCCESS = 0x7F00, TIME_OUT, FAILURE, NOTMYADDRESS,
                      BEGINERROR, ENDERROR, BCCERROR, SUMERROR, LENGTHERROR,
                      UNKNOWNERROR
                     } TError;
typedef enum _TCom {COM1, COM2, COM3, COM4, COM5, COM6, COM7, COM8, COM9} TCom;

/* 
 *Server call/Terminal response mode is used for serial communication.
 */

/*-------------------------------------*
 * sign        value     description
 * TER_STX	   0xAA   	 call to communicate
 * TER_STY	   0xAB	  	 acknowledge to communicate
 * TER_ETX	   0xA5	  	 communicatin end
 * ADD		          		 receive address
 * LEN		          		 packet size=N+1
 * BCC		          		 Xor-parity 
 * SUM		          		 SUM-parity
 *------------------------------------*/
  /*call start,acknowledge start, communication end*/
enum {_STX = 0xAA, _STY = 0xAB, _ETX = 0xA5};

/*-------------------------------------*
 * Server address ADD£½0£¬Terminal address ADD£½1~254£¬ADD=255 is broadcast address£¬no response
 * BCC= ADD^LEN^DATA[0]^DATA[1]^¡­^DATA[N]  	Xor-parity
 * SUM= ADD+LEN+DATA[0]+DATA[1]+¡­+DATA[N]		SUM-parity£¨high-bytes are ignored£©
 *------------------------------------*/
/* Terminal addresses are between SERVERADD and BROADCASTADD */
enum {_SERVERADDR = 0x00, _BROADCASTADDR = 0xFF};

/* 
 * diagram show the structure of packet transfered between terminal and server£º
 *                                   |¡û----- packet -----¡ú|
 * +----------------------------------------------------------------------+
 * |ADD | LEN | INSTRUCTION |ITEMNUM |USER DATA(MAX 253 BYTES)| BCC | SUM |
 * +----------------------------------------------------------------------+
 *            |¡û-------- 255 bytes£¨TOTALLENGTH£©-----------¡ú|
 */ 

enum {_ADDRESS, _LENGTH, _INSTRUCTION, _ITEMNUM};

/* work mode£ºterminal or server */
typedef enum _TWorkMode {_TERMINAL, _SERVER} TWorkMode;

/* data packet */
typedef char TPackage[MAXLENGTH];

#define MAXCOM 20
extern unsigned char _begin_putch[MAXCOM], _begin_getch[MAXCOM]; /* default : terminal mode */

typedef struct __TData {
    unsigned char address;     /* address of receive£¨server£©/send£¨terminal£© */
    unsigned char nbytes;      /* the count of data in user_data */
   unsigned char instruction; /* instruction to receive or send */
  unsigned  char itemnum;     /* the count of logic-independance data units */
  unsigned  char user_data[_USERDATALEN + 1]; /* user_data */
} _TData;


int start_com_io(TCom com, long baud_rate);
void end_com_io(TCom com);
void ComPort_Clear(int ComPort);

int _set_address(unsigned char address);
char _get_address(void);

void _set_work_mode(TCom com,TWorkMode work_mode) ;
TWorkMode _get_work_mode(TCom com);

TError _put_data(TCom com,_TData *data);
int _get_data(TCom com,_TData *data);

//add by aduo 2013.7.22
//protocol for synergy
/*
 * diagram show the structure of packet transfered between terminal and server£º
 *                   |¡û----- packet -----¡ú|
 * +----------------------------------------------------------------------+
 * |ADD | LEN(H) | LEN(L) |  USER DATA(MAX 1024 BYTES)| BCC | SUM |
 * +----------------------------------------------------------------------+
 *                        |¡û-1024 bytes£¨TOTALLENGTH£©¡ú|
 */
//<!--
typedef char TComPackage[1030];
#define NET_DATALEN 992
struct package{
	char command[30];
	short datalen;
	char data[992];
};

int comm_put_data(TCom com,struct package *data);
int comm_get_data(TCom com,struct package *data);
//-->

#endif
