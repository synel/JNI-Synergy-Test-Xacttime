#ifndef __STRINGCHAR_H
#define __STRINGCHAR_H

/**
 * @chinese
 * @file   stringchar.c
 * @author 胡俊远
 * @date   Thu Oct 13 10:00:00 2011
 *
 * @brief  字符串和char处理模块
 * @endchinese
 *
 * @english
 * @file   stringchar.c
 * @author Hu Junyuan
 * @date   Thu Oct 13 10:00:00 2011
 *
 * @brief string and char handling module
 * @endenglish
 */
#include<stdlib.h>
#include <string.h>
#include <stdio.h>
void cut_rn(char *buf);
int string_get_item_with_sep(char *src, char *sep, int index, char *dest, int dest_len);
int hex2string(unsigned char* pSrc, char* pDst, int nSrcLength);
int string2hex(char* pSrc, char* pDst, int nSrcLength);
void print_hex(char * data, int len);
void printf_none(char *format, ...);
long GetTickCount( );
unsigned int HexToUInt(void *in, unsigned int size, int flag);
int UIntToHex(unsigned int in, void * out, unsigned int size, int flag);
int get_sdk_version(char *data);
int get_library_version(char *data);

int StrToBCD(const char *Src,char *Des,int iDesLen);
int BCDToStr(const char *Src, char *Des);
int thex2bcd(const char *in, unsigned char *out, int outLen);
int bcd2thex(const unsigned char *in, char *out, int len);

int str_replace(char *p_result,char* p_source,char* p_seach,char *p_repstr);

extern int PLATFORM_TYPE ;  // 硬件平台类型, 0-A20, 1-RK3288，2-M186,3-A83 gorui,4-335x
#define PLATFORM_TYPE_MAX 10 // lfg edit to 10

#if 1 // lfg 20160128 add

struct TTime
{
        unsigned int year;
        unsigned int month;
        unsigned int day;
        unsigned int hour;
        unsigned int minute;
        unsigned int second;
};

#define TIME20000101 (946684800L); //949276800;//2000ū1?1?,0?0ī

int TimeToUInt(struct TTime *in, unsigned int *out, unsigned int RefYear, unsigned int UnitSec);
int IntToHex(int in, void * out, unsigned int size, int flag);
int HexToInt(void *in, unsigned int size, int flag);
int getDay(unsigned int in, unsigned int RefYear, unsigned int UnitSec);
int getMonth(unsigned int in, unsigned int RefYear, unsigned int UnitSec);
int getYear(unsigned int in, unsigned int RefYear, unsigned int UnitSec);

int BcdToTHex(const unsigned char *in, char *out, int len);
int THexToBcd(const char *in, unsigned char *out, int outLen);
unsigned int atoUInt(char * str, int len);
int isdec(char * str, int len);
int TimeStrToUInt(char *in, unsigned int *out, unsigned int RefYear, unsigned int UnitSec, char *format);
int BcdToUInt(unsigned char * in, unsigned int *out, unsigned int size);
int UIntToTimeStr(unsigned int in, char *out, unsigned int RefYear, unsigned int UnitSec, char *format);
int isLeapYear(unsigned int year);
int ExtUIntToBit(unsigned int in, void * out, unsigned int ByteOffset, unsigned int BitOffset, unsigned int UnitLen, unsigned int MainLen);
unsigned int ExtBitToUInt(void *in, unsigned int ByteOffset, unsigned int BitOffset, unsigned int UnitLen, unsigned int MainLen);
int UIntToBit(unsigned int in, void *out, unsigned int ByteOffset, unsigned int BitOffset, unsigned int BitLen);
unsigned int BitToUInt(void * in, unsigned int ByteOffset, unsigned  int BitOffset, unsigned int BitLen);
int UIntToHex(unsigned int in, void * out, unsigned int size, int flag);
unsigned int HexToUInt(void *in,  unsigned int size, int flag);
int BcdToHex(const unsigned char *in, unsigned char *out, int len);
int HexToBcd(const unsigned char *in, unsigned char *out, int len);
int THexToHex(const char * in, unsigned char * out, int outMaxLen);
int HexToTHex(const unsigned char * in, char * out, int len, char fillchar);
int UIntToBcd(unsigned int in, unsigned char * out, unsigned int size);
//int LeeDivideStr(char *In, char Out[][64], unsigned int oSize, char Sep);
char inputNumber(char* prompt, int defValue);
int cutSpaceChar(const char* pSrc, unsigned char* pDst);
int stringToBytes(const char* pSrc, unsigned char* pDst, int nSrcLength);
int create_multi_dir(const char *path);

void readini2( FILE *file, char *group, char *member, char *values );



#endif
#endif
