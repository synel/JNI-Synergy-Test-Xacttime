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
#include <sys/times.h>
#include <sys/time.h>
#include <unistd.h>
#ifndef _ARM_A23
#include <execinfo.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>
 #include <sys/types.h>
#include <errno.h>
#include "config.h"
#include "debug.h"
struct timeval oldtime;
int PLATFORM_TYPE = 0;  // 硬件平台类型, 0-A20, 1-RK3288，2-M186,3-A83 gorui，4-3352 psam
char  *PLATFORM_TYPE_NAME[PLATFORM_TYPE_MAX+1]={
                            "A20",
                            "XHRK3288",
                            "3QM186",
                            "GRA83",
                            "Unknown"
                            };

/**
 * @chinese
 * 截取字符串末尾的"\r\n"
 *
 * @param buf 要处理的字符串
 *
 * @return 无
 * @endchinese
 *
 * @english
 * remove "\r\n" from end of string
 *
 * @param buf  string to handle
 *
 * @return none
 * @endenglish
 *
 */
void cut_rn(char *buf)
{
    if(buf==NULL) return;
    while(*buf)
    {
        if(*buf=='\n'||*buf=='\r')
        {
            *buf=0;
            return;
        }
        else
        {
            buf++;
        }
    }
}

/**
 * @chinese
 * 根据分隔符sep，将src中的第index列字符传给dest
 * 如分隔符为","，src为"aaa,bb,ccccc,,d,"，则第0列为aaa，第1列为bb，第3列为空
 * 注意：分隔符长度为1
 *
 * @param src 源字符串
 * @param sep 分隔符
 * @param index 第几列
 * @param dest 保存目标地址
 * @param dest_len 目标地址长度
 *
 * @return 第index列字符串长度
 * @endchinese
 *
 * @english
 * get the index string from src according to sep
 *
 * @param src src
 * @param sep sep
 * @param index index
 * @param dest dest
 * @param dest_len dest lenth
 *
 * @return len of dest
 * @endenglish
 *
 */
int string_get_item_with_sep(char *src, char *sep, int index, char *dest, int dest_len)
{
    int len_tmp=0,len=0,retlen=0;
    int i=0;

    if(src == NULL ||dest== NULL || strlen(src) == 0||sep == NULL)
    {
        return ERROR;
    }
    for(i=0;i<=index;i++)
    {
        len_tmp=strcspn(src+len, sep);
        if(len>=strlen(src))
        {
            break;
        }

        if(i==index)
        {
            memset(dest,0,dest_len);
            retlen=dest_len>len_tmp?len_tmp:dest_len;
            memcpy(dest,src+len,retlen);
        }

        len=len+len_tmp+1;
    }
    return retlen;
}

/**
 * @chinese
 * 字节数据转换为可打印字符串
 * 如：{0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01} --> "C8329BFD0E01"
 *
 * @param pSrc 源数据指针
 * @param pDst 目标字符串指针
 * @param nSrcLength 源数据长度
 *
 * @return 目标字符串长度
 * @endchinese
 *
 * @english
 * change byte data string to printable string.
 * for example：{0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01} --> "C8329BFD0E01"
 *
 * @param pSrc pointer which point to source string
 * @param pDst pointer which point to  resulting string.
 * @param nSrcLength the length of source data
 *
 * @return length of the resulting string

 * @endenglish
 *
 */
int hex2string(unsigned char* pSrc, char* pDst, int nSrcLength)
{
    const char tab[]="0123456789ABCDEF";//0x0-0xf的字符查找表
    int i=0;

    for(i=0;i<nSrcLength;i++)
    {
        *pDst++ = tab[*pSrc >> 4];	//输出高4位
        *pDst++ = tab[*pSrc & 0x0f];// 输出低4位
        pSrc++;
    }
    *pDst = '\0';

    return (nSrcLength * 2);
}

/**
 * @chinese
 * 可打印字符串转换为字节数据
 * 如："C8329BFD0E01" --> {0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01}
 *
 * @param pSrc 源数据指针
 * @param pDst 目标字符串指针
 * @param nSrcLength 源数据长度
 *
 * @return 目标字符串长度
 * @endchinese
 *
 * @english
 * change byte printable string to data string
 * for example："C8329BFD0E01" --> {0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01}
 *
 * @param pSrc pointer which point to source string
 * @param pDst pointer which point to  resulting string.
 * @param nSrcLength the length of source data
 *
 * @return length of the resulting string

 * @endenglish
 *
 */
int string2hex(char* pSrc, char* pDst, int nSrcLength)
{
    int i=0;
    for (i=0;i<nSrcLength;i+=2)
    {
        //输出高4位
        if((*pSrc>='0')&&(*pSrc<='9'))
        {
            *pDst=(*pSrc-'0')<<4;
        }
        else
        {
            *pDst=(*pSrc-'A'+10)<<4;
        }
        pSrc++;

        // 输出低4位
        if((*pSrc>='0')&&(*pSrc<='9'))
        {
            *pDst|=*pSrc-'0';
        }
        else
        {
            *pDst|=(*pSrc-'A'+10);
        }
        pSrc++;
        pDst++;
    }
    return (nSrcLength/2);
}

/**
 * @chinese
 * 打印字符的十六进制
 *
 * @param data 字符
 * @param len 字符长度
 *
 * @return none
 * @endchinese
 *
 * @english
 *
 * @param data data
 * @param len len of data
 *
 * @return none
 *
 * @endenglish
 *
 */
void print_hex(char * data, int len)
{
    int i=0;

    if(len<=0)
        return;

    plog("\r\n---------------------\r\n");
    for(i=0;i<len;i++)
    {
        plog("%02X ",data[i]);
    }
    plog("\r\n---------------------\r\n");
}

void printf_none(char *format, ...)
{
    return;
}


long GetTickCount( )
{
    long tck;
    struct tms tmp;

    tck = sysconf( _SC_CLK_TCK );

//    return times( NULL ) * ( 1000 / tck );
    return times( &tmp ) * ( 1000 / tck ); //modify by aduo 2014.9.11

}

//函数功能 1到4字节的HEX数值数据转换为无符号整型
//参数：   in：输入首址
//         size: 输入的字符数，若超过4，则取4
//         flag: 输入的主机字节序，0：小端模式 其它：大端模式 (输出固定为小端模式)
//返回值   转换结果
unsigned int HexToUInt(void *in, unsigned int size, int flag)
{
    unsigned int ret = 0;
    unsigned char * str = (unsigned char *)&ret;
    unsigned char * indata = (unsigned char *)in;

    if (size > 4)
    {
        size = 4;
    }

    if (flag)
    {
        switch (size)
        {
        case 4:
            *(str + 0) = *(indata + 3);
            *(str + 1) = *(indata + 2);
            *(str + 2) = *(indata + 1);
            *(str + 3) = *(indata + 0);

            break;
        case 3:
            *(str + 0) = *(indata + 2);
            *(str + 1) = *(indata + 1);
            *(str + 2) = *(indata + 0);

            break;
        case 2:
            *(str + 0) = *(indata + 1);
            *(str + 1) = *(indata + 0);

            break;
        case 1:
            *(str + 0) = *(indata + 0);

            break;
        default:
            break;
        }
    }
    else
    {
        switch (size)
        {
        case 4:
            *(str + 3) = *(indata + 3);
        case 3:
            *(str + 2) = *(indata + 2);
        case 2:
            *(str + 1) = *(indata + 1);
        case 1:
            *(str + 0) = *(indata + 0);

            break;
        default:
            break;
        }
    }

    return ret;

}

