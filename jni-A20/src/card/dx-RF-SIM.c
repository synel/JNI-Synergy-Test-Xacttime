/**
 * @chinese
 * @file   dx-RF-SIM.c
 * @author 刘训
 * @date   Wed Jul 13 09:34:15 2011
 *
 * @brief 手机卡操作模块
 * @endchinese
 *
 * @english
 * @file   dx-RF-SIM.c
 * @author Liu Xun
 * @date   Wed Jul 13 09:34:15 2011
 *
 * @brief phone card operating module
 * @endenglish
 *
 *
 *
 */
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/times.h>
#include <unistd.h>
#include <sched.h>
#include "dx-RF-SIM.h"
#include "public.h"
#include "stringchar.h"
#include "config.h"
#include "serial.h"
#include "debug.h"
#ifndef _ARM_A23
#include "WLTLib.h"
#else
#include "../android/wedsa23.h"
#endif
#include "gpio.h"
#include "uartlib.h"

const char* liccode = "0503-20131111-0001570760";
const char* datpath = "./base.dat";
const char* wltpath = "./zp.wlt";
const char* outputBmp = "./zp.bmp" ;

/**
 *
 *通过RF_SIM协议读取串口数据
 * @param uart_port 使用的串口号
 * @param framet_ype 协议类型
 * @param value 通过串口读取到的有效数据
 *
 * @return 成功-读取的有效数据长度,失败-(-1)
 */
int read_RFID_SIM_data(int uart_port,char framet_ype,char *value)
{
    unsigned char ch = 0;
    int retval=0,nbytes=-1;
#if 1
    int overtime=0;
    struct timeval tv1,tv2;

    gettimeofday(&tv1,NULL);
    gettimeofday(&tv2,NULL);

    overtime=tv1.tv_sec *1000000+tv1.tv_usec;
    while(1)
    {
        if(abs( (tv2.tv_sec *1000000 + tv2.tv_usec) - overtime) > 50000)//2015-3-7lxy
        {
            return ERROR;
        }
        gettimeofday(&tv2,NULL);
        retval = serial_recv_onebyte((int)uart_port,(char*)&ch);
        if(retval == ERROR)   /* receive begin */
        {
            return ERROR;
        }

        if(ch != framet_ype)
            continue;
        else
            break;
    }
#endif

#if 0
    retval = serial_recv_onebyte((int)uart_port,(char*)&ch);
    //    printf("%02X,%d\n",ch,retval);
    if(retval == ERROR || ch != framet_ype)   /* receive begin */
    {
        return ERROR;
    }
#endif
    /* according to protocol,this byte is address */
    retval = serial_recv_onebyte((int)uart_port,(char*)&ch);
    if(retval == ERROR)   /* receive begin */
    {
        return ERROR;
    }

    nbytes = (int)ch;
    //    printf("=============louloulou====%x===%d\n",ch,nbytes);
    memset(value,0,sizeof(value));
    retval = serial_recv_data(uart_port,(unsigned char *)value,nbytes);
    if(retval == ERROR)
    {
        return ERROR;
    }

    return nbytes;
}

/*******************************************
 *SHNM201A 型 RF-SIM 卡读卡模块
 *******************************************/
/**
 *
 * 读取0201a手机卡类型的序列号 电信 只读序列号
 * UART, 9600, 1 位停止位
 *B0: 帧头,09:数据长度(不包括帧头和自身) 00: 保留字节,后,面 8 个字节为卡号。
 *注:串口输出时,16 位的 16 进制数卡号,被合并成 8 个字节。
   如:
     手机菜单显示卡号为:99 00 00 00 00 10 02 58
     则对应 UART 输出: B0 09 00 99 00 00 00 00 10 02 58
 * @param uart_port 使用的串口号
 * @param value 卡序列号
 *
 * @return 成功 1; 失败 -1
 */
int read_RFID_SIM_0201A(int uart_port,char *value)
{
    char data[4096];
    int retval = 0;
    int i = 0;
    static char data_buffer[64];
    static int buff_len = 0;
#if 0
    retval = read_RFID_SIM_data(uart_port,0XB0,data);
    if(retval == -1 || retval < 2)
        return ERROR;
#endif
    /*
    retval = serial_recv_onebyte((int)uart_port,(char*)&byte);
    if(retval == ERROR || byte != 0XB0)
    {
        return ERROR;
    }


    retval = serial_recv_onebyte((int)uart_port,(char*)&byte);
    if(retval == ERROR || byte != 0X09)
    {
        return ERROR;
    }

    //nbytes = (int)nbytes;
    nbytes = (int)byte;//lxy 2012-11-20
    memset(data,0,sizeof(data));
    retval = serial_recv_data(uart_port,(unsigned char *)data,nbytes);
    if(retval == ERROR || retval != nbytes)
    {
        return ERROR;
    }
    memset(value,0,sizeof(value));
    gsmBytes2String((unsigned char*)&data[1], value, retval-1);
    serial_clear((TCOM)uart_port); //清空串口缓存
    return SUCCESS;
*/
    // 先去数据再分析
    memset(data,0,sizeof(data));
    retval = serial_recv_data_all(uart_port,(unsigned char *)data);
    if(retval == ERROR)
    {
    	return ERROR;
    }
    else if ((buff_len + retval) >= 64)
    {
        memset(data_buffer,0,sizeof(data_buffer));
        buff_len = 0;
        serial_clear((TCOM)uart_port); //清空串口缓存
        return ERROR;
    }
    else
    {
    	memcpy(&data_buffer[buff_len], data, retval);
        buff_len += retval;
        if(buff_len < 11) return ERROR;
    }
    for (i = buff_len - 11; i >= 0 ; i--)
    {
        if ((data_buffer[i] == 0xB0) && (data_buffer[i+1] == 0x09))// && (data_buffer[i+2] == 0x00)
        {
            memset(value,0,sizeof(value));
            gsmBytes2String((unsigned char*)&data_buffer[i + 3], value, 8);
            memset(data_buffer,0,sizeof(data_buffer));
            buff_len = 0;
            serial_clear((TCOM)uart_port); //清空串口缓存
            return SUCCESS;
        }
    }
    memset(data_buffer,0,sizeof(data_buffer));
    buff_len = 0;
    serial_clear((TCOM)uart_port); //清空串口缓存
    return ERROR;
}

/******************************************
 *读取直通电讯RFID卡
*******************************************/
/**
 *
 * 连接RF
 * @param uart_port 使用的串口号
 *
 * @return 成功 1; 失败 -1
 */
int RF_init(int uart_port)
{
    char data[128];
    int retval=0;
    unsigned char init_buf[7]={0x80,0x05,0x90,0xB0,0x01,0x00,0x00};

    retval = serial_send_data(uart_port,(char *)init_buf, 7);
    if(retval == ERROR)
    {
        return ERROR;
    }

    retval = read_RFID_SIM_data(uart_port,0X90,data);
    if(retval == -1)
    {
        return ERROR;
    }

    if(data[0] != 0X90 && data[1]!= 0X00)
    {
        return ERROR;
    }
    return SUCCESS;
}

/**
 *
 * 断开RF
 * @param uart_port 使用的串口号
 *
 * @return 成功 1; 失败 -1
 */
int RF_uninit(int uart_port)
{
    char data[128];
    int retval=0;
    unsigned char uninit_buf[7]={0x80,0x05,0x90,0xB0,0x00,0x00,0x00};

    retval = serial_send_data(uart_port,(char *)uninit_buf, 7);
    if(retval == ERROR)
    {
        return ERROR;
    }

    retval = read_RFID_SIM_data(uart_port,0X90,data);
//    int i=0;
//    for(i=0;i<retval;i++)
//            printf("%02X ",data[i]);
//    printf("\n");
    if(retval == ERROR)
    {
        return ERROR;
    }

    if(data[0] != 0X90 && data[1]!= 0X00)
    {
        return ERROR;
    }
    return SUCCESS;
}

/**
 *
 *查询
 * @param uart_port 使用的串口号
 *
 * @return 成功 1; 失败 0
 */
int RF_request(int uart_port)
{
    char data[128];
    int retval=0;
    unsigned char ask_buf[14]={0x80,0x05,0x90,0xB0,0x04,0x00,0x00};

    retval = serial_send_data(uart_port,(char *)ask_buf, 7);
    if(retval == ERROR)
    {
        return ERROR;
    }
    memset(data,0,sizeof(data));
    retval = read_RFID_SIM_data(uart_port,0X90,data);
    if(retval == ERROR)
    {
        return ERROR;
    }
    //卡已连上
//    int i=0;
//    for(i=0;i<retval;i++)
//        printf("%02X ",data[i]);
//    printf("\n");
    if(data[0] == 0X9C && data[1]== 0X02)
    {
        return SUCCESS;
    }
    return  ERROR;
}

/**
 *
 *读取ID
 * @param uart_port uart_port 使用的串口号
 * @param value 通过串口读取到的有效数据
 *
 * @return 成功 1; 失败 0
 */
int RF_read_ID(int uart_port,char *value)
{
    char data[128];
    int retval=0;
    unsigned char read_buf[14]={0x80,0x05,0x90,0xB0,0x05,0x00,0x00};

    memset(value,0,sizeof(value));
    serial_send_data(uart_port,(char *)read_buf, 7);
    if(retval == ERROR)
    {
        return FALSE;
    }
    retval = read_RFID_SIM_data(uart_port,0X90,data);
//    int i=0;
//    for(i=0;i<retval;i++)
//            printf("%02X ",data[i]);
//    printf("\n");
//    retval = read_RFID_SIM_data(uart_port,0X90,data);
//    for(i=0;i<retval;i++)
//            printf("%02X ",data[i]);
//    printf("\n");
    if(retval == ERROR || retval < 10)
    {
        return ERROR;
    }

    if(data[retval-2] == 0X90 && data[retval-1]== 0X00)
    {
        gsmBytes2String((unsigned char*)&data[retval - 10], value, 8);
        //    for(i=56;i<64;i++)
        //        sprintf(value+2*(i-56),"%02X",data[i]);
        return SUCCESS;
    }
    return ERROR;
}


//
/**
 *
 * 读取0100d类型手机卡的序列号 电信
 * @param uart_port  使用的串口号
 * @param value 通过串口读取到的有效数据
 *
 * @return 成功 1; 失败 0
 */
int read_RFID_SIM_0100D(int uart_port,char *value)
{
    int retval = 0;
    static int init_flag = 0;
    if(!init_flag)
    {
        retval = RF_init(uart_port);
        if(retval == ERROR)
        {
            return ERROR;
        }
        init_flag = 1;
    }
    retval = RF_request(uart_port);
    if(retval == ERROR)
    {
        return ERROR;
    }
    retval = RF_read_ID(uart_port,value);
    if(retval == ERROR)
    {
        return  ERROR;
    }
    retval = RF_uninit(uart_port);
    if(retval == ERROR)
    {
        //printf("%s ------------------------\n",__func__);
        //return  ERROR;
    }
    init_flag = 0;
    return SUCCESS;
}



/********************************************
 * 杰瑞M300型号高频设备读取的卡号
 ********************************************/
/**
 * @chinese
 * 读取M300卡数据
 *
 * @param uart_port  使用的串口号
 * @param value 通过串口读取到的有效数据
 *
 * @return 成功 1; 失败 0
 * @endchinese
 *
 * @english
 * read data from M300 card
 *
 * @param uart_port  uart port
 * @param value save data
 *
 * @return success-1,fail-0
 * @endenglish
 *
 */
int read_M300_card(int uart_port, char *value)
{
    char data[512],tmp[128];
    int nbytes = 0,i=0,retval = 0,len=0;
    char ask_buf[]={0x01,0x04,0x01,0x03};

    if(serial_send_data(uart_port, ask_buf, 4) == ERROR)
        return FALSE;

    memset(data,0,sizeof(0));
    memset(value,0,sizeof(0));

    while(nbytes < sizeof(data))
    {
        retval=serial_recv_onebyte(uart_port,(data+nbytes));
        if(retval ==0 || retval== ERROR)   /* receive begin */
        {
            break;
        }
        if(data[0]==0XFF && data[1] == 0XFF && data[nbytes-1] == 0XEE && data[nbytes]==0XEE)
            break;

        nbytes += 1;
    }

    if(nbytes < 6)
    {
        return FALSE;
    }

    if(data[0]!=0XFF || data[1] != 0XFF || data[nbytes-1] != 0XEE || data[nbytes]!=0XEE)
        return FALSE;
    for(i=5;i<=nbytes-2;)
    {
        memset(tmp,0,sizeof(tmp));
        //        sprintf(tmp,"%d",hex_to_int((unsigned char*)data+i,4,0));
        len = gsmBytes2String((unsigned char*)data+i, tmp, 4);
        strncat(value,tmp,len);
        i+=5;
        if(i<=nbytes-2)
        {
            strcat(value,",");
        }
    }

    return TRUE;
}

/**
 * @chinese
 * 清除M300卡数据
 *
 * @param uart_port  使用的串口号
 *
 * @return 成功 1; 失败 0
 * @endchinese
 *
 * @english
 * clear M300 card
 *
 * @param uart_port  uart port
 *
 * @return success-1,fail-0
 * @endenglish
 *
 */
