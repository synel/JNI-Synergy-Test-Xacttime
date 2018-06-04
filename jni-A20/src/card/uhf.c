/*
 * uhf.c
 *
 *  Created on: 2014-5-5
 *      Author: aduo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "serial.h"
#include "config.h"
#include <pthread.h>

#include "serial/uartlib.h"


//multi tag
static unsigned char data[500*12];//缓存读到的所有卡号
static int data_len = 0;//缓存读到的所有卡号的长度
static int recv_rfid(int uart_port,unsigned char *package)
{
	int card_len = 12;//每个卡号的长度

	int i = 0,k = 0,count=0;

	unsigned char ch = 0;
	int number_of_bytes_to_read = 0,len = 0;
	unsigned char buf[500*12];
	int epc_tag = 0;

	//printf("start recv %d data\r\n",uart_port);
	memset(buf,0,sizeof(buf));
	while(1){
		//soi
		len = serial_recv_data(uart_port, &ch, 1);
		//printf("soi len %d\r\n",len);
		if (len <= 0){
			break;
		}

		//printf("%02X\r\n",ch); //0xcc
		if (ch != 0xCC){
			break;
		}
        //addr
		len = serial_recv_data(uart_port, &ch, 1);//0xff
		if (len <= 0){
			break;
		}
		//printf("%02X\r\n",ch);
		len = serial_recv_data(uart_port, &ch, 1);//0xff
		if (len <= 0){
			break;
		}
		//printf("%02X\r\n",ch);
        //cid1
		len = serial_recv_data(uart_port, &ch, 1);//0x11
		if (len <= 0){
			break;
		}
		//printf("%02X\r\n",ch);
		if (ch == 0x10){//single tag
			epc_tag = 0;
		}else
		if (ch == 0x11){//multi tag
			epc_tag = 1;
		}else{
			break;
		}
        //cid2
		len = serial_recv_data(uart_port, &ch, 1);//0x32
		if (len <= 0){
			break;
		}
		if (ch != 0x32){
			break;
		}
		//printf("%02X\r\n",ch);
		//length
		if (epc_tag == 0){ //single tag
		  len = serial_recv_data(uart_port, &ch, 1);
			if (len <= 0){
				break;
			}
			number_of_bytes_to_read = ch;
		  //printf("%02X\r\n",ch);
		}else if (epc_tag == 1){ //multi tag
		  len = serial_recv_data(uart_port, &ch, 1);
		  if (len <= 0){
				break;
			}
		  //printf("%02X\r\n",ch); //0x01
		  count = ch;
		  //printf("count %d\r\n",count);
		  len = serial_recv_data(uart_port, &ch, 1);
		  if (len <= 0){
				break;
			}
		  number_of_bytes_to_read = count * 14;
		  //printf("%02X\r\n",ch); //0x0E
		}

		//number_of_bytes_to_read = count * 14;
		//printf("length %d\r\n",number_of_bytes_to_read);
		//info
		if (number_of_bytes_to_read > sizeof(buf)) {
		  len = serial_recv_data(uart_port, buf, sizeof(buf));
		}else{
		  len = serial_recv_data(uart_port, buf, number_of_bytes_to_read);
		}
		if (len <= 0){
			break;
		}
		if (len != number_of_bytes_to_read){
			break;
		}
		//printf("len = %d\r\n",len);
		if (epc_tag == 0){ //single tag
			for (i = 0;i < len - 1; i++){
				//if (i ==0 ){
				//	printf("sub addr %2X\r\n",buf[i]);
				//}
				if (data_len < sizeof(data)){
				  data[data_len] = buf[i + 1];
				  data_len++;
				}
			}
		}else if (epc_tag == 1){//multi tag
			count = len / 14;
			for (k = 0; k < count;k++){
			  //printf("the %d one\r\n",k+1);
			  for (i = 0;i < card_len; i++){
				  //if (i ==0 ){
				  //	printf("sub addr %2X\r\n",buf[i]);
				  //}
				  if (data_len < sizeof(data)){
				  data[data_len] = buf[i + 1];
				  //printf("%d,%02X\r\n",data_len,data[data_len]);
				  data_len++;
				  }
			  }
			  //printf("sub check %2X\r\n",buf[13]);
			  memmove(buf,buf + (k + 1) * 14,len - (k + 1) * 14);
			}
		}

		//check
		len = serial_recv_data(uart_port, &ch, 1);
		//if (len <= 0){
		//	return -1;
		//}
		break;
	}

	if (data_len >= card_len){

		//printf("data_len %d,%d\r\n",data_len,card_len);
/*
		memcpy(package,data,card_len);

		memmove(data,data + card_len,data_len - card_len);

		data_len -= card_len;
*/
		number_of_bytes_to_read = data_len;
		memcpy(package,data,data_len);
		memset(data,0,sizeof(data));
		data_len = 0;

		return number_of_bytes_to_read;
	}

	return 0;
}