//函数功能：将小端模式的无符号整数转换为任何字节序的HEX格式
//参数：   in：输入无符号整数，小端模式（逆序）
//         out:输出首址
//         size:输入的字节数，1、2、3、4
//         flag:输出主机字节序，0：小端模式 其它：大端模式 (输入固定为小端模式)
//返回值   0：转换成功，-1：输出位数不足
int UIntToHex(unsigned int in, void * out, unsigned int size, int flag)
{
    int ret = 0;
    unsigned char * str = (unsigned char *)out;
    unsigned char * indata = (unsigned char *)&in;

    if (size > 4)
    {
        size = 4;
    }

    if (flag)
    {
        switch (size)
        {
        case 4:
            *(str + 3) = *(indata + 0);
            *(str + 2) = *(indata + 1);
            *(str + 1) = *(indata + 2);
            *(str + 0) = *(indata + 3);
            break;
        case 3:
            *(str + 2) = *(indata + 0);
            *(str + 1) = *(indata + 1);
            *(str + 0) = *(indata + 2);
            break;
        case 2:
            *(str + 1) = *(indata + 0);
            *(str + 0) = *(indata + 1);
            break;
        case 1:
            *(str + 0) = *(indata + 0);

            break;
        default:
            break;
        }
    }
    else
    {
        switch (size)
        {
        case 4:
            *(str + 3) = *(indata + 3);
        case 3:
            *(str + 2) = *(indata + 2);
        case 2:
            *(str + 1) = *(indata + 1);
        case 1:
            *(str + 0) = *(indata + 0);

            break;
        default:
            break;
        }
    }

    switch (size)
    {
    case 1:
        if (in > 0x000000FF)
        {
            ret = -1;
        }
        break;
    case 2:
        if (in > 0x0000FFFF)
        {
            ret = -1;
        }
        break;
    case 3:
        if (in > 0x00FFFFFF)
        {
            ret = -1;
        }
        break;
    default:
        break;
    }

    return ret;
}

/**
 * @chinese
 * 获得sdk编译版本
 *
 * @param data sdk编译版本(值如下:_DM365/_ARM_2410);
 *
 * @return 成功返回TRUE,失败返回FALSE
 * @endchinese
 *
 * @english
 * get sdk compilation version
 * @param data sdk compilation version(value:(_DM365/_ARM_2410);
 *
 * @return success return TRUE,false return FALSE
 *
 * @endenglish
 *
 */
int get_sdk_version(char *data)
{
    if(data== NULL)
    {
        return FALSE;
    }
    memset(data,0,sizeof(data));
#ifdef _DM365
    strcpy(data,"_DM365");
#endif

#ifdef _ARM_2410
    strcpy(data,"_ARM_2410");
#endif

#ifdef _AM335X
    strcpy(data,"_AM335X");
#endif

#ifdef _ARM_A23
    if(PLATFORM_TYPE>=PLATFORM_TYPE_MAX)
        PLATFORM_TYPE=PLATFORM_TYPE_MAX;
    sprintf(data,"_%s_android",PLATFORM_TYPE_NAME[PLATFORM_TYPE]);
#endif
    return TRUE;
}
//获取库版本号
int get_library_version(char *data)
{
    if(data==NULL)
    {
        return FALSE;
    }
#ifdef DEVICE_REGISTER
    char *reg="REG ";
#else
    char *reg="";
#endif
    //memset(data,0,sizeof(data));
#ifdef _DM365
    strcpy(data,"_DM365");
#endif

#ifdef _ARM_2410
    strcpy(data,"_ARM_2410");
#endif
#ifdef _ARM_2416
    sprintf(data,"180315 3.0.0.4 %s %s",reg,__TIME__);
#endif

#ifdef _AM335X
    //
    sprintf(data,"7.3.0.5 %s180301 %s",reg,__TIME__);
#endif

#ifdef _A20_UBUNTU
    sprintf(data,"1.3.0.5 %s180301 %s",reg,__TIME__);
#endif

#ifdef _ARM_A23
    if(PLATFORM_TYPE>=PLATFORM_TYPE_MAX)
        PLATFORM_TYPE=PLATFORM_TYPE_MAX;
    sprintf(data,"1.3.0.5 %s180410 %s %s",reg,PLATFORM_TYPE_NAME[PLATFORM_TYPE],__TIME__);
   // PLATFORM_TYPE = 0;  // 硬件平台类型, 0-A20, 1-RK3288，2-M186,3-A83 gorui
#endif
printf("%s %s\n",__func__,data);
    return TRUE;
}

// just for synel 20170804
char *get_a20sdk_version()
{
        static char *ver = "9.01.00.00.20171130";
	return ver;
}


/***************************************************************************************************************
FunctionName:StrToBCD
FunctionDescription:change string to BCD code
return value:if success return true,otherwise false
***************************************************************************************************************/
int StrToBCD(const char *Src,char *Des,int iDesLen)
{
    char chTemp = 0;
    int iSrcLen = 0,i=0;

    if (NULL == Src)
    {
        return -1;
    }
    if (NULL == Des)
    {
        return -1;
    }
    if (0 == iDesLen)
    {
        return -1;
    }

    iSrcLen = strlen(Src);

    if (iSrcLen > iDesLen * 2)
    {
        return -1;
    }

    chTemp = 0;
    for (i = 0; i < iSrcLen; i++)
    {
        if (i % 2 == 0)
        {
            chTemp = (Src[i] << 4) & 0xF0;
        }
        else
        {
            chTemp = (chTemp & 0xF0) | (Src[i] & 0x0F);
            Des[i / 2] = chTemp;
        }
    }
    if (i % 2 != 0)
    {
        Des[i / 2] = chTemp;
    }

    return 1;
}

//与这个对应的BCD码转换为字符串的函数
int BCDToStr(const char *Src, char *Des)
{
    int iSrcLen = 0,i=0;
    char chTemp ,chDes;

    if (NULL == Src)
    {
        return -1;
    }
    if (NULL == Src)
    {
        return -1;
    }

    iSrcLen = strlen(Src);
    chTemp = 0;
    chDes = 0;
    for ( i = 0; i < iSrcLen; i++)
    {
        chTemp = Src[i];
        chDes = chTemp >> 4;
        Des[i * 2] = chDes + '0';
        chDes = (chTemp & 0x0F) + '0';
        Des[i * 2 + 1] = chDes;
    }
    return 1;
}


