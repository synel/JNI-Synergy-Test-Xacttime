#ifndef INIT_COM_H
#define INIT_COM_H
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "protocol.h"
#include "../_precomp.h"
#include "../version.h"

#define KEYA		0x01
#define KEYB		0x02
//operation instructions
#define READCARD_KEY			0X80		//read block (with key)
#define READCARD_UNKEY			0X82		//read block (without key)
#define WERITECARD_KEY			0X81		//write block (with key)
#define WERITECARD_UNKEY			0X83	//write block (without key)
#define CLOSE_TERMINAL			0X8F		//shutdown machine

//acknowledge  instructions
//#define WEIGANDCARD				0x70		//wigand card
//#define FP_DEV				0x71		//finger prints

#define KEYBOARD				0X90		//read keypad value
#define READMAGNETIC			0X91		//read magic-card
#define READBARCODE				0X92		//read bar-card
#define READEMCARD  			0X93		//read EM No.
#define READMF1CARD  			0X94		//read MF1 No.
#define READHID				0x9C		//read HID wg data
#define READCARD_BLOCK			0x80		//read block 
//#define READCARD_SUCCESS			0X00		//successive operation

//acknowledge  content	
#define OPT_SUCCESS				0X00		//success
#define OPT_ERROR				0X01		//instruction operation error
#define VALID_ERROR				0X10		//parity error
#define DISCERC_ERROR			0X11		//can not distingish instruction
#define INSTRUCTION_INVALID		0X12		//instruction can not execute

//enum _peripheral_type{pt_reader,pt_printer,pt_wireless,pt_computer};
//enum _peripheral_model{pm_r001,pm_r002,pm_r003,pm_r004,pm_r005,pm_s310s,pm_em310,pm_m35,pm_x86};
//pm_r001 MF1
//pm_r002 EM
//pm_r003 BarCode
//pm_r004 Magnetic
//pm_r005 Hid

typedef struct _com_INFO
{
	//int peripheral_type; //add by aduo 2013.7.18
	//int peripheral_model;//add by aduo 2013.7.18
	int type;		//type 0 uart 1 spi  add by aduo 2013.7.4
	int enable;		//enable or not
	int baudrate;	//baud rate
	int overtime;	//overtime
	int workmode;	//work mode
}COMINFO;

extern TCom serialport0,serialport1,serialport2,serialport3;
extern int comovertime;
extern _TData com1value;		//used for reading card

int opencomm(int mode,int baud);//used for test

int OpenCom(IN COMINFO *com_info);
int ReadCom1(void);
void UnCom(int comnum);


int Set_Machine_Mode(IN int mode);

int mifs_request();
int mifs_read(char *cardsno,char Sector, char Block, unsigned char *Data, char mode,char *key);
int mifs_write(char *cardsno,char Sector, char Block, unsigned char *Data, char mode,char *key);

//int read_mac_com(char * mac);
//int write_mac_com(char *mac);
int get_keyboardVer(char *ver);
int set_track(int track);

#endif
