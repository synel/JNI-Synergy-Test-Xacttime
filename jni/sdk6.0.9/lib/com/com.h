#ifndef RS485_H
#define RS485_H
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../public/protocol.h"
#include "../net/net_tcp.h"


enum {
	ANSWER=0X0,			//Success
	RETIME=0X04,			//Calibrate
	LINEZD=0X40,			//Link
	CLOSE=0X41,			//Disconnect
	NEW_UPNAME_ANSWER=0X47,	//In new ruls:the response to UPNAME is 0x47,and 0x48 is for UPLOAD
	NEW_UPLOAD_ANSWER=0X48,
	DWNAME=0X44,			//download filename
	DWLOAD=0X45,			//download filebody
	DWLOAD_DUP=0X46,
	UPNAME=0X47,			//upload filename
	UPLOAD=0X48,			//upload filename
	UPLOAD_DUP=0X49,
	DELETE_FILE=0X4A,			//Delete file
	APPEND=0X50,			//Add a new record 
	PLAYER=0X51,			//Play audio
	CHECKFP=0X52,		//Check fingerprint templates
	RELAY=0X53,			//Relay opt
};
enum {
	ANSWER_SUCCESS=0,    //Success
	AMSWER_VERIFY_ERR=10,  //Verification Failure
	ANSWER_DIMMEDINSTRUCTION=11, //Valid instruction
	ANSWER_INSTRUCTIONNOTRUN=12,  //command is unable to preform
	ANSWER_LINE_ERR=20,          //Non-normal Link error
	ANSWER_LINEZD_ERR=21         //Linking verification error
};

//typedef char TComPackage[1030];
int linezd_com( _TData *data);         //Connect authentication

void answer_put_data(unsigned char value);  //answer

void new_com_upname(_TData *data);
void com_upload(_TData *data);        //upload data
void com_upload_dup(_TData *data);
void new_com_dwname(_TData *data);      //download data
void com_dwload(_TData *data);       //download data
void com_dwload_dup(_TData *data);
void new_com_delete(_TData *data);//delete file
int com_retime(_TData *data);

//extern TCom com;
//extern int OpenCom(int port,int baudrate,int overtime,int workmode);
 int AcceptLinecom();              //Waiting for connect
 int ComSend();

int com_sendfile();                   //function for serial port communication control
int com_udp_linezd(char *tmpbuff,int datalen);int com_filetofile();
int new_linezd_com(_TData *data);

int new_com_player(_TData *data);
int new_com_gpio(_TData *data);int new_com_checkfp(_TData *data);

//int comm_put_data(TCom com,struct package *data);
//int comm_get_data(TCom com,struct package *data);
#endif