int clear_M300_readcard_data(int uart_port)
{
    char value[512];
    int nbytes = 0,retval=0;
    unsigned char ask_buf[]={0x01,0x04,0x02,0x03};


    if(serial_send_data(uart_port,(char*)ask_buf, 4) == TIME_OUT)
        return FALSE;
    memset(value,0,sizeof(0));
    while(nbytes<sizeof(value))
    {
        retval=serial_recv_onebyte(uart_port,(value+nbytes));
        if(retval ==0 || retval== ERROR)   /* receive begin */
        {
            break;
        }

        if(value[0] == 0XFF && value[1] == 0XFF && value[2] == 0X01 && value[3] == 0X06 &&
           value[4] == 0XEE && value[5] == 0XEE )
        {
            break;
        }

        nbytes += 1;
    }
    if(nbytes < 6)
        return FALSE;

    if(value[0] != 0XFF || value[1] != 0XFF || value[2] != 0X01 || value[3] != 0X06 ||
       value[4] != 0XEE || value[5] != 0XEE )
        return FALSE;

    return TRUE;
}

/********************************************
 *读取新东方定制卡号
 ********************************************/
int read_xdf_card(int uart_port, char *value)
{
    int retval = 0;
    int len = 8;
    char tmp[64];

    if(value == NULL)
    {
        return ERROR;
    }
    memset(tmp,0,sizeof(tmp));
    memset(value,0,sizeof(value));
    retval = serial_recv_data(uart_port,(unsigned char *)tmp,len);
    if(retval == -1)
    {
        return ERROR;
    }
    len = gsmBytes2String((unsigned char*)tmp, value, retval);
    serial_clear(uart_port);
    //    printf("value:%s\n",value);
    return TRUE;
}

int ReadDataCom( int fd, unsigned char * addr, int len, long *dwCount, char *s )
{
    int count = 0;
    if( ioctl( fd, FIONREAD, &count ) == -1 )
    {
        return FALSE;
    }
    if( count == 0 )
    {
        *dwCount = count;
        return TRUE;
    }
    count = read( fd, addr, len );
    if( count < 0 )
    {
        return FALSE;
    }
    *dwCount = count;
    return TRUE;
}

int WriteDataCom( int fd, unsigned char *addr, int len, long *dwCount, char *s )
{
    int count;
    count = write( fd, addr, len );
    if( count < 0 )
    {
        return FALSE;
    }
    *dwCount = count;
    return TRUE;
}


//函数功能：串口发送和接收。根据帧头判断采用的协议，并自动填充校验；接收时自动判断校验是否正确，若通讯错误，
//          重试次数判断是否重发。
//参数：    hcom: 打开的串口句柄，若为空，则直接返回
//          command: 发送缓冲区首址，可以不填充校验字符
//          RecvBuf: 接收缓冲区首址
//          RetryCount: 当超时、校验错时的重试次数
//          timeout: 单次命令的超时时间，以毫秒计
//返回值：  大于0: 成功，值表示返回的长度（数据域长度+6），其它值：失败 ERR_HEADINVALID: 发送数据帧头错误
//          ERR_COMERR: 串口操作失败,ERR_INVALIDHANDLE:串口句柄无效
//          ERR_OVERRETRY: 超过重试次数，ERR_OVERBUF:超过最大数据长度104
int sendAndRecvComData(unsigned char *SendBuf, unsigned char *RecvBuf, int RetryCount, int timeout, int m_hcom)
{
    if (-1== m_hcom)
    {
        return -1;
    }

    int bWriteStat,bReadStat;
    long dwCount;

    unsigned char * buf = RecvBuf;
    unsigned char bcc = 0;

    unsigned char * addr;

    unsigned int time1;
    int len;
    int datalen, commLen;
    int i;

    if (SendBuf[0] != 0x02)
    {
        return -4;
    }

    commLen = HexToUInt(SendBuf+1, 2, 1);
    if (commLen > (512 - 5))
    {
        return -3;
    }

    for (i = 3; i < commLen + 3; i++) //长度
    {
        bcc ^= SendBuf[i];
    }

    SendBuf[commLen+3] = bcc;
    SendBuf[commLen+4] = 0x03;

    for (i = 0; i < RetryCount + 1; i++)
    {
        tcflush(m_hcom,TCIOFLUSH); //清空缓冲区
        bWriteStat = WriteDataCom(m_hcom, SendBuf, commLen+5, &dwCount, NULL);

        if(!bWriteStat)
        {
            printf("%s 111111\n", __func__);
            return -2; //写串口失败
        }

        time1 = timeout + GetTickCount();
        sched_yield();		//让出cpu,;

        len = 1;
        addr = buf;

        while (len)
        {
            bReadStat = ReadDataCom(m_hcom, addr, len, &dwCount, NULL);

            if(!bReadStat)
            {
            	printf("%s 222222\n", __func__);
                return -2; //读串口失败
            }

            len -= dwCount;
            addr += dwCount;

            if (len > 0)
            {
                if (GetTickCount() > time1)
                {
                    goto timeout4; //超时
                }
                else if (!dwCount)
                {
                    sched_yield();		//让出cpu,;
                }
            }
            else if (0x02 != buf[0]) //未读到帧头
            {
                len = 1;
                addr = buf;
            }
        }

        len = 2;
        while (len) //读返回码+长度,只关心长度
        {
            bReadStat = ReadDataCom(m_hcom, addr, len, &dwCount, NULL);

            if(!bReadStat)
            {
            	printf("%s 33333\n", __func__);
                return -2; //读串口失败
            }

            len -= dwCount;
            addr += dwCount;

            if (len > 0)
            {
                if (GetTickCount() > time1)
                {
                    goto timeout4; //超时
                }
                else if (!dwCount)
                {
                    sched_yield();		//让出cpu,;
                }
            }
        }

        datalen = HexToUInt(buf+1, 2, 1);
        if (datalen > (512-5))
        {
            datalen = 512-5;
        }

        len = datalen + 2;

        while(len) //接收后续数据
        {
            bReadStat = ReadDataCom(m_hcom, addr, len, &dwCount, NULL);

            if(!bReadStat)
            {
            	printf("%s 4444\n", __func__);
                return -2; //读串口失败
            }

            len -= dwCount;
            addr += dwCount;

            if (len > 0)
            {
                if (GetTickCount() > time1)
                {
                    goto timeout4; //超时
                }
                else if (!dwCount)
                {
                    sched_yield();		//让出cpu,;
                }
            }
        }

        bcc = 0;

        for (i = 3; i < datalen + 4; i++) //长度
        {
            bcc ^= buf[i];
        }

        if ((!bcc) && buf[datalen+4] == 0x03)
        {
            return datalen+5; //返回正确
        }
        timeout4:
	;
    }

    return -3; //超过重试次数
}
//国民技术APDU通道 20130609 GQH

int nationzTechApduChannel(unsigned char *buf, unsigned char buflen, unsigned char *answerBuf, unsigned char maxAnswerlen, int timeout, int fd)
{
    if (buflen > 246)
    {
        return -4;//ERR_OVERBUF; //超过最大APDU长度
    }

    unsigned char Fhead[] = {0x02, 0x00, 0x00, 0xa2, 0x33}; // 帧头+设备地址+数据长度+指令字+数据包数量
    unsigned char SendBuf[260]; //255+5(0X104)
    unsigned char RecvBuf[265];
    memset(RecvBuf, 0, 6);

    memcpy(SendBuf, Fhead, 5);
    SendBuf[1] = (buflen + 2) / 256; //数据长度高字节
    SendBuf[2] = (buflen + 2) % 256; //数据长度低字节
    memcpy((char *)SendBuf + 5, buf, buflen);
    int len = sendAndRecvComData(SendBuf, RecvBuf, 3, timeout,fd);
    //    printf("%s %d\n", __func__, len);
    if (len < 0)
    {
        return len;
    }
    else if (len == 7)  //只有返回码，无数据
    {
        unsigned int ret = HexToUInt(RecvBuf+3, 2, 1);
        switch (ret)
        {
        case 0xa002:  //卡片连接失败
        case 0xa006:  //操作卡片数据无回应
        case 0xa007:  //操作卡片数据出现错误
            return	-2;// ERR_NOCARD;
            break;
        default:
            return -3;//ERR_INVALID; //无效返回
            break;
        }
    }
    else if (len > 8)  //最少2字节数据sw1 sw2
    {
        if ((RecvBuf[3] == 0) && (RecvBuf[4] == 0)) //返回OK
        {
            if (maxAnswerlen < (RecvBuf[1] * 256 + RecvBuf[2]))
            {
                return -4;//ERR_OVERBUF; //超过缓冲区大小
            }
            memcpy(answerBuf, RecvBuf+5, (RecvBuf[1] * 256 + RecvBuf[2] - 2));
            return (RecvBuf[1] * 256 + RecvBuf[2] - 2); //返回apdu数据长度
        }
        else
        {
            return -3;//ERR_INVALID; //无效返回
        }
    }
    else
    {
        return -3;//ERR_INVALID; //无效返回
    }
}

//通过名称选择文件 GQH
int selectFileByName(unsigned char *fileName, unsigned char nameLen, unsigned char *buf, unsigned char len, int fd)
{
    unsigned char cla = 0x00;
    unsigned char ins = 0xA4;

    unsigned char apdu[128];
    memset(apdu, 0, 128);

    unsigned char *addr = apdu;
    *addr = cla;
    addr++;

    *addr = ins;
    addr++;

    *addr = 4; //p1
    addr++;

    *addr = 0; //p2
    addr++;

    *addr = nameLen; //lc,文件名长度
    addr++;

    memcpy(addr, fileName, nameLen);
    addr+= nameLen;

    //APDU已准备好
    unsigned char answerBuf[256];
    int ret;
    ret = nationzTechApduChannel(apdu, (unsigned char)5 + nameLen, answerBuf, 255, 1000,fd);
    if(ret == 2)
    {

        return ret;
    }
    else if (ret > 2) //20111013
    {
        if ((answerBuf[ret - 2] == 0x90) && (answerBuf[ret - 1] == 0))
        {

            memcpy(buf, answerBuf, ret-2);
            return ret-2;
        }
        else
        {
            return -3; //ERR_INVALID
        }
    }
    else if (ret < 0)
    {
        return ret;
    }
    else
    {
        return -3; //ERR_INVALID
    }
}
/******************************************
 *支持2.4G手机卡 2011-02-03
 *****************************************/

#if 0
int read_24G_card(int fd,char *value)
{
    int ret;
    static int error_num=0,error_num_hw=0, flag = 0;
    unsigned char simCardNo[16];
    unsigned char versionNO[256];
    if(fd == -1)
    {
        return -1;
    }

    memset(simCardNo,0,sizeof(simCardNo));
    memset(versionNO,0,sizeof(versionNO));

    //    ret = query_version(fd,versionNO);

    ret = queryPhoneCard(fd, 0, simCardNo);
    if (ret != 0)
    {
    	error_num++;
    	error_num_hw++;
        if(error_num>40)
    	{
            error_num=0;
            reset_module(fd);
    	}
    	if(error_num_hw > 480)//480
    	{
            error_num_hw=0;

            printf("reset 2\n");

            //哈尔滨新中新飞线
            set_gpio_on(RED_LED);
            usleep(150000);
            set_gpio_off(RED_LED);
    	}

        return -1;
    }
    error_num=0;
    error_num_hw=0;
    memset(value, 0, sizeof(value));

    sprintf(value, "%02X%02X%02X%02X%02X%02X%02X%02X",
            simCardNo[0],simCardNo[1],simCardNo[2],simCardNo[3],simCardNo[4],simCardNo[5],
            simCardNo[6],simCardNo[7]);
    //    sprintf(value, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
    //    		versionNO[0],versionNO[1],versionNO[2],versionNO[3],versionNO[4],versionNO[5],
    //    		versionNO[6],versionNO[7],versionNO[8],versionNO[9],versionNO[10],versionNO[11],
    //    		versionNO[12],versionNO[13],versionNO[14],versionNO[15],versionNO[16],versionNO[17],
    //    		versionNO[18],versionNO[19],versionNO[20],versionNO[21],versionNO[22],versionNO[23],
    //    		versionNO[24]);

    serial_clear((TCOM)fd); //清空串口缓存
    return 1;

}