//返回多张卡，以逗号分隔
int read_uhf_card(int uart_port,char *value){
	unsigned char package[500*12];
	unsigned short datalen = 0;
	int ret = 0, i = 0;
    int fd = -1;

    fd = get_uard_fd(uart_port);
    if(fd <= 0)
    {
    	//printf("port open fail\r\n");
        return ERROR;
    }

    memset(package,0,sizeof(package));
	ret = recv_rfid(uart_port, package);
	//printf("recv card no len %d\r\n",ret);
	if (ret <= 0) {
		return FALSE;
	}
/*
	datalen = ret - 9;
	for (i = 0; i < datalen; i++)
		sprintf(value + i * 2, "%02X", package[i]);
*/
	datalen = ret;
	//printf("datelen %d\r\n",datalen);
	for (i = 0;i < datalen; i++){
		sprintf(value + i * 2, "%02X", package[i]);
		if ((i+1) % 12 == 0){
			strcat(value,",");
			value++;
		}
	}


	return TRUE;
}


// 广西大学门禁扩展板支持,485波特率 57600

static int mjext_fd = -1;

static unsigned char mjOutStat = 0x00;
static unsigned char mjInStat = 0x00;

unsigned int mj_stat[5][3];	// 5路继电器的状态,value,delay,curtime

static int mj_ext_init()
{
	int ret = -1;
	int num = 0;
	unsigned char cmd[8]={0xAA,0x00,0x02,0x0C,0x00,0x00,0x00,0xA5};		// 0x0C
	
    if(mjext_fd <= 0)
    {
        goto ERR;
    }    

	cmd[4]=mjOutStat;
	cmd[5]=cmd[1]^cmd[2]^cmd[3]^cmd[4];
	cmd[6]=cmd[1]+cmd[2]+cmd[3]+cmd[4];
	
	num = UartWrite(mjext_fd, cmd, sizeof(cmd));
	if(num < 0)
	{		
		return -1;
	}
	
	memset(mj_stat, 0, sizeof(mj_stat));
//	printf("mj_ext_init OK\n\n");
	return 0;
	
ERR:
	
//	printf("mj_ext_init Fail......\n\n");
	return -1;
	
}


// value: 4路输入状态, 0x0F每一位代表一路的状态(0开关按下或1开关断开) ,取值0~15

