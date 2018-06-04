/**
 * @file   public.h
 * @author 刘训
 * @date   Wed Jul 13 10:05:32 2011
 *
 * @brief
 *
 *
 */
#ifndef PUBLIC__H
#define PUBLIC__H
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/time.h>

#define JREC_FILE

extern char tmp_file[128],send_file_path[128];
extern char *cardRuleFileName;
extern int communication_type,communication_over_time,connect_success_flag;  //通讯方式,超时时间
/** 枚举常量 终端机通讯方式定义*/
enum {
    NET_SERVER = 0, /**<以太网通讯方式  */
    //RS485,/**<RS485/RS232通讯方式  */
    //GPRS,/**<GPRS通讯方式  */
};

int  GetCount(char *format,char starter);

int gsmString2Bytes(const char* pSrc, unsigned char* pDst, int nSrcLength);
int gsmBytes2String(unsigned char* pSrc, char* pDst, int nSrcLength);

int read_terminal_number(char *param);
char *read_mac_config();

int FormatDateTime(const char * format,char *result,time_t the_time);
char *getkernelinfo();
char *getcpuinfo();
char *getmeminfo();
int pid_get(char *current_process);
int killp(int pid);
int SHA1(char *iData, char *oData);
unsigned short int crc16_check(const void *in, unsigned int len, int append, unsigned short int crc);
void init_daemon();
void sync_fclose(FILE *file);
void printftime( char *fun );
long GetTickCount( );

unsigned short int CalcCRC16( unsigned char *in, unsigned int len, int append );
unsigned short CheckCRCModBus(unsigned char * pDataIn, int iLenIn);
int CalcCRC8( unsigned char *buf, int flag );
unsigned long FNVHash1(char * data,int len);

int sdk_sync();


int RelativeTimer(struct timespec refertime, long int microff);
int DifferentTimer(struct timespec refertime);

void LeeLog( char *iStrData, unsigned char *iHexData, unsigned int dataLen, char flag );


/*
 获取文件的MD5值
 */

int MD5_file(char *path,int size,char *Str_md5);



#endif