#else	// 20160620 Lee debug
int read_24G_card(int fd,char *value)
{
    int ret;
    static int error_num=0,error_num_hw=0, flag = 0;
    unsigned char simCardNo[16];
    unsigned char versionNO[256];
    if(fd == -1)
    {
        return -1;
    }

    if(flag==0)//2016.11.3 guo add  给模块上电
    {
            //printf("%s 1\n",__func__);
            set_gpio_off(RED_LED);
            flag++;
    }
    memset(simCardNo,0,sizeof(simCardNo));
    memset(versionNO,0,sizeof(versionNO));

    //    ret = query_version(fd,versionNO);

    ret = queryPhoneCard(fd, 0, simCardNo);
    //if(ret!=-2)
   // printf("%s 2 %d %d %d \n",__func__,ret,error_num,error_num_hw);
   // printf("ret == %d\n", ret);
    if (ret != 0)
    {
        error_num++;
        error_num_hw++;
        if(error_num>40)
        {
            error_num=0;
            reset_module(fd);
        }
        if(error_num_hw > 480)//480
        {
            error_num_hw=0;
            error_num=0;
            set_gpio_on(RED_LED);
            usleep(500000);
            set_gpio_off(RED_LED);
            //usleep(100000);
            //reset_module(fd);
        }

        return -1;
    }
    error_num=0;
    error_num_hw=0;

    memset(value, 0, sizeof(value));

    sprintf(value, "%02X%02X%02X%02X%02X%02X%02X%02X",
            simCardNo[0],simCardNo[1],simCardNo[2],simCardNo[3],simCardNo[4],simCardNo[5],
            simCardNo[6],simCardNo[7]);
    //    sprintf(value, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
    //    		versionNO[0],versionNO[1],versionNO[2],versionNO[3],versionNO[4],versionNO[5],
    //    		versionNO[6],versionNO[7],versionNO[8],versionNO[9],versionNO[10],versionNO[11],
    //    		versionNO[12],versionNO[13],versionNO[14],versionNO[15],versionNO[16],versionNO[17],
    //    		versionNO[18],versionNO[19],versionNO[20],versionNO[21],versionNO[22],versionNO[23],
    //    		versionNO[24]);
//printf("%s 3 %s\n",__func__,value);
    serial_clear((TCOM)fd); //清空串口缓存
    return 1;

}

#endif

/******************************************
 *支持国民技术2.4G手机卡 读写 2014-11-24 115200 baud
 *
 * #移动手机卡读写
'1 选m1应用
00A404000ED15600010154686DB50A03000256
'2 认证m1 key
842400010CFFFFFFFFFFFFFFFFFFFFFFFF
'3 Select adf adf's aid=fff...
8426000110FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
'4 auth sector=05
8440050006FFFFFFFFFFFF
'5 read block 01
8442010110
'6 update block 01
844400011000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
'7 read block 01
8442010110

844400011001FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
***************************/
/******************************************
 *支持国民技术2.4G手机卡 2014-11-24 115200 baud 读扇区数据
 ******************************************/
int gmjs_24G_read(int fd)
{
    unsigned char src[][32] = {
        {0x00,0xA4,0x04,0x00,0x0E,0xD1,0x56,
         0x00,0x01,0x01,0x54,0x68,0x6D,0xB5,0x0A,0x03,0x00,0x02,0x56},
{0x84,0x24,0x00,0x01,0x0C,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},
{0x84,0x26,0x00,0x01,0x10,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},
{0x84,0x40,0x05,0x00,0x06,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},
{0x84,0x42,0x01,0x01,0x10},
};
    unsigned char slen[] = {19, 17, 21, 11, 5};
    unsigned char answerBuf[256];
    int ret, j;
    for (j = 0; j < 5; j++)
    {
        memset (answerBuf, 0 , sizeof(answerBuf));
        ret = nationzTechApduChannel(src[j], slen[j],
                                     answerBuf, 255, 1000, serial_fd[fd]);
        //		int i = 0;
        //		printf("%d Return==:",j );
        //		for (; i < ret; i++)
        //		{
        //			printf("%02X", answerBuf[i]);
        //		}
        //		printf("\n");
        if (ret < 2)
        {
            break;
        }
        else
        {
            if (answerBuf[ret - 2] == 0x90 && answerBuf[ret - 1] == 0x00)
            {
                if (j == 4)
                {
                    ret = (int)answerBuf[0];
                }
            }
            else
            {
                ret = -99;
                break;
            }
        }
    }
    return ret;
}
/******************************************
 *支持国民技术2.4G手机卡 2014-11-24 115200 baud 写扇区数据
 *
 * #移动手机卡写 */

int gmjs_24G_write(int fd, unsigned char value)
{
    unsigned char src[][32] = {
        {0x00,0xA4,0x04,0x00,0x0E,0xD1,0x56,
         0x00,0x01,0x01,0x54,0x68,0x6D,0xB5,0x0A,0x03,0x00,0x02,0x56},
{0x84,0x24,0x00,0x01,0x0C,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},
{0x84,0x26,0x00,0x01,0x10,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},
{0x84,0x40,0x05,0x00,0x06,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},
{0x84,0x44,0x00,0x01,0x10,0x00,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}
    };
    unsigned char slen[] = {19, 17, 21, 11, 21};
    unsigned char answerBuf[256];
    int ret, j;
    src[4][5] = value;
    for (j = 0; j < 5; j++)
    {
        memset (answerBuf, 0 , sizeof(answerBuf));
        ret = nationzTechApduChannel(src[j], slen[j],
                                     answerBuf, 255, 1000, serial_fd[fd]);
        //		int i = 0;
        //		printf("%d Return==:",j );
        //		for (; i < ret; i++)
        //		{
        //			printf("%02X", answerBuf[i]);
        //		}
        //		printf("\n");
        if (ret < 2)
            break;
        else
        {
            if (answerBuf[ret - 2] == 0x90 &&
                answerBuf[ret - 1] == 0x00)
            {
                if (j == 4)
                {
                    ret = 1;
                    break;
                }
            }
            else
            {
                ret = -99;
                break;
            }
        }
    }

    return ret;
}

/******************************************
 *支持2.4G手机卡 2013-11-01 115200 baud 读取应用序列号
 *****************************************/
int read_24G_card_app(int fd,char *value)
{
    int ret;
    static int error_num=0,error_num_hw=0, flag = 0;
    unsigned char simCardNo[8];
    char sscnId[] = "D1560001018003800000000100001002";
    unsigned char scnId[16];
    unsigned char recvBuf[128];
    unsigned char *addr = NULL;
    int len = 0;
    if(fd == -1 || serial_fd[fd] <= 0 )
    {
        return -1;
    }

    if(flag==0)
    {
        set_gpio_on(RED_LED);
        flag++;
    }

    ret = queryPhoneCard(fd, 0, simCardNo);
    if (ret != 0)
    {
        error_num++;
        if(error_num % 40 == 0)
        {
            set_gpio_off(RED_LED);
            usleep(150000);
            set_gpio_on(RED_LED);
        }

        return -1;
    }

    memset(scnId, 0, sizeof(scnId));
    ret = gsmString2Bytes(sscnId, scnId, strlen(sscnId));
    if (ret != 16)
    {
        return -1;
    }
    memset(recvBuf, 0, sizeof(recvBuf));
    //寻卡后，读取卡应用序列号
    //ret = selectFileByName((unsigned char*)&scnId, 16,recvBuf, 128, fd);
    ret = selectFileByName((unsigned char*)&scnId, 16,recvBuf, 128, serial_fd[fd]);
    addr = recvBuf;

    if (*addr != 0x6F)
    {
        return -1;
    }


    addr++;

    len = *addr;
    if (len != ret - 2)
    {
        return -1;
    }

    addr++;

    if (*addr != 0x84)
    {
        return -1;
    }

    addr++;

    len = *addr;

    addr += len;
    addr++;
    if (*addr != 0xA5)
    {
        return -1;
    }

    addr++;

    addr += 4;
    addr++;
    if (*addr != 0x9F && *(addr+1) != 0x0C)
    {
        return -1;
    }

    addr += 2;

    addr += 22;

    //gsmBytes2String(addr, value, 4);
    memset(value, 0, sizeof(value));
    sprintf(value, "%02X%02X%02X%02X",*(addr+3),*(addr+2),*(addr+1),*addr);

    //printf("2.4.sno = %s\n", value);

    /*//在pit项目中不需要断开，否则卡片放在卡头前会连续读卡
    // 查询卡状态
    ret = checkLinkStatus(fd);
    if (0 == ret)	// 连接，则断开
    {
    	//printf("1\n");
        ret = releaseCardLink(fd, 0);
        if (ret)	// 断开失败,复位
        {
        	//printf("2\n");
            for (i = 0; i < 3; i++)
            {
                ret = softResetReader(fd);
                if (0 == ret)
                {
                    break;
                }
            }

            if(3 == i)
            {
                // 复位失败，报错
            	printf("== card reset error == \n");

            }
        }
    }
*/
    serial_clear((TCOM)fd); //清空串口缓存

    return 1;
}

// 20160331 新中新
int read_24G_card_xzx(int fd,char *value)
{
    int ret;
    static int error_num=0,error_num_hw=0, flag = 0;
    unsigned char simCardNo[8];	
    char sscnId[] = "D1560001018000000000000100000000";		// 新中新读移动2.4G应用序列号 ，文件名
    unsigned char scnId[16];
    unsigned char recvBuf[128];
    unsigned char *addr = NULL;
    int len = 0;
    if(fd == -1 || serial_fd[fd] <= 0 )
    {
        return -1;
    }

    if(flag==0)
    {
        set_gpio_off(RED_LED);
        flag++;
    }

    ret = queryPhoneCard(fd, 0, simCardNo);
    if (ret != 0)
    {
        error_num++;
        if(error_num % 40 == 0)
        {
            set_gpio_on(RED_LED);
            usleep(150000);
            set_gpio_off(RED_LED);
        }

        return -1;
    }

    memset(scnId, 0, sizeof(scnId));
    ret = gsmString2Bytes(sscnId, scnId, strlen(sscnId));
    if (ret != 16)
    {
        return -1;
    }
    memset(recvBuf, 0, sizeof(recvBuf));

    //寻卡后，读取卡应用序列号
    ret = selectFileByName((unsigned char*)&scnId, 16,recvBuf, 128, serial_fd[fd]);
    addr = recvBuf;
    if (*addr != 0x6F)
    {
        return -1;
    }

    addr++;

    len = *addr;
    if (len != ret - 2)
    {
        return -1;
    }

    addr++;

    if (*addr != 0x84)
    {
        return -1;
    }

    addr++;

    len = *addr;

    addr += len;
    addr++;
    if (*addr != 0xA5)
    {
        return -1;
    }

    addr++;

    addr += 4;
    addr++;
    if (*addr != 0x9F && *(addr+1) != 0x0C)
    {
        return -1;
    }

    addr += 2;

    addr += 11;

    //gsmBytes2String(addr, value, 4);
    memset(value, 0, sizeof(value));
    sprintf(value, "%02X%02X%02X%02X%02X%02X%02X%02X",*(addr+0),*(addr+1),*(addr+2),*(addr+3),*(addr+4),*(addr+5),*(addr+6),*(addr+7));

    //printf("2.4.sno = %s\n", value);

    serial_clear((TCOM)fd); //清空串口缓存

    return 1;
}



// 连接卡片
//delaytime  -- 等待卡进入感应区的时间 (为0时没有卡就返回)

int queryPhoneCard(unsigned int comfd, unsigned short delaytime, unsigned char *simCardNo)
{
    int ret;
    int paramLen = 2;
    unsigned short retStatus;
    unsigned char cardLen;
    unsigned char recvData[64];

    unsigned char cmdType = 0XA2;
    unsigned char cmdCode = 0X31;
    unsigned char cmdParam[2];

    cmdParam[0] = (delaytime >> 8) & 0XFF;
    cmdParam[1] = delaytime & 0XFF;
    ret = transferCommand(cmdType, cmdCode, cmdParam, paramLen, comfd);
    if (-1 == ret)
    {
        //    printf("no data recevice\n");
        return -3;	// 发送命令失败
    }

    memset(recvData, 0, sizeof(recvData));
    ret = receiveRespond(comfd, recvData);
    if (-1 == ret)
    {
        //		printf("^^^ timeout or error data ^^^\n");
        return -3;	// 超时,或未接收到有效数据
    }
    serial_clear(comfd);
    retStatus = recvData[0] * 0X100 + recvData[1];	// 数据状态
    //printf("retstauts = %04X\n", retStatus);

    switch (retStatus)
    {
    case 0:
        if (!simCardNo)
        {
            return -1;
        }
        cardLen = recvData[2];
        if ((cardLen + 3) != ret)
        {
            return -1;
        }
        memset(simCardNo, 0, sizeof(simCardNo));
        memcpy(simCardNo, recvData + 3, cardLen);

        break;

    case 0XA001:
        return -2;	//未连接卡
        break;

    case 0XA006:
        return -3;	//超时
        break;

    default:
        return -99;
        break;


    }

    return 0;

}
int query_version(unsigned int comfd, unsigned char *versionNo)
{
    int ret;
    int paramLen = 0;
    unsigned short retStatus;
    unsigned char cardLen;
    unsigned char recvData[256];

    unsigned char cmdType = 0XA1;
    unsigned char cmdCode = 0X11;
    unsigned char cmdParam[2];
    memset(cmdParam,0,sizeof(cmdParam));

    ret = transferCommand(cmdType, cmdCode, cmdParam, paramLen, comfd);
    if (-1 == ret)
    {
        return -1;	// 发送命令失败
    }

    memset(recvData, 0, sizeof(recvData));
    ret = receiveRespond(comfd, recvData);
    if (-1 == ret)
    {
        return -1;	// 超时,或未接收到有效数据
    }

    retStatus = recvData[0] * 0X100 + recvData[1];	// 数据状态

    switch (retStatus)
    {
    case 0:
        if (!versionNo)
        {
            return -1;
        }
        cardLen = recvData[18];
        if ((cardLen + 19) != ret)
        {
            return -1;
        }
        memset(versionNo, 0, sizeof(versionNo));
        memcpy(versionNo, recvData + 19, cardLen);

        break;

    case 0X0002:
        return -2;
        break;

    default:
        return -99;
        break;


    }

    return 0;

}
int reset_module(unsigned int comfd)
{
    int ret;
    int paramLen = 0;
    unsigned short retStatus;
    unsigned char cardLen;
    unsigned char recvData[256];

    unsigned char cmdType = 0XA1;
    unsigned char cmdCode = 0X12;
    unsigned char cmdParam[2];
    memset(cmdParam,0,sizeof(cmdParam));

    ret = transferCommand(cmdType, cmdCode, cmdParam, paramLen, comfd);
    if (-1 == ret)
    {
        return -1;	// 发送命令失败
    }

    memset(recvData, 0, sizeof(recvData));
    ret = receiveRespond(comfd, recvData);
    if (-1 == ret)
    {
        return -1;	// 超时,或未接收到有效数据
    }

    retStatus = recvData[0] * 0X100 + recvData[1];	// 数据状态

    switch (retStatus)
    {
    case 0:
        break;

    case 0X0002:
        return -2;
        break;

    default:
        return -99;
        break;
    }

    return 0;

}