int mj_ext_event(int port, char* value)
{
	unsigned char rBuf[8];
	unsigned char cmd = 0x00;
	unsigned char stat = 0x00;
	unsigned char *p = NULL;
	int num = 0, len = 0, cnt = 0;
	static int doflag = 0;
    
    if(mjext_fd <= 0)
    {
	    mjext_fd = get_uard_fd(port);
        if(mjext_fd <= 0)
        {
			return -1;
		}
    }    

	if(doflag == 0)
	{
		if(mj_ext_init() == 0)
		{
			doflag = 1;
		}
	}
	
	p = rBuf;
REDO:
	num = UartRead(mjext_fd, p, 1, 50);	// read head
	if(num != 1)
	{//printf("======== Not get head char =======\n");
		return -1;
	}
	
	if(*p != 0xAA)
	{
		//printf("%02X-1\n", *p);
		if(cnt++ > 256)
		{
			return -1;
		}		
		goto REDO;
	}
	p++;
	
	num = UartRead(mjext_fd, p, 7, 30);	// read left 7 bytes
	if(num != 7)
	{
		return -1;
	}

	if(rBuf[5]==(rBuf[1]^rBuf[2]^rBuf[3]^rBuf[4]))
	{
		if(rBuf[6]==(rBuf[1]+rBuf[2]+rBuf[3]+rBuf[4]))	
		{
			cmd = rBuf[3];
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}

	switch(cmd)
	{
		case 0x0A:		// 扩展板每次上电后，都会主动上送该指令	{0xAA,0x00,0x02,0x0A,0x00,0x08,0x0C,0xA5}
			doflag = 0;
			return -1;
			break;
		case 0x0B:		// 输入状态变化后，上送，否则不上送  (((IN1?1:0)<<0)|((IN2?1:0)<<1)|((IN3?1:0)<<2)|((IN4?1:0)<<3))
			stat = rBuf[4];
			if(stat == mjInStat)
			{
				return -1;
			}
			mjInStat = stat;
			sprintf(value, "%d", stat);
//			printf("in state = %s\n", value);
			break;
		default:
			return -1;
			break;
	}
	
	return 0;
}

// 5路继电器设置 {RL1=(x>>0)&0x01;RL2=(x>>1)&0x01;RL3=(x>>2)&0x01;RL4=(x>>3)&0x01;RL5=(x>>4)&0x01;}
// index: 继电器索引
// value :0或1
int mj_ext_set(unsigned int index, unsigned int value)
{
	int ret = -1;
	int num = 0;
	unsigned char cmd[8]={0xAA,0x00,0x02,0x0C,0x00,0x00,0x00,0xA5}; 	// 0x0C	 
	unsigned char stat = 0x00;
	if(mjext_fd <= 0 || index < 1 || index > 5 || value > 1)
	{
		goto ERR;
	}			

	stat = (mjOutStat & (~(1<<(index-1)))) + (value<<(index -1));

	cmd[4] = stat;
	cmd[5] = cmd[1]^cmd[2]^cmd[3]^cmd[4];
	cmd[6] = cmd[1]+cmd[2]+cmd[3]+cmd[4];
	
	num = UartWrite(mjext_fd, cmd, sizeof(cmd));
	if(num < 0)
	{		
		return -1;
	}

	mjOutStat = stat;
//	printf("mj_ext_set OK-1\n");
	return 0;
	
ERR:
	
//	printf("mj_ext_set Fail-1\n");
	return -1;
	

}

extern pthread_mutex_t	gpio_mutex;

// 扩展接口
// delay: 延时时间，单位ms， 0-表示无超时，>0,则超过该时间，继电器状态变更
int mj_ext_set_ex(unsigned int index, unsigned int value, unsigned int delay)
{
	int ret = -1;
	int num = 0;
	unsigned char cmd[8]={0xAA,0x00,0x02,0x0C,0x00,0x00,0x00,0xA5}; 	// 0x0C	 
	unsigned char stat = 0x00;int i;

	if(mjext_fd <= 0 || index < 1 || index > 5 || value > 1)
	{
		goto ERR;
	}			

	stat = (mjOutStat & (~(1<<(index-1)))) + (value<<(index -1));

	cmd[4] = stat;
	cmd[5] = cmd[1]^cmd[2]^cmd[3]^cmd[4];
	cmd[6] = cmd[1]+cmd[2]+cmd[3]+cmd[4];
	
	num = UartWrite(mjext_fd, cmd, sizeof(cmd));
	if(num < 0)
	{		
		return -1;
	}

	mjOutStat = stat;
	if(delay > 0)
	{
		pthread_mutex_lock( &gpio_mutex );
		mj_stat[index-1][0] = value;
		mj_stat[index-1][1] = delay;
		mj_stat[index-1][2] = GetTickCount();
		pthread_mutex_unlock( &gpio_mutex );		
	}
//	for(i = 0; i < 5; i++)
//	{
//		printf("mj_ext_set_ex = index = %d, value = %d, delay = %d, time = %d\n", i, mj_stat[i][0], mj_stat[i][1], mj_stat[i][2]);

//	}

//		printf("index = %d,%d,%d,%d\n", index, mj_stat[index-1][0], mj_stat[index-1][1], mj_stat[index-1][2]);
	
//	printf("mj_ext_set OK-2\n");
	return 0;
	
ERR:
	
//	printf("mj_ext_set Fail-2\n");
	return -1;
	

}




// 915M HR系列 超高频读写器 西安合芯 20170420

static int devfd = -1;

static unsigned char CheckBcc(unsigned char *Data, int DataSize)  
{  
    unsigned char DataBcc = 0;  
    int i = 0;
    for(i=0; i<DataSize; i++) 
    {  
        DataBcc^=Data[i];  
    }  
    return DataBcc;  
}



// cmd : pc-->header
// oData: 应答的有效数据
// oLen: 应答的有效数据长度
// oTime: 读取应答数据的首字节需要等待的时间，单位 ms
static int GetResponseData(int fd, unsigned char *cmd, unsigned char *oData, unsigned int *oLen, unsigned int oTime)
{
	unsigned char rBuf[1024];
	unsigned char Bcc = 0;
	unsigned char *p = NULL;
	int num = 0, len = 0, cnt = 0;
	
	p = rBuf;
REDO:
	num = UartRead(fd, p, 1, oTime);	// read head
	if(num != 1)
	{//printf("======== Not get head char =======\n");
		return -1;
	}
	
	if(*p != cmd[0])
	{
		//printf("%02X-1\n", *p);
		if(cnt++ > 256)
		{
			return -1;
		}		
		goto REDO;
	}
	p++;
	
	num = UartRead(fd, p, 1, 30);	// read cmd
	if(num != 1)
	{
		return -1;
	}
	
	if( *p != cmd[1])
	{//printf("%02X-2\n", *p);
		if(cnt++ > 256)
		{
			return -1;
		}
		goto REDO;
	}
	p++;
	
	num = UartRead(fd, p, 2, 30);	// read sequence: 0x01, opcode: 0xff
	if(num != 2)
	{
		return -1;
	}
	if(*p != 0x01)
	{//printf("%02X-3\n", *p);
		if(cnt++ > 256)
		{
			return -1;
		}
		goto REDO;
	}
	p += 2;

	num = UartRead(fd, p, 2, 30);	// read len
	if(num != 2)
	{
		return -1;
	}		
	len = *p + *(p+1) * 256;
	p += 2;	
	
	num = UartRead(fd, p, len + 1, 30);
	if(num != len + 1)
	{
		return -1;
	}
	
	Bcc = CheckBcc(p, len);
	if(Bcc != *(p + len))
	{
		return -1;
	}	

//	dbgShowHexData(rBuf, len+7, 1, '<', "RECV");
	
	memset(oData, 0, sizeof(oData));
	memcpy(oData, p, len);
	*oLen = len;	
	
	return 0;
}


// -->1B 02 00 00 00 00 00
// <--1B 02 01 FF 07 00 56 35 2E 30 34 31 52 2A 

static int CmdGetDevVersion(int fd)
{
	unsigned char cmd[] = {0x1B, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00};
	unsigned char oData[128];
	unsigned int oLen = 0;
	unsigned int oTime = 0;
	int num = 0, ret = -1;
	
	num = UartWrite(fd, cmd, sizeof(cmd));
	if(num < 0)
	{		
		return -1;
	}
	
	memset(oData, 0, sizeof(oData));

	oTime = 350; // ms
	ret = GetResponseData(fd, cmd, oData, &oLen, oTime);
	if(ret != 0)
	{
		return -1;
	}
	
	if(oLen == 1 && *oData == 0x80)
	{
		printf("CmdGetDevVersion Err\n");
		return 1;
	}
	else if(oLen > 0)
	{
		printf("Ver: %s\n", oData);
	}
	
	return 0;
}

static int CmdFreedReader(int fd)
{
	unsigned char cmd[] = {0x1B, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
	unsigned char oData[128];
	unsigned int oLen = 0;
	unsigned int oTime = 0;
	int num = 0, ret = -1;
	
	num = UartWrite(fd, cmd, sizeof(cmd));
	if(num < 0)
	{		
		return -1;
	}
	
	oTime = 350; // ms
	ret = GetResponseData(fd, cmd, oData, &oLen, oTime);
	if(ret != 0)
	{
		return -1;
	}
	
	if(oLen == 1 && *oData == 0x80)
	{
		printf("CmdFreedReader Err\n");
		return 1;
	}
	else if(oLen > 0)
	{
//		dbgShowHexData(oData, oLen, 1, '<', "RECV");
	}
	
	return 0;
}

// 获取用户配置
static int CmdGetUserConfig(int fd, unsigned char *cfg)
{
	unsigned char cmd[] = {0x1B, 0x1B, 0x00, 0x00, 0x00, 0x00, 0x00};
	unsigned char oData[128];
	unsigned int oLen = 0;
	unsigned int oTime = 0;
	int num = 0, ret = -1;
	
	num = UartWrite(fd, cmd, sizeof(cmd));
	if(num < 0)
	{		
		return -1;
	}
	
	oTime = 350; // ms
	ret = GetResponseData(fd, cmd, oData, &oLen, oTime);
	if(ret != 0)
	{
		return -1;
	}
	
	if(oLen == 1 && *oData == 0x80)
	{
		printf("CmdGetUserConfig Err\n");
		return 1;
	}
	else if(oLen == 64)
	{
//		dbgShowHexData(oData, oLen, 1, '<', "RECV");
		memcpy(cfg, oData, oLen);
	}
	else
	{
		return -1;
	}
	
	return 0;
}

// 设置用户配置
static int CmdSetUserConfig(int fd, unsigned char *cfg)
{
	unsigned char cmd[71] = {0x1B, 0x1A, 0x00, 0x00, 0x40, 0x00, 0x00};
	unsigned char oData[128];
	unsigned int oLen = 0;
	unsigned int oTime = 0;
	int num = 0, ret = -1;
	unsigned int Bcc = 0;
	
	unsigned char *p  = cmd;
	p += 6;	
	memcpy(p, cfg, 64);
	
	Bcc = CheckBcc(p, 64);
	cmd[70] = Bcc;
    
	num = UartWrite(fd, cmd, sizeof(cmd));
	if(num < 0)
	{		
		return -1;
	}
	
	oTime = 350; // ms
	ret = GetResponseData(fd, cmd, oData, &oLen, oTime);
	if(ret != 0)
	{
		return -1;
	}
	
	if(oLen == 1 && *oData == 0x80)
	{
		printf("CmdSetUserConfig Err\n");
		return 1;
	}
	else if(oLen > 0)
	{
//		dbgShowHexData(oData, oLen, 1, '<', "RECV");
	}
	
	return 0;
}

// 获取RF天线配置
static int CmdGetRFantConfig(int fd, unsigned char *cfg)
{
	unsigned char cmd[] = {0x1B, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00};
	unsigned char oData[128];
	unsigned int oLen = 0;	
	unsigned int oTime = 0;
	int num = 0, ret = -1;
	
	num = UartWrite(fd, cmd, sizeof(cmd));
	if(num < 0)
	{		
		return -1;
	}

	oTime = 350; // ms
	ret = GetResponseData(fd, cmd, oData, &oLen, oTime);
	if(ret != 0)
	{
		return -1;
	}
	
	if(oLen == 1 && *oData == 0x80)
	{
		printf("CmdGetRFantConfig Err\n");
		return 1;
	}
	else if(oLen == 64)
	{
//		dbgShowHexData(oData, oLen, 1, '<', "RECV");
		memcpy(cfg, oData, oLen);
	}
	else
	{
		return -1;
	}
	
	return 0;
}


// 设置RF天线配置参数
// 该指令至少需要等等3s，才能返回结果，其它指令基本在300ms就可以返回结果
static int CmdSetRFantConfig(int fd, unsigned char *cfg)
{
	unsigned char cmd[71] = {0x1B, 0x33, 0x00, 0x00, 0x40, 0x00, 0x00};
	unsigned char oData[128];
	unsigned int oLen = 0;	
	unsigned int oTime = 0;
	int num = 0, ret = -1;
	unsigned int Bcc = 0;
	
	unsigned char *p  = cmd;
	p += 6;	
	memcpy(p, cfg, 64);
	
	Bcc = CheckBcc(p, 64);
	cmd[70] = Bcc;
    
	num = UartWrite(fd, cmd, sizeof(cmd));
	if(num < 0)
	{		
		return -1;
	}
	
	oTime = 3500; // ms
	ret = GetResponseData(fd, cmd, oData, &oLen, oTime);
	if(ret != 0)
	{
		return -1;
	}
	
	if(oLen == 1 && *oData == 0x80)
	{
		printf("CmdSetRFantConfig Err\n");
		return 1;
	}
	else if(oLen > 0)
	{
//		dbgShowHexData(oData, oLen, 1, '<', "RECV");
	}
	
	return 0;
}

// 写标签指定数据
// iData:
// Data [0] bank + Data [1] Offset + Data [2] Len + Data [3~6] Password+ Data [7] Antenna + Data [8~x] TagData
// iLen: 输入数据长度
static int _cmdWriteTagData(int fd, unsigned char *iData, unsigned int iLen)
{
	unsigned char cmd[128] = {0x1B, 0x36, 0x00, 0x00, 0x00, 0x00};
	unsigned char oData[128];
	unsigned int oLen = 0;	
	unsigned int oTime = 0;
	int num = 0, ret = -1;
	unsigned int Bcc = 0;
	
	unsigned char *p  = cmd;
	p += 6;	
	memcpy(p, iData, iLen);

	cmd[4] = iLen & 0xFF;
	cmd[5] = (iLen >> 8) & 0xFF;
	
	Bcc = CheckBcc(p, iLen);
	cmd[iLen + 6] = Bcc;
    
	num = UartWrite(fd, cmd, iLen + 7);
	if(num < 0)
	{		
		return -1;
	}
	
	oTime = 500; // ms
	ret = GetResponseData(fd, cmd, oData, &oLen, oTime);
	if(ret != 0)
	{
		return -1;
	}
	
	if(oLen == 1 && *oData == 0x80)
	{
		printf("CmdSetRFantConfig Err\n");
		return 1;
	}
	else if(oLen > 0)
	{
//		dbgShowHexData(oData, oLen, 1, '<', "RECV");
	}
	
	return 0;
	
}

static int CmdWriteTagData(int fd)
{
	unsigned char iData[64] = "";
	unsigned int iLen = 0;	

	unsigned char bank = 1;		// 数据块，0-3，Reserve、EPC、TID  及 User
	unsigned char offSet = 0;	// 写入数据的起始位置，单位是word
	unsigned short int len = 4;	// 需要写入的数据长度，单位是word
	unsigned char pwd[4] = {0x00, 0x00, 0x00, 0x00};
	unsigned char ant = 0;	// 天线，0-15
	unsigned char TagData[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};

	unsigned char *p = iData;

	*p = bank; p += 1; iLen++;
	*p = offSet; p += 1; iLen++;
	*p = len & 0xFF00; 
	*(p + 1) = len & 0xFF; 
	p += 2; iLen += 2;
	memcpy(p, pwd, sizeof(pwd));
	p += sizeof(pwd); iLen += sizeof(pwd);
	*p = ant; p += 1; iLen++;
	memcpy(p, TagData, sizeof(TagData));
	p += sizeof(TagData); iLen += sizeof(TagData);
    
	return _cmdWriteTagData(fd, iData, iLen);
	
}


// 915读头初始化
// 不同指令之间至少间隔300ms
int HR915M_Init(int port)
{
	int ret = -1;
	unsigned char cfg[64] = "";
	int num = 5;

// 默认用户配置参数
	unsigned char DefUserCfg[64] =
	{
		0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x02,0x00, 
		0x00,0x0B,0x00,0x0C,0x00,0x01,0x0A,0x01,0x08,0x0A,0x0F,0x00, 
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0xFF,0x00,0x00,0x00, 
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
		0x00,0x00,0x00,0x00
	};	

// 默认天线配置参数，启用0~3四路天线
	unsigned char DefFantCfg[64] =
	{
		0x01,0x1E,0x02,0x01,0x1E,0x02,0x01,0x1E,0x02,0x01,0x1E,0x02, 
		0x00,0x1E,0x02,0x00,0x1E,0x02,0x00,0x1E,0x02,0x00,0x1E,0x02, 
		0x00,0x1E,0x02,0x00,0x1E,0x02,0x00,0x1E,0x02,0x00,0x1E,0x02, 
		0x00,0x1E,0x02,0x00,0x1E,0x02,0x00,0x1E,0x02,0x00,0x1E,0x02, 
		0x03,0x00,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
		0x00,0x00,0x00,0x00
	};

	unsigned char *p = NULL;
	p = DefUserCfg;
	*(p + 4) = 2;	 // 同张标签数据暂存时间间隔低位 单位 *s ，重复刷卡间隔
	*(p + 5) = 0;	 // 同张标签数据暂存时间间隔低位 单位 *s ，重复刷卡间隔
	*(p + 6) = 0;	 // 循环盘存标签时间间隔 * 100ms
	*(p + 7) = 0;   // 读标签数据块(0-读 EPC 数据,1-读 TID数据,2-读 EPC和 TID数据), TID是唯一编码，但识别距离要近，无遮挡大约7m远
	*(p + 8) = 0;	// 工作模式 定时	
	*(p + 10) = 2;	// 数据输出端口设置 485
	*(p + 32) = 0xFF;	// 485 设备地址
    
//	tcflush( devfd, TCIOFLUSH );
    
    devfd = get_uard_fd(port);
    if(devfd <= 0)
    {
        goto ERR;
    }    
	
	usleep(400000); // must 20170506		usleep(350000);
	ret = CmdGetDevVersion(devfd);	
	if(ret != 0)
	{
		goto ERR;
	}	

	usleep(400000);
	memset(cfg, 0, sizeof(cfg));
	ret = CmdGetUserConfig(devfd, cfg);
	if(ret == 0)
	{		
		if(memcmp(cfg, DefUserCfg, sizeof(DefUserCfg)) != 0)
		{		
			printf("Reset User Config\n");
			usleep(400000);
			ret = CmdSetUserConfig(devfd, DefUserCfg);
			if(ret != 0)
			{
				goto ERR;
			}		
		}
	}
	else
	{
		goto ERR;
	}

	usleep(400000);
	memset(cfg, 0, sizeof(cfg));
	ret = CmdGetRFantConfig(devfd, cfg);
	if(ret == 0)
	{
		if(memcmp(cfg, DefFantCfg, sizeof(DefFantCfg)) != 0)
		{
			printf("Reset RFant Config\n");
			usleep(400000);
			ret = CmdSetRFantConfig(devfd, DefFantCfg);
			if(ret != 0)
			{
				goto ERR;
			}
		}
	}
	else
	{
		goto ERR;
	}

	usleep(400000);	
	ret = CmdFreedReader(devfd);	
	if(ret != 0)
	{
		goto ERR;
	}
	
	printf("HR915 init OK\n\n");
	return 0;
	
ERR:
	
	printf("HR915 init Fail......\n\n");
	return -1;
	
}


// 读取EPC 卡号数据, 卡号倒序
// <--00 00 00 00 00 00 00 00 FB E6 E1 5B
// 实际卡号为: 5B E1 E6 FB


// 定时模式寻卡
// <--1B 39 01 FF 03 00 00 00 00 00 
//    1B 39 01 FF 03 00 FF FF 00 00 
int HR915MGetCard_TimeMode(char *cards)
{
	unsigned char cmd[] = {0x1B, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00};
	unsigned char oData[1024];
	unsigned char phyCard[4];
	unsigned char tmp =0x00;
	unsigned int oLen = 0;
	unsigned int oTime = 0;
	int ret = -1;
	
	oTime = 50; // ms
	ret = GetResponseData(devfd, cmd, oData, &oLen, oTime);	
	if(ret != 0)
	{
		return -1;
	}
	
	if(oLen == 3)
	{
		if(oData[0] == 0xFF && oData[1] == oData[0])
		{
//			printf("All RF Over, ch=%d\n", oData[2]);
		}
		else
		{
//			printf("Cur RF Over, ch=%d, cnt=%d\n", oData[2], oData[0]+oData[1]*256);
		}
	}
	else
	{
//		dbgShowHexData(oData, oLen, 1, '<', "RECV");

//   倒序后，此卡号与IC卡号相符
		phyCard[0] = *(oData + 15);
		phyCard[1] = *(oData + 14);
		phyCard[2] = *(oData + 13);
		phyCard[3] = *(oData + 12);
		gsmBytes2String(phyCard, cards, 4);
		return 4;
	}
	
	return 0;
}


