//函数功能：将10进制文本格式的字符串(0结束)转换为BCD码,不足部分补0xff,空格符作为分隔符被忽略。
//          若缓冲区长度不足，则返回错误。
//          两个有效16进制文本字符转换为1个16进制数值字符，只允许最后字符为单个字符，否则视为格式错误。
//          单个有效10进制文本字符转换为高4位,低4为补"F"。
//参数：    in:        输入文本串地址
//          out:       结果输出串地址
//          outMaxLen: 输出缓冲区大小
//结果：    0: 正确, -1: 错误
//          输入指针为NULL、输出指针为NULL、非法输入值（出现非16进制字符）等，返回 -1
//示例：    输入文本串" 1234   567" ，输出为{0x12, 0x34, 0x56, 0x7F}
int thex2bcd(const char *in, unsigned char *out, int outLen)
{
    int len = 0;
    int a = 0;
    int b = 0;

    int overFlag = 0; //结束标志

    if (in == NULL || out == NULL)
    {
        //AfxMessageBox("y111111111111111111");

        return -1;
    }

    if (!outLen)
    {
        //AfxMessageBox("y22222222222222");

        return 0;
    }

    while (*in)
    {
        switch(*in)
        {
        case ' ':
            if (1 == a) //已读入单个10进制文本字符
            {
                b <<= 4;
                b += 0x0f; //单个字符后面补"F"
                *out++ = b; //写入结果缓冲区
                len++;
                a = 0;
                b = 0;
                overFlag = 1; //结束标志
            }
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            if (overFlag)
            {
                return -1; //非法格式
            }

            if (outLen == len)
            {
                //AfxMessageBox("y3333333333333333333333");

                return -1; //超过缓冲区
            }


            b = (b << 4) + (*in - '0'); //转换后移入临时变量b
            if (1 == a) //读入2个10进制文本字符，转换结果写缓冲区
            {
                *out++ = b;
                len++;
                a = 0;
                b = 0;
            }
            else
            {
                a++;
            }
            break;
        default:

            //AfxMessageBox("y44444444444444444444");
            return -1;
            break;
        }

        in++;
    }

    if (outLen > len && 1 == a)
    {
        b <<= 4;
        b += 0x0f; //单个字符后面补"F"
        *out++ = b;
        len++;
    }

    for (; len < outLen; len++)
    {
        *out++ = 0xff; //填充
    }

    //	//AfxMessageBox("y5555555555555555555555");

    return len;
}

//函数说明：BCD码(允许尾部为"F")转换为THEX格式，1个字节转换为2个字节
//参数：    in:  输入压缩BCD字符串首地址
//          out: 结果输出THex数值格式字符串首地址
//          Len: 需转换的压缩BCD字符串长度，以字节计
//结果：    >0:输出字符的个数, 0:完成, -1: 错误
//          输入指针为NULL、输出指针为NULL、len < 0 、非BCD等，返回 -1

int bcd2thex(const unsigned char *in, char *out, int len)
{
    int i;
    int overFlag = 0; //结束标志
    unsigned char tmpchar;
    int outlen = 0;

    if (in == NULL || out == NULL || len < 0)
    {
        return -1;
    }

    if (!len)
    {
        return 0;
    }

    for (i= 0; i < len; i++)
    {
        tmpchar = in[i] >> 4; //处理高位

        if (tmpchar > 9)
        {
            if (tmpchar == 0x0f)
            {
                overFlag = 1; //结束标志
            }
            else
            {
                return -1; //非法
            }
        }
        else
        {
            if (overFlag)
            {
                return -1; //结束标志后有数字
            }
            *out++ = tmpchar + '0';
            outlen++;
        }

        tmpchar = in[i] & 0x0f; //处理低位

        if (tmpchar > 9)
        {
            if (tmpchar == 0x0f)
            {
                overFlag = 1; //结束标志
            }
            else
            {
                return -1; //非法
            }
        }
        else
        {
            if (overFlag)
            {
                return -1; //结束标志后有数字
            }
            *out++ = tmpchar + '0';
            outlen++;
        }
    }

    return outlen;
}


#if 1		// lfg 20160128 add






//函数功能：将16进制数值型字符串转换为16进制文本格式的字符串。
//          一个16进制格式字符转换为两个16进制文本格式字符。
//参数：    in:  输入Hex格式（即16进制）字符串首地址
//          out: 结果输出16进制格式文本串首地址
//          Len: 需转换的Hex格式字符串长度，以字节计
//          fillchar:不为0时为填充的字符，为0不填充。
//结果：    返回输出16进制格式文本字符串的长度，以字节计(输出的16进制文本字符串自动在后面补0)
//          输入指针为NULL、输出指针为NULL、len < 0 等，返回 -1
//示例：    fillchar = " ", 输入文本串{0x12, 0x03, 0x5A, 0x0B, 0x45, 0x68}，输出为"12 03 5A 0B 45 68"