// 断开连接

int releaseCardLink(unsigned int comfd, unsigned short delaytime)
{

    int ret;
    int paramLen = 2;
    unsigned short retStatus;
    unsigned char recvData[32];
    unsigned char cmdType = 0XA2;
    unsigned char cmdCode = 0X32;
    unsigned char cmdParam[2];

    delaytime = 0;		// 固定为0

    cmdParam[0] = 0;
    cmdParam[1] = 0;

    ret = transferCommand(cmdType, cmdCode, cmdParam, paramLen, comfd);
    if (-1 == ret)
    {
        return -1;	// 发送命令失败
    }

    memset(recvData, 0, sizeof(recvData));

    ret = receiveRespond(comfd, recvData);
    if (-1 == ret)
    {
        return -1;	// 超时
    }

    retStatus = recvData[0] * 0X100 + recvData[1];
    if (retStatus)
    {
        return -1;
    }

    return 0;	// 命令执行成功

}


// 终端发送卡操作指令
//struct timeval cmdSendTime;
int transferCommand(unsigned char cmdType, unsigned char cmdCode, unsigned char *cmdParam, unsigned int paramLen, unsigned int comfd)
{
    unsigned short dataLen = 0;
    unsigned char sendData[32];
    unsigned char tmpData[32];
    unsigned char lrcValue = 0;

    int i = 0;
    int len = 0;

    memset(sendData, 0, sizeof(sendData));
    memset(tmpData, 0, sizeof(tmpData));

    sendData[0] = 0X02;

    tmpData[0] = cmdType;
    tmpData[1] = cmdCode;

    if (cmdParam && paramLen)
    {
        memcpy(&tmpData[2], cmdParam, paramLen);
    }

    dataLen = paramLen + 2;

    sendData[1] = (dataLen >> 8) & 0XFFFF;
    sendData[2] = dataLen & 0XFFFF;

    while (i < paramLen + 2)
    {
        lrcValue = lrcValue ^ tmpData[i++];
    }

    sendData[3 + paramLen + 2] = lrcValue;

    sendData[3 + paramLen + 2 + 1]  = 0X03;

    memcpy(&sendData[3], tmpData, paramLen + 2);

    len = serial_send_data(comfd,(char *)sendData, paramLen + 7);
    if(len == paramLen + 7)
    {
        return 0;
    }

    return -1;
}


// 接收读卡器响应数据
// 返回接收的数据长度

int receiveRespond(unsigned int comfd, unsigned char *recvData)
{
    int ret = -1;
    int i = 0;
    fd_set read_fd;
    struct timeval timeout;
    unsigned char byte;
    unsigned int dataLen = 0;
    unsigned char lrcValue = 0;
    unsigned char recvPacket[256];
    unsigned char *p = NULL;

    FD_ZERO (&read_fd);
    FD_SET (comfd, &read_fd);

    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;	// 最大超时时间500ms,500taiman

    memset(recvPacket, 0, sizeof(recvPacket));
    p = recvPacket;

    ret = select (comfd + 1, &read_fd, NULL, NULL, &timeout);
    int retval=0;

    int overtime=0;
    struct timeval tv1,tv2;

    gettimeofday(&tv1,NULL);
    gettimeofday(&tv2,NULL);

    overtime=tv1.tv_sec *1000000+tv1.tv_usec;
    while(1)
    {
        if(abs( (tv2.tv_sec *1000000 + tv2.tv_usec) - overtime) > 80000)//20150330lxy
        {
            return -1;
        }
        gettimeofday(&tv2,NULL);

        ret = serial_recv_onebyte((int)comfd, (char *)&byte);
        if (ret == ERROR || byte != 0X02)	// 包头
        {
            return -1;
        }

        ret = serial_recv_onebyte((int)comfd, (char *)&byte);

        if (ret == ERROR)
        {
            return -1;
        }
        dataLen = byte * 0X100;	// 长度高字节len_H

        ret = serial_recv_onebyte((int)comfd, (char *)&byte);
        if (ret == ERROR)
        {
            return -1;
        }
        dataLen += byte;	// 长度低字节len_L
        if(dataLen > 256)
        {
            return -1;
        }
        // 读取数据域
        retval = serial_recv_data((int)comfd, recvData,dataLen);
        if(retval == ERROR)
        {
            return -1;
        }
        //        i=0;
        //        for(i=0;i<dataLen;i++)
        //        {
        //        	printf("%02x ",recvData[i]);
        //        }
        //        printf("\n");

        ret = serial_recv_onebyte((int)comfd, (char *)&byte);	//校验位
        if (ret == ERROR)
        {
            return -1;
        }
        i = 0;
        while (i < dataLen)
        {
            lrcValue = lrcValue ^ recvData[i];
            i++;
        }

        if (lrcValue != byte)
        {
            return -1;
        }

        ret = serial_recv_onebyte(comfd, (char *)&byte);
        if (ret == ERROR || byte != 0X03)	// 包尾
        {
            return -1;
        }
        break;
    }
    return dataLen;	// 返回实际数据长度
}

// 检测卡片连接状态每隔50~60ms查询一次

int checkLinkStatus(unsigned int comfd)
{
    int ret;
    unsigned short retStatus;

    unsigned char recvData[32];

    unsigned char cmdType = 0XE0;
    unsigned char cmdCode = 0X02;
    ret = transferCommand(cmdType, cmdCode, NULL, 0, comfd);
    if (-1 == ret)
    {
        return -1;	// 发送命令失败
    }

    memset(recvData, 0, sizeof(recvData));
    ret = receiveRespond(comfd, recvData);
    if (-1 == ret)
    {
        return -1;	// 超时
    }

    retStatus = recvData[0] * 0X100 + recvData[1];
    if (retStatus)
    {
        return -1;
    }

    if (0 == recvData[2])
    {
        return 1;	// 已经不在读卡区
    }

    if (1 == recvData[2])
    {
        return 0;	//  还在读卡区
    }

    return -1;

}




// 读取读卡器状态

int getReaderStatus(unsigned int comfd)
{
    int ret;
    unsigned short retStatus;

    unsigned char recvData[32];

    unsigned char cmdType = 0XE0;
    unsigned char cmdCode = 0X09;

    ret = transferCommand(cmdType, cmdCode, NULL, 0, comfd);
    if (-1 == ret)
    {
        return -1;	// 发送命令失败
    }

    memset(recvData, 0, sizeof(recvData));

    ret = receiveRespond(comfd, recvData);
    if (-1 == ret)
    {
        return -1;	// 超时
    }

    retStatus = recvData[0] * 0X100 + recvData[1];

    switch (retStatus)
    {
    case 0:
        return 0;	// 开启卡片通信
        break;

    case 0XA009:
        return -2;	// 上电自检失败
        break;

    case 0XA00A:
        return -3;	//等待卡片靠近
        break;

    case 0XE055:		// 环境异常
        return -4;
        break;

    case 0XE0AA:		//读卡器死锁
        return -5;
        break;

    case 0XE00A:		// 读卡器未激活
        return -6;
        break;

    default:
        return -99;
        break;


    }

    return 0;

}





// 读取上电自检结果

int getPostResult(unsigned int comfd)
{
    int ret;
    unsigned short retStatus;

    unsigned char recvData[32];

    unsigned char cmdType = 0XA1;
    unsigned char cmdCode = 0X16;

    ret = transferCommand(cmdType, cmdCode, NULL, 0, comfd);
    if (-1 == ret)
    {
        return -1;	// 发送命令失败
    }

    memset(recvData, 0, sizeof(recvData));

    ret = receiveRespond(comfd, recvData);
    if (-1 == ret)
    {
        return -1;	// 超时
    }

    retStatus = recvData[0] * 0X100 + recvData[1];

    switch (retStatus)
    {
    case 0:
        return 0;	// 正确
        break;

    default:
        return -99;
        break;


    }

    return 0;

}



// 软复位读卡器,复位后，在500ms内不要操作卡

int  softResetReader(unsigned int comfd)
{
    int ret;
    unsigned short retStatus;

    unsigned char recvData[32];

    unsigned char cmdType = 0XA1;
    unsigned char cmdCode = 0X12;

    ret = transferCommand(cmdType, cmdCode, NULL, 0, comfd);
    if (-1 == ret)
    {
        return -1;	// 发送命令失败
    }

    memset(recvData, 0, sizeof(recvData));

    ret = receiveRespond(comfd, recvData);
    if (-1 == ret)
    {
        return -1;	// 超时
    }


    retStatus = recvData[0] * 0X100 + recvData[1];

    if (2 == retStatus)
    {
        return -1;	// 命令格式不对
    }

    if (0 == retStatus)
    {
        return 0;
    }

    return -1;

}




// 查看版本信息

int  getVersionInfo(unsigned int comfd, unsigned char *cmcc_interface, unsigned char *third_interface, unsigned char *proinfomation)
{
    int ret;
    unsigned short retStatus;

    unsigned char recvData[32];

    unsigned char cmdType = 0XA1;
    unsigned char cmdCode = 0X11;

    if (!cmcc_interface || !third_interface || !proinfomation)
    {
        return -1;
    }

    ret = transferCommand(cmdType, cmdCode, NULL, 0, comfd);
    if (-1 == ret)
    {
        return -1;	// 发送命令失败
    }

    memset(recvData, 0, sizeof(recvData));

    ret = receiveRespond(comfd, recvData);
    if (-1 == ret)
    {
        return -1;	// 超时
    }

    retStatus = recvData[0] * 0X100 + recvData[1];

    if (2 == retStatus)
    {
        return -1;	// 命令参数错误
    }

    if (0 == retStatus)
    {
        memcpy(cmcc_interface, &recvData[2], 8);	// 中国移动接口版本信息
        memcpy(third_interface, &recvData[10], 8);	// 第三方定义的版本信息
        memcpy(proinfomation, &recvData[19], recvData[18]);	// 厂家自定义信息

        return 0;
    }

    return -1;

}

//联通中科讯联
int recvPackage(int uart_port,unsigned char *package)
{
    char data[256];
    int num=0;
    int datalen=1;
    int ret=0;
    ret=serial_recv_onebyte(uart_port,data+num);
    if(ret<1||data[num]!=0x5A)
        return -1;
    num+=ret;

    ret=serial_recv_onebyte(uart_port,data+num);
    if(ret<1)return -1;
    num+=ret;
    ret=serial_recv_data(uart_port,(unsigned char *)data+num,2);
    if(ret<2)return -1;
    num+=ret;
    datalen=data[2];
    datalen=datalen<<8;
    datalen=datalen|data[3];
    if(datalen > 252)return -1;
    ret=serial_recv_data(uart_port,(unsigned char *)data+num,datalen);
    if(ret<datalen)return -1;
    num+=ret;
    ret=serial_recv_onebyte(uart_port,data+num);
    if(ret<1)
        return -1;
    num+=ret;

    ret=serial_recv_onebyte(uart_port,data+num);
    if(ret<1||data[num]!=0XCA)
        return -1;
    num+=ret;

    if(data[num-2]!=serial_cal_bcc((unsigned char*)data+1,datalen+3))
        return -1;

    if(data[num-3]!=0x00)
        return -1;
    if(num > 256)
    	return -1;

    memcpy(package,data,num);
    return datalen;
}

//发送命令
int sendCmdData(int uart_port,unsigned char overtime)
{
    int retval;
    unsigned char data[256];
    unsigned short int datalen=1;

    data[0]=0x5A;
    data[1]=0x10;
    data[2]=(datalen&0xff00)>>8;
    data[3]=datalen&0xff;
    data[4]=overtime;
    data[5]=serial_cal_bcc((unsigned char*)&data[1],4);
    data[6]=0xCA;
    retval = serial_send_data(uart_port,(char*)data,7);
    return TRUE;
}
//读取中科讯联卡头
int read_zkxl_card(int uart_port,char *value)
{
    unsigned char package[256];
    unsigned short datalen=0;
    int ret=0,i=0;

    if(value==NULL)
        return FALSE;

    ret=recvPackage(uart_port,package);
    sendCmdData(uart_port,22);
    if(ret<=0){
        return FALSE;

    }
    datalen=ret;

    memset(value,0,sizeof(value));
    for(i=0;i<datalen-1;i++)
        sprintf(value+i*2,"%02X",package[4+i]);
    serial_clear((TCOM)uart_port); //清空串口缓存
    return TRUE;

}
int query_id_card( int uart_port )
{
    int retval = 0;
    unsigned char RecvBuf[256];
    unsigned char *rec_buf = NULL;
    int overtime=0;
    struct timeval tv1,tv2;
    unsigned char SendBuf[] = { 0xaa, 0xaa, 0xaa, 0x96, 0x69, 0x00, 0x03, 0x20, 0x01, 0x22 };
    memset(RecvBuf, 0, sizeof(RecvBuf));

    rec_buf = (unsigned char *)RecvBuf;
    retval = serial_send_data(uart_port, (char *)SendBuf, 10);

    if(retval == ERROR || retval!= 10)
    {
        return ERROR;
    }

    gettimeofday(&tv1,NULL);
    gettimeofday(&tv2,NULL);

    overtime=tv1.tv_sec *1000000+tv1.tv_usec;
    while(1)
    {
        if(abs( (tv2.tv_sec *1000000 + tv2.tv_usec) - overtime) > 10000)
        {
            return ERROR;
        }
        gettimeofday(&tv2,NULL);
        retval = serial_recv_onebyte(uart_port, (char *)rec_buf);
        if (retval == ERROR )
        {
            return ERROR;
        }
        if(*rec_buf != 0xaa)// 包头
        {
            continue;
        }
        else
        {
            break;
        }
    }
    rec_buf++;
    retval = serial_recv_data(uart_port,rec_buf,14);
    if (retval != 14 || RecvBuf[9]!=0x9f || RecvBuf[8]!=0x00 || RecvBuf[7]!=0x00)
    {
        return ERROR;
    }
    /*
    int i = 0;
    for(i=0;i<15;i++)
    	printf("%02X ",RecvBuf[i]);
    printf("\n");
*/
    return TRUE;

}

int select_id_card( int uart_port )
{
    int retval = 0;
    unsigned char RecvBuf[256];
    unsigned char *rec_buf = NULL;
    int overtime=0;
    struct timeval tv1,tv2;
    unsigned char SendBuf[] = { 0xaa, 0xaa, 0xaa, 0x96, 0x69, 0x00, 0x03, 0x20, 0x02, 0x21 };
    memset(RecvBuf, 0, sizeof(RecvBuf));

    rec_buf = (unsigned char *)RecvBuf;
    retval = serial_send_data(uart_port, (char *)SendBuf, 10);

    if(retval == ERROR || retval!= 10)
    {
        return ERROR;
    }

    gettimeofday(&tv1,NULL);
    gettimeofday(&tv2,NULL);

    overtime=tv1.tv_sec *1000000+tv1.tv_usec;
    while(1)
    {
        if(abs( (tv2.tv_sec *1000000 + tv2.tv_usec) - overtime) > 10000)
        {
            return ERROR;
        }
        gettimeofday(&tv2,NULL);
        retval = serial_recv_onebyte(uart_port, (char *)rec_buf);
        if (retval == ERROR )
        {
            return ERROR;
        }
        if(*rec_buf != 0xaa)// 包头
        {
            continue;
        }
        else
        {
            break;
        }
    }
    rec_buf++;
    retval = serial_recv_data(uart_port,rec_buf,18);
    if (retval != 18 || RecvBuf[9]!=0x90 || RecvBuf[8]!=0x00 || RecvBuf[7]!=0x00)
    {
        return ERROR;
    }
    /*
    int i = 0;
    for(i=0;i<19;i++)
    	printf("%02X ",RecvBuf[i]);
    printf("\n");
    */
    return TRUE;

}

#ifndef _ARM_A23

int Decode2BmpFile(const char * wltFile, const char * bmpFile)
{
    if( access(datpath,0)==-1||access(wltpath,0)==-1)
    {
    	printf("zp.wlt not exist or base.dat not exist\n");
        return 0 ;
    }
    FILE *fp = NULL;
    unsigned char pData[2000];
    memset(pData,0,sizeof(pData));
    fp = fopen(wltFile,"rb");
    fread(pData,1,1024,fp);
    fclose(fp);

    unsigned char pOutBuffer[126 * 102 * 3 + 1024];
    int outlength = 0;
    memset(pOutBuffer,0,sizeof(pOutBuffer));
    //printf("@@@@@@@@===2_2\n");
    int ret = CV_Decode(pData, 1024, pOutBuffer, CV_FORMAT_BMP, &outlength);
    //printf("@@@@@@@@===2_3\n");
    printf("Decode return %d,%d\n",ret,outlength);
    /*
	int i=0;
    for(i=0;i<1025;i++)
    	printf(" %d",pData[i]);
    printf("\n");
    */
    FILE *fp_des = NULL;
    fp_des = fopen(bmpFile,"w+");
    fwrite(pOutBuffer,1,outlength,fp_des);
    fclose(fp_des);
    return ret;
}
int getbmp()
{
    int ret =-1;
    unsigned int filesize=0;
    FILE *fp = NULL;
    struct stat sbuf;
    int pBaseData[2000];

    memset(pBaseData,0,sizeof(pBaseData));
    fp = fopen(datpath,"r");

    fstat(fileno(fp), &sbuf);
    filesize = sbuf.st_size;
    fread((char*)pBaseData,1,filesize,fp);

    /*
    int i=0;
    for(i=0;i<lengthbase;i++)
    	printf(" %d",pBaseData[i]);
    printf("\n");
    */
    int err = CV_Initialize(liccode, pBaseData);
    if(err != CV_ERR_NONE)
    {
        printf("CV_Initialize errno = %d\n",err);
       	return -1;
    }
    else
    {
        ret = Decode2BmpFile(wltpath, outputBmp);
    }
    fclose(fp);
    CV_Finalize();
    return ret;
}
int read_id_info( int uart_port, char *buf )
{
    int retval = 0;
    int ecc = 0, i = 0;
    unsigned char RecvBuf[1300];
    unsigned char photo[1300];
    unsigned char *rec_buf = NULL;
    int overtime=0;
    struct timeval tv1,tv2;
    unsigned char SendBuf[] = { 0xaa, 0xaa, 0xaa, 0x96, 0x69, 0x00, 0x03, 0x30, 0x01, 0x32 };
    memset(RecvBuf, 0, sizeof(RecvBuf));

    rec_buf = (unsigned char *)RecvBuf;
    retval = serial_send_data(uart_port, (char *)SendBuf, 10);

    if(retval == ERROR || retval!= 10)
    {
        return ERROR;
    }

    gettimeofday(&tv1,NULL);
    gettimeofday(&tv2,NULL);

    overtime=tv1.tv_sec *1000000+tv1.tv_usec;
    while(1)
    {
        if(abs( (tv2.tv_sec *1000000 + tv2.tv_usec) - overtime) > 2000000)
        {
            return ERROR;
        }
        gettimeofday(&tv2,NULL);
        retval = serial_recv_onebyte(uart_port, (char *)rec_buf);
        if (retval == ERROR )
        {
            return ERROR;
        }
        if(*rec_buf != 0xaa)// 包头
        {
            continue;
        }
        else
        {
            break;
        }
    }
    rec_buf++;
    retval = serial_recv_data(uart_port,rec_buf,1294);
    /*
	for(i=0;i<1300;i++)
		printf("%02X ",RecvBuf[i]);
	printf("\n");
	*/
    if (retval != 1294 || RecvBuf[9]!=0x90 || RecvBuf[8]!=0x00 || RecvBuf[7]!=0x00)
    {
        return ERROR;
    }

    for(i=5;i<1294;i++)
    	ecc ^= RecvBuf[i];
    if(ecc != RecvBuf[1294])
    {
    	printf("ecc error\n");
    	return ERROR;
    }
    printf("ecc ok\n");
    memcpy(buf,RecvBuf+14,256);
    memset(photo,0,sizeof(photo));
    memcpy(photo,RecvBuf+270,1024);
    /*
	for(i=0;i<1295;i++)
		printf("%02X ",photo[i]);
	printf("\n");
	*/

    FILE *fp = NULL;
    int length = 0;
    fp = fopen(wltpath,"w+");
    if(fp == NULL)
    {
    	return ERROR;
    }
    length = fwrite(photo,1,1024,fp);
    if(length != 1024)
    	return ERROR;
    fclose(fp);
    //getbmp();
    return TRUE;

}

/*****************************************************************************
 * 将一个字符的Unicode(UCS-2和UCS-4)编码转换成UTF-8编码.
 *
 * 参数:
 *    unic     字符的Unicode编码值
 *    pOutput  指向输出的用于存储UTF8编码值的缓冲区的指针
 *
 * 返回值:
 *    返回转换后的字符的UTF8编码所占的字节数, 如果出错则返回 0 .
 *
 * 注意:
 *     1. UTF8没有字节序问题, 但是Unicode有字节序要求;
 *        字节序分为大端(Big Endian)和小端(Little Endian)两种;
 *        在Intel处理器中采用小端法表示, 在此采用小端法表示. (低地址存低位)
 *     2. 请保证 pOutput 缓冲区有最少有 6 字节的空间大小!
 ****************************************************************************/