int HexToTHex(const unsigned char *in, char *out, int len, char fillchar)
{
        char table1[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

        int i = 0;

        if (in == NULL || out == NULL || len < 0)
        {
                return -1;
        }

        if (!len)
        {
                return 0;
        }

        for (i= 0; i < len; i++)
        {
                *out++ = table1[in[i] >> 4];
                *out++ = table1[in[i] & 0x0F];
                if (fillchar)
                {
                        *out++ = fillchar;
                }
        }

        if ((len) && (fillchar))
        {
                out--;
        }

        *out = 0;

        return fillchar ? (len * 3 - 1) : (len * 2);

}

int UIntToBcd(unsigned int in, unsigned char *out, unsigned int size)
{
        unsigned int i = 0;

        if ((size > 5) || (size == 0))
        {
                return -1; //参数错误
        }

        unsigned char hexbuff[5];

        for (i = 0; i < size; i++)
        {
                hexbuff[4-i] = in % 100;
                in = in / 100;
        }

        if (in)
        {
                return -1; //超限
        }

        return HexToBcd(hexbuff+(5-size), out, size);
}


//函数功能：将16进制文本格式的字符串（0结束）转换为16进制数值型字符串（不自动补0），空格符作为分隔符被忽略。
//          两个有效16进制文本字符转换为1个16进制数值字符，当出现单个字符（结尾字符或后面为空格）时，单个有
//          效16进制文本字符转换为1个16进制数值字符的高4位(低4位为0)。
//参数：    in:        输入文本串地址
//          out:       结果输出串地址
//          outMaxLen: 输出缓冲区大小
//结果：    返回输出字符串的长度，以字节计
//          输入指针为NULL、输出指针为NULL、非法输入值（出现非16进制字符）等，返回 -1
//示例：    输入文本串" 123   5a  b  4568" ，输出为{0x12, 0x03, 0x5A, 0x0B, 0x45, 0x68}

int THexToHex(const char *in, unsigned char *out, int outMaxLen)
{
        int len = 0;
        int a = 0;
        int b = 0;

        if (in == NULL || out == NULL)
        {
                return -1;
        }

        if (!outMaxLen)
        {
                return 0;
        }

        memset(out, 0, outMaxLen);  //20130108

        while (*in)
        {
                switch(*in)
                {
                        case ' ':
                                if (1 == a) //已读入单个16进制文本字符
                                {
                                        b <<= 4;
                                        *out++ = b; //写入结果缓冲区
                                        len++;
                                        a = 0;
                                        b = 0;
                                }
                                break;
                        case '0':
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6':
                        case '7':
                        case '8':
                        case '9':
                                b = (b << 4) + (*in - '0'); //转换后移入临时变量b
                                if (1 == a) //读入2个16进制文本字符，转换结果写缓冲区
                                {
                                        *out++ = b;
                                        len++;
                                        a = 0;
                                        b = 0;
                                }
                                else
                                {
                                        a++;
                                }
                                break;
                        case 'A':
                        case 'B':
                        case 'C':
                        case 'D':
                        case 'E':
                        case 'F':
                                b = (b << 4) + (*in - 'A' + 0x0A);
                                if (1 == a)
                                {
                                        *out++ = b;
                                        len++;
                                        a = 0;
                                        b = 0;
                                }
                                else
                                {
                                        a++;
                                }
                                break;
                        case 'a':
                        case 'b':
                        case 'c':
                        case 'd':
                        case 'e':
                        case 'f':
                                b = (b << 4) + (*in - 'a' + 0x0A);
                                if (1 == a)
                                {
                                        *out++ = b;
                                        len++;
                                        a = 0;
                                        b = 0;
                                }
                                else
                                {
                                        a++;
                                }
                                break;
                        default:
                                return -1;
                                break;
                }

                if (outMaxLen == len)
                {
                        break;
                }
                in++;
        }

        if (outMaxLen > len && 1 == a)
        {
                b <<= 4;
                *out++ = b;
                len++;
        }

        return len;
}



//函数说明：16进制数值格式字符串（字符数组）转换为压缩BCD码格式字符串(字符数组）
//参数：    in:  输入Hex数值格式字符串首地址
//          out: 结果输出压缩BCD格式字符串首地址
//          Len: 需转换长度，以字节计
//结果：    返回输出压缩BCD串的长度，以字节计
//          输入指针为NULL、输出指针为NULL、len < 0 等，返回 -1
//示例：    输入文本串{0x12, 0x03, 0x55, 0x08, 0x45, 0x62}，输出为{0x18, 0x03, 0x85, 0x08, 0x69, 0x98}

int HexToBcd(const unsigned char *in, unsigned char *out, int len)
{
        int i;

        if (in == NULL || out == NULL || len < 0)
        {
                return -1;
        }

        if (!len)
        {
                return 0;
        }

        for (i= 0; i < len; i++)
        {
                if (in[i] > 99)
                {
                        return -1;
                }

                *out++ = ((in[i]/10) << 4) + in[i]%10;
        }

        return len;
}

//函数说明：压缩BCD码格式的字符串转换为HEX数值格式的字符串，1个字节转换为1个字节
//参数：    in:  输入压缩BCD字符串首地址
//          out: 结果输出Hex数值格式字符串首地址
//          Len: 需转换的压缩BCD字符串长度，以字节计
//结果：    返回输出Hex数值格式字符串的长度，以字节计
//          输入指针为NULL、输出指针为NULL、len < 0 、非BCD等，返回 -1
//示例：    输入文本串{0x18, 0x03, 0x85, 0x08, 0x69, 0x98}，输出为{0x12, 0x03, 0x55, 0x08, 0x45, 0x62}

int BcdToHex(const unsigned char *in, unsigned char *out, int len)
{
        int i;

        if (in == NULL || out == NULL || len < 0)
        {
                return -1;
        }

        if (!len)
        {
                return 0;
        }

        for (i= 0; i < len; i++)
        {
                if (((in[i]&0x0f) > 9) || ((in[i] >> 4) > 9))
                {
                        return -1;
                }

                *out++ = (in[i] >> 4) * 10 + (in[i] & 0x0f);
        }

        return len;
}


int BcdToUInt(unsigned char *in, unsigned int *out, unsigned int size)
{
        int i = 0;
        if ((size > 5) || (size == 0))
        {
                return -1; //参数错误
        }

        unsigned char hexbuff[5];
        memset(hexbuff, 0 ,5);

        int ret;

        ret = BcdToHex(in, hexbuff+(5-size), size);

        if (ret < 0)
        {
                return ret;
        }

        *out = 0;

        for (i = 0; i < 5; i++)
        {
                *out = (*out * 100 + hexbuff[i]);
        }

        return 0;
}


//函数功能：将位串转换为无符号整型
//参数：    in: 输入首址
//          ByteOffset：开始位所在字节相对输入首址的偏移量，以字节计
//          BitOffset：开始位相对所在字节开始位的偏移量，以位计
//          BitLen：位串的位数
//返回值：  转换结果
unsigned int BitToUInt(void *in, unsigned int ByteOffset, unsigned int BitOffset, unsigned int BitLen)
{
        unsigned int ret = 0;
        unsigned char * str;
        unsigned char tmp;

        unsigned char table1[] = {0x00, 0x01, 0x03, 0x07 ,0x0F, 0x1F, 0x3F, 0x7F, 0xFF};

        if (NULL == in) //无输入，返回0
        {
                return 0;
        }

        BitLen = BitLen > 32 ? 32 : BitLen;
        BitOffset = BitOffset > 7 ? 0 : BitOffset;

        str = (unsigned char *)in + ByteOffset;

        if ((8 - BitOffset) > BitLen) //所有位在一个字节内，且未右对齐
        {
                tmp = *str >> (8 - BitOffset - BitLen); //右对齐
                ret = tmp & (table1[BitLen]); //取出结果
                BitLen = 0;
        }
        else
        {
                tmp = *str; //位所在首字符值
                ret = tmp & (table1[8 - BitOffset]); //取出首字符中的位
                BitLen -= (8 - BitOffset); //未处理的位
        }

        BitOffset = 0; //剩余字节位的偏移量置初值0
        str++; //指向下一个字节

        while (BitLen >0)
        {
                if (8 > BitLen) //不足1字节
                {
                        tmp = *str >> (8 - BitLen); //取出剩余结果
                        ret = (ret << BitLen) + tmp; //得到结果值
                        BitLen = 0;
                }
                else
                {
                        tmp = *str;
                        ret = (ret << 8) + tmp;	//当前字节内容加到结果
                        BitLen -= 8;
                }

                str++;
        }

        return ret;
}

//函数功能：将无符号整数转换为位串填充到指定位置
//参数：    in: 输入无符号整数
//          out: 需要填充位串的首地址
//          ByteOffset：开始位所在字节相对输入首址的偏移量，以字节计
//          BitOffset：开始位相对所在字节开始位的偏移量，以位计
//          BitLen：位串的位数
//返回值：  0：成功，-1：失败
int UIntToBit(unsigned int in, void *out, unsigned int ByteOffset, unsigned int BitOffset, unsigned int BitLen)
{
        unsigned char * str;
        unsigned char tmp;

        if ((NULL == out)||(BitLen > 32)||(BitOffset > 7)||(in >= ((unsigned int)1 << BitLen)))
        {
                return -1;
        }

        str = (unsigned char *)out + ByteOffset;

        if ((8 - BitOffset) > BitLen)
        {
                tmp = in << (8 - BitOffset - BitLen); //移位到BitOffset
                *str = (((0xFF >> BitOffset) ^ (0xFF << (8 - BitOffset - BitLen))) & *str) | tmp; //填充
                BitLen = 0;
        }
        else
        {
                tmp = in >> (BitLen - (8 - BitOffset)); //取出最左端所在字节需填充的内容
                *str = (~(0xFF >> BitOffset) & *str) | tmp; //填充
                BitLen -= (8 - BitOffset);
        }

        BitOffset = 0;
        str++;

        while (BitLen >0)
        {
                if (8 > BitLen)
                {
                        tmp = in << (8 - BitLen);
                        *str = ((0xFF >> BitLen) & *str) | tmp;
                        BitLen = 0;
                }
                else
                {
                        *str = in >> (BitLen - 8); //自动丢弃高字节内容
                        BitLen -= 8;
                }

                str++;
        }

        return 0;
}


//函数功能：判断是否闰年
//参数：	year:年份
//返回值：	1：闰年, 0：普通年
int isLeapYear(unsigned int year)
{
        if ((year%400 == 0) || ((year%100 != 0) && (year%4 == 0)))
        {
                return 1;
        }
        else
        {
                return 0;
        }
}

//函数功能：将以若干秒为单位的数值转换为时间
//参数：    in:从参考年的开始累积的单位秒数
//          RefYear:参考年，从该年的1月1日0时0分0秒开始计算
//          UnitSec:单位，即每单位的秒数
//返回值：
//
//
int UIntToTimeStr(unsigned int in, char *out, unsigned int RefYear, unsigned int UnitSec, char *format)
{
        unsigned int table1[2][13] = {{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30 ,31}, {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30 ,31}};
        unsigned int table2[2] = {365, 366};

        char * formatStr = format;
        struct TTime time1 = {RefYear, 1 , 1, 0, 0, 0};

        unsigned int days , secs;
        int LeapFlag = 0;


        if ((RefYear > 3000) || (UnitSec > 3600) || ((in/UnitSec) > 0xFFFFFFFF))
        {
                return -1;
        }

        days = in * UnitSec / (24 * 3600);
        secs = in * UnitSec % (24 * 3600);

        while (1)
        {
                LeapFlag = isLeapYear(time1.year);

                if (days < table2[LeapFlag])
                {
                        while (1)
                        {
                                if (days < table1[LeapFlag][time1.month])
                                {
                                        time1.day += days;
                                        break;
                                }
                                else
                                {
                                        days -= table1[LeapFlag][time1.month];
                                        time1.month++;
                                }
                        }
                        break;
                }
                else
                {
                        time1.year++;
                        days -= table2[LeapFlag];
                }
        }
        time1.hour = secs / 3600;
        time1.minute = (secs % 3600) / 60;
        time1.second = secs % 60;

        if (formatStr && out)
        {
                while (*formatStr)
                {
                        switch (*formatStr)
                        {
                                case 'Y':
                                case 'y':
                                        if ((*(formatStr + 1) == 'Y') || (*(formatStr + 1) == 'y'))
                                        {
                                                if ((*(formatStr + 2) == 'Y') || (*(formatStr + 2) == 'y'))
                                                {
                                                        if ((*(formatStr + 3) == 'Y') || (*(formatStr + 3) == 'y'))
                                                        {
                                                                if ((*(formatStr + 4) == 'Y') || (*(formatStr + 4) == 'y'))
                                                                {
                                                                        return -1;//非法（5位年）
                                                                }
                                                                else
                                                                {
                                                                        sprintf(out, "%04d", time1.year % 10000);//4位年
                                                                        out += 4;
                                                                        formatStr += 4;
                                                                }
                                                        }
                                                        else
                                                        {
                                                                return -1;//非法（3位年）
                                                        }
                                                }
                                                else
                                                {
                                                        sprintf(out, "%02d", time1.year % 100); //2位年
                                                        out += 2;
                                                        formatStr += 2;
                                                }
                                        }
                                        else
                                        {
                                                return -1;//非法（1位年）
                                        }

                                        break;
                                case 'M':
                                        if (*(formatStr + 1) == 'M')
                                        {
                                                if (*(formatStr + 2) == 'M')
                                                {
                                                        return -1;//非法（3位月）
                                                }
                                                else
                                                {
                                                        sprintf(out, "%02d", time1.month); //2位月
                                                        out += 2;
                                                        formatStr += 2;
                                                }
                                        }
                                        else
                                        {
                                                return -1;//非法（1位月）
                                        }

                                        break;
                                case 'D':
                                case 'd':
                                        if ((*(formatStr + 1) == 'D') || (*(formatStr + 1) == 'd'))
                                        {
                                                if ((*(formatStr + 2) == 'D') || (*(formatStr + 2) == 'd'))
                                                {
                                                        return -1;//非法（3位天）
                                                }
                                                else
                                                {
                                                        sprintf(out, "%02d", time1.day); //2位天
                                                        out += 2;
                                                        formatStr += 2;
                                                }
                                        }
                                        else
                                        {
                                                return -1;//非法（1位天）
                                        }

                                        break;
                                case 'H':
                                case 'h':
                                        if ((*(formatStr + 1) == 'H') || (*(formatStr + 1) == 'h'))
                                        {
                                                if ((*(formatStr + 2) == 'H') || (*(formatStr + 2) == 'h'))
                                                {
                                                        return -1;//非法（3位小时）
                                                }
                                                else
                                                {
                                                        sprintf(out, "%02d", time1.hour); //2位小时
                                                        out += 2;
                                                        formatStr += 2;
                                                }
                                        }
                                        else
                                        {
                                                return -1;//非法（1位小时）
                                        }

                                        break;
                                case 'm':
                                        if (*(formatStr + 1) == 'm')
                                        {
                                                if (*(formatStr + 2) == 'm')
                                                {
                                                        //非法（3位分钟）
                                                }
                                                else
                                                {
                                                        sprintf(out, "%02d", time1.minute); //2位分钟
                                                        out += 2;
                                                        formatStr += 2;
                                                }
                                        }
                                        else
                                        {
                                                return -1;//非法（1位分钟）
                                        }

                                        break;
                                case 'S':
                                case 's':
                                        if ((*(formatStr + 1) == 'S') || (*(formatStr + 1) == 's'))
                                        {
                                                if ((*(formatStr + 2) == 'S') || (*(formatStr + 2) == 's'))
                                                {
                                                        return -1;//非法（3位秒）
                                                }
                                                else
                                                {
                                                        sprintf(out, "%02d", time1.second);//2位秒
                                                        out += 2;
                                                        formatStr += 2;
                                                }
                                        }
                                        else
                                        {
                                                return -1;//非法（1位秒）
                                        }

                                        break;
                                default:
                                        sprintf(out, "%c", *formatStr++);
                                        out++;

                                        break;

                        }
                }
        }

        return 0;
}


//函数功能：  判断字符串是否为指定长度的数字ASCII码组成
int isdec(char *str, int len)
{
        int i = 0;

        for (i = 0; i < len; i++)
        {
                if ((str[i] < '0') || (str[i] > '9'))
                {
                        return 0;
                }
        }

        return 1;
}

//函数功能：  将数字ASCII码组成的字符串转换为无符号整型，最大长度为len,遇到非法字符结束
unsigned int atoUInt(char *str, int len)
{
        unsigned int ret = 0;
        int i = 0;

        for (i = 0; i < len; i++)
        {
                if ((str[i] < '0') || (str[i] > '9'))
                {
                        return ret;
                }
                ret = ret * 10 + (str[i] - '0');
        }
        return ret;
}


//函数功能：将10进制文本格式的字符串(0结束)转换为BCD码,不足部分补0xff,空格符作为分隔符被忽略。
//          若缓冲区长度不足，则返回错误。
//          两个有效16进制文本字符转换为1个16进制数值字符，只允许最后字符为单个字符，否则视为格式错误。
//          单个有效10进制文本字符转换为高4位,低4为补"F"。
//参数：    in:        输入文本串地址
//          out:       结果输出串地址
//          outMaxLen: 输出缓冲区大小
//结果：    0: 正确, -1: 错误
//          输入指针为NULL、输出指针为NULL、非法输入值（出现非16进制字符）等，返回 -1
//示例：    输入文本串" 1234   567" ，输出为{0x12, 0x34, 0x56, 0x7F}
int THexToBcd(const char *in, unsigned char *out, int outLen)
{
        int len = 0;
        int a = 0;
        int b = 0;

        int overFlag = 0; //结束标志

        if (in == NULL || out == NULL)
        {
                //AfxMessageBox("y111111111111111111");

                return -1;
        }

        if (!outLen)
        {
                //AfxMessageBox("y22222222222222");

                return 0;
        }

        while (*in)
        {
                switch(*in)
                {
                        case ' ':
                                if (1 == a) //已读入单个10进制文本字符
                                {
                                        b <<= 4;
                                        b += 0x0f; //单个字符后面补"F"
                                        *out++ = b; //写入结果缓冲区
                                        len++;
                                        a = 0;
                                        b = 0;
                                        overFlag = 1; //结束标志
                                }
                                break;
                        case '0':
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6':
                        case '7':
                        case '8':
                        case '9':
                                if (overFlag)
                                {
                                        return -1; //非法格式
                                }

                                if (outLen == len)
                                {
                //AfxMessageBox("y3333333333333333333333");

                                        return -1; //超过缓冲区
                                }


                                b = (b << 4) + (*in - '0'); //转换后移入临时变量b
                                if (1 == a) //读入2个10进制文本字符，转换结果写缓冲区
                                {
                                        *out++ = b;
                                        len++;
                                        a = 0;
                                        b = 0;
                                }
                                else
                                {
                                        a++;
                                }
                                break;
                        default:

                //AfxMessageBox("y44444444444444444444");
                                return -1;
                                break;
                }

                in++;
        }

        if (outLen > len && 1 == a)
        {
                b <<= 4;
                b += 0x0f; //单个字符后面补"F"
                *out++ = b;
                len++;
        }

        for (; len < outLen; len++)
        {
                *out++ = 0xff; //填充
        }

                //	//AfxMessageBox("y5555555555555555555555");

        return len;
}

//函数说明：BCD码(允许尾部为"F")转换为THEX格式，1个字节转换为2个字节
//参数：    in:  输入压缩BCD字符串首地址
//          out: 结果输出THex数值格式字符串首地址
//          Len: 需转换的压缩BCD字符串长度，以字节计
//结果：    >0:输出字符的个数, 0:完成, -1: 错误
//          输入指针为NULL、输出指针为NULL、len < 0 、非BCD等，返回 -1

int BcdToTHex(const unsigned char *in, char *out, int len)
{
        int i;
        int overFlag = 0; //结束标志
        unsigned char tmpchar;
        int outlen = 0;

        if (in == NULL || out == NULL || len < 0)
        {
                return -1;
        }

        if (!len)
        {
                return 0;
        }

        for (i= 0; i < len; i++)
        {
                tmpchar = in[i] >> 4; //处理高位

                if (tmpchar > 9)
                {
                        if (tmpchar == 0x0f)
                        {
                                overFlag = 1; //结束标志
                        }
                        else
                        {
                                return -1; //非法
                        }
                }
                else
                {
                        if (overFlag)
                        {
                                return -1; //结束标志后有数字
                        }
                        *out++ = tmpchar + '0';
                        outlen++;
                }

                tmpchar = in[i] & 0x0f; //处理低位

                if (tmpchar > 9)
                {
                        if (tmpchar == 0x0f)
                        {
                                overFlag = 1; //结束标志
                        }
                        else
                        {
                                return -1; //非法
                        }
                }
                else
                {
                        if (overFlag)
                        {
                                return -1; //结束标志后有数字
                        }
                        *out++ = tmpchar + '0';
                        outlen++;
                }
        }

        return outlen;
}



int getYear(unsigned int in, unsigned int RefYear, unsigned int UnitSec)
{
        unsigned int table1[2][13] = {{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30 ,31}, {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30 ,31}};
        unsigned int table2[2] = {365, 366};

        struct TTime time1 = {RefYear, 1 , 1, 0, 0, 0};

        unsigned int days , secs;
        int LeapFlag = 0;

        if ((RefYear > 3000) || (UnitSec > 3600) || ((in/UnitSec) > 0xFFFFFFFF))
        {
                return -1;
        }

        days = in * UnitSec / (24 * 3600);
        secs = in * UnitSec % (24 * 3600);

        while (1)
        {
                LeapFlag = isLeapYear(time1.year);

                if (days < table2[LeapFlag])
                {
                        while (1)
                        {
                                if (days < table1[LeapFlag][time1.month])
                                {
                                        time1.day += days;
                                        break;
                                }
                                else
                                {
                                        days -= table1[LeapFlag][time1.month];
                                        time1.month++;
                                }
                        }
                        break;
                }
                else
                {
                        time1.year++;
                        days -= table2[LeapFlag];
                }
        }
        time1.hour = secs / 3600;
        time1.minute = (secs % 3600) / 60;
        time1.second = secs % 60;

        return time1.year - RefYear;
}

int getMonth(unsigned int in, unsigned int RefYear, unsigned int UnitSec)
{
        unsigned int table1[2][13] = {{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30 ,31}, {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30 ,31}};
        unsigned int table2[2] = {365, 366};

        struct TTime time1 = {RefYear, 1 , 1, 0, 0, 0};

        unsigned int days , secs;
        int LeapFlag = 0;

        if ((RefYear > 3000) || (UnitSec > 3600) || ((in/UnitSec) > 0xFFFFFFFF))
        {
                return -1;
        }

        days = in * UnitSec / (24 * 3600);
        secs = in * UnitSec % (24 * 3600);

        while (1)
        {
                LeapFlag = isLeapYear(time1.year);

                if (days < table2[LeapFlag])
                {
                        while (1)
                        {
                                if (days < table1[LeapFlag][time1.month])
                                {
                                        time1.day += days;
                                        break;
                                }
                                else
                                {
                                        days -= table1[LeapFlag][time1.month];
                                        time1.month++;
                                }
                        }
                        break;
                }
                else
                {
                        time1.year++;
                        days -= table2[LeapFlag];
                }
        }
        time1.hour = secs / 3600;
        time1.minute = (secs % 3600) / 60;
        time1.second = secs % 60;

        return time1.month;
}

int getDay(unsigned int in, unsigned int RefYear, unsigned int UnitSec)
{
        unsigned int table1[2][13] = {{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30 ,31}, {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30 ,31}};
        unsigned int table2[2] = {365, 366};

        struct TTime time1 = {RefYear, 1 , 1, 0, 0, 0};

        unsigned int days , secs;
        int LeapFlag = 0;

        if ((RefYear > 3000) || (UnitSec > 3600) || ((in/UnitSec) > 0xFFFFFFFF))
        {
                return -1;
        }

        days = in * UnitSec / (24 * 3600);
        secs = in * UnitSec % (24 * 3600);

        while (1)
        {
                LeapFlag = isLeapYear(time1.year);

                if (days < table2[LeapFlag])
                {
                        while (1)
                        {
                                if (days < table1[LeapFlag][time1.month])
                                {
                                        time1.day += days;
                                        break;
                                }
                                else
                                {
                                        days -= table1[LeapFlag][time1.month];
                                        time1.month++;
                                }
                        }
                        break;
                }
                else
                {
                        time1.year++;
                        days -= table2[LeapFlag];
                }
        }
        time1.hour = secs / 3600;
        time1.minute = (secs % 3600) / 60;
        time1.second = secs % 60;

        return time1.day;
}

//函数功能 1到4字节的HEX数值数据转换为有符号整型
//参数：   in：输入首址
//         size: 输入的字符数，若超过4，则取4
//         flag: 输入的主机字节序，0：小端模式 其它：大端模式 (输出固定为小端模式)
//返回值   转换结果
int HexToInt(void *in, unsigned int size, int flag)
{
        unsigned char indata[4];
        memset(indata, 0, 4);

        if (size > 4)
        {
                size = 4;
        }
        else if (size == 0)
        {
                return 0;
        }

        if (flag) //大端
        {
                if (*(char *)in & 0x80)
                {
                        memset(indata, 0xff, 4);
                }

                memcpy(indata+(4-size), in, size);
//		indata[0] |= (indata[4-size]&(unsigned char)0x80);
//		indata[4-size] &= (unsigned char)0x7F;
        }
        else
        {
                if (*((char *)in + size - 1) & 0x80)
                {
                        memset(indata, 0xff, 4);
                }

                memcpy(indata, in, size);
//		indata[3] |= (indata[size-1]&(unsigned char)0x80);
//		indata[size-1] &= (unsigned char)0x7F;
        }

        return HexToUInt(indata, 4, flag);
}

//函数功能：将小端模式的有符号整数转换为任何字节序的HEX格式
//参数：   in：输入有符号整数，小端模式（逆序）
//         out:输出首址
//         size:输入的字节数，1、2、3、4
//         flag:输出主机字节序，0：小端模式 其它：大端模式 (输入固定为小端模式)
//返回值   0：转换成功，-1：输出位数不足
int IntToHex(int in, void *out, unsigned int size, int flag)
{
        int ret = 0;
        //unsigned char * str = (unsigned char *)out;
        unsigned char indata[4];

        if (size > 4)
        {
                size = 4;
        }
        else if (size == 0)
        {
                return 0;
        }

        ret = UIntToHex(in, indata, 4, flag);
        if (ret)
        {
                return ret;
        }

        if (flag) //大端
        {
                memcpy(out, indata+(4-size), size);
                *((unsigned char *)out) |= (indata[0]&(unsigned char)0x80);
        }
        else
        {
                memcpy(out, indata, size);
                *((unsigned char *)out+size-1) |= (indata[3]&(unsigned char)0x80);
        }

        switch (size)
        {
                case 1:
                        if ((in > 0x7F) || (in < (0 - 0x80)))
                        {
                                ret = -1;
                        }
                        break;
                case 2:
                        if ((in > 0x7FFF) || (in < (0 - 0x8000)))
                        {
                                ret = -1;
                        }
                        break;
                case 3:
                        if ((in > 0x7FFFFF) || (in < (0 - 0x800000)))
                        {
                                ret = -1;
                        }
                        break;
                default:
                        break;
        }

        return ret;
}

int TimeToUInt(struct TTime *in, unsigned int *out, unsigned int RefYear, unsigned int UnitSec)
{
        unsigned int table1[2][13] = {{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30 ,31}, {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30 ,31}};
        unsigned int table2[2] = {365, 366};

        unsigned int days , secs;
        int LeapFlag = 0;

        if ((RefYear > 3000) || (UnitSec > 3600))
        {
                return -1;
        }

        if ((in->year < RefYear) || (in->year > 9999))
        {
                return -1; //非法年
        }

        if ((in->month > 12) || (in->month < 1))
        {
                return -1; //非法月
        }

        LeapFlag = isLeapYear(in->year);

        if (in->day > table1[LeapFlag][in->month])
        {
                return -1; // 非法日
        }

//	in->hour = 23;
//	in->minute = 59;
//	in->second = 59;

        days = 0;
        secs = 0;
        unsigned int i;

        for (i = RefYear; i < in->year; i++)
        {
                LeapFlag = isLeapYear(i);
                days += table2[LeapFlag];

                if (days > (0xffffffff / (24 * 3600) * UnitSec))
                {
                        return -1; //超限
                }
        }

        LeapFlag = isLeapYear(in->year);

        for (i = 1; i < in->month; i++)
        {
                days += table1[LeapFlag][i];

                if (days > (0xffffffff / (24 * 3600) * UnitSec))
                {
                        return -1; //超限
                }
        }

        days += (in->day - 1);

        if (days > (0xffffffff / (24 * 3600) * UnitSec))
        {
                return -1; //超限
        }

        *out = days * ((24 * 3600) / UnitSec);

        secs += (in->hour * 3600);
        secs += (in->minute * 60);
        secs += in->second;

        *out += secs / UnitSec;

        return 0;

}


// 字符串分隔 20130716
// 该函数不会改变原字符串内容
// In--待分隔字符串
// Out--目标字符数组, 二维数组，第二维固定长度64
// oSize -- 目标字符数组大小
// Sep--分隔符如"," ":" ...
// 返回实际解析的字段个数
#if 0
int LeeDivideStr(char *In, char Out[][64], unsigned int oSize, char Sep)
{
        int	i = 0, j = 0, k = 0;

        if (In == NULL || oSize > 512)
        {
                return -1;
        }

        memset(Out, 0, 64 * oSize);

        for( i = 0, j = 0; i < oSize; i++, j++ )
        {
                k = 0;
                while( In[j] && In[j] != Sep && k < 63)
                {
                        Out[i][k] = In[j];
                        j++;
                        k++;
                }

                if (k == 63)
                {
                        while(In[j] && In[j] != Sep)
                        j++;
                }

                Out[i][k] = '\0'; // must do before if (!buf[j])

                if( !In[j] )       // must judge
                {
                        break;
                }
        }

        return i+1;

}
#endif

char inputNumber(char* prompt, int defValue)
{
        char vKey, vRet;
        char vStr[16] = {0}, *vStr2;

//	PRINTF("\n%s, def=%d\n", prompt, defValue);

        while(1)
        {
                vKey = getchar();
                if ((vKey >= '0' && vKey <= '9')
                   ||(vKey >= 'A' && vKey <= 'Z')
                   || (vKey >= 'a' && vKey <= 'z'))
                {
                        vRet = vKey;
                        printf("vret = %c\n", vRet);
                        break;
                }
//		else if ( vKey == '\n' )
//		{
//			if (vCnt == 0)
//				vRet = defValue;
//			goto RET;
//		}
        }

        while(1)
        {
                vKey = getchar();
                if ( vKey == 0x0A )		// '\n'
                        break;
        }

RET:
        return vRet;
}


// AB 01 03 33 01 33 03 6D A5   --> AB0103330133036DA5


int cutSpaceChar(const char* pSrc, unsigned char* pDst)
{
        char *p = (char *)pSrc;

        while(*p)
        {
                if (*p != ' ')	// cut space
                {
                        *pDst++ = *p;
                }

                p++;
        }

        return 0;
}



// 可打印字符串转换为字节数据
// 如："C8329BFD0E01" --> {0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01}
// 输入: pSrc - 源字符串指针
// nSrcLength - 源字符串长度
// 输出: pDst - 目标数据指针
// 返回: 目标数据长度

int stringToBytes(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
                int i=0;

                for ( i = 0; i < nSrcLength; i += 2)
                {
                                if (!isxdigit(*pSrc) || !isxdigit(*(pSrc + 1)))
                                {
                                        return -1;
                                        break;
                                }
                                // 输出高4位
                                if ((*pSrc >= '0') && (*pSrc <= '9'))
                                {
                                                *pDst = (*pSrc - '0') << 4;
                                }
                                else if ((*pSrc >= 'A') && (*pSrc <= 'F'))
                                {
                                                *pDst = (*pSrc - 'A' + 10) << 4;
                                }
                                else
                                {
                                                *pDst = (*pSrc - 'a' + 10) << 4;
                                }
                                pSrc++;

                                // 输出低4位
                                if ((*pSrc>='0') && (*pSrc<='9'))
                                {
                                                *pDst |= *pSrc - '0';
                                }
                                else if ((*pSrc >= 'A') && (*pSrc <= 'F'))
                                {
                                                *pDst |= *pSrc - 'A' + 10;
                                }
                                else
                                {
                                                *pDst |= *pSrc - 'a' + 10;
                                }

                                pSrc++;
                                pDst++;

                }

                return (nSrcLength / 2);
}


// 创建多级目录
int create_multi_dir(const char *path)
{
        int i, len;

        len = strlen(path);
        char dir_path[len+1];
        dir_path[len] = '\0';

        strncpy(dir_path, path, len);

        for (i=0; i<len; i++)
        {
                if (dir_path[i] == '/' && i > 0)
                {
                        dir_path[i]='\0';
                        if (access(dir_path, F_OK) < 0)
                        {
                                if (mkdir(dir_path, 0755) < 0)
                                {
                                        printf("mkdir=%s:msg=%s\n", dir_path, strerror(errno));
                                        return -1;
                                }
                        }
                        dir_path[i]='/';
                }
        }

        return 0;
}



void readini2( FILE *file, char *group, char *member, char *values )
{
        char	buf[256], tmpbuf1[256], tmpbuf2[256], *p1;
        int		len = 0, flag = 0;

        memset( buf, 0, 256 );
        rewind( file );              //pointer to file header
        while( fgets( buf, 256, file ) )
        {
                p1 = buf;
                cut_rn( buf );
                if( strcmp( buf, group ) == 0 )
                {
                        flag = 1;
                        continue;
                }       // find the group position and set it
                if( flag )
                {
                        if(  *p1 == '[' )
                        {
                                values = NULL;
                                break;
                        }

                        if(  strlen( buf ) == 0 )		// 20130718 lee 允许空行
                        {
                                continue;
                        }

                        len = strcspn( buf, "=" ); //cut up members of group and value with '='
                        memset( tmpbuf1, 0, sizeof( tmpbuf1 ) );
                        memset( tmpbuf2, 0, sizeof( tmpbuf2 ) );
                        strncpy( tmpbuf1, buf, len );
                        p1 += len;
                        p1++;
                        if( strcmp( tmpbuf1, member ) == 0 )
                        {
                                strcpy( values, p1 );
                                break;
                        } //read values of members
                }
                memset( buf, 0, sizeof( buf ) );
        }

        cut_rn( values );
}


#endif

/********************************************************************
*  Function：   my_strstr()
*  Description: 在一个字符串中查找一个子串;
*  Calls:       无;
*  Called By:   无
*  Input：      ps: 源;       pd：子串
*  Output：     无;
*  Return :     0：源字符串中没有子串; 1：源字符串中有子串;
*  Author:      ChenZhiFa
*  Others:      无;
*  date of completion:
*********************************************************************/
char * my_strstr(char * ps,char *pd)
{
        char *pt = pd;
        int c = 0;

        while(*ps != '\0')
        {
                if(*ps == *pd)
                {

                        while(*ps == *pd && *pd!='\0')
                        {
                                ps++;
                                pd++;
                                c++;
                        }
                }

                else
                {
                        ps++;

                }

                if(*pd == '\0')
                {
                        //sum++;
                        return (ps - c);
                }
                        c = 0;
                        pd = pt;
        }

        return 0;
}

///********************************************************************
//*  Function：   memcpy()
//*  Description: 复制一个内存区域到另一个区域;
//*  Calls:       无;
//*  Called By:   无
//*  Input：       src: 源;
//                 count: 复制字节数.
//*  Output：      dest: 复制目的地;
//*  Return :      dest;
//*  Author:       ChenZhiFa
//*  Others:       无;
//*  date of completion:
//*********************************************************************/
//void * memcpy(void * dest,const void *src,size_t count)
//{
//        char *tmp = (char *) dest, *s = (char *) src;

//        while (count--)
//                *tmp++ = *s++;

//        return dest;
//}

/********************************************************************
*  Function：   str_replace()
*  Description: 在一个字符串中查找一个子串，并且把所有符合的子串用
                另一个替换字符串替换。
*  Calls:        memcpy();
*  Called By:    无
*  Input：       p_source:要查找的母字符串； p_seach要查找的子字符串;
                 p_repstr：替换的字符串;
*  Output：      p_result:存放结果;
*  Return :      返回替换成功的子串数量;
*  Author:       ChenZhiFa
*  Others:       p_result要足够大的空间存放结果，所以输入参数都要以\0结束;
*  date of completion:
*********************************************************************/
int str_replace(char *p_result,char* p_source,char* p_seach,char *p_repstr)
{

        int c = 0;
        int repstr_leng = 0;
        int searchstr_leng = 0;

        char *p1;
        char *presult = p_result;
        char *psource = p_source;
        char *prep = p_repstr;
        char *pseach = p_seach;
        int nLen = 0;

        repstr_leng = strlen(prep);
        searchstr_leng = strlen(pseach);

        do{
                p1 = my_strstr(psource,p_seach);

                if (p1 == 0)
                {
                        strcpy(presult,psource);
                        return c;
                }

                c++;  //匹配子串计数加1;
                printf("结果:%s\r\n",p_result);
                printf("源字符:%s\r\n",p_source);

                // 拷贝上一个替换点和下一个替换点中间的字符串
                nLen = p1 - psource;
                memcpy(presult, psource, nLen);

                // 拷贝需要替换的字符串
                memcpy(presult + nLen,p_repstr,repstr_leng);

                psource = p1 + searchstr_leng;
                presult = presult + nLen + repstr_leng;

        }while(p1);

        return c;

}