int enc_unicode_to_utf8_one(unsigned long unic, char *pOutput)
{
    if ( unic <= 0x0000007F )
    {
        // * U-00000000 - U-0000007F:  0xxxxxxx
        *pOutput     = (unic & 0x7F);
        return 1;
    }
    else if ( unic >= 0x00000080 && unic <= 0x000007FF )
    {
        // * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
        *(pOutput+1) = (unic & 0x3F) | 0x80;
        *pOutput     = ((unic >> 6) & 0x1F) | 0xC0;
        return 2;
    }
    else if ( unic >= 0x00000800 && unic <= 0x0000FFFF )
    {
        // * U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
        *(pOutput+2) = (unic & 0x3F) | 0x80;
        *(pOutput+1) = ((unic >>  6) & 0x3F) | 0x80;
        *pOutput     = ((unic >> 12) & 0x0F) | 0xE0;
        return 3;
    }
    else if ( unic >= 0x00010000 && unic <= 0x001FFFFF )
    {
        // * U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        *(pOutput+3) = (unic & 0x3F) | 0x80;
        *(pOutput+2) = ((unic >>  6) & 0x3F) | 0x80;
        *(pOutput+1) = ((unic >> 12) & 0x3F) | 0x80;
        *pOutput     = ((unic >> 18) & 0x07) | 0xF0;
        return 4;
    }
    else if ( unic >= 0x00200000 && unic <= 0x03FFFFFF )
    {
        // * U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        *(pOutput+4) = (unic & 0x3F) | 0x80;
        *(pOutput+3) = ((unic >>  6) & 0x3F) | 0x80;
        *(pOutput+2) = ((unic >> 12) & 0x3F) | 0x80;
        *(pOutput+1) = ((unic >> 18) & 0x3F) | 0x80;
        *pOutput     = ((unic >> 24) & 0x03) | 0xF8;
        return 5;
    }
    else if ( unic >= 0x04000000 && unic <= 0x7FFFFFFF )
    {
        // * U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        *(pOutput+5) = (unic & 0x3F) | 0x80;
        *(pOutput+4) = ((unic >>  6) & 0x3F) | 0x80;
        *(pOutput+3) = ((unic >> 12) & 0x3F) | 0x80;
        *(pOutput+2) = ((unic >> 18) & 0x3F) | 0x80;
        *(pOutput+1) = ((unic >> 24) & 0x3F) | 0x80;
        *pOutput     = ((unic >> 30) & 0x01) | 0xFC;
        return 6;
    }

    return 0;
}
//民族
char *getNation(int code)
{
    switch(code){
    case 0X1: return ("汉");break;
    case 0X2: return ("蒙古");break;
    case 0X3: return ("回");break;
    case 0X4: return ("藏");break;
    case 0X5: return ("维吾尔");break;
    case 0X6: return ("苗");break;
    case 0X7: return ("彝");break;
    case 0X8: return ("壮");break;
    case 0X9: return ("布依");break;
    case 0X10: return ("朝鲜");break;
    case 0X11: return ("满");break;
    case 0X12: return ("侗");break;
    case 0X13: return ("瑶");break;
    case 0X14: return ("白");break;
    case 0X15: return ("土家");break;
    case 0X16: return ("哈尼");break;
    case 0X17: return ("哈萨克");break;
    case 0X18: return ("傣");break;
    case 0X19: return ("黎");break;
    case 0X20: return ("傈僳");break;
    case 0X21: return ("佤");break;
    case 0X22: return ("畲");break;
    case 0X23: return ("高山");break;
    case 0X24: return ("拉祜");break;
    case 0X25: return ("水");break;
    case 0X26: return ("东乡");break;
    case 0X27: return ("纳西");break;
    case 0X28: return ("景颇");break;
    case 0X29: return ("柯尔克孜");break;
    case 0X30: return ("土");break;
    case 0X31: return ("达斡尔");break;
    case 0X32: return ("仫佬");break;
    case 0X33: return ("羌");break;
    case 0X34: return ("布朗");break;
    case 0X35: return ("撒拉");break;
    case 0X36: return ("毛南");break;
    case 0X37: return ("仡佬");break;
    case 0X38: return ("锡伯");break;
    case 0X39: return ("阿昌");break;
    case 0X40: return ("普米");break;
    case 0X41: return ("塔吉克");break;
    case 0X42: return ("怒");break;
    case 0X43: return ("乌孜别克");break;
    case 0X44: return ("俄罗斯");break;
    case 0X45: return ("鄂温克");break;
    case 0X46: return ("德昂");break;
    case 0X47: return ("保安");break;
    case 0X48: return ("裕固");break;
    case 0X49: return ("京");break;
    case 0X50: return ("塔塔尔");break;
    case 0X51: return ("独龙");break;
    case 0X52: return ("鄂伦春");break;
    case 0X53: return ("赫哲");break;
    case 0X54: return ("门巴");break;
    case 0X55: return ("珞巴");break;
    case 0X56: return ("基诺");break;
    case 0X97: return ("其他");break;
    case 0X98: return ("外国血统中国籍人士");break;
    default : return ("");
}
}
int read_ID2_card(int uart_port,char *value)
{
    int ret = 0, i = 0, len = 0;
    unsigned long uni_c;
    char buf[300];
    unsigned char uni_buf[10];
    char tmp[10],utf_buf[10];
    char ret_value[1024];
    //printf("===========1\n");
    ret = query_id_card( uart_port );
    if(ret!=TRUE)
    {
        return ERROR;
    }
    ret =select_id_card( uart_port );
    if(ret!=TRUE)
    {
        return ERROR;
    }
    memset(buf,0,sizeof(buf));
    ret = read_id_info(uart_port, buf);
    if(ret!=TRUE)
    {
        return ERROR;
    }
    /*
	for(i=0;i<256;i++)
		printf("%02X ",buf[i]);
	printf("\n");
	*/
    //printf("===========4\n");
    memset(ret_value,0,sizeof(ret_value));
    for(i=0;i<15;i++)
    {
        memset(uni_buf,0,sizeof(uni_buf));
        memset(tmp,0,sizeof(tmp));
        memset(utf_buf,0,sizeof(utf_buf));
        sprintf(tmp,"%02X%02X",buf[i*2+1],buf[i*2]);
        gsmString2Bytes(tmp, uni_buf, strlen(tmp));
        uni_c = HexToUInt(uni_buf, 2, 1);
        //printf("===%x\n",uni_c);
        len = enc_unicode_to_utf8_one(uni_c, utf_buf);
        //printf("===%d\n",len);
        strncat(ret_value,utf_buf,len);
    }
    strcat(ret_value,",");
    switch (buf[30])
    {
    case 0X32:strcat(ret_value,"女");break;
    case 0X31:strcat(ret_value,"男");break;
    case 0X39:strcat(ret_value,"未说明");break;
    case 0X30:strcat(ret_value,"未知");break;
    default:strcat(ret_value,"其他");break;
    }
    strcat(ret_value,",");
    char nation_buf[10];
    unsigned char nation[10];
    int len_nation = 0;

    memset(nation_buf,0,sizeof(nation_buf));
    memset(nation,0,sizeof(nation));

    sprintf(nation_buf,"%c%c",buf[32],buf[34]);
    gsmString2Bytes(nation_buf, nation, strlen(nation_buf));
    len_nation = HexToUInt(nation, 1, 1);
    strcat(ret_value,getNation(len_nation));
    strcat(ret_value,",");

    char birth[32];
    memset(birth,0,sizeof(birth));
    sprintf(birth,"%c%c%c%c%c%c%c%c",buf[36],buf[38],buf[40],buf[42],buf[44],buf[46],buf[48],buf[50]);
    //printf("birth============%s\n",birth);
    strcat(ret_value,birth);
    strcat(ret_value,",");

    unsigned char addr_buf[20];
    char addr_tmp[10],addr[20];
    unsigned long  uni_addr=0,len_addr=0;
    for(i=0;i<35;i++)
    {
        memset(addr_buf,0,sizeof(addr_buf));
        memset(addr_tmp,0,sizeof(addr_tmp));
        memset(addr,0,sizeof(addr));
        sprintf(addr_tmp,"%02X%02X",buf[52+i*2+1],buf[52+i*2]);
        gsmString2Bytes(addr_tmp, addr_buf, strlen(addr_tmp));
        uni_addr = HexToUInt(addr_buf, 2, 1);
        //printf("===%x\n",uni_addr);
        len_addr = enc_unicode_to_utf8_one(uni_addr, addr);
        //printf("===%d\n",len_addr);
        strncat(ret_value,addr,len_addr);
    }
    strcat(ret_value,",");

    char id_num[20];
    memset(id_num,0,sizeof(id_num));
    sprintf(id_num,"%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
            buf[122],buf[124],buf[126],buf[128],buf[130],buf[132],buf[134],buf[136],
            buf[138],buf[140],buf[142],buf[144],buf[146],buf[148],buf[150],buf[152],
            buf[154],buf[156]);
    //printf("id_num============%s\n",id_num);
    strcat(ret_value,id_num);
    strcat(ret_value,",");
    unsigned char gov_buf[20];
    char gov_tmp[10],gov[20];
    int uni_gov=0,len_gov=0;
    for(i=0;i<15;i++)
    {
        memset(gov_buf,0,sizeof(gov_buf));
        memset(gov_tmp,0,sizeof(gov_tmp));
        memset(gov,0,sizeof(gov));
        sprintf(gov_tmp,"%02X%02X",buf[158+i*2+1],buf[158+i*2]);
        gsmString2Bytes(gov_tmp, gov_buf, strlen(gov_tmp));
        uni_gov = HexToUInt(gov_buf, 2, 1);
        len_gov = enc_unicode_to_utf8_one(uni_gov, gov);
        strncat(ret_value,gov,len_gov);
    }
    strcat(ret_value,",");

    char date_num[20];
    memset(date_num,0,sizeof(date_num));
    sprintf(date_num,"%c%c%c%c%c%c%c%c,%c%c%c%c%c%c%c%c",
            buf[188],buf[190],buf[192],buf[194],buf[196],buf[198],buf[200],buf[202],
            buf[204],buf[206],buf[208],buf[210],buf[212],buf[214],buf[216],buf[218]);
    //printf("date_num============%s\n",date_num);
    strcat(ret_value,date_num);

    strcpy(value,ret_value);
    //printf("===========8\n");
    return ret;
}
#endif
/*
int set_id_card_photo_addr(char *path)
{
	if(path == NULL)
	{
		return ERROR;
	}
	memset(outputBmp,0,sizeof(outputBmp));
	strcpy(outputBmp,path);
	return TRUE;
}
*/

const unsigned short crc_ta[16] = { 0x0000,0x1021,0x2042,0x3063,
                                    0x4084,0x50a5,0x60c6,0x70e7,
                                    0x8108,0x9129,0xa14a,0xb16b,
                                    0xc18c,0xd1ad,0xe1ce,0xf1ef};

unsigned short crc16_datas(unsigned char *ptr, unsigned char len)
{
    unsigned char da;
    unsigned short  crc16_value=0;
    unsigned char i=0;
    for(i=0;i<len;i++)
    {
        da=crc16_value>>12;
        crc16_value<<=4;
        crc16_value^=crc_ta[da^(ptr[i]/16)];

        da=crc16_value>>12;
        crc16_value<<=4;
        crc16_value^=crc_ta[da^(ptr[i]&0x0f)];
    }
    return crc16_value;
}
int read_mj_card(int uart_port,char *value)
{
    char data[4096],*p,p_value[4096];
    char card_num[16];
    char rf_value[96],crc_value[4],crc[4];
    int retval = 0, p_double = 0;
    int i = 0, p_sign = 0;
    static char data_buffer[4096];
    static int buff_len = 0;
    unsigned short  crc16_value=0;
    memset(data,0,sizeof(data));
    memset(card_num,0,sizeof(card_num));
    retval = serial_recv_data_all(uart_port,(unsigned char *)data);

    if(retval == 0)
    {
    	return ERROR;
    }
    memcpy(&data_buffer[buff_len], data, retval);

    buff_len += retval;
    if(buff_len < 24)
    {
    	return ERROR;
    }
    //printf("sdk====================%d\n",buff_len);
    //int j = 0;
    //for(j=0;j<buff_len;j++)
    //{
    //	printf("%02X ",data_buffer[j]);
    //}
    //printf("\n");
    memset(p_value,0,sizeof(p_value));
    for (i = buff_len - 24; i >= 0 ; i--)
    {

        if ((data_buffer[i] == 0xFF) && (data_buffer[i+1] == 0xFF) && (data_buffer[i+2] == 0x00)&& (data_buffer[i+3] == 0x18))
        {

            memset(rf_value,0,sizeof(rf_value));
            memcpy(rf_value,&data_buffer[i],24);

            crc16_value = crc16_datas(rf_value, 22);
            memset(crc_value,0,sizeof(crc_value));
            sprintf(crc_value,"%04X",crc16_value);
            memset(crc,0,sizeof(crc));
            string2hex(crc_value, crc, 4);
            if((crc[0]!=data_buffer[i+22])||(crc[1]!=data_buffer[i+23]))
            {
                continue;
            }
            gsmBytes2String((unsigned char*)&data_buffer[i + 8], card_num, 4);
            //            printf("aaaaaaaaaaaaa=====%s\n",p_value);
            p = strtok(p_value,",");
            while(p!=NULL)
            {
                //            	printf("ccccccccccccccc=====%s,%s\n",p,card_num);
            	if(strcmp(p,card_num)==0)
            	{

                    p_double = 1;
                    break;
            	}
            	p=strtok(NULL,",");

            }
            if(p_double==1)
            {
            	memcpy(p_value,value,strlen(value));
            	p_double = 0;
            	continue;
            }
            memset(p_value,0,sizeof(p_value));
            strcat(value,card_num);
            strcat(value,",");
            memcpy(p_value,value,strlen(value));
            //			printf("hhhhhhhhhhhhh=====%s\n",value);
            p_sign = 1;

        }
    }
    if(p_sign)
    {
        //    	printf("====value===%s\n",value);
    	memset(data_buffer,0,sizeof(data_buffer));
    	buff_len = 0;
    	return TRUE;
    }
    memset(data_buffer,0,sizeof(data_buffer));
    buff_len = 0;
    return ERROR;
}
int read_hirf_24g_card(int uart_port,char *value)
{
    char data[4096],*p,p_value[4096];
    char card_num[16];
    char rf_value[96];
    int retval = 0, p_double = 0;
    int i = 0, p_sign = 0;
    static char data_buffer[4096];
    static int buff_len = 0;
    unsigned short  crc16_value=0;
    memset(data,0,sizeof(data));
    memset(card_num,0,sizeof(card_num));
    retval = serial_recv_data_all(uart_port,(unsigned char *)data);
    if(retval == 0)
    {
    	return ERROR;
    }
    memcpy(&data_buffer[buff_len], data, retval);

    buff_len += retval;
    if(buff_len < 17)
    {

    	return ERROR;
    }
    //printf("sdk====================%d,%d\n",buff_len,uart_port);
    //int j = 0;
    //for(j=0;j<buff_len;j++)
    //{
    //	printf("%02x ",data_buffer[j]);
    //}
    //printf("\n");


    memset(p_value,0,sizeof(p_value));
    for (i = buff_len - 17; i >= 0 ; i--)
    {

        if ((data_buffer[i] == 0x02) && (data_buffer[i+15] == 0x0D) && (data_buffer[i+16] == 0x0A))
        {
            sprintf(card_num,"%c%c%c%c%c%c%c%c",
                    data_buffer[i+5],data_buffer[i+6],data_buffer[i+7],data_buffer[i+8],
                    data_buffer[i+9],data_buffer[i+10],data_buffer[i+11],data_buffer[i+12]);

            //            printf("aaaaaaaaaaaaa=====%s\n",p_value);
            p = strtok(p_value,",");
            while(p!=NULL)
            {
            	if(strcmp(p,card_num)==0)
            	{
                    p_double = 1;
                    break;
            	}
            	p=strtok(NULL,",");

            }
            if(p_double==1)
            {
            	memcpy(p_value,value,strlen(value));
            	p_double = 0;
            	continue;
            }
            memset(p_value,0,sizeof(p_value));
            strcat(value,card_num);
            strcat(value,",");
            memcpy(p_value,value,strlen(value));
            //			printf("hhhhhhhhhhhhh=====%s\n",value);
            p_sign = 1;

        }
    }
    if(p_sign)
    {
        //    	printf("====value===%s\n",value);
    	memset(data_buffer,0,sizeof(data_buffer));
    	buff_len = 0;
    	return TRUE;
    }
    memset(data_buffer,0,sizeof(data_buffer));
    buff_len = 0;
    return ERROR;

}
/*
正元读取应用序号
00A4000002 3F00
00A4000002 ADF2
00B0951A04
  */
int zy_24G_card_app(int fd,char *value)
{
    unsigned char src[][32] = {
        {0X00,0XA4,0X00,0X00,0X02,0X3F,0X00},
        {0X00,0XA4,0X00,0X00,0X02,0XAD,0XF2},
        {0X00,0XB0,0X95,0X1A,0X04},
    };
    unsigned char slen[] = {7, 7, 5};
    unsigned char answerBuf[256];
    int ret, j;
    unsigned char simCardNo[8];
    static int error_num=0,error_num_hw=0;

    ret = queryPhoneCard(fd, 0, simCardNo);
    if (ret != 0)
    {
    	error_num++;
    	error_num_hw++;
    	if(error_num>20)
    	{
            error_num=0;
            reset_module(fd);
    	}
    	if(error_num_hw > 480)//480
    	{
            error_num_hw=0;
            //哈尔滨新中新飞线
            set_gpio_on(RED_LED);
            usleep(150000);
            set_gpio_off(RED_LED);
    	}

        return -1;
    }
    error_num=0;
    error_num_hw=0;

    for (j = 0; j < 3; j++)
    {
        memset (answerBuf, 0 , sizeof(answerBuf));
        ret = nationzTechApduChannel(src[j], slen[j],
                                     answerBuf, 255, 1000, serial_fd[fd]);
        if (ret < 2)
        {
            break;
        }
        else
        {
            if (answerBuf[ret - 2] == 0x90 && answerBuf[ret - 1] == 0x00)
            {
                if (j == 2)
                {
                    memset(value, 0, sizeof(value));
                    sprintf(value, "%02X%02X%02X%02X",
                            (answerBuf[3]),(answerBuf[2]),answerBuf[1],answerBuf[0]);
                    serial_clear((TCOM)fd); //清空串口缓存
                    return TRUE;

                }
            }
            else
            {
                ret = -99;
                break;
            }
        }
    }


    serial_clear((TCOM)fd); //清空串口缓存
    return ret;
}


/**
  读取高频卡头
  AA AA AB F5 A1 42 B4 E7 73
  AA AA AB 前导
  F5 A1 42 B4  卡号
  E7 电压值
  73 前5个字节的和校验低字节
  波特率9600
usb	接收数据格式2e a3 6a 76 07 d8 de c6-按键数据
其中2e a3 6a 76 -4字节是IC卡卡号，
0x07高4位是流水号，低4为是键值
分为定位模式和按键模式；其中定位模式为：只要卡片还在读卡区，则会不停的读到该卡片，直到离开读卡区；
按键模式：按一次建，读到一次卡号及按键信息。
  */
#define HID_MAX_PACKET_SIZE 64
#define HID_BUFFER_SIZE (1024)// 1K bytes
static unsigned char *KeyTable_2_4G[] = {"NULL","0","=",".","8","9","7","5","6","4","2","3","1","pause","stop","play"};
typedef struct
{
    char null_array[HID_MAX_PACKET_SIZE];
}buffer_offset_size_t;

static char hid_read_buffer[HID_BUFFER_SIZE];


static int hid_file_handle = 0;
static const char* hid_file_name = "/dev/hidraw0";

int     surplusLen=0; //
char surplusBuf[1024]={""};
time_t usbResetTime;
int powerModuleFlag=0;
int resetTimeInterval=2;
int CARDDEVICEMODE=0;  //0 是定位模式，1是按键模式
int HfCardOverTime=30;//单位：秒，即多长时间读不到卡会返回一次卡离开
long int lasttime;
//overtime :单位：秒，即多长时间读不到卡会返回一次卡离开状态
long int setHfCardOverTime(int overtime)
{
    HfCardOverTime=overtime;
}
//mode ://0 是定位模式，1是按键模式
int setHfCardMode(int mode)
{
    CARDDEVICEMODE=mode&0xFD;
    if(CARDDEVICEMODE)
        setHfCardOverTime(0);
     else setHfCardOverTime(30);
     setCardDeviceMode();
    // resetCardDevice();
}
int powerCardDevice(void)
{

    if(llabs(time(NULL)-usbResetTime) < resetTimeInterval)  //秒为单位
    {
        return -1;
    }
    time(&usbResetTime);
    switch(powerModuleFlag){
    case 0:
        powerModuleFlag = 1;
        set_gpio_on(USB_POWER);
        resetTimeInterval = 2;
        break;
    case 1:
        set_gpio_off(USB_POWER);
        powerModuleFlag = 2;
        resetTimeInterval = 2;
        break;
    case 2:
        resetTimeInterval = 3;
        powerModuleFlag = 3;
    default:
        resetTimeInterval = 2;
        powerModuleFlag = 0;
        hidCardDeviceInit();
    }
    return 1;
}

int resetCardDevice(void)
{
    if(hid_file_handle>0)
    {
        close(hid_file_handle);
        hid_file_handle = 0;        
    }
    powerCardDevice();


}
//
int hidCardDeviceInit(void)
{
    char command[128];
    set_gpio_off(USB_POWER);
    //sleep(2);
    time(&usbResetTime);
    if(access(hid_file_name,F_OK)!=0)
    {
        hid_file_handle = 0;
        return -1;
    }
    if(hid_file_handle>0)
    {
        return hid_file_handle;
    }
    sprintf(command,"/mnt/app/root %s",hid_file_name);
    system(command);

    if ((hid_file_handle = open(hid_file_name, O_RDWR, 0666)) < 0)
    {
        hid_file_handle  = 0;
        return -1;
    }
    setCardDeviceMode();
    initTgCardNoHash();
    lasttime=GetTickCount();
    return hid_file_handle;
}

int hidCardDeviceRead(void* buffer, int buffer_size)
{
    if (hid_file_handle<=0)
    {
        return -1;
    }

    if(buffer == NULL)
    {
        perror("hid_read::pointer error!");
        return -1;
    }
    return uart_recv_data(hid_file_handle, buffer, buffer_size);
}


int hidCardDeviceWrite(void* buffer, int buffer_size)
{
    int return_v = 0;
    int writting_count = buffer_size / HID_MAX_PACKET_SIZE;
    int remainding_size = buffer_size % HID_MAX_PACKET_SIZE;
    buffer_offset_size_t* buffer_offset = (buffer_offset_size_t*)buffer;

    if (hid_file_handle<=0)
    {
        return -1;
    }

    if(buffer == NULL)
    {
        perror("hid_write::pointer error!");
        return -1;
    }

    while(writting_count--)
    {
        return_v = write(hid_file_handle, buffer_offset,HID_MAX_PACKET_SIZE);
        if(return_v < 0)
        {
            perror("hid_write::writting error!");
            return return_v;
        }
        buffer_offset++;
    }

    return_v = write(hid_file_handle, buffer_offset, remainding_size);

    return return_v;
}

//
int setCardDeviceMode()
{
    int ret = 0;
    char hid_write_buffer[HID_BUFFER_SIZE];

    memset(hid_write_buffer, 0x00, HID_BUFFER_SIZE);
    hid_write_buffer [0] = 0X00;
    hid_write_buffer [1] = 0X01;
    hid_write_buffer [2] = CARDDEVICEMODE&0xff;
    hid_write_buffer [3] = 0X01;
    hid_write_buffer [4] = 0X02;
    hid_write_buffer [5] = 0X03;
    hid_write_buffer [6] = 0X04;
    hid_write_buffer [7] = 0X05;
    hid_write_buffer [8] = 0X06;

    ret =  hidCardDeviceWrite(hid_write_buffer, 9);
    ret =  hidCardDeviceWrite(hid_write_buffer, 9);
    return ret;
}

int hidHightFrequencyRecv(unsigned char *value,int len)
{
    int i=0;
    int overtime=0;
    struct timeval tv1,tv2;
    int len_total=0,len_tmp=0;

    gettimeofday(&tv1,NULL);
    gettimeofday(&tv2,NULL);

    overtime=tv1.tv_sec *1000000+tv1.tv_usec;
    while(1)
    {
        if(abs( (tv2.tv_sec *1000000 + tv2.tv_usec) - overtime) > 300000)
        {
            //            printf("%s,time out\n",__func__);
            break;
        }
        len_tmp=0;
        len_tmp=hidCardDeviceRead((char *)value+len_total,len-len_total);
        gettimeofday(&tv2,NULL);
        if(len_tmp<0)
        {
            return len_total;
        }
        else if(len_tmp == 0)
        {
            if((i++)>5)//10 lxy 2013-3-13 text_card 卡
            {
                return len_total;
            }
        }

        len_total=len_total+len_tmp;
    }

    return len_total;
}

//2e a3 6a 76 07 d8 de c6
int readHighFrequency( char *data)
{
    int i=0,j=0;
    int ret=0,sLen=0,dLen=0;
    char tmp[128]={""},tmp2[32];
    memset(data,0,1024);


    if(labs(GetTickCount()-lasttime)>3600000||hid_file_handle<=0){
        ret = -1;
        surplusLen= 0;
        memset(surplusBuf,0,sizeof(surplusBuf));
        resetCardDevice();
    }

    ret=hidCardDeviceRead((char *)surplusBuf+surplusLen,sizeof(surplusBuf)-surplusLen);

  // LOGI("ret:%d,%d\n",ret,surplusLen);

    if(ret >0)
    {
        lasttime=GetTickCount();
        surplusLen = surplusLen+ret;
    }else if(ret<0)
    {
       resetCardDevice();
    }


    for(i=0;i+8<=surplusLen;)
    {
        memset(tmp,0,sizeof(tmp));
        tmp[0] = surplusBuf[i+3];
        tmp[1] = surplusBuf[i+2];
        tmp[2] = surplusBuf[i+1];
        tmp[3] = surplusBuf[i];
        tmp[4] = surplusBuf[i+4];
        memset(tmp2,0,sizeof(tmp2));
        hex2string(&surplusBuf[i],tmp2,8);
       // LOGI("%s %d %s",tmp2,(tmp[4]&0xf0)>>4,KeyTable_2_4G[tmp[4]&0x0f]);
        updateCardNoList(tmp);
        i+=8;
       // surplusLen -= 8;
    }
    if(i<surplusLen)
    {
        memset(tmp,0,sizeof(tmp));
        memcpy(tmp,&surplusBuf[i],surplusLen);
        memset(surplusBuf,0,sizeof(surplusBuf));
        memcpy(surplusBuf,tmp,surplusLen);
    }
    surplusLen = surplusLen -i;

    if(surplusLen <=0)
    {
        surplusLen = 0;
        memset(surplusBuf,0,sizeof(surplusBuf));
    }

    ret = filterCardNo(data);
   // if(ret>0)
   //  LOGI("ret:%d:%s\n",ret,data);
    return ret;
}

_TgCardNo *TgCardNoHash[256];
/*
   1）卡号第一次识别到为进入考勤区域，终端发送：ID，时间，卡号，电池电量，进入(0)
    2）卡号10秒（可设置）未读到为出考勤区域，终端发送：ID，时间，卡号，电池电量，离开(1)

*/
void Print()
{
    int i=0;
    _TgCardNo *p;

    for(i=0;i<256;i++){
        p = TgCardNoHash[i];
        if(p == NULL )
            continue;
        do
        {
            printf ("Print:%s,%d\n", p->cardNo,p->Electricity);
            p = p->next;     //移到下一个节点
        }
        while (p != NULL);
    }

}

/**完成初始化工作*/
void initTgCardNoHash()
{
    int i;
    for(i = 0 ; i < 256; i++){
        TgCardNoHash[i] = NULL;
    }
}
/*
unsigned long FNVHash1(char * data,int len)
{
    int i=0;
    unsigned long p = 16777619;
    unsigned long hash = (unsigned long)2166136261L;
    for(i=0;i<len;i++)
        hash = (hash ^ data[i]) * p;
    hash += hash << 13;
    hash ^= hash >> 7;
    hash += hash << 3;
    hash ^= hash >> 17;
    hash += hash << 5;
    return hash;
}
*/
int updateCardNoList(char *value)
{
    _TgCardNo *p;
    _TgCardNo *newCardNo=NULL;
    time_t cTime;
    char tmp[32]={""};
    unsigned long hashNum=0;

    hex2string(value,tmp,5);
    hashNum=FNVHash1(value,4)%256;
    p = TgCardNoHash[hashNum];
    cTime=time(NULL);
    while ( p != NULL)
    {
        if(strncmp(p->cardNo,tmp,8)==0)
        {
            p->sTime = cTime;
            return 1;
        }

        p = p->next;
    }

    newCardNo=(_TgCardNo *)malloc(sizeof(_TgCardNo));
    if(newCardNo==NULL)
        return FALSE;

    memset(newCardNo,0,sizeof(_TgCardNo));
    strncpy(newCardNo->cardNo,tmp,8);
    newCardNo->sTime = cTime;
    newCardNo->Electricity = (int)(value[4]&0XFF);//电池电量/按键值
    newCardNo->eTime = 0;
    //如果链表为空，将作为第一个节点
    if(TgCardNoHash[hashNum] == NULL)
    {
        TgCardNoHash[hashNum] = newCardNo;
        TgCardNoHash[hashNum]->next = NULL;
    }
    else //如果链表不空，将其放在第一个节点位置
    {
        newCardNo->next = TgCardNoHash[hashNum];
        TgCardNoHash[hashNum] = newCardNo;
    }
    return 1;
}


int updateCardNoList_ztx(char *value)
{
    _TgCardNo *p;
    _TgCardNo *newCardNo=NULL;
    time_t cTime;
    char tmp[32]={""};
    unsigned long hashNum=0;
	int vLen=strlen(value);


    hashNum=FNVHash1(value,vLen)%256;

    p = TgCardNoHash[hashNum];
    cTime=time(NULL);
    while ( p != NULL)
    {
        if(strncmp(p->cardNo,value,vLen)==0)
        {
            p->sTime = cTime;
            return 1;
        }

        p = p->next;
    }

    newCardNo=(_TgCardNo *)malloc(sizeof(_TgCardNo));
    if(newCardNo==NULL)
        return FALSE;

    memset(newCardNo,0,sizeof(_TgCardNo));
    strncpy(newCardNo->cardNo,value,vLen);
    newCardNo->sTime = cTime;
    newCardNo->Electricity = 0;
    newCardNo->eTime = 0;
    //如果链表为空，将作为第一个节点
    if(TgCardNoHash[hashNum] == NULL)
    {
        TgCardNoHash[hashNum] = newCardNo;
        TgCardNoHash[hashNum]->next = NULL;
    }
    else //如果链表不空，将其放在第一个节点位置
    {
        newCardNo->next = TgCardNoHash[hashNum];
        TgCardNoHash[hashNum] = newCardNo;
    }
    return 1;
}

//int filterCardNo(char *cardNo, unsigned char *data)
//jdev_id=jupk_read_jdev_id();
//p=localtime(&p->eTime); /*取得当地时间*/
//printf (“%d%d%d ”, (1900+p->tm_year),( l+p->tm_mon), p->tm_mday);
//printf(“%s%d:%d:%d\n”, wday[p->tm_wday],p->tm_hour, p->tm_min, p->tm_sec);
int filterCardNo( unsigned char *data)
{
    int jdev_id=0;
    int i=0;
    _TgCardNo *p,*p1;
    time_t cTime;
    struct tm *pTime;
    char dTime[128],tmp[128];
    int upData=0,dLen=0;
    int num=0;

    jdev_id=jupk_read_jdev_id();
    time(&cTime);

    for(i=0;i<256;i++)
    {
        p= TgCardNoHash[i];
        if(p==NULL)
            continue;
        p1=p;
        while(p != NULL)
        {
            memset(tmp,0,sizeof(tmp));
            memset(dTime,0,sizeof(dTime));
            upData=0;
            //LOGI("%s\n",p->cardNo);
            if(p->eTime==0)
            {
                pTime=localtime(&p->sTime);
                sprintf (dTime,"%04d-%02d-%02d %02d:%02d:%02d",
                         (1900+pTime->tm_year),( 1+pTime->tm_mon), pTime->tm_mday,pTime->tm_hour, pTime->tm_min, pTime->tm_sec);
                //ID，时间，卡号，电池电量，进入(0)/离开(1),定位模式0/按键模式1，按键值
                if(CARDDEVICEMODE)//按键模式
                     sprintf(tmp,"%d,%s,%.8s,%d,%d,%d,%s",jdev_id,dTime,p->cardNo,0,0,CARDDEVICEMODE,KeyTable_2_4G[(p->Electricity&0x0f)]);
                else //定位模式
                sprintf(tmp,"%d,%s,%.8s,%d,%d,%d,NULL",jdev_id,dTime,p->cardNo,p->Electricity,0,CARDDEVICEMODE);
                p->eTime=cTime;
                upData=1;
            }
            else{
                if(abs((int)difftime(cTime,p->sTime)) > HfCardOverTime)
                {
                    if(HfCardOverTime!=0){  //等于0时，只发送进入状态
                    pTime=localtime(&cTime);
                    sprintf (dTime,"%04d-%02d-%02d %02d:%02d:%02d",
                             (1900+pTime->tm_year),( 1+pTime->tm_mon), pTime->tm_mday,pTime->tm_hour, pTime->tm_min, pTime->tm_sec);
                    sprintf(tmp,"%d,%s,%.8s,%d,%d,%d,NULL",jdev_id,dTime,p->cardNo,(p->Electricity),1,CARDDEVICEMODE);
                }
                    upData=2;
                }
            }
//            printf("4:%s\n",tmp);
            if(upData!=0){
                if( (dLen+strlen(tmp))>=1024)
                    return TRUE;
                if(dLen == 0){
                    strcat(data,tmp);
                }
                else{
                    strcat(data,";");
                    dLen+=1;
                    strcat(data,tmp);
                }
                dLen+=strlen(tmp);

            }
            if(upData == 2)
            {
                if(p==TgCardNoHash[i]){
                    p1=p->next;
                    free(p);
                    p=p1;
                    TgCardNoHash[i]=p1;
                }
                else if(p->next == NULL){
                    p1->next=NULL;
                    free(p);
                    p=NULL;
                }
                else{
                    p1->next = p->next;
                    free(p);
                    p=p1->next;
                }

            }
            else{
                p1=p;
                p=p->next;
            }
        }
    }
//    if(num!=7)
//    LOGI("%s %d",__func__,num);
    if(dLen==0)
        return ERROR;
//    Print();
    return TRUE;


}


int filterCardNo_ztx( unsigned char *data)
{
    int jdev_id=0;
    int i=0;
    _TgCardNo *p,*p1;
    time_t cTime;
    struct tm *pTime;
    char dTime[128],tmp[128];
    int upData=0,dLen=0;

    jdev_id=jupk_read_jdev_id();
    time(&cTime);

    for(i=0;i<256;i++)
    {
        p= TgCardNoHash[i];
        if(p==NULL)
            continue;
        p1=p;
        while(p != NULL)
        {
            memset(tmp,0,sizeof(tmp));
            memset(dTime,0,sizeof(dTime));
            upData=0;
            if(p->eTime==0)
            {
                pTime=localtime(&p->sTime);
                sprintf (dTime,"%04d-%02d-%02d %02d:%02d:%02d",
                         (1900+pTime->tm_year),( 1+pTime->tm_mon), pTime->tm_mday,pTime->tm_hour, pTime->tm_min, pTime->tm_sec);
                //ID，时间，卡号，电池电量，进入(0)
                sprintf(tmp,"%d,%s,%.32s,%d,%d",jdev_id,dTime,p->cardNo,(p->Electricity),0);
                p->eTime=cTime;
                upData=1;
            }
            else{
                if(abs((int)difftime(cTime,p->sTime)) > 30)
                {
                    pTime=localtime(&cTime);
                    sprintf (dTime,"%04d-%02d-%02d %02d:%02d:%02d",
                             (1900+pTime->tm_year),( 1+pTime->tm_mon), pTime->tm_mday,pTime->tm_hour, pTime->tm_min, pTime->tm_sec);
                    sprintf(tmp,"%d,%s,%.32s,%d,%d",jdev_id,dTime,p->cardNo,(p->Electricity),1);
                    upData=2;
                }
            }
//            printf("4:%s\n",tmp);
            if(upData!=0){
                if( (dLen+strlen(tmp))>=1024)
                    return TRUE;
                if(dLen == 0){
                    strcat(data,tmp);
                }
                else{
                    strcat(data,";");
                    dLen+=1;
                    strcat(data,tmp);
                }
                dLen+=strlen(tmp);

            }
            if(upData == 2)
            {
                if(p==TgCardNoHash[i]){
                    p1=p->next;
                    free(p);
                    p=p1;
                    TgCardNoHash[i]=p1;
                }
                else if(p->next == NULL){
                    p1->next=NULL;
                    free(p);
                    p=NULL;
                }
                else{
                    p1->next = p->next;
                    free(p);
                    p=p1->next;
                }

            }
            else{
                p1=p;
                p=p->next;
            }
        }
    }
    if(dLen==0)
        return ERROR;
//    Print();
    return TRUE;


}

//读取900M 至天行
int readHighFrequency_szztx( int fd, unsigned char *data)
{
    int i=0;
    int ret=0,sLen=0,dLen=0;
    char tmp[128]={""};
	char value[1024];
	char *p = NULL;

    memset(data,0,1024);
	memset(value,0,1024);
	
	//调用郭福涛接口
	ret = read_ztx_g20(fd, value);	// 返回值为读取到的卡号数
    if(ret == 0) // 未读取到卡片
    {
        return -1;
    }
//printf("ret1=%d,%s\n", ret, value);
	p = value;
	for(i=0;i<ret;i++)	//按照逗号进行分割
	{
        memset(tmp,0,sizeof(tmp));
		memcpy(tmp, p, 32);
		p += 32;
		p++;
        updateCardNoList_ztx(tmp);				
	}
	
    //    printf("*********\n");
    //    Print();
    //    printf("*************end\n");
    ret = filterCardNo_ztx(data);
	
//    printf("ret2:%d:%s\n",ret,data);
    return ret;
}




//通过串口读二维码扫描仪（E20）数据

int read_qrcode_E20(int fd,unsigned char *data,int len)
{
    int ret=0;
       ret=serial_recv_data(fd,data,len);
       if(ret>0)
           return TRUE;
       return FALSE;
}


// 字节位倒序
static unsigned char Reverse(unsigned char uch) 
{ 
	unsigned char retValue = 0;    
	int i;  
	retValue |= (uch & 1);   
	for (i = 1; i < 8; i++) 
	{ 
		retValue <<= 1; 
		retValue |= (((1 << i) & uch) >> i);   
	}  
	return retValue; 
} 


// 锦州石化 支持iso15693协议卡片、卡头
int read_iso_15693card(int port, unsigned char *oData)
{
	unsigned char rBuf[32], phyCard[4];
	unsigned char xor = 0;
	unsigned char *p = NULL;
	int i = 0, comfd = -1, num = 0;

	comfd = get_uard_fd(port);
	if(comfd <= 0)
	{
		return -99; // 串口未初始化
	}

//printf("in read_iso_15693card, port = %d,	fd = %d\n", port, comfd);

	p = rBuf;
	num = UartRead(comfd, p, 5, 30);	// read head
	if(num != 5)
	{
//	printf("======== Not get card =======\n");
		return -1;
	}

    for (i = 0; i < 4; i++)
    {
        xor ^= p[i];
    }

	if(xor != p[i])	// 校验字节
	{
		return -1;
	}
	
//leeDebugData(rBuf, 5, 5, 2);
//	gsmBytes2String(rBuf, oData, 4);

#if 0	// 20180111
	phyCard[0] = rBuf[3];
	phyCard[1] = rBuf[2];
	phyCard[2] = rBuf[1];
	phyCard[3] = rBuf[0];

	phyCard[0] = Reverse(phyCard[0]);
	phyCard[1] = Reverse(phyCard[1]);
	phyCard[2] = Reverse(phyCard[2]);
	phyCard[3] = Reverse(phyCard[3]);
#endif

	phyCard[0] = rBuf[0];
	phyCard[1] = rBuf[1];
	phyCard[2] = rBuf[2];
	phyCard[3] = rBuf[3];

	gsmBytes2String(phyCard, oData, 4);	
		
	return 0;
}




//通过串口读光照读传感器（JXBS3001）的数据
int send_JXBS3001(int fd)
{
    unsigned char data[8]={0x01,0x03,0x00,0x00,0x00,0x09,0x85,0xcc};
    //printf("%s %04X",__func__,CheckCRCModBus(data,6));
    //printf("%s,%d\n",__func__,fd);
    return serial_send_data(fd,data,8);
}

int read_JXBS3001(int fd,unsigned char *data,int len)
{
    int ret=0,i,crclen,crcflag=0;
    unsigned int  lm=0;
    static unsigned long sendtick=0;
    unsigned char buf[32];
    unsigned short crc;
    unsigned char hcrc,lcrc;
    unsigned short humidity=0,tmp=0;//湿度，
    short temperature=0;//温度
       ret=serial_recv_data(fd,buf,sizeof(buf));

       if(ret>0){
           crclen=buf[2]+3;
           crc=CheckCRCModBus(buf,crclen);
           //printf("\nlen=%d,%04X,%02x,%02x\n",crclen,crc,buf[crclen],buf[crclen+1]);

           hcrc=(crc&0xff00)>>8;//高位
           lcrc=crc&0xff;       //低位
           if(lcrc==buf[crclen]&&hcrc==buf[crclen+1])
           {
               //获取湿度
               humidity=humidity|(buf[3]<<8);
               humidity=humidity|(buf[4]);

               //获取温度
                tmp=tmp|(buf[5]<<8);
                tmp=tmp|(buf[6]);
                //tmp=0xff9b;
                if(tmp&0x8000)//负数，为补码
                {

                    tmp=tmp-1;
                    tmp=~tmp;
                    temperature=tmp;
                    temperature=0-temperature;
                }else temperature=tmp;
                printf("wd =%04X,%d.%d\n",tmp,temperature/10,abs(temperature%10));

               //获取光照度
               lm=lm|(buf[17]<<24);
               lm=lm|(buf[18]<<16);
               lm=lm|(buf[19]<<8);
               lm=lm|buf[20];
               crcflag=1;
               sprintf(data,"Lumens=%d,temperature=%d.%d,humidity=%d.%d",lm,temperature/10,abs(temperature%10),humidity/10,abs(humidity%10));
         // printf("%s\n",data);
           }
       }
       if(GetTickCount()-sendtick>500){
           send_JXBS3001(fd);
           sendtick=GetTickCount();
       }
       if(crcflag>0)
           return TRUE;
       return FALSE;
}
