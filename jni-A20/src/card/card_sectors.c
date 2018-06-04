/**
 *
 * @chinese
 * @file   card_sectors.c
 * @author 刘训
 * @date   Thu Jul 14 16:46:20 2011
 *
 * @brief weds协议卡头操作模块，读写扇区操作
 * @endchinese
 *
 * @english
 * @file   card_sectors.c
 * @author Liu Xun
 * @date   Thu Jul 14 16:46:20 2011
 *
 * @brief reading card module through weds protol@n
 * supprot reading and writing sector operation
 * @endenglish
 *
 *
 *
 */

#include <termios.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include "card_sectors.h"
#include "stringchar.h"
#include "config.h"
#include "device_protocol.h"
#include "serial.h"
#include "debug.h"
#include "yijitong_interface.h"
#include "public.h"
#include  "readcard.h"

#include "../psam/errcode.h"
#include "../psam/psam.h"
#include "../general/inifile.h"
//#include "../comm/rt_comm/Public_.h"





_CARD_CONFIG card_config;

TmasterKey MasterKey;
TCatalog CCatalog;
TissuInfo IssuInfo;
TpubInfo pubInfo;
TpwdInfo pwdInfo;
_CP_INFO CP_INFO;

void init_card_cfg( );
int do_after_get_card(_IN_PARAM in);

static int checkCatalog( PTCatalog PCatalog );
static int readCatalogInfo( PTCatalog PCatalog, int keymode, unsigned char *key );
static int readIssuInfo( PTissuInfo PissuInfo, int keymode, unsigned char *key );
static int checkIssuInfo( PTissuInfo PissuInfo );
static int checkCard( unsigned int currDate );
static int calcKey( unsigned char in[8], unsigned char out[16] );
static int getCpuAuthData( unsigned char keyType, unsigned char *inData, unsigned char *outData );
static int checkPubInfo( PTpubInfo PpubInfo );
static int internalAuthentication();
static int ol_checkPubInfo( PTpubInfo PpubInfo );
static int new_readPub( int keymode, unsigned char *key );
static int new_writePub( );
static int writePwdInfo(PTpwdInfo PpwdInfo);
static int readPwdInfo(PTpwdInfo PpwdInfo, int keymode, unsigned char *key);
static int update_card( int flag );
static int ol_deal_card( );
static int upload_card_rec();
static int do_consume(_CARD_DATA *CARD_DATA);
static int read_card_rec(_IN_PARAM in, _OUT_PARAM *out);

static int uimHalt(int m_hcom);
static int uimSetMifareAppID( int m_hcom, char *data );
static int uimMessageSwitch( int m_hcom, int on_off );
static int uimSelectAccounter(int m_hcom);
int TFexternalAuthentication( unsigned char keyEdition, unsigned char keyIndex, unsigned char *data );

int readtype;
int m_hcom; //串口句柄
static int uimAuth( int sectorno, int keymode, unsigned char *key );
static int uimReadBlock( char blockno, unsigned char *data );
static int uimWriteBlock( char blockno, unsigned char *data );

static int yooAuth( int sectorno, int keymode, unsigned char *key );
static int yooReadBlock( char blockno, unsigned char *data );
static int yooWriteBlock( char blockno, unsigned char *data );
static int ol_readWallet( int *walletAmt, int keymode, unsigned char *key );
static int ol_readSumInfo(PTsumInfo PsumInfo, int keymode, unsigned char *key);
static int ol_readSubsidyInfo(PTsubsidyInfo PsubsidyInfo, int keymode, unsigned char *key);

static int ol_readBaseInfo(PTbaseInfo PbaseInfo, int keymode, unsigned char *key);
static int ol_readDetailByPoint(PTdetailInfo PdetailInfo, int point, int keymode, unsigned char *key);


// S70 共40个扇区 0~31(32个小扇区，每个扇区4块)，32~39(8个大扇区，每个扇区16块)
//逻辑扇区号转换为物理扇区号
char phySectorNo[72] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,  //0-15
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,  //16-31
        0x20, 0x20, 0x20, 0x20, 0x20,
        0x21, 0x21, 0x21, 0x21, 0x21,
        0x22, 0x22, 0x22, 0x22, 0x22,
        0x23, 0x23, 0x23, 0x23, 0x23,
        0x24, 0x24, 0x24, 0x24, 0x24,
        0x25, 0x25, 0x25, 0x25, 0x25,
        0x26, 0x26, 0x26, 0x26, 0x26,
        0x27, 0x27, 0x27, 0x27, 0x27   //64-71
};

//逻辑块号转换为物理块号
signed char phyBlockNo[72][4] = {
        { 0, 1, 2, 3 }, { 0, 1, 2, 3 },
        { 0, 1, 2, 3 }, { 0, 1, 2, 3 },
        { 0, 1, 2, 3 }, { 0, 1, 2, 3 },
        { 0, 1, 2, 3 }, { 0, 1, 2, 3 },
        { 0, 1, 2, 3 }, { 0, 1, 2, 3 },
        { 0, 1, 2, 3 }, { 0, 1, 2, 3 },
        { 0, 1, 2, 3 }, { 0, 1, 2, 3 },
        { 0, 1, 2, 3 }, { 0, 1, 2, 3 },
        { 0, 1, 2, 3 }, { 0, 1, 2, 3 },
        { 0, 1, 2, 3 }, { 0, 1, 2, 3 },
        { 0, 1, 2, 3 }, { 0, 1, 2, 3 },
        { 0, 1, 2, 3 }, { 0, 1, 2, 3 },
        { 0, 1, 2, 3 }, { 0, 1, 2, 3 },
        { 0, 1, 2, 3 }, { 0, 1, 2, 3 },
        { 0, 1, 2, 3 }, { 0, 1, 2, 3 },
        { 0, 1, 2, 3 }, { 0, 1, 2, 3 },

        { 0,  1,  2,  -1 },       //32
        { 3,  4,  5,  -1 },       //33
        { 6,  7,  8,  -1 },       //34
        { 9,  10, 11, -1 },       //35
        { 12, 13, 14, 15 },       //36

        { 0,	1,	2,	-1 },       //37
        { 3,	4,	5,	-1 },       //38
        { 6,	7,	8,	-1 },       //39
        { 9,	10, 11, -1 },       //40
        { 12, 13, 14, 15 },       //41

        { 0,	1,	2,	-1 },       //42
        { 3,	4,	5,	-1 },       //43
        { 6,	7,	8,	-1 },       //44
        { 9,	10, 11, -1 },       //45
        { 12, 13, 14, 15 },       //46

        { 0,	1,	2,	-1 },       //47
        { 3,	4,	5,	-1 },       //48
        { 6,	7,	8,	-1 },       //49
        { 9,	10, 11, -1 },       //50
        { 12, 13, 14, 15 },       //51

        { 0,	1,	2,	-1 },       //52
        { 3,	4,	5,	-1 },       //53
        { 6,	7,	8,	-1 },       //54
        { 9,	10, 11, -1 },       //55
        { 12, 13, 14, 15 },       //56

        { 0,	1,	2,	-1 },       //57
        { 3,	4,	5,	-1 },       //58
        { 6,	7,	8,	-1 },       //59
        { 9,	10, 11, -1 },       //60
        { 12, 13, 14, 15 },       //61

        { 0,	1,	2,	-1 },       //62
        { 3,	4,	5,	-1 },       //63
        { 6,	7,	8,	-1 },       //64
        { 9,	10, 11, -1 },       //65
        { 12, 13, 14, 15 },       //66

        { 0,	1,	2,	-1 },       //67
        { 3,	4,	5,	-1 },       //68
        { 6,	7,	8,	-1 },       //69
        { 9,	10, 11, -1 },       //70
        { 12, 13, 14, 15 }
};

//卡号转换成整型数据
int cardHexToUInt(int len,unsigned char *cardNo)
{
    statInfo.card1 = 0;
    statInfo.card2 = 0;
    statInfo.authSectorKeyType = -1;                                                        //20110711
    statInfo.authSectorNo	   = -1;

    if(len <0 || len >8)
        return -1;
    if(len  == 4)
    {
        statInfo.card1 = HexToUInt( cardNo , 3, 1 );
    }
    else if(len == 8)
    {
        statInfo.card1 = HexToUInt( cardNo , 3, 1 );
        statInfo.card2 = HexToUInt( cardNo + 3, 4, 1 );
    }
    return 1;
}

//设置卡头和读头类型
int setCardType(int cardtype,int readertype)
{
    statInfo.cardType = 0 ;
    statInfo.readerType = 0;
    if(cardtype<0 || readertype<0)
    {
        return -1;
    }
    statInfo.cardType = cardtype;
    statInfo.readerType = readertype;
    return 1;
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
int sendAndRecvCom( unsigned char *SendBuf, unsigned char *RecvBuf, int RetryCount, int timeout )
{
    if( -1 == m_hcom )
    {
        return ERR_INVALIDHANDLE;
    }

    int			bWriteStat;
    int			bReadStat;
    long			dwCount;

    unsigned char	* buf  = RecvBuf;
    unsigned char	bcc	   = 0;
    unsigned char	sum	   = 0;

    unsigned char	* addr;

    long			time1;
    int				len;
    int				i;
    switch( statInfo.readerType ) //卡头类型
    {
    case WEDS_READER:
    case WEDS_READER_CPU:
        if( SendBuf[0] != 0xaa )
        {
            return ERR_HEADINVALID;
        }

        for( i = 0; i < SendBuf[2] + 2; i++ ) //长度
        {
            bcc	  ^= SendBuf[i + 1];
            sum	  += SendBuf[i + 1];
        }

        SendBuf[SendBuf[2] + 3]	   = bcc;
        SendBuf[SendBuf[2] + 4]	   = sum;
        SendBuf[SendBuf[2] + 5]	   = 0xa5;

        for( i = 0; i < RetryCount + 1; i++ )
        {
            tcflush( m_hcom, TCIOFLUSH );   //清空缓冲区
            bWriteStat = WriteFile( m_hcom, SendBuf, SendBuf[2] + 6, &dwCount, NULL );

            if( !bWriteStat )
            {
                return ERR_COMERR;          //写串口失败
            }

            time1 = timeout + GetTickCount( );
            sched_yield( );                 //让出cpu,

            len	   = 1;
            addr   = buf;

            while( len )
            {
                bReadStat = ReadFile( m_hcom, addr, len, &dwCount, NULL );

                if( !bReadStat )
                {
                    return ERR_COMERR; //读串口失败
                }

                len	  -= dwCount;
                addr  += dwCount;

                if( len > 0 )
                {
                    if( GetTickCount( ) > time1 )
                    {
                        goto timeout1;      //超时
                    }else if( !dwCount )
                    {
                        sched_yield( );     //让出cpu,;
                    }
                }else if( 0xab != buf[0] )  //未读到帧头
                {
                    len	   = 1;
                    addr   = buf;
                }
            }

            len = 2;
            while( len )                //读设备地址和数据域长度
            {
                bReadStat = ReadFile( m_hcom, addr, len, &dwCount, NULL );

                if( !bReadStat )
                {
                    return ERR_COMERR;  //读串口失败
                }

                len	  -= dwCount;
                addr  += dwCount;

                if( len > 0 )
                {
                    if( GetTickCount( ) > time1 )
                    {
                        goto timeout1;  //超时
                    }else if( !dwCount )
                    {
                        sched_yield( ); //让出cpu,;
                    }
                }
            }

            len = buf[2] + 3;

            while( len )                //接收后续数据
            {
                bReadStat = ReadFile( m_hcom, addr, len, &dwCount, NULL );

                if( !bReadStat )
                {
                    return ERR_COMERR;  //读串口失败
                }

                len	  -= dwCount;
                addr  += dwCount;

                if( len > 0 )
                {
                    if( GetTickCount( ) > time1 )
                    {
                        goto timeout1;  //超时
                    }else if( !dwCount )
                    {
                        sched_yield( ); //让出cpu,;
                    }
                }
            }

            bcc	   = 0;
            sum	   = 0;

            for( i = 0; i < buf[2] + 2; i++ ) //长度
            {
                bcc	  ^= buf[i + 1];
                sum	  += buf[i + 1];
            }

            if( ( buf[buf[2] + 3] == bcc ) && ( buf[buf[2] + 4] == sum ) )
            {
                return buf[2] + 6;  //返回正确
            }
timeout1:
            ;
        }
        return ERR_OVERRETRY;       //超过重试次数
        break;
#if 0
    case SHNM100_READER:
    case MA102_READER:
        //0x80: //RFID-UIM卡，发往POS指令，无扩展长度，无校验
        //0x81: //RFID-UIM卡，发往POS指令，有扩展长度，无校验
        //0x82: //RFID-UIM卡，发往POS指令，无扩展长度，有校验
        //0x83: //RFID-UIM卡，发往POS指令，有扩展长度，有校验
        //0xA0: //RFID-UIM卡，发往UIM指令，无扩展长度，无校验
        //0xA1: //RFID-UIM卡，发往UIM指令，有扩展长度，无校验
        //0xA2: //RFID-UIM卡，发往UIM指令，无扩展长度，有校验
        //0xA3: //RFID-UIM卡，发往UIM指令，有扩展长度，有校验
        if( ( ( SendBuf[0] & 0xfc ) != 0x80 ) && ( ( SendBuf[0] & 0xfc ) != 0xa0 ) )
        {
            return ERR_HEADINVALID;
        }

        datalen = (int)( SendBuf[0] & 0x01 ) * 256 + SendBuf[1];
        if( datalen > 0x104 )
        {
            return ERR_OVERBUF;             //超过最大数据长度
        }

        if( SendBuf[0] & 0x02 )             //  有校验
        {
            for( i = 0; i < datalen; i++ )  //长度
            {
                bcc	  ^= SendBuf[i + 2];
                sum	  += SendBuf[i + 2];
            }

            SendBuf[datalen + 3]   = bcc;
            SendBuf[datalen + 2]   = sum;

            datalen += 2;                   //加上校验的长度
        }

        for( i = 0; i < RetryCount + 1; i++ )
        {
            tcflush( m_hcom, TCIOFLUSH );   //清空缓冲区
            bWriteStat = WriteFile( m_hcom, SendBuf, datalen + 2, &dwCount, NULL );

            if( !bWriteStat )
            {
                return ERR_COMERR;          //写串口失败
            }
            time1 = timeout + GetTickCount( );

            len	   = 1;
            addr   = buf;

            while( len )
            {
                bReadStat = ReadFile( m_hcom, addr, len, &dwCount, NULL );

                if( !bReadStat )
                {
                    return ERR_COMERR; //读串口失败
                }

                len	  -= dwCount;
                addr  += dwCount;

                if( len > 0 )
                {
                    if( GetTickCount( ) > time1 )
                    {
                        goto timeout2;                  //超时
                    }

                }else if( 0x90 != ( buf[0] & 0xFC ) )   //未读到帧头
                {
                    len	   = 1;
                    addr   = buf;
                }
            }
            len = 1;
            while( len )                                //读数据域长度字节
            {
                bReadStat = ReadFile( m_hcom, addr, len, &dwCount, NULL );

                if( !bReadStat )
                {
                    return ERR_COMERR;                  //读串口失败
                }

                len	  -= dwCount;
                addr  += dwCount;

                if( len > 0 )
                {
                    if( GetTickCount( ) > time1 )
                    {
                        goto timeout2;  //超时
                    }

                }
            }
            len = (int)( buf[0] & 0x01 ) * 256 + ( buf[0] & 0x02 ) + buf[1];
            if( len > 0x104 )
            {
                len = 0x104;
            }

            while( len )                //接收后续数据
            {
                bReadStat = ReadFile( m_hcom, addr, len, &dwCount, NULL );

                if( !bReadStat )
                {
                    return ERR_COMERR;  //读串口失败
                }

                len	  -= dwCount;
                addr  += dwCount;

                if( len > 0 )
                {
                    if( GetTickCount( ) > time1 )
                    {
                        goto timeout2;  //超时
                    }

                }
            }

            len = (int)( buf[0] & 0x01 ) * 256 + buf[1];
            if( len > 0x104 )
            {
                len = 0x104;
            }

            if( buf[0] & 0x02 )
            {
                bcc	   = 0;
                sum	   = 0;

                for( i = 0; i < len; i++ ) //长度
                {
                    bcc	  ^= buf[i + 2];
                    sum	  += buf[i + 2];
                }

                if( ( buf[len + 3] == bcc ) && ( buf[len + 2] == sum ) )
                {
                    return len + 4; //返回正确
                }
            }else
            {
                return len + 2;     //返回正确
            }
timeout2:
            ;
        }
        return ERR_OVERRETRY;       //超过重试次数

        break;

    case MINGWAH_READER:
        if( SendBuf[0] != 0xaa )
        {
            return ERR_HEADINVALID;
        }

        for( i = 0; i < SendBuf[3] + 4; i++ ) //长度
        {
            bcc ^= SendBuf[i];
        }

        SendBuf[SendBuf[3] + 4] = bcc;

        for( i = 0; i < RetryCount + 1; i++ )
        {
            tcflush( m_hcom, TCIOFLUSH );   //清空缓冲区
            //				bWriteStat = WriteFile(m_hcom, SendBuf, SendBuf[2] + 6, &dwCount, NULL);
            bWriteStat = WriteFile( m_hcom, SendBuf, SendBuf[3] + 5, &dwCount, NULL );

            if( !bWriteStat )
            {
                return ERR_COMERR;          //写串口失败
            }

            time1 = timeout + GetTickCount( );
            sched_yield( );                 //让出cpu,;

            len	   = 1;
            addr   = buf;

            while( len )
            {
                bReadStat = ReadFile( m_hcom, addr, len, &dwCount, NULL );

                if( !bReadStat )
                {
                    return ERR_COMERR; //读串口失败
                }

                len	  -= dwCount;
                addr  += dwCount;

                if( len > 0 )
                {
                    if( GetTickCount( ) > time1 )
                    {
                        goto timeout3;      //超时
                    }else if( !dwCount )
                    {
                        sched_yield( );     //让出cpu,;
                    }
                }else if( 0x55 != buf[0] )  //未读到帧头
                {
                    len	   = 1;
                    addr   = buf;
                }
            }

            len = 3;
            while( len )                //读返回码+长度,只关心长度
            {
                bReadStat = ReadFile( m_hcom, addr, len, &dwCount, NULL );

                if( !bReadStat )
                {
                    return ERR_COMERR;  //读串口失败
                }

                len	  -= dwCount;
                addr  += dwCount;

                if( len > 0 )
                {
                    if( GetTickCount( ) > time1 )
                    {
                        goto timeout3;  //超时
                    }else if( !dwCount )
                    {
                        sched_yield( ); //让出cpu,;
                    }
                }
            }

            len = buf[3] + 1;

            while( len )                //接收后续数据
            {
                bReadStat = ReadFile( m_hcom, addr, len, &dwCount, NULL );

                if( !bReadStat )
                {
                    return ERR_COMERR;  //读串口失败
                }

                len	  -= dwCount;
                addr  += dwCount;

                if( len > 0 )
                {
                    if( GetTickCount( ) > time1 )
                    {
                        goto timeout3;          //超时
                    }else if( !dwCount )
                    {
                        sched_yield( );         //让出cpu,;
                    }
                }
            }

            bcc = 0;

            for( i = 0; i < buf[3] + 5; i++ )   //长度
            {
                bcc ^= buf[i];
            }

            if( !bcc )
            {
                return buf[3] + 5;              //返回正确
            }
timeout3:
            ;
        }

        return ERR_OVERRETRY;                   //超过重试次数
        break;

    case YOOCAN_READER:
        if( SendBuf[0] != 0x5a )
        {
            return ERR_HEADINVALID;
        }

        if( ( SendBuf[1] != 0x10 ) && ( SendBuf[1] != 0x20 ) && ( SendBuf[1] != 0x22 ) )
        {
            return ERR_FALSE;
        }

        datalen				   = SendBuf[2] * 256 + SendBuf[3];
        SendBuf[datalen + 4]   = 0;             // 校验字节，如果填充出错，可能造成灾难后果
        for( i = 1; i < ( datalen + 4 ); i++ )  //填充校验字节
        {
            SendBuf[datalen + 4] ^= SendBuf[i];
        }

        SendBuf[datalen + 5] = 0xca;            //帧尾

        for( i = 0; i < RetryCount + 1; i++ )
        {
            tcflush( m_hcom, TCIOFLUSH );       //清空缓冲区
            bWriteStat = WriteFile( m_hcom, SendBuf, datalen + 6, &dwCount, NULL );

            if( !bWriteStat )
            {
                return ERR_COMERR;              //写串口失败
            }

            time1 = timeout + GetTickCount( );
            sched_yield( );

            len	   = 1;                         //帧头字节数
            addr   = buf;

            while( len )
            {
                bReadStat = ReadFile( m_hcom, addr, len, &dwCount, NULL );

                if( !bReadStat )
                {
                    return ERR_COMERR; //读串口失败
                }

                len	  -= dwCount;
                addr  += dwCount;

                if( len > 0 )
                {
                    if( GetTickCount( ) > time1 )
                    {
                        goto timeout5;      //超时
                    }else if( !dwCount )
                    {
                        //Sleep(10);
                        sched_yield( );
                    }
                }else if( 0x5a != buf[0] )  //未读到帧头
                {
                    len	   = 1;
                    addr   = buf;
                }
            }

            //
            len = 3;
            while( len )                //读命令类型和数据域长度
            {
                bReadStat = ReadFile( m_hcom, addr, len, &dwCount, NULL );

                if( !bReadStat )
                {
                    return ERR_COMERR;  //读串口失败
                }

                len	  -= dwCount;
                addr  += dwCount;

                if( len > 0 )
                {
                    if( GetTickCount( ) > time1 )
                    {
                        goto timeout5;  //超时
                    }else if( !dwCount )
                    {
                        //Sleep(10);
                        sched_yield( );
                    }
                }
            }

            if( buf[1] != SendBuf[1] )
            {
                return ERR_INVALID;     //无效的返回
            }

            len = buf[2] * 256 + buf[3] + 2;

            while( len )                //接收后续数据
            {
                bReadStat = ReadFile( m_hcom, addr, len, &dwCount, NULL );

                if( !bReadStat )
                {
                    return ERR_COMERR;  //读串口失败
                }

                len	  -= dwCount;
                addr  += dwCount;

                if( len > 0 )
                {
                    if( GetTickCount( ) > time1 )
                    {
                        goto timeout5;  //超时
                    }else if( !dwCount )
                    {
                        sched_yield( );
                    }
                }
            }

            len	   = buf[2] * 256 + buf[3];
            bcc	   = 0;                 //异或校验
            for( i = 0; i < len + 3; i++ )
            {
                bcc ^= buf[i + 1];
            }

            if( buf[len + 1 + 3] == bcc )
            {
                return len + 6;     //返回正确
            }
timeout5:
            ;
        }
        return ERR_OVERRETRY;       //超过重试次数
        break;
#endif
    default:
        return ERR_INVALIDREADER;   //不支持的读卡器
        break;
    }
}
int ReadFile( int fd, unsigned char * addr, int len, long *dwCount, char *s )
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
//dbgShowHexData(addr, count, 1, '<', NULL);
    *dwCount = count;
    return TRUE;
}



int WriteFile( int fd, unsigned char *addr, int len, long *dwCount, char *s )
{
    int count;
//dbgShowHexData(addr, len, 1, '>', NULL);
    count = write( fd, addr, len );
    if( count < 0 )
    {
        return FALSE;
    }
    *dwCount = count;
    return TRUE;
}


//int block_write( unsigned int cardNo, int sectorno, signed char blockno, unsigned char *data, int keymode, unsigned char *key, int checkflag )
//{
//    char cardno[4];


//    unsigned char	SendBuf[37];
//    unsigned char	RecvBuf[265];
//    unsigned char	tmpbuf[16];
//    unsigned char       tmpBuf[256];

//    int	 len, ret;

//    if( statInfo.readerType == WEDS_READER )    //早期读卡器/卡头存在BUG 20120621
//    {
//        statInfo.authSectorNo = -1;       //test 20110726
//    }

//    if( ( NULL == data ) || ( sectorno < 0 ) || ( blockno < 0 ) || ( sectorno > ( 15 + 16 + 8 ) ) )
//    {
//        return ERR_PARAM;
//    }

//    if( sectorno < 32 ) //前2KB
//    {
//        if( blockno > 3 )
//        {
//            return ERR_PARAM;
//        }
//    }else
//    {
//        if( blockno > 15 )
//        {
//            return ERR_PARAM;
//        }
//    }

//    if( statInfo.accessMsec + statInfo.authDelayMsec < GetTickCount( ) )
//    {
//        statInfo.authSectorNo = -1; //认证扇区超时

//        statInfo.currDF	   = -1;    //20121015 cpu
//        statInfo.currBF	   = -1;
//        statInfo.antiStat  = 0;
//    }


//UIntToHex( cardNo, cardno, 4, 1 );
//    switch( statInfo.cardType )
//    {
//    case CARD_IC:
//    case CARD_S70:
//        if( statInfo.card1 == 0 )     //4字节卡号 20120920
//        {
//            SendBuf[0] = 0xaa;     //帧头
//            SendBuf[1] = 0x01;     //设备地址

//            SendBuf[2] = 0x18;     //数据长度
//            SendBuf[3] = 0x83;     //指令字
//            SendBuf[4] = 0x01;     //数据包数量

//            UIntToHex( statInfo.card2, SendBuf + 5, 4, 1 );//20120920 不再支持参数传入的卡号

//            SendBuf[9]	   = sectorno;     //扇区号
//            SendBuf[10]	   = blockno;      //块号
//            if( key && ( ( statInfo.authSectorNo != sectorno ) || ( statInfo.authSectorKeyType != keymode ) ) ) //调试20110719
//            {
//                SendBuf[2]	   = 0x1f;     //数据长度
//                SendBuf[3]	   = 0x81;     //指令字
//                SendBuf[11]	   = keymode;     //密钥类型
//                memcpy( SendBuf + 12, key, 6 );     //填充密钥
//                memcpy( SendBuf + 18, data, 16 );      //填充块数据
//            }else
//            {
//                memcpy( SendBuf + 11, data, 16 );      //填充块数据
//            }
//        }
//        else //8字节卡号
//        {
//            SendBuf[0] = 0xaa;     //帧头
//            SendBuf[1] = 0x01;     //设备地址

//            SendBuf[2] = 0x18 + 3;      //数据长度
//            SendBuf[3] = 0x83;     //指令字

//            SendBuf[4] = 0x01;     //数据包数量

//            UIntToHex( statInfo.card1, SendBuf + 5, 3, 1 );     //20120719 不再支持参数传入的卡号
//            UIntToHex( statInfo.card2, SendBuf + 5 + 3, 4, 1 );       //20120719 不再支持参数传入的卡号

//            SendBuf[9 + 3]	   = sectorno;       //扇区号
//            SendBuf[10 + 3]	   = blockno;     //块号
//            if( key && ( ( statInfo.authSectorNo != sectorno ) || ( statInfo.authSectorKeyType != keymode ) ) ) //调试20110719
//            {
//                SendBuf[2]		   = 0x1f + 3;      //数据长度
//                SendBuf[3]		   = 0x81;       //指令字
//                SendBuf[11 + 3]	   = keymode;       //密钥类型
//                memcpy( SendBuf + 12 + 3, key, 6 );       //填充密钥
//                memcpy( SendBuf + 18 + 3, data, 16 );     //填充块数据
//            }else
//            {
//                memcpy( SendBuf + 11 + 3, data, 16 );     //填充块数据
//            }
//        }

//        len = sendAndRecvCom( SendBuf, RecvBuf, 0, 200 );

//        if( ( len == 9 ) && ( RecvBuf[3] == 0x00 ) && ( RecvBuf[4] == 0x01 ) )
//        {
//            switch( RecvBuf[5] )
//            {
//            case RET_SUCCESS:     //错误码：成功

//                statInfo.authSectorKeyType = keymode;     //20110711
//                statInfo.authSectorNo	   = sectorno;
//                statInfo.accessMsec		   = GetTickCount( );   //更新访问时间

//                ret = ERR_OK;
//                if( checkflag )
//                {
//                    if( cardNo )
//                    {
//                        ret = block_read( cardNo, sectorno, blockno, tmpbuf, keymode, NULL );
//                    }else
//                    {
//                        ret = block_read( statInfo.card2, sectorno, blockno, tmpbuf, keymode, NULL );
//                    }
//                    if( ERR_OK == ret )
//                    {
//                        if( !memcmp( data, tmpbuf, 16 ) )
//                        {
//                            return ERR_OK;
//                        }else
//                        {
//                            return ERR_FALSE;
//                        }
//                    }
//                }
//                return ret;

//                break;
//            case RET_OPERR:      //指令操作失败
//                return ERR_OPERR;
//                break;
//            case RET_CHECKERR:      //校验错误
//                return ERR_OPERR;
//                break;
//            case RET_INVALID:       //不能识别的指令
//                return ERR_OPERR;
//                break;
//            case RET_NOTCONDITION:  //指令无法执行
//                return ERR_OPERR;
//                break;
//            default:
//                return ERR_INVALID; //无效的返回
//            }
//        }else if( len < 0 )
//        {
//            return len;
//        }else
//        {
//            return ERR_INVALID;
//        }

//        break;
//#if 1
//    case CARD_UIM:
//        if( ( statInfo.authSectorNo != sectorno ) || ( statInfo.authSectorKeyType != keymode ) || ( statInfo.accessMsec + statInfo.authDelayMsec < GetTickCount( ) ) )
//        {
//            ret = uimAuth( sectorno, keymode, key ); //认证
//            if( ret != ERR_OK )
//            {
//                return ret;
//            }
//            statInfo.authSectorKeyType = keymode;
//            statInfo.authSectorNo	   = sectorno;
//        }

//        ret = uimWriteBlock( blockno, data );
//        if( ( ret == ERR_OK ) && ( checkflag ) )
//        {
//            ret = uimReadBlock( blockno, RecvBuf );
//            if( ret == ERR_OK )
//            {
//                ret = memcmp( data, RecvBuf, 16 );
//                if( ret )
//                {
//                    return ERR_FALSE;
//                }
//            }
//        }
//        return ret;
//        break;

//    case CARD_USIM:       //20120704    注: 可能需要调整
//        if( ( statInfo.authSectorNo != sectorno ) || ( statInfo.authSectorKeyType != keymode ) || ( statInfo.accessMsec + statInfo.authDelayMsec < GetTickCount( ) ) )
//        {
//            ret = yooAuth( sectorno, keymode, key );    //认证
//            if( ret != ERR_OK )
//            {
//                return ret;
//            }
//            statInfo.authSectorKeyType = keymode;
//            statInfo.authSectorNo	   = sectorno;
//        }

//        ret = yooWriteBlock( blockno, data );
//        if( ( ret == ERR_OK ) && ( checkflag ) )
//        {
//            ret = yooReadBlock( blockno, RecvBuf );
//            if( ret == ERR_OK )
//            {
//                ret = memcmp( data, RecvBuf, 16 );
//                if( ret )
//                {
//                    return ERR_FALSE;
//                }
//            }
//        }

//        return ret;
//        break;

//    case CARD_CPU:     //20121015
//        if( statInfo.currDF != 0xaf01 )     //非WEDS应用目录
//        {
//            statInfo.currDF	   = -1;
//            statInfo.currBF	   = -1;
//            statInfo.antiStat  = 0;

//            if( statInfo.currDF != 0x3F00 ) //非卡片根目录
//            {
//                ret = selectFile( (unsigned short)( 0x3f00 ), tmpBuf, 128 );
//                if( ret < ERR_OK )
//                {
//                    return ret;
//                }
//            }

//            ret = selectFile( (unsigned short)0xaf01, tmpBuf, 128 ); //选择WEDS1App应用
//            if( ret < ERR_OK )
//            {
//                return ret;
//            }

//            statInfo.currDF = 0xaf01;
//        }

//        if( sectorno < 2 )
//        {
//            if( statInfo.antiStat < 8 ) //安全状态不满足写此文件
//            {
//                ret = getChallenge( &random );
//                if( ret != ERR_OK )
//                {
//                    return ret;
//                }

//                memset( desData, 0, 8 );
//                UIntToHex( random, desData, 4, 1 );

//                ret = DesBase.RunDes( 0, (char *)desData, (char *)desData, 8, (char *)statInfo.writeManageSectorKey, 16 );
//                if( ret == 0 )
//                {
//                    ret = ERR_FALSE;
//                    return ret;
//                }

//                ret = TFexternalAuthentication( 0, 0xF3, desData );

//                if( ret != ERR_OK )
//                {
//                    return ret;
//                }

//                statInfo.antiStat |= 0x09;
//            }

//            ret = updateBinary( 0x11, sectorno * 48 + 16 * blockno, data, 16 );
//            if( ( ret == ERR_OK ) && ( checkflag ) )
//            {
//                ret = readBinary( 0x11, sectorno * 48 + 16 * blockno, 16, RecvBuf );
//                if( ret == ERR_OK )
//                {
//                    ret = memcmp( data, RecvBuf, 16 );
//                    if( ret )
//                    {
//                        return ERR_FALSE;
//                    }
//                }
//            }
//        }else
//        {
//            if( ( statInfo.antiStat & 0x04 ) == 0 ) //安全状态不满足写此文件 20121015
//            {
//                ret = getChallenge( &random );
//                if( ret != ERR_OK )
//                {
//                    return ret;
//                }

//                memset( desData, 0, 8 );
//                UIntToHex( random, desData, 4, 1 );

//                if (0 == deviceinfo.psamenable)
//                {
//                    ret = DesBase.RunDes(0, (char *)desData, (char *)desData, 8, (char *)statInfo.dealKey, 16);
//                    if (ret != TRUE)
//                    {
//                        ret = ERR_FALSE;
//                        return ret;
//                    }
//                }
//                else
//                {
//                    ret = psam_initfordescrypt(0x27, 0x11, 0x08, statInfo.InData);	// 生成临时密钥
//                    if (ret != TRUE)
//                    {
//                        ret = ERR_FALSE;
//                        return ret;
//                    }

//                    ret = psam_descrypt(0x00, 0x08, desData);
//                    if (ret != TRUE)
//                    {
//                        ret = ERR_FALSE;
//                        return ret;
//                    }
//                }
//                ret = TFexternalAuthentication( 0, 0xF2, desData ); //此参数歪打正着，同样适用于FMCOS2.0 20120801

//                if( ret != ERR_OK )
//                {
//                    return ret;
//                }

//                statInfo.antiStat |= 0x07;
//            }

//            if( statInfo.currBF != ( 0x11 + 1 ) )     //非当前文件
//            {
//                if( ( ( sectorno - 2 ) * 48 + 16 * blockno ) > 255 )    //地址偏移量超过一个字节
//                {
//                    ret = selectFile( 0x12, tmpBuf, 128 );     //选择该文件
//                    if( ret != ERR_OK )
//                    {
//                        return ret;
//                    }
//                }
//            }

//            ret = updateBinary( 0x11 + 1, ( sectorno - 2 ) * 48 + 16 * blockno, data, 16 );
//            if( ( ret == ERR_OK ) && ( checkflag ) )
//            {
//                ret = readBinary( 0x11 + 1, ( sectorno - 2 ) * 48 + 16 * blockno, 16, RecvBuf );
//                if( ret == ERR_OK )
//                {
//                    ret = memcmp( data, RecvBuf, 16 );
//                    if( ret )
//                    {
//                        return ERR_FALSE;
//                    }
//                }
//            }
//        }

//        if( ret == ERR_OK )
//        {
//            statInfo.accessMsec = GetTickCount( ); //更新访问时间
//        }

//        return ret;
//        break;
//#endif
//    default:
//        return ERR_INVALIDCARDTYPE;
//        break;
//    }
//}

/////*
////   //考虑怎样支持s70? 扇区结构采用逻辑扇区结构（每扇区3个数据块），调用block_read()、block_write()时转换为实际扇区结构
////   //是否需要带密钥认证自动进行处理，加快访问速度
//// */
//int block_read( unsigned int cardNo, int sectorno, signed char blockno, unsigned char *data, int keymode, unsigned char *key )
//{

//    if( ( NULL == data ) || ( sectorno < 0 ) || ( blockno < 0 ) || ( sectorno > ( 15 + 16 + 8 ) ) )
//    {
//        return ERR_PARAM;
//    }

//    if( sectorno < 32 ) //前2KB
//    {
//        if( blockno > 3 )
//        {
//            return ERR_PARAM;
//        }
//    }else
//    {
//        if( blockno > 15 )
//        {
//            return ERR_PARAM;
//        }
//    }

//    if( statInfo.accessMsec + statInfo.authDelayMsec < GetTickCount( ) )
//    {
//        statInfo.authSectorNo = -1; //认证扇区超时

//        statInfo.currDF	   = -1;    //20121015 cpu
//        statInfo.currBF	   = -1;
//        statInfo.antiStat  = 0;
//    }

//    char cardno[4];
//    UIntToHex( cardNo, cardno, 4, 1 );

//    unsigned char	SendBuf[21];
//    unsigned char	RecvBuf[265];

//    int				len;

//    switch( statInfo.cardType )
//    {
//    case CARD_IC:
//    case CARD_S70:

//        if( statInfo.card1 == 0 )                                                                           //4字节卡号 20120920
//        {
//            SendBuf[0] = 0xaa;                                                                              //帧头
//            SendBuf[1] = 0x01;                                                                              //设备地址

//            SendBuf[2] = 0x08;                                                                              //数据长度
//            SendBuf[3] = 0x82;                                                                              //指令字

//            SendBuf[4] = 0x01;                                                                              //数据包数量
//            UIntToHex( statInfo.card2, SendBuf + 5, 4, 1 );                                        //不再支持传入卡号参数 20120920 支持7字节卡号

//            SendBuf[9]	   = sectorno;                                                                      //扇区号
//            SendBuf[10]	   = blockno;                                                                       //块号
//            if( key )
//            {
//                if( ( statInfo.authSectorNo != sectorno ) || ( statInfo.authSectorKeyType != keymode ) )    //调试20110719
//                {
//                    SendBuf[2]	   = 0x0f;                                                                  //数据长度
//                    SendBuf[3]	   = 0x80;                                                                  //指令字
//                    SendBuf[11]	   = keymode;                                                               //密钥类型
//                    memcpy( SendBuf + 12, key, 6 );                                                         //填充密钥
//                }
//            }
//        }else //7字节卡号 20120920
//        {
//            SendBuf[0] = 0xaa;                                                                              //帧头
//            SendBuf[1] = 0x01;                                                                              //设备地址

//            SendBuf[2] = 0x08 + 3;                                                                          //数据长度
//            SendBuf[3] = 0x82;                                                                              //指令字

//            SendBuf[4] = 0x01;                                                                              //数据包数量

//            UIntToHex( statInfo.card1, SendBuf + 5, 3, 1 );                                        //不再支持传入卡号参数 20120719 支持7字节卡号
//            UIntToHex( statInfo.card2, SendBuf + 5 + 3, 4, 1 );                                    //不再支持传入卡号参数 20120719 支持7字节卡号

//            SendBuf[9 + 3]	   = sectorno;                                                                  //扇区号
//            SendBuf[10 + 3]	   = blockno;                                                                   //块号
//            if( key )
//            {
//                if( ( statInfo.authSectorNo != sectorno ) || ( statInfo.authSectorKeyType != keymode ) )    //调试20110719
//                {
//                    SendBuf[2]		   = 0x0f + 3;                                                          //数据长度
//                    SendBuf[3]		   = 0x80;                                                              //指令字
//                    SendBuf[11 + 3]	   = keymode;                                                           //密钥类型
//                    memcpy( SendBuf + 12 + 3, key, 6 );                                                     //填充密钥
//                }
//            }
//        }

//        len = sendAndRecvCom( SendBuf, RecvBuf, 0, 200 );

//        if( ( len == 24 ) && ( RecvBuf[3] == 0x80 ) && ( RecvBuf[4] == 0x01 ) )
//        {
//            memcpy( data, RecvBuf + 5, 16 );

//            statInfo.authSectorKeyType = keymode;           //20110711
//            statInfo.authSectorNo	   = sectorno;
//            statInfo.accessMsec		   = GetTickCount( );   //更新访问时间

//            return ERR_OK;
//        }else if( ( len == 9 ) && ( RecvBuf[3] == 0x00 ) && ( RecvBuf[4] == 0x01 ) )
//        {
//            switch( RecvBuf[5] )
//            {
//            case RET_SUCCESS:                           //错误码：成功
//                return ERR_INVALID;                     //无效的返回
//                break;
//            case RET_OPERR:                             //指令操作失败
//                return ERR_OPERR;
//                break;
//            case RET_CHECKERR:                          //校验错误
//                return ERR_OPERR;
//                break;
//            case RET_INVALID:                           //不能识别的指令
//                return ERR_OPERR;
//                break;
//            case RET_NOTCONDITION:                      //指令无法执行
//                return ERR_OPERR;
//                break;
//            default:
//                return ERR_INVALID;                     //无效的返回
//            }
//        }else if( len < 0 )
//        {
//            return len;
//        }else
//        {
//            return ERR_INVALID;
//        }

//        break;
//    default:
//        break;
//    }
//    return ERR_INVALIDCARDTYPE;
//}

#if 0
//函数功能:  连续写数据块
//参数:      keymode: 密钥类型(A密钥或B密钥),
//           key: 密钥
//           firstsector: 起始扇区
//           firstblock:  起始扇区的开始块号
//           len:  需要写的数据长度
//           data: 数据缓冲区地址
//返回:      大于0为写到的数据长度，以字节计，其它：按错误码解释
int writeDataBlock( int keymode, unsigned char *key, int firstSector, int firstBlock, int len, unsigned char *data ,int uart_port)
{
    m_hcom = serial_fd[uart_port];
    if( m_hcom <= 0 )
    {
        return ERR_INVALIDHANDLE;
    }
    if( ( firstSector > 71 ) || ( firstBlock > 2 ) )
    {
        return ERR_PARAM;
    }

    if( ( firstBlock + len / 16 + ( len % 16 ) ? 1 : 0 ) > ( 72 - firstSector ) * 3 )   //超过S70卡容量
    {
        return ERR_PARAM;
    }

    int				offset = 0;
    unsigned char	buf[16];
    unsigned char	* addrkey = key;
    int				ret;
    int				checkFlag = 1;
    if( statInfo.cardType == CARD_UIM )
    {
        checkFlag = 0;
    }

    while( len )
    {
        memset( buf, 0, 16 );

        if( len > 15 )
        {
            memcpy( buf, data + offset, 16 );
        }else
        {
            memcpy( buf, data + offset, len );
        }

        ret = block_write( statInfo.card2, phySectorNo[firstSector], phyBlockNo[firstSector][firstBlock], buf, keymode, addrkey, checkFlag );

        if( ERR_OK == ret )
        {
            if( len > 15 )
            {
                offset	  += 16;
                len		  -= 16;
            }else
            {
                offset	  += len;
                len		   = 0;
            }
        }else
        {
            return ret;
        }

        firstBlock++;
        firstBlock %= 3;
        if( firstBlock == 0 )
        {
            firstSector++;
        }
    }
    return ERR_OK; //20110712
}
#endif

#if 0
//函数功能:  连续读数据块
//参数:      keyMode: 密钥类型(A密钥或B密钥),
//           key: 密钥
//           firstsector: 起始扇区
//           firstblock:  起始扇区的开始块号
//           len:  需要读的数据长度
//           data: 输出数据缓冲区地址
//返回:      大于0为读到的数据长度，以字节计，其它：按错误码解释
int readDataBlock( int keyMode, unsigned char *key, int firstSector, int firstBlock, int len, unsigned char *data, int uart_port)
{
    m_hcom = serial_fd[uart_port];
    if( m_hcom <= 0 )
    {
        return ERR_INVALIDHANDLE;
    }

    if( ( firstSector > 71 ) || ( firstBlock > 2 ) )
    {
        return ERR_PARAM;
    }

    if( ( firstBlock + len / 16 + ( len % 16 ) ? 1 : 0 ) > ( 72 - firstSector ) * 3 ) //超过S70卡容量
    {
        return ERR_PARAM;
    }

    int				offset = 0;
    unsigned char	buf[16];
    unsigned char	* addrkey = key;
    int				ret;

    while( len )
    {
        ret = block_read( statInfo.card2, phySectorNo[firstSector], phyBlockNo[firstSector][firstBlock], buf, keyMode, addrkey );

        if( ERR_OK == ret )
        {
            if( len > 15 )
            {
                memcpy( data + offset, buf, 16 );
                offset	  += 16;
                len		  -= 16;
            }else
            {
                memcpy( data + offset, buf, len );
                offset	  += len;
                len		   = 0;
            }
        }else
        {
            return ret;
        }

        firstBlock++;
        firstBlock %= 3;
        if( firstBlock == 0 )
        {
            firstSector++;
        }
    }
    return ERR_OK; //20110712
}

#endif

int send_recv_data(int uart_port, unsigned char *SendBuf, unsigned char RecvBuf[])
{
    unsigned char *rec_buf = NULL;
    unsigned char	bcc	   = 0;
    unsigned char	sum	   = 0;
    int retval = 0, i = 0;
    int overtime=0;
    struct timeval tv1,tv2;

    rec_buf = (unsigned char *)RecvBuf;

    for( i = 0; i < SendBuf[2] + 2; i++ ) //长度
    {
        bcc	  ^= SendBuf[i + 1];
        sum	  += SendBuf[i + 1];
    }

    SendBuf[SendBuf[2] + 3]	   = bcc;
    SendBuf[SendBuf[2] + 4]	   = sum;
    SendBuf[SendBuf[2] + 5]	   = 0xa5;



    retval = serial_send_data(uart_port, (char *)SendBuf, SendBuf[2] + 6);
    if(retval == ERROR || retval!= SendBuf[2] + 6)
    {
    	printf("return1 :%d\n", retval);
        return ERROR;
    }

    gettimeofday(&tv1,NULL);
    gettimeofday(&tv2,NULL);

    overtime=tv1.tv_sec *1000000+tv1.tv_usec;
    while(1)
    {
        if(abs( (tv2.tv_sec *1000000 + tv2.tv_usec) - overtime) > 300000)
        {
            return ERROR;
        }
        gettimeofday(&tv2,NULL);
        retval = serial_recv_onebyte(uart_port, (char *)rec_buf);
        if (retval == ERROR )
        {
            return ERROR;
        }
        if(*rec_buf != 0xab)// 包头
        {
            continue;
        }
        else
        {
            break;
        }
    }
    rec_buf++;
    retval = serial_recv_data(uart_port,rec_buf,2);
    if (retval != 2)
    {
        return ERROR;
    }
    rec_buf += 2;
    retval = serial_recv_data(uart_port,rec_buf,RecvBuf[2]+3);
    if (retval != (RecvBuf[2]+3))
    {
        return ERROR;
    }
    rec_buf += RecvBuf[2]+3;
    bcc = 0;
    sum = 0;

    for( i = 0; i < RecvBuf[2] + 2; i++ ) //长度
    {
        bcc	  ^= RecvBuf[i + 1];
        sum	  += RecvBuf[i + 1];
    }

//    printf("====recbuf:");
//    for(i=0;i<(RecvBuf[2] + 6);i++)
//        if((i+1)%21==0)printf("\n");
//    else
//        printf("%02x ",RecvBuf[i]);
//    printf("\n");
// printf("%s bbbbb\n",__func__);

    if( ( RecvBuf[RecvBuf[2] + 3] != bcc ) || ( RecvBuf[RecvBuf[2] + 4] != sum ) )
    {
        printf(" bcc/sum error\n");
        return ERROR;
    }
    return RecvBuf[2] + 6;  //返回接收数据长度;
}
int query_cpu_card( int uart_port, unsigned char *buf )
{
    int retval = 0;
    unsigned char RecvBuf[256];
    //AA 01 03 62 01 95 F4 FC A5
    unsigned char SendBuf[] = { 0xaa, 0x01, 0x03, 0x62, 0x01, 0x95, 0x00, 0x00, 0x00 };
    memset(RecvBuf, 0, sizeof(RecvBuf));
    retval = send_recv_data(uart_port,SendBuf,RecvBuf);
    //AB 01 17 62 01 04 B0 F4 35 EC 0F 0D 78 80 80 02 00 73 C8 40 00 00 90 00 00 00 FF E5 A5
    //AB 01 03 00 01 01 02 06 A5 寻卡failed
    //AB 01 06 95 01 B0 F4 35 EC 0E 62 A5
    if(retval < 0 || RecvBuf[3] != 0x62)
    {
        return ERROR;
    }
    memcpy( buf, RecvBuf + 6, 4 );
    return TRUE;

}

int wedsApduChannel( unsigned char *buf, unsigned char buflen, unsigned char *answerBuf, int uart_port )
{
    int c_len=0;
    unsigned char	Fhead[] = { 0xAA, 0x01, 0x00, 0x31, 0x01 };             // 帧头+设备地址+数据长度+指令字+数据包数量
    unsigned char	SendBuf[1024];                                           //255+5(0X104)
    unsigned char	Recv_Buf[1024];
    int len = 0;
    int i=0;
    memset( Recv_Buf, 0, sizeof(Recv_Buf) );
    memset( SendBuf, 0, sizeof(SendBuf) );
    memcpy( SendBuf, Fhead, 5 );
    SendBuf[2] = buflen + 2;                                                //数据长度
    memcpy( (unsigned char *)SendBuf + 5, buf, buflen );

    len = send_recv_data(uart_port,SendBuf,Recv_Buf);
    if(len < 0)
    {
        return ERROR;
    }
    if(Recv_Buf[2] >= 0X3E)
    {
        c_len = 1;//   20150923   原来为c_len=2
    }
    else
    {
        c_len = 1;
    }

//    for(i=0;i<len;i++)
//    {
//        if((i+1)%30==0)printf("%02X \n",Recv_Buf[i]);
//        else printf("%02X ",Recv_Buf[i]);
//    }
//   printf("\n");

    if( ( Recv_Buf[3] == 0x31 ) && ( Recv_Buf[4] == 0x01 ) &&
            Recv_Buf[Recv_Buf[2]+c_len] == 0x90 ) //最少数据为2字节返回码sw1 sw2
    {
        memcpy( answerBuf, Recv_Buf + 5, Recv_Buf[2] - 2 );

        return Recv_Buf[2] - 2;                                              //返回apdu数据长度
    }
    else
    {
        return ERROR;
    }
}

int select_app(char *appID, int uart_port)
{
    unsigned char head[5] = {0x00,0xA4,0x04,0x00,0x10};
    unsigned char apdu[128];
    unsigned char answerBuf[256];
    unsigned char *addr = apdu;
    int retval = 0;
    memset( apdu, 0, 128 );
    memset( answerBuf, 0, sizeof(answerBuf) );
    memcpy(apdu, head,5);
    addr = apdu + 5;
    memcpy(addr,appID,16);
    //AA 01 17 31 01 00 A4 04 00 10 F0 00 00 00 00 01 90 88 8F 00 00 4D 00 00 00 81 3C 68 A5
    retval = wedsApduChannel( apdu, (unsigned char)21, answerBuf, uart_port );
    //AB 01 1B 31 01 6F 15 84 10 F0 00 00 00 00 01 90 88 8F 00 00 4D 00 00 00 03 A5 01 42 90 00 9A C6 A5
    if(retval < 0)
    {
        //printf("6666666666\n");
        return ERROR;
    }
    return TRUE;
}

int select_public_app(char *appID,int mode,int uart_port)
{
    unsigned char head[5] = {0x00,0xA4,0x04,0x00};
    unsigned char apdu[128];
    unsigned char answerBuf[256];
    unsigned char *addr = apdu;
    int retval = 0;
    int num=0;
    memset( apdu, 0, 128 );
    memset( answerBuf, 0, sizeof(answerBuf) );

    head[2]=mode;
    head[4]= string2hex(appID,apdu+5, strlen(appID));;
    memcpy(apdu, head,5);
   // memcpy(addr,appID,num);
    num=head[4]+5;
    //printf("%s %s\n",__func__,apdu+5);
    //AA 01 17 31 01 00 A4 04 00 10 F0 00 00 00 00 01 90 88 8F 00 00 4D 00 00 00 81 3C 68 A5
    retval = wedsApduChannel( apdu, (unsigned char)num, answerBuf, uart_port );
    //AB 01 1B 31 01 6F 15 84 10 F0 00 00 00 00 01 90 88 8F 00 00 4D 00 00 00 03 A5 01 42 90 00 9A C6 A5
    if(retval < 0)
    {
        //printf("6666666666\n");
        return ERROR;
    }
    return TRUE;
}

int read_record_file( int uart_port, unsigned char *buf )
{
    int				ret;
//    char *des_data = "00B095001E";
    char des_data[5] = {0x00,0xB2,0x01,0x01,0x00};
    unsigned char	apdu[128];
    unsigned char	answerBuf[256];
    memset( apdu, 0, 128 );

    memcpy(apdu,des_data,5);

    //AA 01 0F 31 01 04 B2 01 01 08 29 B0 09 31 8F 8A 70 78 2C 16 A5
    //AB 01 2A 31 01 5F 24 04 20 24 01 01 5F 34 01 01 5A 08 90 59 06 00 00 00 00 03 5F 14 04 20 14 01 01 5F 44 02 00 00 5F 54 02 00 00 90 00 6E AA A5
    //APDU已准备好
    ret = wedsApduChannel( apdu, (unsigned char)(5), answerBuf, uart_port );
//    printf("Read Record: %d %lX\n",ret , answerBuf);
    if( ret > 0 )
    {
        memcpy( buf, answerBuf, ret );
        return ret;
    }
    else
    {
        return ERROR; //返回数据无效，无法解释
    }
}

int read_dx_cpu_card_app(int uart_port,char *value)
{
    int ret = 0;
    //    char *aid = "F0000000000190888F00004D00000081";
    char aid[64];
    char d_aid[64];
    unsigned char recv_buf[256];
    memset(recv_buf,0,sizeof(recv_buf));

    if(!readtype)
    {
        return FALSE;
    }

    ret = query_cpu_card( uart_port, recv_buf );
    if(ret < 0)
    {
        return ERROR;
    }

    memset(aid,0,sizeof(aid));
    yijitong_get_aid(aid);
    memset(d_aid,0,sizeof(d_aid));
    string2hex(aid, d_aid, strlen(aid));
    ret = select_app(d_aid, uart_port);
    if(ret < 0)
    {
        return ERROR;
    }
    memset(recv_buf,0,sizeof(recv_buf));
    ret = read_record_file( uart_port, recv_buf );
    if(ret < 0)
    {
        return ERROR;
    }

    gsmBytes2String((unsigned char *)&recv_buf[10], value, 10);
    printf("value:%s\n",value);
    return TRUE;
}

int read_dx_cpu_card_app_sjy(int uart_port,char *value)
{
    int ret = 0;

    unsigned char aid[64] = {
    		0xF0,0x00,0x00,0x00,
    		0x00,0x01,0x90,0x88,
    		0x8F,0x00,0x00,0x4D,
    		0x00,0x00,0x00,0x81
    };
    unsigned char recv_buf[256];
    memset(recv_buf,0,sizeof(recv_buf));

    ret = select_app(aid, uart_port);
    if(ret < 0)
    {
        return ERROR;
    }
    memset(recv_buf,0,sizeof(recv_buf));
    ret = read_record_file( uart_port, recv_buf );
    if(ret < 0)
    {
        return ERROR;
    }
    gsmBytes2String((unsigned char *)&recv_buf[13], value, 8);
    return TRUE;
}

void set_read_cpu_type(int type)
{
    readtype = type;
}
/*
 * 北京叁角弈CPU卡定制
 * 读取应用序列号
 * 选择应用指令”00A404000A477265655F45442F4550“
 * 返回
 * 6F33840A477265655F45442F4550A5259F0801029F0C1E62640022333300010301FFFFFFFF
 * 888800003238 卡号
 * 20140623 卡启用日期
 * 20240620 卡失效日期
 * 00019000
 * */
int read_serialno_sjy(int uart_port,char *value)
{
	int retval = 0;
    unsigned char apdu[20] = {
    		0x00,0xA4,0x04,0x00,
    		0x0A,0x47,0x72,0x65,
    		0x65,0x5F,0x45,0x44,
    		0x2F,0x45,0x50
    };
    char datebuf[9],card_time_start[9],card_time_end[9];
    unsigned char	answerBuf[256];
    time_t curtime;
    curtime=time(NULL);
	memset(datebuf,0,sizeof(datebuf));
	memset(card_time_start,0,sizeof(card_time_start));
	memset(card_time_end,0,sizeof(card_time_end));
	FormatDateTime((const char *)"YYYYMMDD",datebuf,curtime);

    memset(answerBuf, 0, sizeof(answerBuf));

    retval = wedsApduChannel( apdu, (unsigned char)15, answerBuf, uart_port );
    if(retval < 0)
    {
    	strcpy(value,"FFFFFFFFFFFF");
    	serial_clear(uart_port);
        return ERROR;
    }
    gsmBytes2String((unsigned char *)&answerBuf[43], card_time_start, 4);
    gsmBytes2String((unsigned char *)&answerBuf[47], card_time_end, 4);

    if(strcmp(datebuf,card_time_start)>=0&&strcmp(datebuf,card_time_end)<=0)
    {
    	   gsmBytes2String((unsigned char *)&answerBuf[37], value, 6);
    	   serial_clear(uart_port);
    	   return TRUE;
    }
    strcpy(value,"FFFFFFFFFFFF");
    serial_clear(uart_port);
    return ERROR;
}
int nj_find_card(int uart_port)
{
	char buf[9]={0x02,0x00,0x04,0x32,0x24,0x00,0x00,0x16,0x03};
	int retval = 0, i = 0;
    char data[4096];
    static char data_buffer[64];
    static int buff_len = 0;


    memset(data,0,sizeof(data));
    retval = serial_recv_data_all(uart_port,(unsigned char *)data);
//    printf("%s,%d\n",__func__,retval);
//    int j=0;
//    for(j=0;j<retval;j++)
//    {
//    	printf("%02X ",data[j]);
//    }
//    printf("\n");
    if(retval == ERROR)
    {

        return ERROR;
    }
    else if ((buff_len + retval) >= 80)
    {
		memset(data_buffer,0,sizeof(data_buffer));
		buff_len = 0;
		return ERROR;
    }
    else
    {
            retval = serial_send_data(uart_port, (char *)buf, 9);
    	memcpy(&data_buffer[buff_len], data, retval);
        buff_len += retval;
        if(buff_len < 7)
        	return ERROR;
    }
    for (i = buff_len - 7; i >= 0 ; i--)
    {
        if ((data_buffer[i] == 0x02)&&(data_buffer[i+3]==0x00)&&(data_buffer[i+4]==0x00))
        {
        	serial_clear((TCOM)uart_port); //清空串口缓存
			memset(data_buffer,0,sizeof(data_buffer));
			buff_len = 0;
			return SUCCESS;
        }
    }
    serial_clear((TCOM)uart_port); //清空串口缓存
	memset(data_buffer,0,sizeof(data_buffer));
	buff_len = 0;
    return ERROR;
}
int nj_select_app(int uart_port)
{
	char buf[12]={0x02,0x00,0x07,0x32,0x26,0xFF,0x5A,0x01,0x00,0x00,0xB0,0x03};
	int retval = 0;
    int overtime=0;
    struct timeval tv1,tv2;
    char RecvBuf[128];
    char *rec_buf = NULL;
    rec_buf = (char *)RecvBuf;
    retval = serial_send_data(uart_port, (char *)buf, 12);

    if(retval == ERROR || retval != 12)
    {
        return ERROR;
    }

    gettimeofday(&tv1,NULL);
    gettimeofday(&tv2,NULL);

    overtime=tv1.tv_sec *1000000+tv1.tv_usec;
    memset(RecvBuf,0,sizeof(RecvBuf));
    while(1)
    {
        if(abs( (tv2.tv_sec *1000000 + tv2.tv_usec) - overtime) > 500000)
        {
            return ERROR;
        }
        gettimeofday(&tv2,NULL);
        retval = serial_recv_onebyte(uart_port, (char *)rec_buf);
        if (retval == ERROR )
        {
            return ERROR;
        }
        if(*rec_buf != 0x02)// 包头
        {
            continue;
        }
        else
        {
            break;
        }
    }
    retval = serial_recv_data(uart_port,RecvBuf,22);
//	retval = serial_recv_data_all(uart_port,(unsigned char *)RecvBuf);

//	int j=0;
//	for(j=0;j<retval;j++)
//	{
//		printf("%02X ",RecvBuf[j]);
//	}
//	printf("\n");
        gettimeofday(&tv1,NULL);
        gettimeofday(&tv2,NULL);

	overtime=tv1.tv_sec *1000000+tv1.tv_usec;
	memset(RecvBuf,0,sizeof(RecvBuf));
	while(1)
	{
                if(abs( (tv2.tv_sec *1000000 + tv2.tv_usec) - overtime) > 500000)
		{
			return ERROR;
		}
		gettimeofday(&tv2,NULL);
		retval = serial_recv_onebyte(uart_port, (char *)rec_buf);
		if (retval == ERROR )
		{
			return ERROR;
		}
		if(*rec_buf != 0x02)// 包头
		{
			continue;
		}
		else
		{
			break;
		}
        }
//        retval = serial_recv_data_all(uart_port,(unsigned char *)RecvBuf);
//	for(j=0;j<retval;j++)
//	{
//		printf("%02X ",RecvBuf[j]);
//	}
//	printf("\n");
    if( ( RecvBuf[2] == 0x00 ) && (RecvBuf[3] == 0x00 ) )
    {
        return SUCCESS;
    }
    return ERROR;
}
int nj_read_data(int uart_port,char *value)
{
        char buf[16]={0x02,0x00,0x0B,0x32,0x26,0xFF,0xBD,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x59,0x03};
        int retval = 0;

    int overtime=0;
    struct timeval tv1,tv2;
    char RecvBuf[128];
    char *rec_buf = NULL;
    rec_buf = (char *)RecvBuf;
    retval = serial_send_data(uart_port, (char *)buf, 16);
    if(retval == ERROR || retval != 16)
    {
        return ERROR;
    }
    gettimeofday(&tv1,NULL);
    gettimeofday(&tv2,NULL);

    overtime=tv1.tv_sec *1000000+tv1.tv_usec;
    memset(rec_buf,0,sizeof(rec_buf));
    while(1)
    {
        if(abs( (tv2.tv_sec *1000000 + tv2.tv_usec) - overtime) > 1000000)
        {
            return ERROR;
        }
        gettimeofday(&tv2,NULL);
        retval = serial_recv_onebyte(uart_port, (char *)rec_buf);
        if (retval == ERROR )
        {
            return ERROR;
        }
        if(*rec_buf != 0x02)// 包头
        {
            continue;
        }
        else
        {

            break;
        }
    }
//	retval = serial_recv_data_all(uart_port,(unsigned char *)RecvBuf);
	retval = serial_recv_data(uart_port,RecvBuf,39);
//	int j=0;
//	for(j=0;j<retval;j++)
//	{
//		printf("%02X ",RecvBuf[j]);
//	}
//	printf("\n");

    if( ( RecvBuf[2] == 0x00 ) && (RecvBuf[3] == 0x00 ) )
    {
    	if(RecvBuf[retval-1] != 0x03)
    	{
    		return ERROR;
    	}
    	memcpy(value,RecvBuf,retval);
//    	int j=0;
//    	for(j=0;j<retval;j++)
//    	{
//    		printf("%02X ",value[j]);
//    	}
//    	printf("\n");
        return retval;
    }
    return ERROR;

}
//客户卡头，在有weds_devices的情况下，寻卡返回两条；其他情况未验证已发货
int read_nj_citizen_card(int uart_port,char *value)
{
	int ret = 0;
	char ret_value[128];
        static unsigned int num=0;
        if(num++%2!=0)
            return ERROR;

	ret = nj_find_card(uart_port);
        //printf("1 %d\n",ret);
	if(ret == ERROR)
	{
		return ERROR;
	}
	ret = nj_select_app(uart_port);
       // printf("2 %d\n",ret);
	if(ret == ERROR)
	{
		return ERROR;
	}
	memset(ret_value,0,sizeof(ret_value));
	ret = nj_read_data(uart_port,ret_value);
      //  printf("3 %d\n",ret);
	if(ret == ERROR)
	{
		return ERROR;
	}
	gsmBytes2String((unsigned char *)&ret_value[ret-7], value, 4);
	return TRUE;
}
int hz_reset_card(int uart_port)
{
	char buf[6]={0xAA,0x02,0x21,0x23,0xCC};
	int retval = 0;
    int overtime=0;
    struct timeval tv1,tv2;
    char RecvBuf[128];
    char *rec_buf = NULL;
    rec_buf = (char *)RecvBuf;

    retval = serial_send_data(uart_port, (char *)buf, 5);
    if(retval == ERROR || retval != 5)
    {
        return ERROR;
    }
    gettimeofday(&tv1,NULL);
    gettimeofday(&tv2,NULL);

    overtime=tv1.tv_sec *1000000+tv1.tv_usec;
    memset(RecvBuf,0,sizeof(RecvBuf));
    while(1)
    {
        if(abs( (tv2.tv_sec *1000000 + tv2.tv_usec) - overtime) > 3000000)
        {
            return ERROR;
        }
        gettimeofday(&tv2,NULL);
        retval = serial_recv_onebyte(uart_port, (char *)rec_buf);
        if (retval == ERROR )
        {
            return ERROR;
        }
        if(*rec_buf != 0xBB)// 包头
        {
            continue;
        }
        else
        {
            break;
        }
    }
	retval = serial_recv_data_all(uart_port,(unsigned char *)RecvBuf);
//	int j = 0;
//	for(j=0;j<retval;j++)
//	{
//		printf("%02X ",RecvBuf[j]);
//	}
//	printf("\n");
    if( RecvBuf[2] == 0x00 )
    {
    	serial_clear(uart_port);
        return SUCCESS;
    }
    return ERROR;


}

int hz_read_card_ic(int uart_port,char *value)
{
	char buf[18]={0xAA,0x02,0xD1,0xD3,0xCC};
	int retval = 0;
    int overtime=0;
    struct timeval tv1,tv2;
    char RecvBuf[128];
    char *rec_buf = NULL;
    rec_buf = (char *)RecvBuf;

    retval = serial_send_data(uart_port, (char *)buf, 5);
    if(retval == ERROR || retval != 5)
    {
        return ERROR;
    }
    gettimeofday(&tv1,NULL);
    gettimeofday(&tv2,NULL);
    overtime=tv1.tv_sec *1000000+tv1.tv_usec;
    memset(RecvBuf,0,sizeof(RecvBuf));
    while(1)
    {
        if(abs( (tv2.tv_sec *1000000 + tv2.tv_usec) - overtime) > 300000)
        {
            return ERROR;
        }
        gettimeofday(&tv2,NULL);
        retval = serial_recv_onebyte(uart_port, (char *)rec_buf);
        if (retval == ERROR )
        {
            return ERROR;
        }
        if(*rec_buf != 0xBB)// 包头
        {
            continue;
        }
        else
        {
            break;
        }
    }
    retval = serial_recv_data(uart_port,RecvBuf,8);
    if(retval != 8)
    {
    	return ERROR;
    }
//    printf("=======send2===%d\n",retval);
//	int j = 0;
//	for(j=0;j<retval;j++)
//	{
//		printf("%02X ",RecvBuf[j]);
//	}
//	printf("\n");
    if(RecvBuf[1] == 0XD1&&RecvBuf[7] == 0XCC)
    {
    	memcpy(value,RecvBuf,retval);
        return retval;
    }
    return ERROR;
}
int hz_read_card(int uart_port,char *value)
{
	char buf[18]={0xAA,0x0E,0xD0,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,
			0xDE,0xCC};
	int retval = 0;
    int overtime=0;
    struct timeval tv1,tv2;
    char RecvBuf[128];
    char *rec_buf = NULL;
    rec_buf = (char *)RecvBuf;
    retval = serial_send_data(uart_port, (char *)buf, 17);
    if(retval == ERROR || retval != 17)
    {
        return ERROR;
    }
    gettimeofday(&tv1,NULL);
    gettimeofday(&tv2,NULL);
    overtime=tv1.tv_sec *1000000+tv1.tv_usec;
    memset(RecvBuf,0,sizeof(RecvBuf));
    while(1)
    {
        if(abs( (tv2.tv_sec *1000000 + tv2.tv_usec) - overtime) > 1000000)
        {
            return ERROR;
        }
        gettimeofday(&tv2,NULL);
        retval = serial_recv_onebyte(uart_port, (char *)rec_buf);
        if (retval == ERROR )
        {
            return ERROR;
        }
        if(*rec_buf != 0xBB)// 包头
        {
            continue;
        }
        else
        {
            break;
        }
    }
    retval = serial_recv_data(uart_port,RecvBuf,61+2);
	if(retval != 63)
	{
		return ERROR;
	}
//	printf("=======send1===%d\n",retval);
//	int j = 0;
//	for(j=0;j<retval;j++)
//	{
//		printf("%02X ",RecvBuf[j]);
//	}
//	printf("\n");
    if( RecvBuf[2] == 0x00 && retval > 8)
    {
    	memcpy(value,RecvBuf,retval);
        return retval;
    }
    return ERROR;


}
int read_hz_citizen_card(int uart_port,char *value)
{
	int ret = 0;
	char ret_value[128];
	memset(ret_value,0,sizeof(ret_value));
//	hz_reset_card(uart_port);
	ret = hz_read_card_ic(uart_port,ret_value);
	if(ret == ERROR)
	{
		return ERROR;
	}
	sprintf(value,"%02X%02X%02X%02X",ret_value[5],ret_value[4],ret_value[3],ret_value[2]);
	return TRUE;
}
//int read_hz_citizen_card(int uart_port,char *value)
//{
//	int ret = 0;
//	char d0_value[128],d1_value[128];
//	memset(d0_value,0,sizeof(d0_value));
//	memset(d1_value,0,sizeof(d1_value));
//	ret = hz_read_card_ic(uart_port,d1_value);
//	if(ret == ERROR)
//	{
//		return ERROR;
//	}
//	serial_clear(uart_port);
//	ret = hz_read_card(uart_port,d0_value);
//	if(ret == ERROR)
//	{
//		sprintf(value,"%02X%02X%02X%02X",d1_value[5],d1_value[4],d1_value[3],d1_value[2]);
//	}
//	else
//		sprintf(value,"%02X%02X%02X%02X",d0_value[7],d0_value[6],d0_value[5],d0_value[4]);
//	return TRUE;
//}


/*
 mode: 00- 按文件标识符选择 MF 或 DF     02- 选择 EF    04-按文件名选择应用
 value:  aid or 标识符

 */

int read_cpu_data1(int uart_port,int mode,char *value)
{
 unsigned char apdu[128],answerBuf[128];
 int num=0,i=0,ret;
 int c_len=0;
 unsigned char	Fhead[] = { 0xAA, 0x01, 0x00, 0x31, 0x01 };             // 帧头+设备地址+数据长度+指令字+数据包数量
 unsigned char	SendBuf[1024];                                           //255+5(0X104)
 unsigned char	Recv_Buf[1024];


 //make apdu
 apdu[0]=0x00;
 apdu[1]=0xA4;
 apdu[2]=mode; //00- 按文件标识符选择 MF 或 DF     02- 选择 EF    04-按文件名选择应用
 apdu[3]=0x00;
 apdu[4]=gsmString2Bytes(value,apdu+5,strlen(value));
 num=5+apdu[4]; //apdu length
 //strcpy(apdu+5,value);
 memset(answerBuf,0,sizeof(answerBuf));
//make card command
 int len = 0;
 memset( Recv_Buf, 0, sizeof(Recv_Buf) );
 memset( SendBuf, 0, sizeof(SendBuf) );
 memcpy( SendBuf, Fhead, 5 );
 SendBuf[2] = num + 2;                                                //数据长度
 memcpy( (unsigned char *)SendBuf + 5, apdu, num );

 //send command
 len = send_recv_data(uart_port,SendBuf,Recv_Buf);
/*
 for(i=0;i<len;i++)
     printf("%02X ",Recv_Buf[i]);
 printf("%s \n",__func__);
*/
 if(len < 0)
 {
     return ERROR;
 }
 if(Recv_Buf[2] >= 0X3E)
 {
     c_len = 2;
 }
 else
 {
     c_len = 1;
 }
 if( ( Recv_Buf[3] == 0x31 ) && ( Recv_Buf[4] == 0x01 ) &&
         Recv_Buf[Recv_Buf[2]+c_len] == 0x90 ) //最少数据为2字节返回码sw1 sw2
 {
     memcpy( answerBuf, Recv_Buf + 5, Recv_Buf[2] - 2 );
    // return Recv_Buf[2] - 2;                                              //返回apdu数据长度
 }
 else
 {
     return ERROR;
 }




  return TRUE;
}


int read_record_file_public( int uart_port,int num,unsigned char sfi,int le,unsigned char *buf )
{
    int				ret,i;
//    char *des_data = "00B095001E";
    char des_data[5] = {0x00,0xB2,0x00,0x0,0x0};
    unsigned char	apdu[128];
    unsigned char	answerBuf[256];
    memset( apdu, 0, 128 );
    des_data[2]=num;
    des_data[3]=sfi<<3|4;
    des_data[4]=le;
    memcpy(apdu,des_data,5);
    //apdu[2]=num;
    //apdu[3]=


    //AA 01 0F 31 01 04 B2 01 01 08 29 B0 09 31 8F 8A 70 78 2C 16 A5
    //AB 01 2A 31 01 5F 24 04 20 24 01 01 5F 34 01 01 5A 08 90 59 06 00 00 00 00 03 5F 14 04 20 14 01 01 5F 44 02 00 00 5F 54 02 00 00 90 00 6E AA A5
    //APDU已准备好

    ret = wedsApduChannel( apdu, (unsigned char)(5), answerBuf, uart_port );
    printf("Read Record: %d \n",ret );

    if( ret > 0 )
    {
        memcpy( buf, answerBuf, ret );
        return ret;
    }
    else
    {
        return ERROR; //返回数据无效，无法解释
    }
}

int read_binary_file_public(int uart_port, int efId,int offset,int len,unsigned char *buf)
{
    int				ret;
//    char *des_data = "00B095001E";
    char des_data[5] = {0x00,0xB0,00,0x01,0x00};
    unsigned char	apdu[128];
    unsigned char	answerBuf[256];

    des_data[2]=0x80|efId;
    des_data[3]=offset;
    des_data[4]=len;
    memset( apdu, 0, 128 );


    memcpy(apdu,des_data,5);

      //APDU已准备好
    ret = wedsApduChannel( apdu, (unsigned char)(5), buf, uart_port );
    if( ret > 0 )
    {
       // memcpy( buf, answerBuf, ret );
        return ret;
    }
    else
    {
        return ERROR; //返回数据无效，无法解释
    }
}
/*
uart_port  com
mode 0/4           0，文件标识符，4文件名称
aid
efit  efile id
offset
*/

int read_cpu_data(int uart_port,int mode,char *aid,int efid,int offset,int le,unsigned char *buf)
{
    int ret = 0,num;
    char d_aid[64];
    unsigned char recv_buf[256];
    char value[64],cardno[16];
    memset(recv_buf,0,sizeof(recv_buf));

    ret = select_public_app(aid,mode ,uart_port);
    if(ret < 0)
    {
        return ERROR;
    }
    memset(recv_buf,0,sizeof(recv_buf));
    //printf("%s,select aid success\n",__func__);
    //ret = read_record_file( uart_port, recv_buf );
    ret=read_binary_file_public(uart_port,efid,offset,le,recv_buf);
    //printf("%s, read_binary ret %d\n",__func__,ret);
    if(ret < 0)
    {
        return ERROR;
    }
    hex2string(recv_buf,buf,ret-2);
    return ret;
}
int read_binary_file_xiangtan(int uart_port, int efId,int offset,int len,unsigned char *buf)
{
    int				ret;
//    char *des_data = "00B095001E";
    char des_data[5] = {0x00,0xB0,00,0x01,0x00};
    unsigned char	apdu[128];
    unsigned char	answerBuf[256];

    des_data[2]=efId;
    des_data[3]=offset;
    des_data[4]=len;
    memset( apdu, 0, 128 );


    memcpy(apdu,des_data,5);

      //APDU已准备好
    ret = wedsApduChannel( apdu, (unsigned char)(5), buf, uart_port );
    if( ret > 0 )
    {
       // memcpy( buf, answerBuf, ret );
        return ret;
    }
    else
    {
        return ERROR; //返回数据无效，无法解释
    }
}
//湘潭大学
int read_cpu_data_xiangtan(int uart_port,char *no)
{
    int ret = 0,num;
    unsigned char recv_buf[32],cardno[32];
    char value[64];
    char name[32];
    memset(recv_buf,0,sizeof(recv_buf));
    ret=device_recv_data(uart_port,cardno);
    if(!(ret==0x95||ret==0x94))
    {
        return ERROR;
    }

   ret = select_public_app("48554E2E48595359532E4444463031",4 ,uart_port);//HUN.HYSYS.DDF01
    if(ret < 0)
    {
       return ERROR;
    }
    ret = select_public_app("BAFEC4CFBDA8D00101",4 ,uart_port);
    if(ret < 0)
    {
        return ERROR;
    }
    ret=read_binary_file_xiangtan(uart_port,8,31,20,recv_buf);
//}

    if(ret < 0)
    {
        return ERROR;
    }
    sprintf(no,"%s,%s",cardno,recv_buf);
    return TRUE;
}

int read_cpu_get_response(int uart_port,int len,char *data)
{
    int				ret;
    char apdu[5] = {0x00,0xC0,0x00,0x00,00};
    unsigned char	answerBuf[256];
    apdu[4]=len;  //le

    //AA 01 0F 31 01 04 B2 01 01 08 29 B0 09 31 8F 8A 70 78 2C 16 A5
    //AB 01 2A 31 01 5F 24 04 20 24 01 01 5F 34 01 01 5A 08 90 59 06 00 00 00 00 03 5F 14 04 20 14 01 01 5F 44 02 00 00 5F 54 02 00 00 90 00 6E AA A5
    //APDU已准备好
    ret = wedsApduChannel( apdu, (unsigned char)(5), answerBuf, uart_port );
    printf("Read Record: %d \n",ret );

    if( ret > 0 )
    {
        memcpy( data, answerBuf, ret );
        return ret;
    }
    else
    {
        return ERROR; //返回数据无效，无法解释
    }
}


int read_cpu_get_gpo(int uart_port,int len,unsigned char *data)
{
    unsigned char head[5] = {0x80,0xA8,0x00,0x00};
    unsigned char apdu[128];
    unsigned char answerBuf[256];
    unsigned char *addr = apdu;
    int retval = 0;
    int num=0,i=0;
    memset( apdu, 0, sizeof(apdu) );
    memset( answerBuf, 0, sizeof(answerBuf) );

    head[4]= string2hex(data,apdu+5, strlen(data));
    //printf("%s,%02X,%02X,%02X\n",__func__,head[4],len,strlen(data));
    memcpy(apdu, head,5);
   // memcpy(addr,appID,num);
    num=head[4]+5;

    //printf("%s %s\n",__func__,apdu+5);
    //AA 01 17 31 01 00 A4 04 00 10 F0 00 00 00 00 01 90 88 8F 00 00 4D 00 00 00 81 3C 68 A5
    retval = wedsApduChannel( apdu, (unsigned char)num, answerBuf, uart_port );
    //AB 01 1B 31 01 6F 15 84 10 F0 00 00 00 00 01 90 88 8F 00 00 4D 00 00 00 03 A5 01 42 90 00 9A C6 A5
    if(retval < 0)
    {
        //printf("6666666666\n");
        return ERROR;
    }
    return TRUE;
}



//读银行卡号
int read_cpu_data_yhkh(int uart_port,char *no)
{
    int ret = 0,num=0;
    unsigned char recv_buf[128],cardno[128];
    char value[128];
    char name[32],sfz[32];
    memset(recv_buf,0,sizeof(recv_buf));
    ret=device_recv_data(uart_port,cardno);
    if(!(ret==0x95||ret==0x94))
    {
       return ERROR;
    }
    //选择银行应用
   ret = select_public_app("315041592E5359532E4444463031",4 ,uart_port);//1PAY.SYS.DDF01
//printf("%s,1 %d\n",__func__,ret);
    if(ret < 0)
    {
       return ERROR;
    }
    //读取公共信息文件
     ret=read_record_file_public(uart_port,0x01,0x01,0xffff,recv_buf);
   //  printf("%s,2 %d\n",__func__,ret);
     if(ret<0)
     {
         return ERROR;
     }
     //获取应用目录名称
    if(recv_buf[4]==0x4F)
     {
        memset(value,0,sizeof(value));
        num=recv_buf[5];
        hex2string(recv_buf+6,value,num);
    }
     //选择应用目录
    ret=select_public_app(value,4,uart_port);//"A000000333010101"

//printf("%s,3 %d\n",__func__,ret);
    if(ret<0)
    {
        return ERROR;
    }
    memset(recv_buf,0,sizeof(recv_buf));
    //读取应用基本信息文件
    ret=read_record_file_public(uart_port,1,1,0xffff,recv_buf);
    if(ret<0)
    {
        return ERROR;
    }

    int i=0;
    memset(sfz,0,sizeof(sfz));
    memset(name,0,sizeof(name));
    for(i=0;i<ret-13;i++)
    {
        if(recv_buf[i]==0x57&&recv_buf[i+1]==0x13)
        {
            hex2string(recv_buf+i+2,no,10);
            no[19]=0;
           // printf("yhkh %s\n",no);
            break;
        }
        if(recv_buf[i]==0x9f&&recv_buf[i+1]==0x61)
        {
            memcpy(sfz,recv_buf+i+3,recv_buf[i+2]-1);
            printf("sfz %s\n",sfz);
        }
        if(recv_buf[i]==0x5f&&recv_buf[i+1]==0x20)
        {
            memcpy(name,recv_buf+i+3,recv_buf[i+2]-1);
            printf("------------------------------------xm %d %s\n",recv_buf[i+2],name);
        }
    }

   // ret=read_cpu_get_response(uart_port,0x2C,recv_buf);
//ret=read_cpu_get_gpo(uart_port,0x1c,"831B9F66049F069F03069F1A0295055F2A029A039C019F3704DF015F");


    //sprintf(no,"%s,%s",cardno,recv_buf);
    return TRUE;
}





//读指令卡号，中广瑞波
//00A4040007 A000000333010101
//00B2010C00
//70 30 57 13 6229160010469736 D2
//6229160010469736 是卡号
int read_cpu_data_zhongguangruibo(int uart_port,char *no)
{
    int ret = 0,num=0,i;
    unsigned char recv_buf[128],cardno[128];
    char value[128];
    char name[32],sfz[32];
    static int flag=0;
    memset(recv_buf,0,sizeof(recv_buf));
    ret=device_recv_data(uart_port,cardno);
    if(!(ret==0x95||ret==0x94))
    {
       return ERROR;
    }
    //printf("%s,0 %d,%s\n",__func__,ret,cardno);
    //末字节时08，则卡号时随机卡号，需要读指令卡号；末字节是08，则需要读指令卡号
    if(strcmp(cardno+6,"08")!=0)
    {
        strcpy(no,cardno);
        return TRUE;
    }
    //选择应用
    ret = select_public_app("A0000003330101",4 ,uart_port);//
    //printf("%s,1 %d\n",__func__,ret);
    if(ret < 0)
    {
       return ERROR;
    }
    //读取信息文件
     ret=read_record_file_public(uart_port,0x01,0x01,0x0,recv_buf);
    // printf("%s,2 %d\n",__func__,ret);
     if(ret<0||recv_buf[0]!=0x70||recv_buf[2]!=0x57)
     {
         return ERROR;
     }
//    sprintf(no,"%s,%s",cardno,recv_buf);
//     for(i=0;i<ret;i++)
//         printf("%02X ",recv_buf[i]);
     ret=hex2string(recv_buf+4,no,recv_buf[3]);
     //以‘D’为卡号结束符，不包括’D‘
     for(i=0;i<ret;i++)
         if(no[i]=='D')
             no[i]=0;
    return TRUE;
}


// 20171116 Beijing leisen apple pay
// 20171120 用户提供的这种方式不能满足要求，废弃，仍然使用接口 read_cpu_data_zhongguangruibo
int read_cpu_data_beijingleisen_appleapy(int uart_port,char *no)
{
    int ret = 0,num=0,i;
    unsigned char recv_buf[128],cardno[128];
    char value[128];
    char name[32],sfz[32];
    static int flag=0;
    memset(recv_buf,0,sizeof(recv_buf));
    ret=device_recv_data(uart_port,cardno);

    if(!(ret==0x95||ret==0x94))
    {
       return ERROR;
    }

    //选择应用
    ret = select_public_app("A000000333010101",4 ,uart_port);
    //printf("%s,1 %d\n",__func__,ret);
    if(ret < 0)		// 非随机卡号
    {	
	   sprintf(no,"%s",cardno);
	   printf("apple pay 3=%s\n", no);
	   return TRUE;
//       return ERROR;
    }

    //读取信息文件
     ret=read_record_file_public(uart_port,0x01,0x0C,0x00,recv_buf);
    // printf("%s,2 %d\n",__func__,ret);
     if(ret<0)
     {printf("apple pay 4\n");
         return ERROR;
     }
	 printf("apple pay 5\n");
//     for(i=0;i<ret;i++)
//         printf("%02X ",recv_buf[i]);
     ret=hex2string(recv_buf+5,no,8);

    return TRUE;
}



//读逻辑卡号，公司自有消费CPU卡
//
int read_cpu_data_weds(int uart_port,char *no)
{
   int ret = 0,num=0,i;
//    unsigned char recv_buf[32],cardno[128];
//    char value[128];
//    char name[32],sfz[32];
//    static int flag=0;
//    memset(recv_buf,0,sizeof(recv_buf));
//    ret=device_recv_data(uart_port,cardno);

//    if(!(ret==CPU_CARDNO||ret==MF1_S50_CARDNO))
//    {
//       return ERROR;
//    }
//    ret=active_read_card_cpu(uart_port,cardno);
//    if(ret!=TRUE)
//        return ERROR;

//    memset(recv_buf,0,sizeof(recv_buf));
//    ret=read_cpu_data(uart_port,0,"af01",0x11,16*3+4,4,recv_buf);
//    if(ret>0)
//    {
//        sprintf(no,"%s,%s",cardno,recv_buf);
//        return TRUE;
//    }

    ret=read_cpu_data_xn(uart_port,no);
    if(ret>0)
        return TRUE;

    return ERROR;
}




#if 1  // lfg 20160128 add


int serial_recv_nbytes(int comfd, unsigned char *data, int len)
{
	int  cnt =0;

	if(comfd <= 0)
	{
		return ERROR;
	}

	if(statInfo.isSpi == 0)
	{
		cnt=uart_recv_data(comfd, data, len);
	}
	else
	{
		cnt=spi_recv_data(comfd, data, len);
	}

	return cnt;

}


int serial_send_nbytes(int comfd, unsigned char *data, int len)
{
	int cnt=0;

	if(comfd <= 0)
	{
		return ERROR;
	}

	if(statInfo.isSpi == 0)
	{
		cnt=uart_send_data(comfd, data, len);
	}
	else
	{
		cnt=spi_send_data(comfd, data, len);
	}

	return cnt;

}


int sdk_get_comfd(int com)
{
	if(!is_uart(com))
	{
		statInfo.isSpi = 1;
	}

	statInfo.comfd = serial_fd[com];
	
     return statInfo.comfd;
}



// 读取weds读头数据 AB A5协议
// m_hcom: 串口号
// RecvBuf: 接收数据
int card_data_r( int m_hcom, unsigned char *RecvBuf )
{
	int 		retSWLen;		// 读写长度

    int			bWriteStat;
    int			bReadStat;
    long		dwCount;

    unsigned char	* buf  = RecvBuf;
    unsigned char	bcc	   = 0;
    unsigned char	sum	   = 0;

    unsigned char	* addr = NULL;

    long			time1 = GetTickCount( );
    int				len;
    int				datalen;
    int				i;

    len	   = 1;
    addr   = buf;
    while( len )
    {
#if 0    
        bReadStat = ReadFile( m_hcom, addr, len, &dwCount, NULL );
        if( !bReadStat )
        {
                return ERR_COMERR; //读串口失败
        }
#else
		retSWLen = serial_recv_nbytes(m_hcom, addr, len);
		if(retSWLen < 0)
		{
			return ERR_COMERR; //读串口失败
		}
		dwCount = retSWLen;
#endif
		
        len   -= dwCount;
        addr  += dwCount;
        if( len > 0 )
        {
                if( GetTickCount( ) - time1 > 10 )
                {
                        return -1;          //超时
                }else if( !dwCount )
                {
                        sched_yield( );     //让出cpu,;
                }
        }else if( 0xab != buf[0] )  //未读到帧头
        {
                len	   = 1;
                addr   = buf;
        }
    }
    len = 2;
    while( len )                    //读设备地址和数据域长度
    {
#if 0    
		bReadStat = ReadFile( m_hcom, addr, len, &dwCount, NULL );
		if( !bReadStat )
		{
				return ERR_COMERR; //读串口失败
		}
#else
		retSWLen = serial_recv_nbytes(m_hcom, addr, len);
		if(retSWLen < 0)
		{
			return ERR_COMERR; //读串口失败
		}
		dwCount = retSWLen;
#endif
		
        len	  -= dwCount;
        addr  += dwCount;
        if( len > 0 )
        {
                if( GetTickCount( ) - time1 > 30 )
                {
                        return -1;      //超时
                }else if( !dwCount )
                {
                        sched_yield( ); //让出cpu,;
                }
        }
    }

    len	   = buf[2] + 3;
    time1  = GetTickCount( );
    while( len )                //接收后续数据
    {
#if 0    
		bReadStat = ReadFile( m_hcom, addr, len, &dwCount, NULL );
		if( !bReadStat )
		{
				return ERR_COMERR; //读串口失败
		}
#else
		retSWLen = serial_recv_nbytes(m_hcom, addr, len);
		if(retSWLen < 0)
		{
			return ERR_COMERR; //读串口失败
		}
		dwCount = retSWLen;
#endif		
        len	  -= dwCount;
        addr  += dwCount;
        if( len > 0 )
        {
                if( GetTickCount( ) - time1 > 20 )
                {
                        return -1;              //超时
                }else if( !dwCount )
                {
                        sched_yield( );         //让出cpu,;
                }
        }
    }

    bcc	   = 0;
    sum	   = 0;
    for( i = 0; i < buf[2] + 2; i++ )   //长度
    {
            bcc	  ^= buf[i + 1];
            sum	  += buf[i + 1];
    }

    if( ( buf[buf[2] + 3] == bcc ) && ( buf[buf[2] + 4] == sum ) )
    {
            return buf[2] + 6;                              //返回正确
    }
	
    return -1;
	
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
int card_data_sr( int m_hcom, unsigned char *SendBuf, unsigned char *RecvBuf, int RetryCount, int timeout )
{
        int			bWriteStat;
        int			bReadStat;
		int			retSWLen;		// 读写长度
        long			dwCount;
        unsigned char	* buf  = RecvBuf;
        unsigned char	bcc	   = 0;
        unsigned char	sum	   = 0;
        unsigned char	* addr;
        long			time1;
        int				len;
        int				datalen;
        int				i;
        switch( statInfo.readerType ) //卡头类型
        {
                case WEDS_READER:
                case WEDS_READER_CPU:
                        if( SendBuf[0] != 0xaa )
                        {
                                return ERR_HEADINVALID;
                        }

                        for( i = 0; i < SendBuf[2] + 2; i++ ) //长度
                        {
                                bcc	  ^= SendBuf[i + 1];
                                sum	  += SendBuf[i + 1];
                        }

                        SendBuf[SendBuf[2] + 3]	   = bcc;
                        SendBuf[SendBuf[2] + 4]	   = sum;
                        SendBuf[SendBuf[2] + 5]	   = 0xa5;

                        for( i = 0; i < RetryCount + 1; i++ )
                        {
#if 0                        
                                tcflush( m_hcom, TCIOFLUSH );   //清空缓冲区
                                bWriteStat = WriteFile( m_hcom, SendBuf, SendBuf[2] + 6, &dwCount, NULL );
                                if( !bWriteStat )
                                {
                                        return ERR_COMERR;          //写串口失败
                                }
#else
								retSWLen = serial_send_nbytes(m_hcom, SendBuf, SendBuf[2] + 6);
//							leeDebugData(SendBuf, SendBuf[2] + 6, SendBuf[2] + 6, 1);

								if(retSWLen == ERROR || retSWLen != SendBuf[2] + 6)
								{
									return ERR_COMERR;
								}
#endif

                                time1 = timeout + GetTickCount( );
                                sched_yield( );                 //让出cpu,

                                len	   = 1;
                                addr   = buf;

                                while( len )
                                {
#if 0                                
                                        bReadStat = ReadFile( m_hcom, addr, len, &dwCount, NULL );
                                        if( !bReadStat )
                                        {
                                            return ERR_COMERR; //读串口失败
                                        }
#else										
										retSWLen = serial_recv_nbytes(m_hcom, addr, len);
										if(retSWLen < 0)
										{
											return ERR_COMERR; //读串口失败
										}
										dwCount = retSWLen;
#endif										

                                        len	  -= dwCount;
                                        addr  += dwCount;

                                        if( len > 0 )
                                        {
                                                if( GetTickCount( ) > time1 )
                                                {
                                                        goto timeout1;      //超时
                                                }else if( !dwCount )
                                                {
                                                        sched_yield( );     //让出cpu,;
                                                }
                                        }else if( 0xab != buf[0] )  //未读到帧头
                                        {
                                                len	   = 1;
                                                addr   = buf;
                                        }
                                }

                                len = 2;
                                while( len )                //读设备地址和数据域长度
                                {
#if 0                                
                                        bReadStat = ReadFile( m_hcom, addr, len, &dwCount, NULL );
                                        if( !bReadStat )
                                        {
                                                return ERR_COMERR;  //读串口失败
                                        }
#else										
										retSWLen = serial_recv_nbytes(m_hcom, addr, len);
										if(retSWLen < 0)
										{
											return ERR_COMERR; //读串口失败
										}
										dwCount = retSWLen;
#endif	

                                        len	  -= dwCount;
                                        addr  += dwCount;

                                        if( len > 0 )
                                        {
                                                if( GetTickCount( ) > time1 )
                                                {
                                                        goto timeout1;  //超时
                                                }else if( !dwCount )
                                                {
                                                        sched_yield( ); //让出cpu,;
                                                }
                                        }
                                }

                                len = buf[2] + 3;

                                while( len )                //接收后续数据
                                {
#if 0                                
										bReadStat = ReadFile( m_hcom, addr, len, &dwCount, NULL );
										if( !bReadStat )
										{
												return ERR_COMERR;	//读串口失败
										}
#else										
										retSWLen = serial_recv_nbytes(m_hcom, addr, len);
										if(retSWLen < 0)
										{
											return ERR_COMERR; //读串口失败
										}
										dwCount = retSWLen;
#endif	

                                        len	  -= dwCount;
                                        addr  += dwCount;

                                        if( len > 0 )
                                        {
                                                if( GetTickCount( ) > time1 )
                                                {
                                                        goto timeout1;  //超时
                                                }else if( !dwCount )
                                                {
                                                        sched_yield( ); //让出cpu,;
                                                }
                                        }
                                }

                                bcc	   = 0;
                                sum	   = 0;

                                for( i = 0; i < buf[2] + 2; i++ ) //长度
                                {
                                        bcc	  ^= buf[i + 1];
                                        sum	  += buf[i + 1];
                                }

                                if( ( buf[buf[2] + 3] == bcc ) && ( buf[buf[2] + 4] == sum ) )
                                {
//                                leeDebugData(buf, buf[2] + 6, buf[2] + 6, 2);
                                        return buf[2] + 6;  //返回正确
                                }
timeout1:
                                ;
                        }
                        return ERR_OVERRETRY;       //超过重试次数
                        break;

                case SHNM100_READER:
                case MA102_READER:
                        if( ( ( SendBuf[0] & 0xfc ) != 0x80 ) && ( ( SendBuf[0] & 0xfc ) != 0xa0 ) )
                        {
                                return ERR_HEADINVALID;
                        }

                        datalen = (int)( SendBuf[0] & 0x01 ) * 256 + SendBuf[1];
                        if( datalen > 0x104 )
                        {
                                return ERR_OVERBUF;             //超过最大数据长度
                        }

                        if( SendBuf[0] & 0x02 )             //  有校验
                        {
                                for( i = 0; i < datalen; i++ )  //长度
                                {
                                        bcc	  ^= SendBuf[i + 2];
                                        sum	  += SendBuf[i + 2];
                                }

                                SendBuf[datalen + 3]   = bcc;
                                SendBuf[datalen + 2]   = sum;

                                datalen += 2;                   //加上校验的长度
                        }

                        for( i = 0; i < RetryCount + 1; i++ )
                        {
                                tcflush( m_hcom, TCIOFLUSH );   //清空缓冲区
                                bWriteStat = WriteFile( m_hcom, SendBuf, datalen + 2, &dwCount, NULL );

                                if( !bWriteStat )
                                {
                                        return ERR_COMERR;          //写串口失败
                                }
                                time1 = timeout + GetTickCount( );
//				sched_yield();		//让出cpu,;

                                len	   = 1;
                                addr   = buf;

                                while( len )
                                {
                                        bReadStat = ReadFile( m_hcom, addr, len, &dwCount, NULL );

                                        if( !bReadStat )
                                        {
                                                return ERR_COMERR; //读串口失败
                                        }

                                        len	  -= dwCount;
                                        addr  += dwCount;

                                        if( len > 0 )
                                        {
                                                if( GetTickCount( ) > time1 )
                                                {
                                                        goto timeout2;                  //超时
                                                }
                                        }else if( 0x90 != ( buf[0] & 0xFC ) )   //未读到帧头
                                        {
                                                len	   = 1;
                                                addr   = buf;
                                        }
                                }
                                len = 1;
                                while( len )                                //读数据域长度字节
                                {
                                        bReadStat = ReadFile( m_hcom, addr, len, &dwCount, NULL );

                                        if( !bReadStat )
                                        {
                                                return ERR_COMERR;                  //读串口失败
                                        }

                                        len	  -= dwCount;
                                        addr  += dwCount;

                                        if( len > 0 )
                                        {
                                                if( GetTickCount( ) > time1 )
                                                {
                                                        goto timeout2;  //超时
                                                }
                                        }
                                }
                                len = (int)( buf[0] & 0x01 ) * 256 + ( buf[0] & 0x02 ) + buf[1];
                                if( len > 0x104 )
                                {
                                        len = 0x104;
                                }

                                while( len )                //接收后续数据
                                {
                                        bReadStat = ReadFile( m_hcom, addr, len, &dwCount, NULL );

                                        if( !bReadStat )
                                        {
                                                return ERR_COMERR;  //读串口失败
                                        }

                                        len	  -= dwCount;
                                        addr  += dwCount;

                                        if( len > 0 )
                                        {
                                                if( GetTickCount( ) > time1 )
                                                {
                                                        goto timeout2;  //超时
                                                }
                                        }
                                }

                                len = (int)( buf[0] & 0x01 ) * 256 + buf[1];
                                if( len > 0x104 )
                                {
                                        len = 0x104;
                                }

                                if( buf[0] & 0x02 )
                                {
                                        bcc	   = 0;
                                        sum	   = 0;

                                        for( i = 0; i < len; i++ ) //长度
                                        {
                                                bcc	  ^= buf[i + 2];
                                                sum	  += buf[i + 2];
                                        }

                                        if( ( buf[len + 3] == bcc ) && ( buf[len + 2] == sum ) )
                                        {
                                                return len + 4; //返回正确
                                        }
                                }else
                                {
                                        return len + 2;     //返回正确
                                }
timeout2:
                                ;
                        }
                        return ERR_OVERRETRY;       //超过重试次数

                        break;

                case MINGWAH_READER:
                        if( SendBuf[0] != 0xaa )
                        {
                                return ERR_HEADINVALID;
                        }

                        for( i = 0; i < SendBuf[3] + 4; i++ ) //长度
                        {
                                bcc ^= SendBuf[i];
                        }

                        SendBuf[SendBuf[3] + 4] = bcc;

                        for( i = 0; i < RetryCount + 1; i++ )
                        {
                                tcflush( m_hcom, TCIOFLUSH );   //清空缓冲区
//				bWriteStat = WriteFile(m_hcom, SendBuf, SendBuf[2] + 6, &dwCount, NULL);
                                bWriteStat = WriteFile( m_hcom, SendBuf, SendBuf[3] + 5, &dwCount, NULL );

                                if( !bWriteStat )
                                {
                                        return ERR_COMERR;          //写串口失败
                                }

                                time1 = timeout + GetTickCount( );
                                sched_yield( );                 //让出cpu,;

                                len	   = 1;
                                addr   = buf;

                                while( len )
                                {
                                        bReadStat = ReadFile( m_hcom, addr, len, &dwCount, NULL );

                                        if( !bReadStat )
                                        {
                                                return ERR_COMERR; //读串口失败
                                        }

                                        len	  -= dwCount;
                                        addr  += dwCount;

                                        if( len > 0 )
                                        {
                                                if( GetTickCount( ) > time1 )
                                                {
                                                        goto timeout3;      //超时
                                                }else if( !dwCount )
                                                {
                                                        sched_yield( );     //让出cpu,;
                                                }
                                        }else if( 0x55 != buf[0] )  //未读到帧头
                                        {
                                                len	   = 1;
                                                addr   = buf;
                                        }
                                }

                                len = 3;
                                while( len )                //读返回码+长度,只关心长度
                                {
                                        bReadStat = ReadFile( m_hcom, addr, len, &dwCount, NULL );

                                        if( !bReadStat )
                                        {
                                                return ERR_COMERR;  //读串口失败
                                        }

                                        len	  -= dwCount;
                                        addr  += dwCount;

                                        if( len > 0 )
                                        {
                                                if( GetTickCount( ) > time1 )
                                                {
                                                        goto timeout3;  //超时
                                                }else if( !dwCount )
                                                {
                                                        sched_yield( ); //让出cpu,;
                                                }
                                        }
                                }

                                len = buf[3] + 1;

                                while( len )                //接收后续数据
                                {
                                        bReadStat = ReadFile( m_hcom, addr, len, &dwCount, NULL );

                                        if( !bReadStat )
                                        {
                                                return ERR_COMERR;  //读串口失败
                                        }

                                        len	  -= dwCount;
                                        addr  += dwCount;

                                        if( len > 0 )
                                        {
                                                if( GetTickCount( ) > time1 )
                                                {
                                                        goto timeout3;          //超时
                                                }else if( !dwCount )
                                                {
                                                        sched_yield( );         //让出cpu,;
                                                }
                                        }
                                }

                                bcc = 0;

                                for( i = 0; i < buf[3] + 5; i++ )   //长度
                                {
                                        bcc ^= buf[i];
                                }

                                if( !bcc )
                                {
                                        return buf[3] + 5;              //返回正确
                                }
timeout3:
                                ;
                        }

                        return ERR_OVERRETRY;                   //超过重试次数
                        break;

// fuglee 20120705
                case YOOCAN_READER:
//				case YOOCAN_READONLY:
                        if( SendBuf[0] != 0x5a )
                        {
                                return ERR_HEADINVALID;
                        }

                        if( ( SendBuf[1] != 0x10 ) && ( SendBuf[1] != 0x20 ) && ( SendBuf[1] != 0x22 ) )
                        {
                                return ERR_FALSE;
                        }

                        datalen				   = SendBuf[2] * 256 + SendBuf[3];
                        SendBuf[datalen + 4]   = 0;             // 校验字节，如果填充出错，可能造成灾难后果
                        for( i = 1; i < ( datalen + 4 ); i++ )  //填充校验字节
                        {
                                SendBuf[datalen + 4] ^= (SendBuf[i]);
                        }

                        SendBuf[datalen + 5] = 0xca;            //帧尾

                        for( i = 0; i < RetryCount + 1; i++ )
                        {
                                                                    //	PurgeComm(m_hcom,PURGE_TXCLEAR|PURGE_RXCLEAR); //清空缓冲区
                                tcflush( m_hcom, TCIOFLUSH );       //清空缓冲区
                                bWriteStat = WriteFile( m_hcom, SendBuf, datalen + 6, &dwCount, NULL );

                                if( !bWriteStat )
                                {
                                        return ERR_COMERR;              //写串口失败
                                }

                                time1 = timeout + GetTickCount( );
                                sched_yield( );

                                len	   = 1;                         //帧头字节数
                                addr   = buf;

                                while( len )
                                {
                                        bReadStat = ReadFile( m_hcom, addr, len, &dwCount, NULL );

                                        if( !bReadStat )
                                        {
                                                return ERR_COMERR; //读串口失败
                                        }

                                        len	  -= dwCount;
                                        addr  += dwCount;

                                        if( len > 0 )
                                        {
                                                if( GetTickCount( ) > time1 )
                                                {
                                                        goto timeout5;      //超时
                                                }else if( !dwCount )
                                                {
                                                                            //Sleep(10);
                                                        sched_yield( );
                                                }
                                        }else if( 0x5a != buf[0] )  //未读到帧头
                                        {
                                                len	   = 1;
                                                addr   = buf;
                                        }
                                }

                                len = 3;
                                while( len )                //读命令类型和数据域长度
                                {
                                        bReadStat = ReadFile( m_hcom, addr, len, &dwCount, NULL );

                                        if( !bReadStat )
                                        {
                                                return ERR_COMERR;  //读串口失败
                                        }

                                        len	  -= dwCount;
                                        addr  += dwCount;

                                        if( len > 0 )
                                        {
                                                if( GetTickCount( ) > time1 )
                                                {
                                                        goto timeout5;  //超时
                                                }else if( !dwCount )
                                                {
                                                                        //Sleep(10);
                                                        sched_yield( );
                                                }
                                        }
                                }

                                if( buf[1] != SendBuf[1] )
                                {
                                        return ERR_INVALID;     //无效的返回
                                }

                                len = buf[2] * 256 + buf[3] + 2;

                                while( len )                //接收后续数据
                                {
                                        bReadStat = ReadFile( m_hcom, addr, len, &dwCount, NULL );

                                        if( !bReadStat )
                                        {
                                                return ERR_COMERR;  //读串口失败
                                        }

                                        len	  -= dwCount;
                                        addr  += dwCount;

                                        if( len > 0 )
                                        {
                                                if( GetTickCount( ) > time1 )
                                                {
                                                        goto timeout5;  //超时
                                                }else if( !dwCount )
                                                {
                                                                        //Sleep(10);
                                                        sched_yield( );
                                                }
                                        }
                                }

                                len	   = buf[2] * 256 + buf[3];
                                bcc	   = 0;                 //异或校验
                                for( i = 0; i < len + 3; i++ )
                                {
                                        bcc ^= buf[i + 1];
                                }

                                if( buf[len + 1 + 3] == bcc )
                                {
                                        return len + 6;     //返回正确
                                }
timeout5:
                                ;
                        }
                        return ERR_OVERRETRY;       //超过重试次数
                        break;

                default:
                        return ERR_INVALIDREADER;   //不支持的读卡器
                        break;
        }
}


////考虑怎样支持s70? 扇区结构采用逻辑扇区结构（每扇区3个数据块），调用block_read()、block_write()时转换为实际扇区结构
////是否需要带密钥认证自动进行处理，加快访问速度

//// 注: 仅支持 IC

int block_read( unsigned int cardNo, int sectorno, signed char blockno,
                                                                unsigned char *data, int keymode, unsigned char *key )
{
    // surport cpu
    unsigned char	tmpBuf[128];	//20120713
    unsigned char	desData[8];
    unsigned int	random = 0; 	//获取随机数
    // end

    int RetryCount;  //20140524 QSIM

    if( ( NULL == data ) || ( sectorno > ( 15 + 16 + 8 ) ) )
    {
        return ERR_PARAM;
    }

    if( sectorno < 32 ) //前2KB
    {
        if( blockno > 3 )
        {
            return ERR_PARAM;
        }
    }else
    {
        if( blockno > 15 )
        {
            return ERR_PARAM;
        }
    }

    if( statInfo.accessMsec + statInfo.authDelayMsec < GetTickCount( ) )
    {
        statInfo.authSectorNo = -1; //认证扇区超时

        statInfo.currDF	   = -1;    //20121015 cpu
        statInfo.currBF	   = -1;
        statInfo.antiStat  = 0;
    }

    unsigned char	SendBuf[21];
    unsigned char	RecvBuf[265];
    int				len, ret;

    switch( statInfo.cardType )
	{
        case CARD_IC:
        case CARD_S70:
            RetryCount = 3;  //20140524 QSIM
            while (1)
        	{
                RetryCount--;
                if( statInfo.card1 == 0 )                                                                           //4字节卡号 20120920
                {
                    SendBuf[0] = 0xaa;                                                                              //帧头
                    SendBuf[1] = 0x01;                                                                              //设备地址
                    SendBuf[2] = 0x08;                                                                              //数据长度
                    SendBuf[3] = 0x82;                                                                              //指令字
                    SendBuf[4] = 0x01;                                                                              //数据包数量
                    UIntToHex( statInfo.card2, SendBuf + 5, 4, 1 );                                        //不再支持传入卡号参数 20120920 支持7字节卡号
                    SendBuf[9]	   = sectorno;                                                                      //扇区号
                    SendBuf[10]	   = blockno;                                                                       //块号
                    if( key )
                    {
                        if( ( statInfo.authSectorNo != sectorno ) || ( statInfo.authSectorKeyType != keymode ) )    //调试20110719
                        {
                            SendBuf[2]	   = 0x0f;                                                                  //数据长度
                            SendBuf[3]	   = 0x80;                                                                  //指令字
                            SendBuf[11]	   = keymode;                                                               //密钥类型
                            memcpy( SendBuf + 12, key, 6 );                                                         //填充密钥
                        }
                    }
                }
                else //7字节卡号 20120920
                {
                    SendBuf[0] = 0xaa;                                                                              //帧头
                    SendBuf[1] = 0x01;                                                                              //设备地址
                    SendBuf[2] = 0x08 + 3;                                                                          //数据长度
                    SendBuf[3] = 0x82;                                                                              //指令字
                    SendBuf[4] = 0x01;                                                                              //数据包数量
                    UIntToHex( statInfo.card1, SendBuf + 5, 3, 1 );                                        //不再支持传入卡号参数 20120719 支持7字节卡号
                    UIntToHex( statInfo.card2, SendBuf + 5 + 3, 4, 1 );                                    //不再支持传入卡号参数 20120719 支持7字节卡号
                    SendBuf[9 + 3]	   = sectorno;                                                                  //扇区号
                    SendBuf[10 + 3]	   = blockno;                                                                   //块号
                    if( key )
                    {
                        if( ( statInfo.authSectorNo != sectorno ) || ( statInfo.authSectorKeyType != keymode ) )    //调试20110719
                        {
                            SendBuf[2]		   = 0x0f + 3;                                                          //数据长度
                            SendBuf[3]		   = 0x80;                                                              //指令字
                            SendBuf[11 + 3]	   = keymode;                                                           //密钥类型
                            memcpy( SendBuf + 12 + 3, key, 6 );                                                     //填充密钥
                        }
                    }
                }

                len = card_data_sr( statInfo.comfd, SendBuf, RecvBuf, 0, 200 );
                if( ( len == 24 ) && ( RecvBuf[3] == 0x80 ) && ( RecvBuf[4] == 0x01 ) )
                {
                    memcpy( data, RecvBuf + 5, 16 );
                    statInfo.authSectorKeyType = keymode;           //20110711
                    statInfo.authSectorNo	   = sectorno;
                    statInfo.accessMsec		   = GetTickCount( );   //更新访问时间
                    return ERR_OK;
                }else if( ( len == 9 ) && ( RecvBuf[3] == 0x00 ) && ( RecvBuf[4] == 0x01 ) )
                {
                    switch( RecvBuf[5] )
                    {
                        case RET_SUCCESS:                           //错误码：成功
                            return ERR_INVALID;                     //无效的返回
                            break;
                        case RET_OPERR:                             //指令操作失败
                            if (RetryCount == 0)  //20140505
                            {
                                return ERR_OPERR;
                            }
                            statInfo.authSectorNo = -1;	// 20140521
                            break;
                        case RET_CHECKERR:                          //校验错误
                            return ERR_OPERR;
                            break;
                        case RET_INVALID:                           //不能识别的指令
                            return ERR_OPERR;
                            break;
                        case RET_NOTCONDITION:                      //指令无法执行
                            return ERR_OPERR;
                            break;
                        default:
                            return ERR_INVALID;                     //无效的返回
                    }
                }
				else if( len < 0 )
                {
                    return len;
                }else
                {
                    return ERR_INVALID;
                }
        	}
            break;

        case CARD_UIM:
            if( ( statInfo.authSectorNo != sectorno ) || ( statInfo.authSectorKeyType != keymode )
                                            || ( statInfo.accessMsec + statInfo.authDelayMsec < GetTickCount( ) ) )
            {
                ret = uimAuth( sectorno, keymode, key ); //认证
                if( ret != ERR_OK )
                {
                    return ret;
                }
                statInfo.authSectorKeyType = keymode;
                statInfo.authSectorNo	   = sectorno;
            }
            ret = uimReadBlock( blockno, data );
            return ret;

        case CARD_USIM:                                     //20120704
            if( ( statInfo.authSectorNo != sectorno ) || ( statInfo.authSectorKeyType != keymode )
                                    || ( statInfo.accessMsec + statInfo.authDelayMsec < GetTickCount( ) ) )
            {
                ret = yooAuth( sectorno, keymode, key );    //认证
                if( ret != ERR_OK )
                {
                    return ret;
                }
                statInfo.authSectorKeyType = keymode;
                statInfo.authSectorNo	   = sectorno;
            }
            return yooReadBlock( blockno, data );
            break;

        case CARD_CPU:
            if( statInfo.currDF != statInfo.cpuAdfFileID )	//非WEDS应用目录 20121217
            {
                statInfo.currBF    = -1;
                statInfo.antiStat  = 0;

                if( statInfo.currDF != 0x3F00 ) 			//非卡片根目录
                {
                    ret = selectFile( (unsigned short)( 0x3f00 ), tmpBuf, 128 );
                    if( ret < ERR_OK )
                    {
                        return ret;
                    }
                }

                if (statInfo.parentDFfileID != 0x3f00)	//20130607 add
                {
                    ret = selectFile(statInfo.parentDFfileID, tmpBuf, 128);
                    if (ret < ERR_OK)
                    {
                        return ret;
                    }
                }

                ret = selectFile( statInfo.cpuAdfFileID, tmpBuf, 128 ); 																	//选择WEDS1App应用
                if( ret < ERR_OK )
                {
                    return ret;
                }

                                                                                                                                                                                                                                                                        //	statInfo.currDF = 0xaf01;
                statInfo.currDF = statInfo.cpuAdfFileID;
            }
            if( sectorno < 2 )
            {
                if( statInfo.issuReadLimit )	// 发行区是否认证																							 //需要认证 20121213
                {
                    if( statInfo.antiStat < statInfo.issuSectorAcceLimit[0][0] || statInfo.antiStat > statInfo.issuSectorAcceLimit[0][1] )	//安全状态不满足读此文件
                    {
                        ret = getChallenge( &random );
                        if( ret != ERR_OK )
                        {
                            return ret;
                        }

                        memset( desData, 0, 8 );
                        UIntToHex( random, desData, 4, 1 );
                        if( keymode == 1 )																				//查询密钥
                        {
                            ret = getCpuAuthData( KEY_TYPE_ISSU_READ, desData, desData );								//20121213
                            if( ret != ERR_OK )
                            {
                                return ret;
                            }
                            ret = TFexternalAuthentication( 0, statInfo.issuSectorKeyEdition_KeyID[0][1], desData );	//0xF0;  //发行区读密钥的密钥标识（或索引）
                            if( ret != ERR_OK )
                            {
                                return ret;
                            }
                            statInfo.antiStat = statInfo.issuSectorKeyEdition_KeyID[0][2];								// = 0x01;	//发行区读密钥的后续权限
                        }
                        else //发行密钥
                        {
                            ret = getCpuAuthData( KEY_TYPE_ISSU, desData, desData );									//20121213
                            if( ret != ERR_OK )
                            {
                                return ret;
                            }
                            ret = TFexternalAuthentication( 0, statInfo.issuSectorKeyEdition_KeyID[1][1], desData );	//0xF3;  //发行区写密钥的密钥标识（或索引）
                            if( ret != ERR_OK )
                            {
                                return ret;
                            }
                            statInfo.antiStat = statInfo.issuSectorKeyEdition_KeyID[1][2];								//0x08;  //发行区写密钥的后续权限
                        }
                    }
                }

                if( statInfo.currBF != statInfo.issuSectorFileID )														//非当前文件 0x11;		//发行区短文件标识符
                {
                    if( ( ( sectorno ) * 48 + 16 * blockno + statInfo.issuSectorFileOffset ) > 255 )					//地址偏移量超过一个字节
                    {
                        ret = selectFile( statInfo.issuSectorFileID, tmpBuf, 128 ); 									//选择该文件
                        if( ret != ERR_OK )
                        {
                            return ret;
                        }
                    }
                }

                ret = readBinary( statInfo.issuSectorFileID, sectorno * 48 + 16 * blockno + statInfo.issuSectorFileOffset, 16, data );	//0x11; 	 //发行区短文件标识符
                if( ret == ERR_OK )
                {
                    statInfo.currBF 	   = statInfo.issuSectorFileID; 																//0x11;
                    statInfo.accessMsec    = GetTickCount( );																			//更新访问时间
                }else
                {
                    statInfo.currBF = -1;
                }
                return ret;
            }
            else
            {
                if( statInfo.antiStat < statInfo.dataSectorAcceLimit[0][0] || statInfo.antiStat > statInfo.dataSectorAcceLimit[0][1] ) //安全状态不满足读此文件
                {
                    ret = getChallenge( &random );
                    if( ret != ERR_OK )
                    {
                        return ret;
                    }

                    memset( desData, 0, 8 );
                    UIntToHex( random, desData, 4, 1 );

                    if( keymode == 1 )																				//查询密钥
                    {
                        ret = getCpuAuthData( KEY_TYPE_REQ, desData, desData ); 									//20121213
                        if( ret != ERR_OK )
                        {
                            return ret;
                        }
                        ret = TFexternalAuthentication( 0, statInfo.dataSectorKeyEdition_KeyID[0][1], desData );	// = 0xF1;	//数据区读密钥的密钥标识（或索引）
                        if( ret != ERR_OK )
                        {
                            return ret;
                        }
                        statInfo.antiStat = statInfo.dataSectorKeyEdition_KeyID[0][2];								// = 0x02;	//数据区读密钥的后续权限
                    }
                    else
                    {
                        ret = getCpuAuthData( KEY_TYPE_DEAL, desData, desData );									//20121213
                        if( ret != ERR_OK )
                        {
                            return ret;
                        }
                        ret = TFexternalAuthentication( 0, statInfo.dataSectorKeyEdition_KeyID[1][1], desData );	//0xF2;  //数据区写密钥的密钥标识（或索引）
                        if( ret != ERR_OK )
                        {
                            return ret;
                        }
                        statInfo.antiStat = statInfo.dataSectorKeyEdition_KeyID[1][2];								// = 0x04;	//数据区写密钥的后续权限
                    }
                }
                                                                                                                                                                                                                            //if (statInfo.currBF != (0x11+1))	//非当前文件
                if( statInfo.currBF != statInfo.dataSectorFileID )													//非当前文件 0x12;		//数据区短文件标识符
                {
                    if( ( ( sectorno - 2 ) * 48 + 16 * blockno + statInfo.dataSectorFileOffset ) > 255 )			//地址偏移量超过一个字节
                    {
                        ret = selectFile( statInfo.dataSectorFileID, tmpBuf, 128 ); 								//选择该文件
                        if( ret != ERR_OK )
                        {
                            return ret;
                        }
                    }
                }
                ret = readBinary( statInfo.dataSectorFileID, ( sectorno - 2 ) * 48 + 16 * blockno + statInfo.dataSectorFileOffset, 16, data );
                if( ret == ERR_OK )
                {
                    statInfo.currBF 	   = statInfo.dataSectorFileID;
                    statInfo.accessMsec    = GetTickCount( );	//更新访问时间
                }else
                {
                    statInfo.currBF = -1;
                }

                return ret;
            }
            break;

        default:
            break;
	}

    return ERR_INVALIDCARDTYPE;
}

// cardNo
// sectorno
// blockno,
// data
// keymode
// key
// checkflag

int block_write( unsigned int cardNo, int sectorno, signed char blockno,
                                        unsigned char *data, int keymode, unsigned char *key, int checkflag )
{
    // surport cpu
    unsigned char	tmpBuf[128];	//20120713
    unsigned char	desData[8];
    unsigned int	random = 0; 	//获取随机数
    // end

    int RetryCount;  //20140524

    if( statInfo.readerType == WEDS_READER )    //早期读卡器/卡头存在BUG 20120621
    {
        statInfo.authSectorNo = -1;             //test 20110726
    }

    if( ( NULL == data ) || ( sectorno > ( 15 + 16 + 8 ) ) )
    {
        return ERR_PARAM;
    }

    if( sectorno < 32 ) //前2KB
    {
        if( blockno > 3 )
        {
            return ERR_PARAM;
        }
    }else
    {
        if( blockno > 15 )
        {
            return ERR_PARAM;
        }
    }

    if( statInfo.accessMsec + statInfo.authDelayMsec < GetTickCount( ) )
    {
        statInfo.authSectorNo = -1; //认证扇区超时

        statInfo.currDF	   = -1;    //20121015 cpu
        statInfo.currBF	   = -1;
        statInfo.antiStat  = 0;
    }

    unsigned char	SendBuf[37];
    unsigned char	RecvBuf[265];
    unsigned char	tmpbuf[16];
    int	len, ret;

    switch( statInfo.cardType )
	{
        case CARD_IC:
        case CARD_S70:
            if( statInfo.card1 == 0 )                                                                               //4字节卡号 20120920
            {
                SendBuf[0] = 0xaa;                                                                                  //帧头
                SendBuf[1] = 0x01;
                SendBuf[2] = 0x18;                                                                                  //数据长度
                SendBuf[3] = 0x83;                                                                                  //指令字
                SendBuf[4] = 0x01;                                                                                  //数据包数量
                UIntToHex( statInfo.card2, SendBuf + 5, 4, 1 );                                            //20120920 不再支持参数传入的卡号

                SendBuf[9]	   = sectorno;                                                                          //扇区号
                SendBuf[10]	   = blockno;                                                                           //块号
                if( key && ( ( statInfo.authSectorNo != sectorno ) || ( statInfo.authSectorKeyType != keymode ) ) ) //调试20110719
                {
                    SendBuf[2]	   = 0x1f;                                                                          //数据长度
                    SendBuf[3]	   = 0x81;                                                                          //指令字
                    SendBuf[11]	   = keymode;                                                                       //密钥类型
                    memcpy( SendBuf + 12, key, 6 );                                                                 //填充密钥
                    memcpy( SendBuf + 18, data, 16 );                                                               //填充块数据
                }else
                {
                    memcpy( SendBuf + 11, data, 16 );                                                               //填充块数据
                }
            }
            else //7字节卡号
            {
                SendBuf[0] = 0xaa;                                                                                  //帧头
                SendBuf[1] = 0x01;                                                                                  //设备地址

                SendBuf[2] = 0x18 + 3;                                                                              //数据长度
                SendBuf[3] = 0x83;                                                                                  //指令字

                SendBuf[4] = 0x01;                                                                                  //数据包数量

                UIntToHex( statInfo.card1, SendBuf + 5, 3, 1 );                                            //20120719 不再支持参数传入的卡号
                UIntToHex( statInfo.card2, SendBuf + 5 + 3, 4, 1 );                                        //20120719 不再支持参数传入的卡号

                SendBuf[9 + 3]	   = sectorno;                                                                      //扇区号
                SendBuf[10 + 3]	   = blockno;                                                                       //块号
                if( key && ( ( statInfo.authSectorNo != sectorno ) || ( statInfo.authSectorKeyType != keymode ) ) ) //调试20110719
                {
                    SendBuf[2]		   = 0x1f + 3;                                                                  //数据长度
                    SendBuf[3]		   = 0x81;                                                                      //指令字
                    SendBuf[11 + 3]	   = keymode;                                                                   //密钥类型
                    memcpy( SendBuf + 12 + 3, key, 6 );                                                             //填充密钥
                    memcpy( SendBuf + 18 + 3, data, 16 );                                                           //填充块数据
                }else
                {
                    memcpy( SendBuf + 11 + 3, data, 16 );                                                           //填充块数据
                }
            }

            RetryCount = 3;  //20140505 增加重试
            while (1)//20140505
            {
                RetryCount--; //20140505
                len = card_data_sr( statInfo.comfd, SendBuf, RecvBuf, 0, 200 );
                if( ( len == 9 ) && ( RecvBuf[3] == 0x00 ) && ( RecvBuf[4] == 0x01 ) )
                {
                    switch( RecvBuf[5] )
                	{
                        case RET_SUCCESS:
                            statInfo.authSectorKeyType = keymode;           //20110711
                            statInfo.authSectorNo	   = sectorno;
                            statInfo.accessMsec		   = GetTickCount( );   //更新访问时间
                            ret = ERR_OK;
                            if( checkflag )
                            {
                                if( cardNo )
                                {
                                    ret = block_read( cardNo, sectorno, blockno, tmpbuf, keymode, key );
                                }else
                                {
                                    ret = block_read( statInfo.card2, sectorno, blockno, tmpbuf, keymode, key );
                                }
                                if( ERR_OK == ret )
                                {
                                    if( !memcmp( data, tmpbuf, 16 ) )
                                    {
                                        return ERR_OK;
                                    }else
                                    {
// 2015.8.6 debug
//LeeLog(NULL, data, 16 , 's');
//LeeLog(NULL, tmpbuf, 16 , 'r');

                                        return ERR_FALSE;
                                    }
                                }
                            }
                            return ret;
                            break;

                        case RET_OPERR:         //指令操作失败
                            if (RetryCount == 0)  //20140505
                            {
                                return ERR_OPERR;
                            }
                            break;
                        case RET_CHECKERR:      //校验错误
                            return ERR_OPERR;
                            break;
                        case RET_INVALID:       //不能识别的指令
                            return ERR_OPERR;
                            break;
                        case RET_NOTCONDITION:  //指令无法执行
                            return ERR_OPERR;
                            break;
                        default:
                            return ERR_INVALID; //无效的返回
                	}
                }
				else if( len < 0 )
                {
                    return len;
                }else
                {
                    return ERR_INVALID;
                }
            }
            break;

        case CARD_UIM:
            if( ( statInfo.authSectorNo != sectorno ) || ( statInfo.authSectorKeyType != keymode )
                            || ( statInfo.accessMsec + statInfo.authDelayMsec < GetTickCount( ) ) )
            {
                ret = uimAuth( sectorno, keymode, key ); //认证
                if( ret != ERR_OK )
                {
                    return ret;
                }
                statInfo.authSectorKeyType = keymode;
                statInfo.authSectorNo	   = sectorno;
            }

            ret = uimWriteBlock( blockno, data );
            if( ( ret == ERR_OK ) && ( checkflag ) )
            {
                ret = uimReadBlock( blockno, RecvBuf );
                if( ret == ERR_OK )
                {
                    ret = memcmp( data, RecvBuf, 16 );
                    if( ret )
                    {
                        return ERR_FALSE;
                    }
                }
            }
            return ret;
            break;

        case CARD_USIM:                                     //20120704    注: 可能需要调整
            if( ( statInfo.authSectorNo != sectorno ) || ( statInfo.authSectorKeyType != keymode )
                                                    || ( statInfo.accessMsec + statInfo.authDelayMsec < GetTickCount( ) ) )
            {
                ret = yooAuth( sectorno, keymode, key );    //认证
                if( ret != ERR_OK )
                {
                    return ret;
                }
                statInfo.authSectorKeyType = keymode;
                statInfo.authSectorNo	   = sectorno;
            }

            ret = yooWriteBlock( blockno, data );
            if( ( ret == ERR_OK ) && ( checkflag ) )
            {
                ret = yooReadBlock( blockno, RecvBuf );
                if( ret == ERR_OK )
                {
                    ret = memcmp( data, RecvBuf, 16 );
                    if( ret )
                    {
                        return ERR_FALSE;
                    }
                }
            }
            return ret;
            break;

        case CARD_CPU:                                      //20121015
            if( statInfo.currDF != statInfo.cpuAdfFileID )  //非WEDS应用目录
            {
                statInfo.currBF	   = -1;
                statInfo.antiStat  = 0;

                if( statInfo.currDF != 0x3F00 )             //非卡片根目录
                {
                    ret = selectFile( (unsigned short)( 0x3f00 ), tmpBuf, 128 );
                    if( ret < ERR_OK )
                    {
                        return ret;
                    }
                }

                if (statInfo.parentDFfileID != 0x3f00)  //20130607 add
                {
                    ret = selectFile(statInfo.parentDFfileID, tmpBuf, 128);
                    if (ret < ERR_OK)
                    {
                        return ret;
                    }
                }

                ret = selectFile( statInfo.cpuAdfFileID, tmpBuf, 128 ); //选择WEDS1App应用
                if( ret < ERR_OK )
                {
                    return ret;
                }

                statInfo.currDF = statInfo.cpuAdfFileID;
            }

            if( sectorno < 2 )
            {                                                                                                                                            //	if( statInfo.antiStat < 8 ) //安全状态不满足写此文件
                if( statInfo.antiStat < statInfo.issuSectorAcceLimit[1][0] || statInfo.antiStat < statInfo.issuSectorAcceLimit[1][1] )  //安全状态不满足写此文件
                {
                    ret = getChallenge( &random );
                    if( ret != ERR_OK )
                    {
                        return ret;
                    }

                    memset( desData, 0, 8 );
                    UIntToHex( random, desData, 4, 1 );
                    ret = getCpuAuthData( KEY_TYPE_ISSU, desData, desData );    //20121213
                    if( ret != ERR_OK )
                    {
                        return ret;
                    }
                    ret = TFexternalAuthentication( 0, statInfo.issuSectorKeyEdition_KeyID[1][1], desData );

                    if( ret != ERR_OK )
                    {
                        return ret;
                    }

                    statInfo.antiStat = statInfo.issuSectorKeyEdition_KeyID[1][2];                      // = 0x08;  //发行区写密钥的后续权限
                }

                if( statInfo.currBF != statInfo.issuSectorFileID )                                      //非当前文件 0x11;      //发行区短文件标识符
                {
                    if( ( ( sectorno ) * 48 + 16 * blockno + statInfo.issuSectorFileOffset ) > 255 )    //地址偏移量超过一个字节
                    {
                        ret = selectFile( statInfo.issuSectorFileID, tmpBuf, 128 );                     //选择该文件
                        if( ret != ERR_OK )
                        {
                            return ret;
                        }
                    }
                }

                ret = updateBinary( statInfo.issuSectorFileID, sectorno * 48 + 16 * blockno + statInfo.issuSectorFileOffset, data, 16 );
                if( ( ret == ERR_OK ) && ( checkflag ) )
                {
                    ret = readBinary( statInfo.issuSectorFileID, sectorno * 48 + 16 * blockno + statInfo.issuSectorFileOffset, 16, RecvBuf );
                    if( ret == ERR_OK )
                    {
                        ret = memcmp( data, RecvBuf, 16 );
                        if( ret )
                        {
                            return ERR_FALSE;
                        }

                        statInfo.currBF = statInfo.issuSectorFileID;
                    }
                }
            }
            else
            {				                                                                                                                        //	if (statInfo.antiStat < 4) //安全状态不满足写此文件				                                                                                                                        //	if( ( statInfo.antiStat & 0x04 ) == 0 ) //安全状态不满足写此文件 20121015
                if( statInfo.antiStat < statInfo.dataSectorAcceLimit[1][0] || statInfo.antiStat > statInfo.dataSectorAcceLimit[1][1] )  //安全状态不满足写此文件 20121015
                {
                    ret = getChallenge( &random );
                    if( ret != ERR_OK )
                    {
                        return ret;
                    }

                    memset( desData, 0, 8 );
                    UIntToHex( random, desData, 4, 1 );

                    ret = getCpuAuthData( KEY_TYPE_DEAL, desData, desData );                                    //20121213
                    if( ret != ERR_OK )
                    {
                        return ret;
                    }

                                                                                                                //	ret = TFexternalAuthentication( 0, 0xF2, desData ); //此参数歪打正着，同样适用于FMCOS2.0 20120801
                    ret = TFexternalAuthentication( 0, statInfo.dataSectorKeyEdition_KeyID[1][1], desData );    //此参数歪打正着，同样适用于FMCOS2.0 20120801
                    if( ret != ERR_OK )
                    {
                        return ret;
                    }

                    statInfo.antiStat = statInfo.dataSectorKeyEdition_KeyID[1][2];                              // = 0x04;  //数据区写密钥的后续权限
                }

                if( statInfo.currBF != statInfo.dataSectorFileID )                                              //非当前文件
                {
                                                                                                                    //	if( ( ( sectorno - 2 ) * 48 + 16 * blockno ) > 255 )    //地址偏移量超过一个字节
                    if( ( ( sectorno - 2 ) * 48 + 16 * blockno + statInfo.dataSectorFileOffset ) > 255 )        //地址偏移量超过一个字节
                    {
                        ret = selectFile( statInfo.dataSectorFileID, tmpBuf, 128 );                             //选择该文件
                        if( ret != ERR_OK )
                        {
                            return ret;
                        }
                    }
                }

                ret = updateBinary( statInfo.dataSectorFileID, ( sectorno - 2 ) * 48 + 16 * blockno + statInfo.dataSectorFileOffset, data, 16 );
                if( ( ret == ERR_OK ) && ( checkflag ) )
                {
                    ret = readBinary( statInfo.dataSectorFileID, ( sectorno - 2 ) * 48 + 16 * blockno + statInfo.dataSectorFileOffset, 16, RecvBuf );
                    if( ret == ERR_OK )
                    {
                        ret = memcmp( data, RecvBuf, 16 );
                        if( ret )
                        {
                            return ERR_FALSE;
                        }

                        statInfo.currBF = statInfo.dataSectorFileID;
                    }
                }
            }

            if( ret == ERR_OK )
            {
                statInfo.accessMsec = GetTickCount( ); //更新访问时间
            }

            return ret;
            break;
        default:
                return ERR_INVALIDCARDTYPE;
                break;
	}
}


//函数功能:  连续写数据块
//参数:      keymode: 密钥类型(A密钥或B密钥),
//           key: 密钥
//           firstsector: 起始扇区
//           firstblock:  起始扇区的开始块号
//           len:  需要写的数据长度
//           data: 数据缓冲区地址
//返回:      大于0为写到的数据长度，以字节计，其它：按错误码解释
int writeDataBlock2( int keymode, unsigned char *key, int firstSector, int firstBlock,
                                                int len, unsigned char *data )
{
        if( ( firstSector > 71 ) || ( firstBlock > 2 ) )
        {
                return ERR_PARAM;
        }

        if( ( firstBlock + len / 16 + ( len % 16 ) ? 1 : 0 ) > ( 72 - firstSector ) * 3 )   //超过S70卡容量
        {
                return ERR_PARAM;
        }

        int				offset = 0;
        unsigned char	buf[16];
        unsigned char	* addrkey = key;
        int				ret;

#if 1 // 20150806 发现有写卡扇区数据错误的情况，故，此处不能屏蔽,必须保留
        int				checkFlag = 1;
        if( statInfo.cardType == CARD_UIM )
        {
                checkFlag = 0;
        }
#endif

        while( len )
        {
                memset( buf, 0, 16 );

                if( len > 15 )
                {
                        memcpy( buf, data + offset, 16 );
                }else
                {
                        memcpy( buf, data + offset, len );
                }

                ret = block_write( statInfo.card2, phySectorNo[firstSector], phyBlockNo[firstSector][firstBlock], buf, keymode, addrkey, checkFlag );
                if( ERR_OK == ret )
                {
                        if( len > 15 )
                        {
                                offset	  += 16;
                                len		  -= 16;
                        }else
                        {
                                offset	  += len;
                                len		   = 0;
                        }
                }else
                {   //printf("write card err2\n");
                        return ret;
                }

                firstBlock++;
                firstBlock %= 3;
                if( firstBlock == 0 )
                {
                        firstSector++;
                }

        }

        return ERR_OK; //20110712

}


//函数功能:  连续读数据块
//参数:      keyMode: 密钥类型(A密钥或B密钥),
//           key: 密钥
//           firstsector: 起始扇区
//           firstblock:  起始扇区的开始块号
//           len:  需要读的数据长度
//           data: 输出数据缓冲区地址
//返回:      按错误码解释

int readDataBlock2( int keyMode, unsigned char *key, int firstSector, int firstBlock,
                                                int len, unsigned char *data )
{
        if( ( firstSector > 71 ) || ( firstBlock > 3 ) )
        {
                return ERR_PARAM;
        }

        if( ( firstBlock + len / 16 + ( len % 16 ) ? 1 : 0 ) > ( 72 - firstSector ) * 3 ) //超过S70卡容量
        {
                return ERR_PARAM;
        }

        int				offset = 0;
        unsigned char	buf[16];
        unsigned char	* addrkey = key;
        int				ret;

        while( len )
        {
                ret = block_read( statInfo.card2, phySectorNo[firstSector], phyBlockNo[firstSector][firstBlock], buf, keyMode, addrkey );
                if( ERR_OK == ret )
                {
                        if( len > 15 )
                        {
                                memcpy( data + offset, buf, 16 );
                                offset	  += 16;
                                len		  -= 16;
                        }else
                        {
                                memcpy( data + offset, buf, len );
                                offset	  += len;
                                len		   = 0;
                        }
                }else
                {
                        return ret;
                }

                firstBlock++;
                firstBlock %= 3;
                if( firstBlock == 0 )
                {
                        firstSector++;
                }
        }

        return ERR_OK; //20110712

}

//函数功能:  连续读数据块sdk接口
//参数:      comfd: 串口句柄
//			 readerType: 读头类型
//		     keyMode: 密钥类型(A密钥或B密钥),
//           key: 密钥
//           firstsector: 起始扇区
//           firstblock:  起始扇区的开始块号
//           len:  需要读的数据长度(必须为16的倍数),最大4个块，16*4个字节
//           data: 输出数据缓冲区地址
//返回:      按错误码解释

int sdkReadIcData( int comfd, unsigned char readerType, int keymode, unsigned char *key,
                                                        int firstSector, int firstBlock, 	int len, unsigned char *data )
{
        statInfo.comfd = comfd;
        statInfo.readerType = readerType;
        return readDataBlock2( keymode, key, firstSector, firstBlock, len, data );
}

//函数功能:  连续写数据块sdk接口
//参数:     comfd: 串口句柄
//			readerType: 读头类型
//			keymode: 密钥类型(A密钥或B密钥),
//           key: 密钥
//           firstsector: 起始扇区
//           firstblock:  起始扇区的开始块号
//           len:  需要写的数据长度(必须为16的倍数),最大3个块，16*3个字节
//           data: 数据缓冲区地址
//返回:      按错误码解释

int sdkWriteIcData( int comfd, unsigned char readerType, int keymode, unsigned char *key,
                                                        int firstSector, int firstBlock, 	int len, unsigned char *data )
{
        statInfo.comfd = comfd;
        statInfo.readerType = readerType;
        return writeDataBlock2( keymode, key, firstSector, firstBlock, len, data );
}

// 修改扇区密钥sdk接口
// comfd: 串口句柄
// readerType: 读头类型
// oldKeyMode: 修改前的操作密钥类型
// oldKey: 修改前的操作密钥6
// sectorNo: 目标扇区号
// AKey: A密钥6
// BKey: B密钥6
// ctrlWord: 控制字4

int sdkModiSectorKey( int comfd, unsigned char readerType, int oldKeyMode, unsigned char *oldKey,
                                                int sectorNo, unsigned char *AKey, unsigned char *BKey, unsigned char *ctrlWord)
{
        statInfo.comfd = comfd;
        statInfo.readerType = readerType;
        return modiSectorKey( oldKeyMode, oldKey, sectorNo, AKey, BKey, ctrlWord );
}


// fun: 被动寻卡
// reader: 卡头类型
// phycard: 读取到的物理卡号
int card_passive( int reader, char *phycard )
{
        int				len, ret;
        int				i = 0;
        unsigned char	SendBuf[] = { 0xaa, 0x01, 0x03, 0x61, 0x01, 0x94, 0x00, 0x00, 0x00 };   //WEDS_READER IC
        unsigned char RecvBuf[512];                                                             //20120208

        statInfo.cardType  = 0;                                                                 //卡片类型
        statInfo.card1	   = 0;
        statInfo.card2	   = 0;

        statInfo.authSectorKeyType = -1;                                                        //20110711
        statInfo.authSectorNo	   = -1;
		statInfo.readerType = reader;

        switch( reader )
        {
                case WEDS_READER:
                case WEDS_READER_CPU:
                        len = card_data_r( statInfo.comfd, RecvBuf );  // 被动寻卡
                        if( ( len == 12 ) && ( RecvBuf[4] == 0x01 ) &&
                                ( ( RecvBuf[3] == CARD_IC ) || ( RecvBuf[3] == CARD_CPU ) || ( RecvBuf[3] == CARD_S70 ) ) )
                        {
                                statInfo.cardType = RecvBuf[3];         //卡片类型
                                if( phycard )
                                {
                                        memset( phycard, 0, 32 );  //sizeof(CARDDATAINFO.TCardSNR));
                                        HexToTHex( RecvBuf + 5, phycard, 4, 0 );
                                }

                                statInfo.card2 = HexToUInt( RecvBuf + 5, 4, 1 );
                                if( reader == WEDS_READER_CPU )     // cpu 再做主动寻卡
                                {
redo:
                                        ret = card_get_cpu( phycard, NULL );
                                        if( ret != CARD_CPU )
                                        {
                                                if(++i < 3)	// 20150624
                                                {
//sprintf(llog, "[redo get cpu %d]", i);
//LeeLog(llog, NULL, 0, 0);
                                                        goto redo;
                                                }
//sprintf(llog, "[redo not get cpu %d]", i);
//LeeLog(llog, NULL, 0, 0);
                                                return ret;
                                        }
                                }

                                return statInfo.cardType;                                                                                                                       //94H（IC卡）95H（CPU卡） 9AH (s70卡)
                        }
                        else if( ( len == 15 ) && ( RecvBuf[4] == 0x01 ) &&
                                ( ( RecvBuf[3] == CARD_IC ) || ( RecvBuf[3] == CARD_CPU ) || ( RecvBuf[3] == CARD_S70 ) ) )    //20120920支持7字节卡号
                        {
                                statInfo.cardType = RecvBuf[3];                                                                                                                 //卡片类型
                                if( phycard )
                                {
                                        memset( phycard, 0, 32 );                                                                                                          //sizeof(CARDDATAINFO.TCardSNR));
                                        HexToTHex( RecvBuf + 5, phycard, 7, 0 );
                                }

                                statInfo.card1 = HexToUInt( RecvBuf + 5, 3, 1 );
                                statInfo.card2 = HexToUInt( RecvBuf + 8, 4, 1 );
                                if( reader == WEDS_READER_CPU )                                                                                                             // cpu 再做主动寻卡
                                {
                                        ret = card_get_cpu( phycard, NULL );
                                        if( ret != CARD_CPU )
                                        {
                                                return ret;
                                        }
                                }
                                //statInfo.currDF = 0x3F00;  //20120713
                                return statInfo.cardType;   //94H（IC卡）95H（CPU卡）
                        }else if( len > 0 )
                        {
                                return ERR_INVALID;         //无效返回
                        }else
                        {				                            //return ERR_NOCARD; //无卡不返回数据
                                return len;
                        }

                        break;

                default:
                        return ERR_PARAM; //不支持的卡头类型
                        break;

        }
}

// 支持weds卡头主动寻卡
// reader: 卡头类型
// phycard: 读取到的物理卡号

int card_active( int reader, char *phycard )
{
        int				len, ret;
        unsigned char	SendBuf[] = { 0xaa, 0x01, 0x03, 0x61, 0x01, 0x94, 0x00, 0x00, 0x00 };   //WEDS_READER IC
        unsigned char RecvBuf[512];                                                             //20120208
        statInfo.cardType  = 0;                                                                 //卡片类型
        statInfo.card1	   = 0;
        statInfo.card2	   = 0;
        statInfo.authSectorKeyType = -1;                                                        //20110711
        statInfo.authSectorNo	   = -1;
		statInfo.readerType = reader;

        switch( reader )
        {
            case WEDS_READER:
                    SendBuf[5] = 0x94;
                    len = card_data_sr( statInfo.comfd, SendBuf, RecvBuf, 0, 200 );
                    if( ( len == 13 ) && ( RecvBuf[3] == 0x61 ) && ( RecvBuf[4] == 0x01 ) )
                    {
                            statInfo.cardType = RecvBuf[5];	// 卡片类型 S50 S70
                            if( phycard )
                            {
                                    memset( phycard, 0, 32 );
                                    HexToTHex( RecvBuf + 6, phycard, 4, 0 );
                            }
                            statInfo.card2 = HexToUInt( RecvBuf + 6, 4, 1 );
                            return statInfo.cardType;
                    }
                    else if( ( len == 16 ) && ( RecvBuf[3] == 0x61 ) && (  RecvBuf[4] == 0x01 ) )    //20120920支持7字节卡号
                    {
                            statInfo.cardType = RecvBuf[5];                                                                                                                 //卡片类型

                            if( phycard )
                            {
                                    memset( phycard, 0, 32 );                                                                                                          //sizeof(CARDDATAINFO.TCardSNR));
                                    HexToTHex( RecvBuf + 6, phycard, 7, 0 );
                            }

                            statInfo.card1 = HexToUInt( RecvBuf + 6, 3, 1 );
                            statInfo.card2 = HexToUInt( RecvBuf + 9, 4, 1 );
                            return statInfo.cardType;   //94H（IC卡）95H（CPU卡）
                    }
                    else if( len < 0 )
                    {
                            if( len == ERR_OVERRETRY )
                            {
                                    len = ERR_NOCARD;
                            }

                            return len;
                    }else
                    {
                            return ERR_INVALID;
                    }
                    break;

            case WEDS_READER_CPU:

                    ret = card_get_cpu( phycard, NULL );
                    if( ret != CARD_CPU )
                    {
                            return ret;
                    }
                    return CARD_CPU;                                                                                                                       //94H（IC卡）95H（CPU卡） 9AH (s70卡)
                    break;

            default:
                    return ERR_PARAM; //不支持的卡头类型
                    break;
        }
}



// 若为cpu卡头，则被动寻到卡后，则需要再做一次主动寻卡
// phycard: 读取到的物理卡号
// cardlen: 物理卡号字节数

int card_get_cpu( char *phycard, unsigned int *cardlen )
{
        int				len, ret;
        int i;
        unsigned char	SendBuf[] = { 0xaa, 0x01, 0x03, 0x62, 0x01, 0x95, 0x00, 0x00, 0x00 };  //WEDS_READER CPU
        unsigned char RecvBuf[512];                                                             //20120208

        statInfo.cardType  = 0;                                                                 //卡片类型
        statInfo.card1	   = 0;
        statInfo.card2	   = 0;

        statInfo.authSectorKeyType = -1;                                                        //20110711
        statInfo.authSectorNo	   = -1;

        //20120713
        statInfo.currDF	   = -1;                                                                //当前卡片目录 主目录3F00，WEDS应用目录AF01   未知：-1
        statInfo.currBF	   = -1;                                                                //当前二进制文件短文件标识符 -1：未知
        statInfo.antiStat  = 0;                                                                 //安全状态
        statInfo.cpuCodeID = 0;
        memset( statInfo.ATS, 0, 32 );

        if(phycard == NULL)
        {
                return ERR_PARAM;
        }

        len = card_data_sr( statInfo.comfd, SendBuf, RecvBuf, 0, 200 );
        //数据包：1字节卡号长度+n字节卡号+1字节复位应答信息长度+n字节复位应答信息；
        if( ( len >= 13 ) && ( RecvBuf[3] == 0x62 ) && ( RecvBuf[4] == 0x01 ) )
        {
                statInfo.cardType = CARD_CPU;   //卡片类型
                if( ( RecvBuf[5] != 4 ) && ( RecvBuf[5] != 7 ) )
                {
                        return ERR_INVALIDCARDTYPE; //不支持的卡片类型 卡号位数不支持
                }

                if(cardlen)
                {
                        *cardlen = RecvBuf[5];	// 卡号长度 20160113
                }

                if( RecvBuf[5] == 4 )
                {
                        statInfo.card2 = HexToUInt( RecvBuf + 6, 4, 1 );
                }else
                {
                        statInfo.card1 = HexToUInt( RecvBuf + 6, 3, 1 );
                        statInfo.card2 = HexToUInt( RecvBuf + 9, 4, 1 );
                }

                HexToTHex( RecvBuf + 6, (char *)phycard, RecvBuf[5], 0 );//add by liu 20130929 物理卡号使用主动寻卡后获得的物理卡号

                memcpy( statInfo.ATS, RecvBuf + 7 + RecvBuf[5], RecvBuf[6 + RecvBuf[5]] + 2 ); //含CRC

                //需要判断是否匹配COS类型
                len = 0;
                if( statInfo.ATS[1] & 0x10 )
                {
                        len++;
                }

                if( statInfo.ATS[1] & 0x20 )
                {
                        len++;
                }

                if( statInfo.ATS[1] & 0x40 )
                {
                        len++;
                }

                len = HexToUInt( statInfo.ATS + 2 + len, 3, 1 );   //计算cosID

                i = 0;
                while( statInfo.cosID[i] )                                  //i最大254
                {
                        if( statInfo.cosID[i] == (unsigned int)len )
                        {
                                statInfo.cpuCodeID = statInfo.codeID[i];
                                break;
                        }
                        i++;
                }

                statInfo.currDF = 0x3F00;   //20120713
                return statInfo.cardType;   //95H（CPU卡）
        }
        else if( ( len == 9 ) && ( RecvBuf[3] == 0x00 ) && ( RecvBuf[4] == 0x01 ) )
        {
                switch( RecvBuf[5] )
                {
                        case RET_SUCCESS:       //错误码：成功
                                return ERR_INVALID; //无效的返回
                                break;
                        case RET_OPERR:         //指令操作失败
                                return ERR_NOCARD;  //ERR_OPERR;
                                break;
                        case RET_CHECKERR:      //校验错误
                                return ERR_OPERR;
                                break;
                        case RET_INVALID:       //不能识别的指令
                                return ERR_OPERR;
                                break;
                        case RET_NOTCONDITION:  //指令无法执行
                                return ERR_OPERR;
                                break;
                        default:
                                return ERR_INVALID; //无效的返回
                }
        }else if( len < 0 )
        {
                if( len == ERR_OVERRETRY )
                {
                        len = ERR_NOCARD;
                }

                return len;
        }else
        {
                return ERR_INVALID;
        }
}

// 修改扇区密钥
// oldKeyMode: 修改前的操作密钥类型
// oldKey: 修改前的操作密钥6
// sectorNo: 目标扇区号
// AKey: A密钥6
// BKey: B密钥6
// ctrlWord: 控制字4
int modiSectorKey( int oldKeyMode, unsigned char *oldKey, int sectorNo,
                                                unsigned char *AKey, unsigned char *BKey, unsigned char *ctrlWord )
{
        if( sectorNo > 71 ) //最大逻辑扇区号
        {
                return ERR_PARAM;
        }

        if( phyBlockNo[sectorNo][3] < 0 )
        {
                return ERR_OK;
        }

        unsigned char ctlbuf[16];
        statInfo.authSectorNo = -1;

        memcpy( ctlbuf, AKey, 6 );
        memcpy( ctlbuf + 6, ctrlWord, 4 );
        memcpy( ctlbuf + 10, BKey, 6 );
//dbgShowHexData(ctlbuf,16,1,'>',"modiSectorKey");
        return block_write( statInfo.card2, phySectorNo[sectorNo], phyBlockNo[sectorNo][3], ctlbuf, oldKeyMode, oldKey, 0 );

}


//cpu卡按标识符选择文件

int selectFile( unsigned short fileID, unsigned char *buf, unsigned char len )
{
        unsigned char	cla	   = 0x00;
        unsigned char	ins	   = 0xA4;

        unsigned char	apdu[128];
        memset( apdu, 0, 128 );

        unsigned char *addr = apdu;
        *addr = cla;
        addr++;

        *addr = ins;
        addr++;

        *addr = 0;                                  //p1
        addr++;

        *addr = 0;                                  //p2
        addr++;

        *addr = 2;                                  //lc,文件标识符长度
        addr++;

        unsigned int tmpInt = fileID;
        UIntToHex( tmpInt, addr, 2, 1 );   //文件标识符
        addr += 2;

//	*addr = 0;
//	addr++;

        //APDU已准备好
        unsigned char	answerBuf[256];
        int				ret;

        switch( statInfo.readerType )
        {
                case WEDS_READER_CPU:
                        ret = wedsApduChannel2( apdu, (unsigned char)7, answerBuf, 255, 300 );
                        break;
                default:
                        return ERR_INVALIDREADER;
                        break;
        }

        if( ret == 2 )
        {
                ret = cpuErrCode( answerBuf );
                if( ret == ERR_GET_RESPONSE )
                {
                        if( len < answerBuf[1] )
                        {
                                return ERR_OVERBUF;                     //超过缓冲区
                        }

                        return getResponse( buf, answerBuf[1] );    //大于0为返回的字节数
                }
                return ret;
        }else if( ret > 2 )                                 //20111013
        {
                if( ( answerBuf[ret - 2] == 0x90 ) && ( answerBuf[ret - 1] == 0 ) )
                {
                        memcpy( buf, answerBuf, ret - 2 );
                        return ret - 2;
                }else
                {
                        return ERR_INVALID; //
                }
        }else if( ret < 0 )
        {
                return ret;
        }else
        {
                return ERR_INVALID;     //返回数据无效，无法解释
        }
}

int getResponse( unsigned char *buf, unsigned char Le )
{
        unsigned char	cla	   = 0x00;
        unsigned char	ins	   = 0xC0;

        unsigned char	apdu[128];
        memset( apdu, 0, 128 );

        unsigned char *addr = apdu;
        *addr = cla;
        addr++;

        *addr = ins;
        addr++;

        *addr = 0;  //p1
        addr++;

        *addr = 0;  //p2
        addr++;

        *addr = Le; //le
        addr++;

        //APDU已准备好
        unsigned char	answerBuf[128];
        int				ret;

        switch( statInfo.readerType )
        {
                case WEDS_READER_CPU:
                        ret = wedsApduChannel2( apdu, 5, answerBuf, 128, 300 );
                        break;
                default:
                        return ERR_INVALIDREADER;
                        break;
        }

        if( ret == Le + 2 ) //期望的返回字节数，数据、SW1、SW2
        {
                ret = cpuErrCode( answerBuf + Le );
                if( ret == ERR_OK )
                {
                        memcpy( buf, answerBuf, Le );
                        return Le;
                }
                return ret;
        }else if( ret < 0 )
        {
                return ret;
        }else
        {
                return ERR_INVALID; //返回数据无效，无法解释
        }
}


int getChallenge( unsigned int *random )
{
        unsigned char	cla	   = 0x00;
        unsigned char	ins	   = 0x84;

        unsigned char	apdu[128];
        memset( apdu, 0, 128 );

        unsigned char *addr = apdu;
        *addr = cla;
        addr++;

        *addr = ins;
        addr++;

        *addr = 0;  //p1
        addr++;

        *addr = 0;  //p2
        addr++;

        *addr = 4;  //le
        addr++;

        //APDU已准备好
        unsigned char	answerBuf[128];
        int				ret;

        switch( statInfo.readerType )
        {
                case WEDS_READER_CPU:
                        ret = wedsApduChannel2( apdu, 5, answerBuf, 128, 300 );
                        break;
                default:
                        return ERR_INVALIDREADER;
                        break;
        }

        if( ret == 6 ) //期望的返回字节数，4字节随机数、SW1、SW2
        {
                if( cpuErrCode( answerBuf + 4 ) == ERR_OK )
                {
                        *random = HexToUInt( answerBuf, 4, 1 );
                        return ERR_OK;
                }else
                {
                        return ERR_INVALID; //返回数据无效，无法解释
                }
        }else if( ret < 0 )
        {
                return ret;
        }else
        {
                return ERR_INVALID;     //返回数据无效，无法解释
        }
}


// 根据应用名，选择目录
int selectFile2( unsigned char P1, unsigned char P2, unsigned char Lc,
                                        unsigned char Le, unsigned char *ioData, unsigned int oLen )
{
        unsigned char	cla	   = 0x00;
        unsigned char	ins	   = 0xA4;

        unsigned char	lApdu = 0; // apdu len
        unsigned char	apdu[128];
        memset( apdu, 0, sizeof(apdu) );
        unsigned char *addr = apdu;
        *addr = cla;
        addr++; lApdu++;
        *addr = ins;
        addr++; lApdu++;
        *addr = P1;
        addr++; lApdu++;
        *addr = P2;
        addr++; lApdu++;

        if(Lc){
                *addr = Lc;
                addr++; lApdu++;
                memcpy(addr, ioData, Lc);
                addr += Lc; lApdu += Lc;
        }

        if(Le){
                *addr = Le;
                addr++; lApdu++;
        }

        unsigned char	answerBuf[256];
        int				ret;

        switch( statInfo.readerType )
        {
                case WEDS_READER_CPU:
                        ret = wedsApduChannel2( apdu, (unsigned char)(lApdu), answerBuf, 255, 300 );
                        break;
                default:
                        return ERR_INVALIDREADER;
                        break;
        }

        if( ret == 2 )
        {
                ret = cpuErrCode( answerBuf );
                if( ret == ERR_GET_RESPONSE )
                {
                        if( oLen < answerBuf[1] )
                        {
                                return ERR_OVERBUF;                     //超过缓冲区
                        }

                        return getResponse( ioData, answerBuf[1] );    //大于0为返回的字节数
                }
                return ret;
        }else if( ret > 2 )                                 //20111013
        {
                if( ( answerBuf[ret - 2] == 0x90 ) && ( answerBuf[ret - 1] == 0 ) )
                {
                        memcpy( ioData, answerBuf, ret - 2 );
                        return ret - 2;
                }else
                {
                        return ERR_INVALID; //
                }
        }else if( ret < 0 )
        {
                return ret;
        }else
        {
                return ERR_INVALID;     //返回数据无效，无法解释
        }
}

// 读二进制文件
int  readBinary( unsigned short fileID, unsigned short offset, unsigned char Le, unsigned char *buf )
{
        if( ( offset > 0x7FFF ) || ( !Le ) || ( Le > 178 ) )
        {
                return ERR_PARAM;
        }

        unsigned char	cla	   = 0x00;
        unsigned char	ins	   = 0xB0;

        unsigned char	apdu[128];
        memset( apdu, 0, 128 );

        unsigned char *addr = apdu;
        *addr = cla;
        addr++;

        *addr = ins;
        addr++;

        unsigned int tmpInt = offset;
        if( offset > 255 )
        {
                UIntToHex( tmpInt, addr, 2, 1 ); //p1p2
                addr += 2;
        }else
        {
                if( fileID > 31 )
                {
                        return ERR_PARAM;
                }
                *addr = (unsigned char)fileID | (unsigned char)0x80;    //p1
                addr++;

                *addr = (unsigned char)offset;                          //p2
                addr++;
        }

        *addr = Le;                                                 //le
        addr++;

        //APDU已准备好
        unsigned char	answerBuf[256];
        int				ret;

        switch( statInfo.readerType )
        {
                case WEDS_READER_CPU:
                        ret = wedsApduChannel2( apdu, (unsigned char)5, answerBuf, 255, 300 );
                        break;
                default:
                        return ERR_INVALIDREADER;
                        break;
        }

        if( ret == Le + 2 ) //期望的返回字节数，数据、SW1、SW2
        {
                ret = cpuErrCode( answerBuf + Le );
                if( ret == ERR_OK )
                {
                        memcpy( buf, answerBuf, Le );
                }
                return ret;
        }else if( ret == 2 )
        {
                ret = cpuErrCode( answerBuf );
                if( ret == ERR_GET_RESPONSE )
                {
                        if( Le != answerBuf[1] )
                        {
                                return ERR_INVALID; //长度不符
                        }
                        return getResponse( buf, answerBuf[1] );
                }
                return ret;
        }else if( ret < 0 )
        {
                return ret;
        }else
        {
                return ERR_INVALID; //返回数据无效，无法解释
        }
}


int updateBinary( unsigned short fileID, unsigned short offset, unsigned char *buf, unsigned char len )
{
        if( ( offset > 0x7FFF ) || ( !len ) || ( len > 178 ) )
        {
                return ERR_PARAM;
        }

        unsigned char	cla	   = 0x00;
        unsigned char	ins	   = 0xD6;

        unsigned char	apdu[256];
        memset( apdu, 0, 256 );

        unsigned char *addr = apdu;
        *addr = cla;
        addr++;

        *addr = ins;
        addr++;

        unsigned int tmpInt = offset;
        if( offset > 255 )
        {
                UIntToHex( tmpInt, addr, 2, 1 ); //p1p2
                addr += 2;
        }else
        {
                if( fileID > 31 )
                {
                        return ERR_PARAM;
                }
                *addr = (unsigned char)fileID | (unsigned char)0x80;    //p1
                addr++;

                *addr = (unsigned char)offset;                          //p2
                addr++;
        }

        *addr = len;                                                //lc
        addr++;

        memcpy( addr, buf, len );
        addr += len;

        //APDU已准备好
        unsigned char	answerBuf[128];
        int				ret;

        switch( statInfo.readerType )
        {
                case WEDS_READER_CPU:
                        ret = wedsApduChannel2( apdu, apdu[4] + 5, answerBuf, 128, 600 );
                        break;

                default:
                        return ERR_INVALIDREADER;
                        break;
        }

        if( ret == 2 ) //期望的返回字节数,SW1、SW2
        {
                return cpuErrCode( answerBuf );
        }else if( ret < 0 )
        {
                return ret;
        }else
        {
                return ERR_INVALID; //返回数据无效，无法解释
        }

}


// 验证pin码
int verifyPIN( unsigned char *password, unsigned char len, int *retrys )
{
        if( ( len < 2 ) || ( len > 6 ) )
        {
                return ERR_PARAM;
        }

        unsigned char	cla	   = 0x00;
        unsigned char	ins	   = 0x20;

        unsigned char	lApdu = 0; // apdu len
        unsigned char	apdu[128];
        memset( apdu, 0, sizeof(apdu) );
        unsigned char *addr = apdu;

        *addr = cla;
        addr++; lApdu++;

        *addr = ins;
        addr++; lApdu++;

        *addr = 0;      //p1
        addr++; lApdu++;

        *addr = 0;      //p2
        addr++; lApdu++;

        *addr = len;    //lc,文件标识符长度
        addr++; lApdu++;

        memcpy( addr, password, len );
        addr += len;  lApdu += len;

        //APDU已准备好
        unsigned char	answerBuf[256];
        int				ret;

//leeDebugData(apdu, lApdu, lApdu, 1);
        switch( statInfo.readerType )
        {
                case WEDS_READER_CPU:
                        ret = wedsApduChannel2( apdu, (unsigned char)(lApdu), answerBuf, 255, 300 );
                        break;
                default:
                        return ERR_INVALIDREADER;
                        break;
        }

//leeDebugData(answerBuf, 2, 2, 2);

        if( ret == 2 )                              //期望的返回字节数，SW1、SW2
        {
                ret = cpuErrCode( answerBuf );
                if( ret == ERR_FALSE )
                {
                        if( ( answerBuf[0] == 0x63 ) && ( ( answerBuf[1] & 0xf0 ) == 0xc0 ) )
                        {
                                *retrys = answerBuf[1] & 0x0F;  //剩余次数
                        }
                }
                return ret;
        }else if( ret < 0 )
        {
                return ret;
        }else
        {
                return ERR_INVALID; //返回数据无效，无法解释
        }
}


int cpuErrCode( unsigned char *sw )
{
        unsigned int statWord;         //数据为大端

        statWord = HexToUInt( sw, 2, 1 );         //数据为大端

        switch( statWord )
        {
                case 0x9000:
                        return ERR_OK;
                        break;
                case 0x6281:                                                //回送的数据可能错误
                        return ERR_ECHOPLEX;
                        break;
                case 0x6283:                                                //选择文件无效，文件或密钥校验错误
                        return ERR_SELECTFILE_INVALID;
                        break;

                case 0x6400:                                                //状态标志未改变
                        return ERR_STAT_NOCHANGED;
                        break;

                case 0x6581:                                                //写EEPROM 不成功
                        return ERR_WRITE_EEPROM;
                        break;

                case 0x6700:                                                //错误的长度
                        return ERR_LENGTH;
                        break;
                case 0x6900:                                                //CLA 与线路保护要求不匹配
                        return ERR_CLA_MISMATCH_LINE;
                        break;
                case 0x6901:                                                //无效的状态
                        return ERR_STAT_INVALID;
                        break;
                case 0x6981:                                                //命令与文件结构不相容
                        return ERR_INS_MISMATCH_FILE;
                        break;
                case 0x6982:                                                //不满足安全状态
                        return ERR_SAFETY_MISMATCH;
                        break;
                case 0x6983:                                                //密钥被锁死
                        return ERR_KEYLOCK;
                        break;
                case 0x6985:                                                //使用条件不满足
                        return ERR_NOCONDITION;
                        break;
                case 0x6987:                                                //无安全报文
                        return ERR_NOSAFETYMESSAGE;
                        break;
                case 0x6988:                                                //安全报文数据项不正确
                        return ERR_SAFETYMESSAGEDATA;
                        break;
                case 0x6a80:                                                //数据域参数错误
                        return ERR_DATAPARAM;
                        break;
                case 0x6a81:                                                //功能不支持或卡中无MF 或卡片已锁定
                        return ERR_NOTSUPPORT;
                        break;
                case 0x6a82:                                                //文件未找到
                        return ERR_FILENOTFOUND;
                        break;
                case 0x6a83:                                                //记录未找到
                        return ERR_RECORDNOTFOUND;
                        break;
                case 0x6a84:                                                //文件无足够空间
                        return ERR_NOSPACE;
                        break;
                case 0x6a86:                                                //参数P1 P2 错误
                        return ERR_P1P2;
                        break;
                case 0x6b00:                                                //在达到Le/Lc 字节之前文件结束，偏移量错误
                        return ERR_OFFSET;
                        break;
                case 0x6e00:                                                //无效的CLA
                        return ERR_CLAINVALID;
                        break;
                case 0x6f00:                                                //数据无效
                        return ERR_DATAINVALID;
                        break;
                case 0x9300:                                                //MAC 错误
                        return ERR_MAC;
                        break;
                case 0x9303:                                                //应用已被锁定
                        return ERR_APPLOCK;
                        break;
                case 0x9401:                                                //金额不足
                        return ERR_AMTLACK;
                        break;
                case 0x9403:                                                //密钥未找到
                        return ERR_KEYNOTFOUND;
                        break;
                case 0x9306:                                                //所需的MAC 不可用
                        return ERR_MAC_NOTUSABLE;
                        break;
                default:
                        break;
        }

        if( sw[0] == 0x61 )                                             //正确执行,XX 表示响应数据长度。可用Get Response 命令取回响应数据。（仅用于T=0）
        {
                return ERR_GET_RESPONSE;                                    //成功，需要取响应 sw2为响应的数据长度
        }else if( ( sw[0] == 0x63 ) && ( ( sw[1] & 0xf0 ) == 0xc0 ) )   //sw2低4位表示还可再试次数
        {
                return ERR_FALSE;                                           //错误，可以重试，X 表示还可再试次数
        }else if( sw[0] == 0x6c )                                       //Le 错误
        {
                return ERR_LE;                                              //Le错误
        }

        return ERR_INVALID;
}


int wedsApduChannel2( unsigned char *buf, unsigned char buflen, unsigned char *answerBuf,
                                                        unsigned char maxAnswerlen, int timeout )
{
        if( buflen > 246 )
        {
                return ERR_OVERBUF;                                                 //超过最大APDU长度
        }

        unsigned char	Fhead[] = { 0xAA, 0x01, 0x00, 0x31, 0x01 };             // 帧头+设备地址+数据长度+指令字+数据包数量
        unsigned char	SendBuf[260];                                           //255+5(0X104)
        unsigned char	RecvBuf[265];
        memset( RecvBuf, 0, 6 );

        memcpy( SendBuf, Fhead, 5 );
        SendBuf[2] = buflen + 2;                                                //数据长度、、SendBuf[2] = buflen + 8; //数据长度
        memcpy( (char *)SendBuf + 5, buf, buflen );

        int len = card_data_sr( statInfo.comfd, SendBuf, RecvBuf, 0, timeout );

//if(len > 0)
//	leeDebugData(RecvBuf,len,len,2);

        if( ( len >= 10 ) && ( RecvBuf[3] == 0x31 ) && ( RecvBuf[4] == 0x01 ) ) //最少数据为2字节返回码sw1 sw2
        {
                if( maxAnswerlen < ( RecvBuf[2] - 2 ) )
                {
                        return ERR_OVERBUF;                                             //超过缓冲区大小
                }
                memcpy( answerBuf, RecvBuf + 5, RecvBuf[2] - 2 );
                return RecvBuf[2] - 2;                                              //返回apdu数据长度
        }else if( ( len == 9 ) && ( RecvBuf[3] == 0x00 ) && ( RecvBuf[4] == 0x01 ) )
        {
                switch( RecvBuf[5] )
                {
                        case RET_SUCCESS:                                               //错误码：成功
                                return ERR_INVALID;                                         //无效的返回
                                break;
                        case RET_OPERR:                                                 //指令操作失败
                                return ERR_OPERR;
                                break;
                        case RET_CHECKERR:                                              //校验错误
                                return ERR_OPERR;
                                break;
                        case RET_INVALID:                                               //不能识别的指令
                                return ERR_OPERR;
                                break;
                        case RET_NOTCONDITION:                                          //指令无法执行
                                return ERR_OPERR;
                                break;
                        default:
                                return ERR_INVALID;                                         //无效的返回
                }
        }else if( len < 0 )
        {
                return len;
        }else
        {
                return ERR_INVALID;
        }
}


#if 1
#endif

// 20160129 业务层接口



int test_std_card( )
{
        int ret, i = 0;
        TCatalog tmpCatalog;
        TissuInfo tmpIssuInfo;
        unsigned char key[16];
        unsigned char	InData[8];

        ret = readCatalogInfo( &tmpCatalog, KEY_TYPE_A,MasterKey.readManageSectorKey );
        if( ret != ERR_OK )
        {
                return ret;
        }
        memcpy( &CCatalog, &tmpCatalog, sizeof( CCatalog ) );

        ret = readIssuInfo( &tmpIssuInfo, KEY_TYPE_A, MasterKey.readManageSectorKey );
        if( ret != ERR_OK )
        {
                return ret;
        }
        memcpy( &IssuInfo, &tmpIssuInfo, sizeof( IssuInfo ) );

        switch( statInfo.scatteringFactor )
        {
                case 0:
                        switch( statInfo.cardType )
                        {
                                case CARD_UIM:
                                        ret = UIntToHex( statInfo.card2, InData, 4, 1 );
                                        break;
                                default:
                                        ret = UIntToHex( statInfo.card2, InData, 4, 0 );
                                        break;
                        }

                        if( ret )
                        {
                                return ret;
                        }

                        ret = UIntToBcd( IssuInfo.issuSerialNo % 10000, InData + 4, 2 );
                        if( ret < 0 )
                        {
                                return ret;
                        }

                        ret = UIntToHex( IssuInfo.cardAuthCode / 0x10000, InData + 6, 2, 1 ); //大端
                        if( ret < 0 )
                        {
                                return ret;
                        }
                        break;

                case 1:                                                     //物理卡号作为分散因子 20130426 add
                        ret = UIntToHex( statInfo.card1 ^ 0x8592A0C7, InData, 4, 0 );
                        if( ret < 0 )
                        {
                                return ret;
                        }

                        ret = UIntToHex( statInfo.card1 ^ statInfo.card2 ^ 0xA3F39563, InData + 4, 4, 1 );
                        if( ret < 0 )
                        {
                                return ret;
                        }
                        break;

                case 2:                                                                 //密钥对卡片非接ATR的前8字节做分散 20130607 国电的握奇卡
                        memset( InData, 0xaa, 8 );
                        UIntToHex( statInfo.card2, InData + 4, 4, 0 );   //小端模式

                        if( statInfo.card1 )
                        {
                                UIntToHex( statInfo.card1, InData, 4, 0 );
                        }
                        break;

                default:
                        return ERR_PARAM;
                        break;

        }

        memcpy( statInfo.InData, InData, 8 ); // 计算卡片分散因子 statInfo.InData
        memset( key, 0, 16 );
        if( !( card_config.isPsam == 1 && statInfo.cardType == CARD_CPU ) )
        {
                ret =  calcKey( statInfo.InData, key );              //交易密钥（原为充值密钥）
                if( ret != ERR_OK)
                {
                        return ret;
                }
//dbgShowHexData(statInfo.InData,8,1,0,"InData");
//dbgShowHexData(key,6,1,'<',"calcKey");
                memcpy( statInfo.dealKey, key, 16 );
                pubInfo.keyType = KEY_TYPE_B;              // Bkey
        }

        if( statInfo.cardType == CARD_CPU )   //CPU卡内部认证 20121015
        {
                if( 0 == card_config.isPsam )
                {
                        unsigned char tmpData[8];

                        for( i = 0; i < 8; i++ )
                        {
                                tmpData[i] = ~statInfo.InData[i];
                        }

                        RunDes( 0, (char *)statInfo.InData, (char *)key, 8, (char *)MasterKey.interAuthKey, 16 );
                        RunDes( 0, (char *)tmpData, (char *)key + 8, 8, (char *)MasterKey.interAuthKey, 16 );

                        memcpy( (char *)statInfo.internalAuthKeybuf, (char *)key, 16 );
                }

                ret = internalAuthentication( );
                if( ret != ERR_OK )
                {
                        return ret;
                }
        }


//printf("name = %s\n", Card_Chb1.pwdInfo.name);
//leeDebugData(Card_Chb1.pwdInfo.pwd,3,3,2);

        CCatalog.pubReadFlag = 1;                                                                                                                            //已读标志
        return ERR_OK;

}




/*
**
** 函数功能: 2.4G 电信手机卡应用ID设置
**
** @param
**			m_hcom: 串口操作句柄
** @return
**			参照返回码定义
**
** @other
**			2.4G电信手机卡头串口初始化后，调用该函数设置应用ID
**
*/

//int sdk_uim_reader_init(int m_hcom)
//{
//	int ret = uimHalt(m_hcom);                               //中断并重新开启RF(halt)
//	if( ret != ERR_OK )
//	{
//		return ret;
//	}
//	usleep( 100000 );                                   //30、35、40、45、50ms不可靠，55偶尔读不到卡，重新上电可能读不到卡，一旦读不到，必须重新上电，否则读不到（怀疑内部节奏乱了）。

//	ret = uimSetMifareAppID(m_hcom, card_config.mifareAppID );    //设置Mifare应用ID
//	if( ret != ERR_OK )
//	{
//		return ret;
//	}
//	usleep( 200000 );                                   //130ms能读到卡，失败率较高（创博的卡）

//	ret = uimMessageSwitch( m_hcom, 1 );                        //打开消息开关20110729
//	if( ret != ERR_OK )
//	{
//		return ret;
//	}
//	usleep( 150000 );                                   //130ms能读到卡，失败率较高（创博的卡）

//	uimSelectAccounter(m_hcom);

//	return ERR_OK;
//}


#if 0
#endif

/*
** 函数功能: 初始化可配置参数
**
** @param
                        in.mifareAppID:设置手机卡的应用id
                        in.isPsam:设置是否使用psam卡认证卡片
** @return
**
**
** @other
**			设备启动后，在初始化流程中调用一次。程序运行中，如相关参数调整，则再次初始化
**
*/
int pit_init_param(_IN_PARAM in)
{
        strcpy( card_config.mifareAppID, in.mifareAppID);	// "Mifare13 App" );
        card_config.isPsam = in.isPsam;
        return ERR_OK;
}


/* 函数功能: 初始化系统配置
**
** @param
**			in: 输入参数，配置文件sys.ini路径
** @return
**
** @other
**			设置母密钥，发行扇区号等信息
**
*/
int pit_init_sysini(_IN_PARAM in)
{
        char	DeviceKey[16]  = { 0x83, 0x65, 0xFD, 0x46, 0x19, 0xA0, 0xC3, 0x68,
                                                                                 0x92, 0x87, 0xa2, 0xB3, 0x17, 0x23, 0x98, 0x6D };
        char buf[128];
        unsigned char tmp[16];
        char *ptr = NULL;
        FILE *fp = NULL;

        if( access( in.path, F_OK | R_OK ) != 0 )
        {
                return ERR_FILE_NOT_EXIST;
        }

        if( ( fp = fopen( in.path, "r" ) ) == NULL )
        {
                return ERR_FILE_OPEN_FALSE;
        }

        memset(buf, 0, sizeof(buf));
        readini2( fp, "[overall]", "devKey", buf );
        ptr = buf;
        THexToHex( ptr, tmp, 16 );
        RunDes( 0, DeviceKey, (char *)card_config.devKey, 16, (char *)tmp, 16 );
        THexToHex( ptr + 32, tmp, 16 );
        RunDes( 1, tmp, (char *)card_config.interAuthKey, 16, DeviceKey, 16 );

        memset(buf, 0, sizeof(buf));
        readini2( fp, "[card_s50]", "catalogSct", buf );
        card_config.mngSect[0] = atoi(buf);

        memset(buf, 0, sizeof(buf));
        readini2( fp, "[card_s70]", "catalogSct", buf );
        card_config.mngSect[1] = atoi(buf);

        memset(buf, 0, sizeof(buf));
        readini2( fp, "[card_uim]", "catalogSct", buf );
        card_config.mngSect[2] = atoi(buf);

        memset( buf, 0, sizeof( buf ) );
        readini2( fp, "[overall]", "devType", buf );     // 读设备类型
        card_config.enLmt = atoi( buf );

        return ERR_OK;

}


/* 函数功能: 初始化卡配置信息(仅调用一次)，设置卡操作相关初始值，实现构造函数功能
**
** @in param
**			无
** @return
**			参考具体返回码
**
** @other
**
*/
void pit_init_card_cfg()
{
        memset(&statInfo, 0, sizeof(statInfo));

        statInfo.comfd = -1; // 初始化串口句柄 20160114
        statInfo.manageSector  = 1;
        statInfo.authSectorNo  = 0xff;
        statInfo.accessMsec = GetTickCount( );
        statInfo.yooReqMsec = 0;
        statInfo.authDelayMsec = 300;

        memcpy(MasterKey.readManageSectorKey, "\xA0\xA1\xA2\xA3\xA4\xA5\xA6\xA7\xA8\xA9\xAA\xAB\xAC\xAD\xAE\xAF", 16);

        statInfo.cpuCodeID = 0;
        memset( statInfo.ATS, 0, 32 );
        memset( statInfo.cosID, 0, sizeof( statInfo.cosID ) );
        memset( statInfo.codeID, 0, sizeof( statInfo.codeID ) );

        statInfo.cosID[0]  = 0x544625; //T1 T2 T3
        statInfo.codeID[0] = COSTF26;
        statInfo.cosID[1]  = 0x544626;
        statInfo.codeID[1] = COSTF26;
        statInfo.cosID[2]  = 0x209000;
        statInfo.codeID[2] = COSFM20;
        statInfo.cosID[3]  = 0x479200;
        statInfo.codeID[3] = COSTimeCOS_PBOC;               //20121228
        statInfo.cosID[4] = 0x477300;
        statInfo.codeID[4] = COSTimeCOS_PBOC1; //20130812 国电的测试卡

        statInfo.psamAdfFileID		   = 0xdf03;            //20121213 支持PSAM加密
        statInfo.keyType_Edition[0][0] = 7;                 //内部认证密钥类型
        statInfo.keyType_Edition[0][1] = 0x31;              //内部认证密钥版本
        statInfo.keyType_Edition[1][0] = 7;                 //发行区读
        statInfo.keyType_Edition[1][1] = 0x02;              //目前不存在此密钥20121213
        statInfo.keyType_Edition[2][0] = 7;                 //数据区读
        statInfo.keyType_Edition[2][1] = 0x01;
        statInfo.keyType_Edition[3][0] = 7;                 //数据区写
        statInfo.keyType_Edition[3][1] = 0x11;  			//消费用
        statInfo.keyType_Edition[4][0] = 7;                 //发行区写
        statInfo.keyType_Edition[4][1] = 0x21;
        statInfo.keyType_Edition[5][0] = 7;                 //扩展区读
        statInfo.keyType_Edition[5][1] = 0x01;              //与数据区同
        statInfo.keyType_Edition[6][0] = 7;                 //扩展区写
        statInfo.keyType_Edition[6][1] = 0x21;              //与发行区同

        //支持CPU卡通用配置
        statInfo.issuReadLimit = 0;                         // 读发行区，初始化为不认证
        statInfo.parentDFfileID = 0x3f00;    //20130607

        statInfo.cpuAdfFileID	   = 0xaf01;                //CPU卡的ADF文件标识符，缺省值标准
        statInfo.issuSectorFileID  = 0x11;                  //发行区短文件标识符
        statInfo.dataSectorFileID  = 0x12;                  //数据区短文件标识符
        statInfo.extSectorFileID   = 0x13;                  //扩展数据区短文件标识符

        statInfo.issuSectorFileOffset  = 0;                 //发行区在文件中的偏移量
        statInfo.dataSectorFileOffset  = 0;                 //数据区在文件中的偏移量
        statInfo.extSectorFileOffset   = 0;                 //扩展数据区在文件中的偏移量

        statInfo.internalAuthKeyID = 1;                     //内部认证DES密钥标识号
        statInfo.internalAuthKeyType = 0;   //PSAM卡使用解密密钥

        statInfo.issuSectorKeyEdition_KeyID[0][0]  = 0;     //发行区读密钥的密钥版本
        statInfo.issuSectorKeyEdition_KeyID[0][1]  = 0xF0;  //发行区读密钥的密钥标识（或索引）
        statInfo.issuSectorKeyEdition_KeyID[0][2]  = 0x01;  //发行区读密钥的后续权限
        statInfo.issuSectorKeyEdition_KeyID[1][0]  = 0;     //发行区写密钥的密钥版本
        statInfo.issuSectorKeyEdition_KeyID[1][1]  = 0xF3;  //发行区写密钥的密钥标识（或索引）
        statInfo.issuSectorKeyEdition_KeyID[1][2]  = 0x08;  //发行区写密钥的后续权限

        statInfo.dataSectorKeyEdition_KeyID[0][0]  = 0;     //数据区读密钥的密钥版本
        statInfo.dataSectorKeyEdition_KeyID[0][1]  = 0xF1;  //数据区读密钥的密钥标识（或索引）
        statInfo.dataSectorKeyEdition_KeyID[0][2]  = 0x02;  //数据区读密钥的后续权限
        statInfo.dataSectorKeyEdition_KeyID[1][0]  = 0;     //数据区写密钥的密钥版本
        statInfo.dataSectorKeyEdition_KeyID[1][1]  = 0xF2;  //数据区写密钥的密钥标识（或索引）
        statInfo.dataSectorKeyEdition_KeyID[1][2]  = 0x04;  //数据区写密钥的后续权限

        statInfo.extSectorKeyEdition_KeyID[0][0]   = 0;     //扩展数据区读密钥的密钥版本
        statInfo.extSectorKeyEdition_KeyID[0][1]   = 0xF1;  //扩展数据区读密钥的密钥标识（或索引）
        statInfo.extSectorKeyEdition_KeyID[0][2]   = 0x02;  //扩展数据区读密钥的后续权限
        statInfo.extSectorKeyEdition_KeyID[1][0]   = 0;     //扩展数据区写密钥的密钥版本
        statInfo.extSectorKeyEdition_KeyID[1][1]   = 0xF3;  //扩展数据区写密钥的密钥标识（或索引）
        statInfo.extSectorKeyEdition_KeyID[1][2]   = 0x08;  //扩展数据区写密钥的后续权限

        statInfo.issuSectorAcceLimit[0][0] = 0;             //发行区读权限最小值
        statInfo.issuSectorAcceLimit[0][1] = 0x0F;          //发行区读权限最大值
        statInfo.issuSectorAcceLimit[1][0] = 8;             //发行区写权限最小值
        statInfo.issuSectorAcceLimit[1][1] = 8;             //发行区写权限最大值

        statInfo.dataSectorAcceLimit[0][0] = 2;             //数据区读权限最小值
        statInfo.dataSectorAcceLimit[0][1] = 8;             //数据区读权限最大值
        statInfo.dataSectorAcceLimit[1][0] = 4;             //数据区写权限最小值
        statInfo.dataSectorAcceLimit[1][1] = 4;             //数据区写权限最大值

        statInfo.extSectorAcceLimit[0][0]  = 2;             //扩展数据区读权限最小值
        statInfo.extSectorAcceLimit[0][1]  = 8;             //扩展数据区读权限最大值
        statInfo.extSectorAcceLimit[1][0]  = 8;             //扩展数据区写权限最小值
        statInfo.extSectorAcceLimit[1][1]  = 8;             //扩展数据区写权限最大值
        statInfo.scatteringFactor = 0;	// 初始化
        return;
}




/*
**
** 函数功能: 卡片操作统一接口，如查询、消费等
**
** @in param
**			in:输入参数
**			out:输出参数
** @return
**			参考具体返回码
**
** @other
**			卡片相关操作最终均调用该接口，通过操作码区分不同的操作
*/

int pit_card_main(_IN_PARAM in, _OUT_PARAM *out)
{
        int ret = -1, i = 0, j = 0, k = 0;
        unsigned int val, num, count;
        unsigned char *addr = NULL, cpdata[8][16]; // 单笔菜品明细最多8个块，56项，故软件限制最多输入56项, 包括记录，总计占3个扇区
        struct 	tm *tp = NULL;
        time_t curTimes, timep;
        _CARD_DATA CARD_DATA;

        if(out == NULL)
        {
                return ERR_PARAM;
        }
        ret = do_after_get_card(in);
        if( ERR_OK != ret )
        {
                return ret;
        }

        ret = ol_deal_card( );
        if( ERR_OK != ret )
        {
                return ret;
        }

        curTimes = in.curTimes - TIME20000101;
        ret = checkCard( curTimes );
        if( ret != ERR_OK )
        {
                return ret;
        }

        if(in.cmd == CMD_READ_CARD_REC)		// 读取卡片离线记录
        {
                if(pubInfo.recNum == 0)
                {
                        out->recblknum = pubInfo.recNum;
                        return ERR_OK;
                }

                addr = out->crec;
                for(i = 0; i < pubInfo.recNum; i++)
                {
                        ret	= readDataBlock2( pubInfo.keyType, statInfo.dealKey,
                                     CCatalog.recArray[i] / 4, CCatalog.recArray[i] % 4, 16, addr );
                        if(ret != ERR_OK)
                        {
                                return ret;
                        }

                        if( CalcCRC16( addr, 16, 0 ) != 0 )
                        {
                                return ERR_FALSE;
                        }
                        addr	   += 16;	// must add here
                }

                out->recblknum = pubInfo.recNum;

                return ERR_OK;

        }
        else if(in.cmd == CMD_CLR_CARD_REC) // 清空卡片离线记录
        {
                if(pubInfo.recNum != 0)
                {
                        pubInfo.recNum = 0;
                        return new_writePub();
                }
                else
                {
                        return ERR_OK;
                }
        }
        else if(in.cmd == CMD_GET_CARD_INFO)	// 读取卡数据
        {
                out->recnum = pubInfo.recNum;
                out->cardLsh = pubInfo.cardLsh;
                out->cardStat = pubInfo.cardStat;
                out->cardSnr = atoi(IssuInfo.CardNo);		// 逻辑卡号
                return ERR_OK;
        }
        else if(in.cmd == CMD_SYNC_CARD_AMT)	// 同步卡余额
        {
                if (in.sync_balance > 0x7FFFFF || in.sync_balance < -(0x800000))
                {
                        return ERR_PARAM;
                }
                pubInfo.walletAmt = in.sync_balance;
                return new_writePub();
        }
        else if(in.cmd == CMD_SYNC_CARD_AMT_LMT)	// 同步卡余额及累计信息
        {
                if (in.sync_balance > 0x7FFFFF || in.sync_balance < -(0x800000))
                {
                        return ERR_PARAM;
                }
                pubInfo.walletAmt = in.sync_balance;

                pubInfo.dayAmt		= in.sync_dayAmt;	// 要判断范围
                pubInfo.mealAmt	= in.sync_prdAmt;
                pubInfo.dayTimes 	= in.sync_dayTimes;
                pubInfo.mealTimes	= in.sync_prdTimes;
                pubInfo.dealDate 	= in.sync_dealDate;
                return new_writePub();
        }
        else if(in.cmd == CMD_SET_CARD_HMD)	// 设置卡片黑名单
        {
                pubInfo.cardStat = 4;
                return new_writePub();
        }
        else if(in.cmd == OFFLINE_QUERY) // 离线查询
        {
                out->balance = pubInfo.walletAmt;
                out->begd = atoi( IssuInfo.beginDateStr );
                out->endd = atoi( IssuInfo.endDateStr);
                tp = localtime(&in.curTimes); //当前时间
                val = (tp->tm_mon+1) * 100 + tp->tm_mday;
                if( val != pubInfo.dealDate ) // 日期改变 init
                {
                        out->dayAmt	   = 0;
                        out->mealAmt 	   = 0;
                        out->dayTimes	   = 0;
                        out->mealTimes   = 0;
                }
                return ERR_OK;
        }
        else if(in.cmd == OFFLINE_TRADE)	// 离线消费
        {
                memset( &CP_INFO, 0, sizeof( CP_INFO ) );
                if(in.cpnum > 56)
                {
                        return ERR_FALSE;
                }

                if(in.cpnum > 0)
                {
                        count = in.cpnum;
                        num = count/7;
                        if(count%7 != 0) num++;
                        k = 0;
                        memset(cpdata, 0, sizeof(cpdata));
                        for(i = 0; i < num; i++)
                        {
                                addr = cpdata[i];
                                for(j = 1; j <= 7; j++)
                                {
                                        val = in.cpdata[k][0] * 16 + in.cpdata[k][1];
                                        UIntToHex( val, addr, 2, 1 );
                                        addr += 2;
                                        k++;
                                        if(k >= in.cpnum)
                                        {
                                                goto MENU_OK;
                                        }
                                }
                        }
MENU_OK:
                        CP_INFO.cpblknum = num;
                        memcpy(CP_INFO.cpHData, cpdata, sizeof(cpdata));
                }

                if(CCatalog.recSctNum * 3 <= (pubInfo.recNum + CP_INFO.cpblknum))	// 卡空间是否已满
                {
                        return ERR_FALSE;
                }

                if (in.optAmt > 0xFFFF) // 单笔最大交易额 655.35
                {
                        return ERR_FALSE;
                }

                if(in.optAmt > pubInfo.walletAmt)
                {
//			sound("yebz");
                        return ERR_FALSE;
                }

                if(in.prdxh == 0 && in.nonprd == 0)	// 非时段不允许消费
                {
                        return ERR_FALSE;
                }

                if(in.prdxh > 15) // 0 ~ 15
                {
                        return ERR_FALSE;
                }

                if(!card_config.enLmt) // 启用限额限次
                {
                        timep = in.curTimes - in.offsetTime;
                        tp = localtime(&timep);
                        val = (tp->tm_mon + 1) * 100 + tp->tm_mday;

                        if( val != pubInfo.dealDate ) // 日期改变 init
                        {
                                pubInfo.dayAmt		= 0;
                                pubInfo.mealAmt 	= 0;
                                pubInfo.dayTimes	= 0;
                                pubInfo.mealTimes	= 0;
                                pubInfo.dealDate	= val;
                        }
                        else if(in.prdxh != pubInfo.mealNo) // 时段改变
                        {
                                pubInfo.mealAmt 	= 0;
                                pubInfo.mealTimes	= 0;
                                pubInfo.mealNo		= in.prdxh;
                        }

                        if(in.prdxh != 0 && in.pwd == 0)
                        {
                                if((in.optAmt + pubInfo.dayAmt > in.dayLmtAmt) || (in.optAmt + pubInfo.dayAmt > 0xFFFF))
                                {
                                        return ERR_FALSE;
                                }
                                pubInfo.dayAmt += in.optAmt;

                                if((pubInfo.dayTimes >= in.dayLmtTimes) || (pubInfo.dayTimes == 31))
                                {
                                        return ERR_FALSE;
                                }
                                pubInfo.dayTimes++;

                                if((in.optAmt + pubInfo.mealAmt > in.prdLmtAmt) || (in.optAmt + pubInfo.mealAmt > 0xFFFF))
                                {
                                        return ERR_FALSE;
                                }
                                pubInfo.mealAmt += in.optAmt;

                                if((pubInfo.mealTimes >= in.prdLmtTimes)	|| (pubInfo.mealTimes == 7))
                                {
                                        return ERR_FALSE;
                                }
                                pubInfo.mealTimes++;
                        }
                }

// 写卡交易数据
                CARD_DATA.devId = in.devId;
                CARD_DATA.lsh = in.lsh;
                CARD_DATA.cardLsh = pubInfo.cardLsh + 1;
                CARD_DATA.relativeTime = curTimes;
                CARD_DATA.optAmt = in.optAmt;
                CARD_DATA.cpblknum = CP_INFO.cpblknum;
                memcpy(CARD_DATA.cpblk, CP_INFO.cpHData, sizeof(CP_INFO.cpHData));

                ret = do_consume(&CARD_DATA);
                if(ret != ERR_OK && ret != ERR_HALF_OK)
                {
//			sound("ye");
                        return ERR_FALSE;
                }

                out->balance = pubInfo.walletAmt;
                out->optAmt = in.optAmt;

                return ERR_OK;

        }
        else
        {
                return ERR_PARAM;
        }

        return ERR_FALSE;

}

/*
**
** 函数功能: 脱机卡片操作统一接口，如查询、消费等
**
** @in param
**			in:输入参数
**			out:输出参数
** @return
**			参考具体返回码
**
** @other
**			卡片相关操作最终均调用该接口，通过操作码区分不同的操作
*/

int pit_card_main_std(_IN_PARAM in, _OUT_PARAM *out)
{
        int ret = -1, i = 0, j = 0, k = 0;
        unsigned int val, num, count;
        unsigned char *addr = NULL, cpdata[8][16]; // 单笔菜品明细最多8个块，56项，故软件限制最多输入56项, 包括记录，总计占3个扇区
        struct 	tm *tp = NULL;
        time_t curTimes, timep;
        _CARD_DATA CARD_DATA;

        if(out == NULL)
        {
                return ERR_PARAM;
        }

        ret = do_after_get_card(in);
        if( ERR_OK != ret )
        {
                return ret;
        }

        ret = deal_card_std( );
        if( ERR_OK != ret )
        {
                return ret;
        }
        curTimes = in.curTimes - TIME20000101;
        ret = checkCard( curTimes );
        if( ret != ERR_OK )
        {
                return ret;
        }

        return ERR_OK;

}



#if 0
#endif

/*
**
** 函数功能: 卡片操作前预处理
**
** @in param
**			in:输入参数
** @return
**			参考具体返回码
**
** @other
**
*/

int do_after_get_card(_IN_PARAM in)
{
        int comfd = -1, ret = -1;
        char phycard[32+1];	// 物理卡号
        unsigned int cardlen = 0;	// 卡号长度，4字节、7字节或8字节

        comfd = sdk_get_comfd(in.com);
        if(comfd < 0)
        {
                return ERR_OK;
        }
        statInfo.comfd = comfd;
        statInfo.readerType = in.reader;
        statInfo.manageBlock = SCT_BLK_2;

        switch(in.cardlx)
        {
                case CARD_IC:
                case CARD_CPU:	// CPU卡实际并不使用该设置
                        statInfo.manageSector  = 1;card_config.mngSect[0];
                        break;
                case CARD_S70:
                        statInfo.manageSector  = card_config.mngSect[1];
                        break;
                case CARD_UIM:
                        statInfo.manageSector  = card_config.mngSect[2];
                        break;
                default:
                        return ERR_FALSE;
                        break;
        }

        if(in.cardlx == CARD_CPU)
        {
                memset(phycard, 0, sizeof(phycard));
                ret = card_get_cpu( phycard, &cardlen);
                if(ret != CARD_CPU)
                {
                        return ERR_FALSE;
                }
                if(strcmp(phycard, in.phycard) !=0 || cardlen != in.cardlen)
                {
                        return ERR_FALSE;
                }
        }
        else
        {
                if( in.cardlen == 4 )
                {
                        statInfo.card2 = HexToUInt( in.phycard, 4, 1 );
                }else
                {
                        statInfo.card1 = HexToUInt( in.phycard, 3, 1 );
                        statInfo.card2 = HexToUInt( in.phycard + 9, 4, 1 );
                }
        }

        return ERR_OK;

}



/*
** 函数功能: 读取卡记录和明细
**
** @in param
**			暂不用
** @out param
**			out.crec: 缓冲区，用于返回卡片记录数据
**			out.recblknum: 离线记录数
**
** @return
**			0-OK,其它参照返回码定义
**
** @other
**			返回成功后了，判断离线记录数out.recblknum，一个扇区块视为一条记录
*/
static int read_card_rec(_IN_PARAM in, _OUT_PARAM *out)
{
        unsigned char *addr  = NULL;
        int i = 0, ret = -1;

        if (out == NULL)
        {
                return ERR_PARAM;
        }

        if(pubInfo.recNum == 0)
        {
                out->recblknum = pubInfo.recNum;
                return ERR_OK;
        }

        addr = out->crec;
        for(i = 0; i < pubInfo.recNum; i++)
        {
                ret	= readDataBlock2( pubInfo.keyType, statInfo.dealKey,
                             CCatalog.recArray[i] / 4, CCatalog.recArray[i] % 4, 16, addr );
                if(ret != ERR_OK)
                {
                        return ret;
                }

                if( CalcCRC16( addr, 16, 0 ) != 0 )
                {
                        return ERR_FALSE;
                }
                addr	   += 16;	// must add here
        }

        out->recblknum = pubInfo.recNum;

        return ERR_OK;

}

static int upload_card_rec()
{
        int	ret = -1, i = 0, j = 0, k = 0, recNum = 0, cpnum = 0, val = 0, len = 0;
        _CARD_DATA CARD_DATA;
        unsigned char *addr = NULL;
        char tmp[48], tbuf[256], sBuf[1024];
        unsigned int cpdetail[56][2];
        struct 	tm *tp = NULL;
        time_t relativeTime;

        _IN_PARAM in;
        _OUT_PARAM out;


        memset(&out, 0, sizeof(out));
        ret = read_card_rec(in, &out);
        if( ret != ERR_OK )
        {
                return ret;
        }

        recNum = out.recblknum;
        if(recNum == 0)
        {
                return ERR_OK;
        }

        memset(sBuf, 0, sizeof(sBuf));
        memset(cpdetail, 0, sizeof(cpdetail));

        for(i = 0; i < recNum; i++)
        {
                addr = (unsigned char *)&out.crec + (i * 16);
                if(cpnum > 0)
                {
                        for(k = 0; k < 7; k++)
                        {
                                val = HexToUInt(addr, 2, 1);
                                addr += 2;
                                cpdetail[j][0] = val / 16;
                                cpdetail[j][1] = val % 16;
                                if(cpdetail[j][0] == 0 || cpdetail[j][1] == 0)
                                {
                                        break;
                                }
                                j++;
                        }

                        if(--cpnum == 0)
                        {
                                memset(tbuf, 0, sizeof(tbuf));
                                for(k = 0; k < j; k++)
                                {
//					sprintf(tmp, "%d|%d,", cpdetail[k][0], cpdetail[k][1]);
                                        sprintf(tmp, "%d_%d|", cpdetail[k][0], cpdetail[k][1]);
                                        strcat(tbuf, tmp);
                                }

                                len += strlen(tbuf) + 1;
                                if(len >= sizeof(sBuf))
                                {
                                        return ERR_FALSE;
                                }
                                strcat(sBuf, tbuf);
                                strcat(sBuf, "\n");
                                memset(cpdetail, 0, sizeof(cpdetail));
                                j = 0;
                        }
                }
                else
                {
                        memset(&CARD_DATA, 0, sizeof(CARD_DATA));
                        CARD_DATA.devId = HexToUInt(addr, 2, 1);
                        addr += 2;

                        CARD_DATA.lsh = HexToUInt(addr, 3, 1);
                        addr += 3;

                        CARD_DATA.cardLsh = HexToUInt(addr, 2, 1);
                        addr += 2;

                        CARD_DATA.relativeTime = HexToUInt(addr, 4, 1);
                        addr += 4;

                        relativeTime = CARD_DATA.relativeTime + TIME20000101;
                        memset(tmp, 0, sizeof(tmp));
                        tp = localtime(&relativeTime);
                        strftime( tmp, sizeof(tmp) , "%04Y-%02m-%02d %02H:%02M:%02S", tp );

                        val = HexToInt(addr, 3, 1);
                        addr += 3;

                        CARD_DATA.optAmt = val / 10;
                        CARD_DATA.cpblknum = val % 10;

                        cpnum = CARD_DATA.cpblknum;

                        if(cpnum > 0)
                        {
                                sprintf(tbuf, "%d,%d,%d,%s,%d,", CARD_DATA.devId, CARD_DATA.lsh, CARD_DATA.cardLsh, tmp, CARD_DATA.optAmt);
                        }
                        else
                        {
                                sprintf(tbuf, "%d,%d,%d,%s,%d\n", CARD_DATA.devId, CARD_DATA.lsh, CARD_DATA.cardLsh, tmp, CARD_DATA.optAmt);
                        }

                        len += strlen(tbuf);
                        if(len >= sizeof(sBuf))
                        {
                                return ERR_FALSE;
                        }

                        strcat(sBuf, tbuf);

                }

        }

#if 0 // // upload rec , 由SDK提供通信，并返回上传结果
        ret = jupk_Online_CardRec( );
        if (ret != 0)
        {
                return ERR_FALSE;
        }
#endif


printf("\n==================\n\n%s\n", sBuf);
#if 1	// 上传成功后，清零卡片离线记录
        pubInfo.recNum = 0;
        return new_writePub();

#endif

        return ERR_FALSE;

}


//消费过程
// recblk --ji, cpblk -- 菜品数据，blknum -- 菜品块数
static int do_consume(_CARD_DATA *CARD_DATA)
{
        int i = 0, point = 0, ret = -1;
        unsigned char *addr = NULL;
        unsigned char blk[16];

        if(CARD_DATA == NULL)
        {
                return ERR_PARAM;
        }

        memset(blk, 0, sizeof(blk));
        addr = blk;
        UIntToHex( CARD_DATA->devId, addr, 2, 1 );
        addr += 2;

        UIntToHex( CARD_DATA->lsh, addr, 3, 1 );
        addr += 3;

        UIntToHex( CARD_DATA->cardLsh, addr, 2, 1 );
        addr += 2;

        UIntToHex( CARD_DATA->relativeTime, addr, 4, 1 );
        addr += 4;

        IntToHex( CARD_DATA->optAmt * 10 + CARD_DATA->cpblknum, addr, 3, 1 );
        addr += 3;

        addr = blk;
        CalcCRC16( addr, 14, 1 );

// 写交易记录
        point = pubInfo.recNum;
        ret	= writeDataBlock2( pubInfo.keyType, statInfo.dealKey,
                                CCatalog.recArray[point] / 4, CCatalog.recArray[point] % 4, 16, addr );
        if( ret != ERR_OK )
        {
                return ret;
        }

        point++;

// 写菜品明细记录
        addr = CARD_DATA->cpblk; // 注: cpblk 未处理校验
        for(i = 0; i < CARD_DATA->cpblknum; i++)
        {
                CalcCRC16( addr, 14, 1 );
                ret	= writeDataBlock2( pubInfo.keyType, statInfo.dealKey,
                                        CCatalog.recArray[point] / 4, CCatalog.recArray[point] % 4, 16, addr );
                if( ret != ERR_OK )
                {
                        return ret;
                }
                addr += 16; // must here
                point++;
        }

        pubInfo.recNum = point;

// 更新公共区数据
        pubInfo.walletAmt -= CARD_DATA->optAmt;
        pubInfo.cardLsh++;

// 写公共区
        return new_writePub();

}

static int readCatalogInfo( PTCatalog PCatalog, int keymode, unsigned char *key )
{
//        if( statInfo.manageSector == 0 )
//        {
//                return ERR_CATALOG_INVALID;
//        }

        int				ret;
        unsigned char	*buf   = PCatalog->blockBuf;
        unsigned char	*addr  = buf;


        memset( PCatalog, 0, sizeof( *PCatalog ) ); //目录结构清零
        ret = readDataBlock2( keymode, key, statInfo.manageSector, statInfo.manageBlock, 16, addr );
        if( ret != ERR_OK )
        {
                return ret;
        }

//        if( CalcCRC8( buf, 0 ) != 0 )
//        {
//                return ERR_CRC;                                                     //CRC校验错误
//        }

        PCatalog->catalogFlag   = HexToUInt( addr, 3, 1 );      //目录区标识
        addr						  += 3;
        PCatalog->issuSectorNo  = HexToUInt( addr, 1, 1 );      //发行扇区号
        addr						  += 1;

        PCatalog->pubSectorNo   = HexToUInt( addr, 1, 1 );      //公共扇区号
        addr						  += 1;

        PCatalog->sumSectorNo   = HexToUInt( addr, 1, 1 );      //累计扇区号
        addr						  += 1;

        PCatalog->baseSectorNo  = HexToUInt( addr, 1, 1 );      //基础扇区号
        addr						  += 1;

        PCatalog->subsidySectorNo   = HexToUInt( addr, 1, 1 );  //补贴扇区号
        addr							  += 1;

        PCatalog->detailSectorNo[0] = HexToUInt( addr, 1, 1 );  //明细扇区号0
        addr							  += 1;

        PCatalog->detailSectorNo[1] = HexToUInt( addr, 1, 1 );  //明细扇区号1
        addr							  += 1;

        PCatalog->detailSectorNo[2] = HexToUInt( addr, 1, 1 );  //明细扇区号2
        addr							  += 1;

        PCatalog->detailSectorNo[3] = HexToUInt( addr, 1, 1 );  //明细扇区号3
        addr							  += 1;

        PCatalog->detailSectorNo[4] = HexToUInt( addr, 1, 1 );  //明细扇区号4
        addr							  += 1;

        PCatalog->copiesSectorNo = HexToUInt( addr, 1, 1 );  //份记录扇区号
        addr							  += 1;

        PCatalog->linkageSectorNo = HexToUInt( addr, 1, 1 );  //联动扇区号
        addr							  += 1;
        ret = checkCatalog( PCatalog );

        if( ret != ERR_OK )
        {
                return ret;
        }
        PCatalog->validFlag = 1;

        return ERR_OK;
}

static int checkCard( unsigned int currDate )
{
        if( IssuInfo.useFlag == 0 )
        {
                return ERR_CARD_NO_USE;         //卡片未启用
        }

        switch( IssuInfo.statFlag )
        {
                case 0:                         //未启用
                        return ERR_CARD_NO_USE;     //卡片未启用
                        break;
                case 1:                         //已启用
                        break;
                case 2:                         //已停用
                        return ERR_CARD_STOP_USE;   //卡片已停用
                        break;
                case 3:                         //已退卡
                        return ERR_CARD_RETURN;     //卡片已退卡
                        break;
                case 4:                         //黑名单卡
                        return ERR_CARD_BLACK;      //黑名单卡
                        break;
                default:
                        return ERR_FALSE;
                        break;
        }

        if( pubInfo.cardStat == 4 )
        {
                return ERR_CARD_BLACK;      //黑名单卡
        }
        if( currDate > IssuInfo.endDate )
        {
                return ERR_OVER_ENDDATE;    //超过有效期
        }

        if( currDate < IssuInfo.beginDate )
        {
                return ERR_OUT_VALID_DATE;  //不在有效期
        }

        return ERR_OK;
}


//大扇区划分为4个小扇区, 32+8*4=64
static int checkCatalog( PTCatalog PCatalog )
{
        unsigned int i = 0, j = 0, k = 0;

        if( PCatalog->catalogFlag != 0x535444 && PCatalog->catalogFlag != 0x677168) // 0x677168 平台发新卡
        {
                return ERR_CATALOG_FLAG;                            //卡片目录标志错误
        }

        unsigned char sectorTimes[72];                          //用于判断扇区是否重复定义
        memset( sectorTimes, 0, 72 );

        if( PCatalog->issuSectorNo > 71 )
        {
                return ERR_ISSU_SECTORNO;                           //发行扇区号超限
        }
        sectorTimes[PCatalog->issuSectorNo]++;           //发行区

        if( PCatalog->pubSectorNo > 71 )
        {
                return ERR_PUB_SECTORNO;                            //公共扇区号超限
        }
        sectorTimes[PCatalog->pubSectorNo]++;            //公共区

//////////////////////////

        if( PCatalog->sumSectorNo > 71 )
        {
                return ERR_SUM_SECTORNO;                            //累计扇区号超限
        }
        if( PCatalog->sumSectorNo )
        {
                sectorTimes[PCatalog->sumSectorNo]++;			//累计区
                PCatalog->sctArray[k++] = PCatalog->sumSectorNo;
                PCatalog->recSctNum++; // 20151203 统计记录区扇区数 不包括发行区和公共区
        }

        if( PCatalog->baseSectorNo > 71 )
        {
                return ERR_BASE_SECTORNO;                           //基础扇区号超限
        }
        if( PCatalog->baseSectorNo )
        {
                sectorTimes[PCatalog->baseSectorNo]++;       //基础区
                PCatalog->sctArray[k++] = PCatalog->baseSectorNo;
                PCatalog->recSctNum++;
        }

        if( PCatalog->subsidySectorNo > 71 )
        {
                return ERR_SUBSIDY_SECTORNO;                        //补贴扇区号超限
        }
        if( PCatalog->subsidySectorNo )
        {
                sectorTimes[PCatalog->subsidySectorNo]++;    //补贴区
                PCatalog->sctArray[k++] = PCatalog->subsidySectorNo;
                PCatalog->recSctNum++;
        }

        for( i = 0; i < 5; i++ )
        {
                if( PCatalog->detailSectorNo[i] == 0 )
                {
                        break;
                }

                if( PCatalog->detailSectorNo[i] > 71 )
                {
                        return ERR_DETAIL_SECTORNO;                                                 //明细扇区号超限
                }

                sectorTimes[PCatalog->detailSectorNo[i]]++;                              //明细区
                PCatalog->sctArray[k++] = PCatalog->detailSectorNo[i];
                PCatalog->recSctNum++;
        }

        if( PCatalog->copiesSectorNo > 71 )                                          //20110803
        {
                return ERR_COPIES_SECTORNO;                                                     //份记录扇区号超限
        }
        if( PCatalog->copiesSectorNo )
        {
                sectorTimes[PCatalog->copiesSectorNo]++;                                 //份记录区
                PCatalog->sctArray[k++] = PCatalog->copiesSectorNo;
                PCatalog->recSctNum++;
        }

        if( PCatalog->linkageSectorNo > 70 )
        {
                return ERR_LINKAGE_SECTORNO;                                                    //联动扇区号超限
        }

        if( PCatalog->linkageSectorNo )
        {
                sectorTimes[PCatalog->linkageSectorNo]++;                                //联动扇区
                sectorTimes[PCatalog->linkageSectorNo + 1]++;                            //联动扇区（考勤机）
        }

        if( sectorTimes[0] )
        {
                return ERR_USE_SECTOR0;                                                         //不能使用0扇区
        }

        for( i = 1; i < 72; i++ )
        {
                if( sectorTimes[i] > 1 )
                {
                        return ERR_SECTOROVERLAP;                                                   //扇区冲突;
                }
        }

// 对记录扇区由小到大排序:冒泡

        int tVal = 0;
        if(PCatalog->recSctNum > 1)
        {
                for( i = 0; i < PCatalog->recSctNum; i++ )
                {
                        for(j = 1; j < PCatalog->recSctNum - i; j++)
                        {
                                if(PCatalog->sctArray[j-1] > PCatalog->sctArray[j])
                                {
                                        tVal = PCatalog->sctArray[j-1];
                                        PCatalog->sctArray[j-1] = PCatalog->sctArray[j];
                                        PCatalog->sctArray[j] = tVal;
                                }
                        }
                }
        }

        for( i = 0; i < PCatalog->recSctNum; i++ )
        {
                for( j = 0; j < 3; j++ )
                {
                        PCatalog->recArray[i * 3 + j] = PCatalog->sctArray[i] * 4 + j;   //明细绝对块号
//			printf("jj = %d,%d\n", i * 3 + j, PoffLineCatalog->sctArray[i] * 4 + j);
                }
        }

        for(i = 0; i < PCatalog->recSctNum; i++)
        {
//		printf("i = %d\n", PoffLineCatalog->sctArray[i]);
        }
////////////////////////////////////////////

//绝对块号的计算：只包含数据块，每个逻辑扇区3个数据块

        for( i = 0; i < 5; i++ )
        {
                if( PCatalog->detailSectorNo[i] == 0 )
                {
                        break;
                }

                for( j = 0; j < 3; j++ )
                {
                        PCatalog->detailArray[i * 3 + j] = PCatalog->detailSectorNo[i] * 4 + j;   //明细绝对块号
                }
        }

        PCatalog->catalogArea = PCatalog->issuSectorNo * 4 + 2;               //目录绝对块号

        PCatalog->issuArea[0]   = PCatalog->issuSectorNo * 4 + 0;             //发行区绝对块号0
        PCatalog->issuArea[1]   = PCatalog->issuSectorNo * 4 + 1;             //发行区绝对块号1
        PCatalog->pubArea[0]	   = PCatalog->pubSectorNo * 4 + 0;              //公共区绝对块号0
        PCatalog->pubArea[1]	   = PCatalog->pubSectorNo * 4 + 1;              //公共区绝对块号1

        PCatalog->walletArea[0] = PCatalog->pubSectorNo * 4 + 2;              //钱包区绝对块号0
        PCatalog->walletArea[1] = PCatalog->sumSectorNo * 4 + 2;              //钱包区绝对块号1
        PCatalog->sumArea[0]	   = PCatalog->sumSectorNo * 4 + 0;              //累计区绝对块号0
        PCatalog->sumArea[1]	   = PCatalog->sumSectorNo * 4 + 1;              //累计区绝对块号1
        if( PCatalog->baseSectorNo )
        {
                PCatalog->baseArea[0]   = PCatalog->baseSectorNo * 4 + 0;         //基础区绝对块号0
                PCatalog->baseArea[1]   = PCatalog->baseSectorNo * 4 + 1;         //基础区绝对块号1
        }else
        {
                PCatalog->baseArea[0]   = 0;                                             //基础区绝对块号0
                PCatalog->baseArea[1]   = 0;                                             //基础区绝对块号1
        }
        if( PCatalog->subsidySectorNo )
        {
                PCatalog->subsidyArea[0]	   = PCatalog->subsidySectorNo * 4 + 0;  //补贴区绝对块号0
                PCatalog->subsidyArea[1]	   = PCatalog->subsidySectorNo * 4 + 1;  //补贴区绝对块号1
        }else
        {
                PCatalog->subsidyArea[0]	   = 0;                                         //补贴区绝对块号0
                PCatalog->subsidyArea[1]	   = 0;                                         //补贴区绝对块号1
        }
        if( PCatalog->baseSectorNo )
        {
                PCatalog->personArea = PCatalog->baseSectorNo * 4 + 2;            //个人区绝对块号
        }else
        {
                PCatalog->personArea = 0;                                                //个人区绝对块号
        }

        if( PCatalog->copiesSectorNo )                                                           //20110803
        {
                PCatalog->copiesArray[0]	   = PCatalog->copiesSectorNo * 4 + 0;
                PCatalog->copiesArray[1]	   = PCatalog->copiesSectorNo * 4 + 1;
        }else
        {
                PCatalog->copiesArray[0]	   = 0;
                PCatalog->copiesArray[1]	   = 0;
        }

        return ERR_OK;
}


static int readIssuInfo( PTissuInfo PissuInfo, int keymode, unsigned char *key )
{
        int				ret;
        unsigned int subdata = 0;
        unsigned char	*addr = PissuInfo->blockBuf;

        memset( PissuInfo, 0, sizeof( TissuInfo ) );

        ret = readDataBlock2( keymode, key, CCatalog.issuArea[0] / 4, CCatalog.issuArea[0] % 4, 32, addr );
        if( ret != ERR_OK )
        {
                return ret;
        }

        if( ( CalcCRC8( addr, 0 ) != 0 ) || ( CalcCRC8( addr + 16, 0 ) != 0 ) )
        {
                return ERR_CRC;
        }

        ret = BcdToUInt( addr, &subdata, 2 );
        if( ret != ERR_OK )
        {
                return ERR_APP_TYPE;            //应用类型标识无效
        }

        PissuInfo->appType = subdata;       //2B, BCD, 应用类型(卡种标识）8665:全国应用 8667:省应用
        addr			  += 2;

        ret = BcdToUInt( addr, &subdata, 2 );
        if( ret != ERR_OK )
        {
                return ERR_AREA_CODE;           //地区代码无效
        }

        PissuInfo->areaCode	   = subdata;   //2B, BCD, 3位电话区号+0
        addr				  += 2;

        ret = BcdToUInt( addr, &subdata, 4 );
        if( ret != ERR_OK )
        {
                return ERR_CARD_NO;             //卡顺序号格式无效
        }

        memset( PissuInfo->CardNo, 0, 16 );
        ret = HexToTHex( addr, PissuInfo->CardNo, 4, 0 );
        if( ret != 8 )
        {
                return ret;
        }
        PissuInfo->issuSerialNo	   = subdata;                           //4B, BCD, 卡顺序号
        addr					  += 4;

        PissuInfo->cardAuthCode = HexToUInt( addr, 4, 1 );  //4B, HEX, 卡认证码
        addr					  += 4;

        PissuInfo->useFlag = HexToUInt( addr, 1, 1 );          //1B, HEX, 启用标志, 0:未启用, 1:启用
        addr			  += 1;

        PissuInfo->deposit = HexToUInt( addr, 2, 1 );          //2B, HEX, 押金,单位:分
        addr			  += 2;

        addr = PissuInfo->blockBuf + 16;

        unsigned char	timebuf[8];
        struct TTime time1;

        ret = BcdToHex( addr, timebuf, 4 );    //4BCD, YYYYMMDD
        if( ret != 4 )
        {
                return ERR_TIMEFORMAT;                      //日期时间格式错误
        }

        time1.year	   = timebuf[0] * 100 + timebuf[1];
        time1.month	   = timebuf[2];
        time1.day	   = timebuf[3];
        time1.hour	   = 0;
        time1.minute   = 0;
        time1.second   = 0;
        HexToTHex( addr, PissuInfo->issuDateStr, 4, 0 );
        ret = TimeToUInt( &time1, &PissuInfo->issuDate, 2000, 1 );
        if( ret != ERR_OK )
        {
                return ERR_TIMEFORMAT;                      //日期时间格式错误
        }
        addr += 4;

        ret = BcdToHex( addr, timebuf, 4 );    //4BCD, YYYYMMDD
        if( ret != 4 )
        {
                return ERR_TIMEFORMAT;                      //日期时间格式错误
        }

        time1.year	   = timebuf[0] * 100 + timebuf[1];
        time1.month	   = timebuf[2];
        time1.day	   = timebuf[3];
        time1.hour	   = 23;
        time1.minute   = 59;
        time1.second   = 59;

        HexToTHex( addr, PissuInfo->endDateStr, 4, 0 );
        ret = TimeToUInt( &time1, &PissuInfo->endDate, 2000, 1 );
        if( ret != ERR_OK )
        {
                return ERR_TIMEFORMAT;                      //日期时间格式错误
        }
        addr += 4;

        ret = BcdToHex( addr, timebuf, 4 );    //4BCD, YYYYMMDD
        if( ret != 4 )
        {
                return ERR_TIMEFORMAT;                      //日期时间格式错误
        }

        time1.year	   = timebuf[0] * 100 + timebuf[1];
        time1.month	   = timebuf[2];
        time1.day	   = timebuf[3];
        time1.hour	   = 0;
        time1.minute   = 0;
        time1.second   = 0;

        HexToTHex( addr, PissuInfo->beginDateStr, 4, 0 );
        ret = TimeToUInt( &time1, &PissuInfo->beginDate, 2000, 1 );
        if( ret != ERR_OK )
        {
                return ERR_TIMEFORMAT;                                  //日期时间格式错误
        }
        addr += 4;

        PissuInfo->statFlag	   = HexToUInt( addr, 1, 1 );  //1B, HEX, 卡状态标志, 0:未启用 1:已启用 2:已停用 3:已退卡 4:黑名单卡
        addr				  += 1;

        PissuInfo->blackTimes  = HexToUInt( addr, 1, 1 );  //1B, HEX 黑名单次数
        addr				  += 1;

// 20140122 卡片类型: 0-用户卡， 1-管理卡，2-商户卡
        PissuInfo->cardType	   = HexToUInt( addr, 1, 1 );  //1B, HEX 黑名单次数
        addr				  += 1;

        return checkIssuInfo( PissuInfo );
}


static int checkIssuInfo( PTissuInfo PissuInfo )
{
        if( PissuInfo->areaCode % 10 )
        {
                return ERR_AREA_CODE;       //地区代码无效
        }

        if( PissuInfo->areaCode > 9999 )
        {
                return ERR_AREA_CODE;       //地区代码无效
        }

        if( PissuInfo->issuSerialNo > 99999999 )
        {
                return ERR_CARD_NO;         //卡顺序号无效
        }

        if( PissuInfo->useFlag > 1 )
        {
                return ERR_USEFLAG;         //启用标志无效
        }

        if( PissuInfo->deposit > 65535 )
        {
                return ERR_DEPOSIT;         //押金超范围
        }

        if( PissuInfo->statFlag > 4 )
        {
                return ERR_CARD_STATFLAG;   //卡片状态标志无效
        }

        if( PissuInfo->blackTimes > 255 )
        {
                return ERR_BLACK_TIMES;     //黑名单次数超范围
        }

        return ERR_OK;
}

// 计算卡操作密钥 2011-03-30
// keyA: M1 keyA密钥
// keyB: M1 keyB密钥

static int calcKey( unsigned char in[8], unsigned char out[16] )
{
        int				ret	   = -1;
        int				i	   = 0;
        unsigned char tmpIn[8];
        unsigned char keyB[16];
        unsigned char ioData[256];

        for( i = 0; i < 8; i++ )
        {
                tmpIn[i] = ~in[i];
        }

        if( card_config.isPsam == 0 )
        {
                if( statInfo.cardType== CARD_CPU )
                {
                        RunDes( 0, (char *)in, (char *)out, 8, (char *)card_config.devKey, 16 );
                        RunDes( 0, (char *)tmpIn, (char *)out + 8, 8, (char *)card_config.devKey, 16 );
                }else
                {
                        RunDes( 0, (char *)in, (char *)out, 8, (char *)card_config.devKey, 16 );
                }
                return ERR_OK;
        }

        if( statInfo.cardType == CARD_CPU ) // 20130813 edit
        {
                return ERR_OK;
        }

        memset(ioData, 0, sizeof(ioData));
        ret = PsamInitForDesCrypt( 0x07, 0x11, 0x00, 0x00, ioData, sizeof(ioData) );
        if( ret < ERR_OK )
        {
                return ERR_FALSE;
        }

        memcpy( keyB, in, 8 );
        memset(ioData, 0, sizeof(ioData));
        ret = PsamDesCrypt( 0x00, 0x00, 0x08, 0x00, keyB, sizeof(keyB) );
        if( ret < ERR_OK)
        {
                return ERR_FALSE;
        }

        memcpy( out, keyB, 6 );		// 取充值密钥的前6个字节作为卡操作密钥

        return ERR_OK;

}

static int getCpuAuthData( unsigned char keyType, unsigned char *inData, unsigned char *outData )
{
        unsigned char	*key;
        unsigned char	key_type, keyEdition;
        unsigned char	initData[8], ioData[256];
        int				ret;

        if( 0 == card_config.isPsam )
        {
                switch( keyType )
                {
                        case  KEY_TYPE_REQ:         //        1  //查询密钥
                                key = statInfo.reqKey;
                                break;
                        case  KEY_TYPE_DEAL:        //        2  //交易密钥
                                key = statInfo.dealKey;
                                break;
                        case  KEY_TYPE_ISSU:        //        3  //发行密钥
                                key = statInfo.writeManageSectorKey;
                                break;
                        case  KEY_TYPE_ISSU_READ:   //        4  //发行区只读
                                key = MasterKey.readManageSectorKey;
                                break;
                        case  KEY_TYPE_INTERNAL:    //        5  //内部认证密钥
                                key = statInfo.internalAuthKeybuf;
                                break;
                        case  KEY_TYPE_EXT:         //        6  //扩展区密钥
                                key = statInfo.writeManageSectorKey;
                                break;
                        case  KEY_TYPE_EXT_READ:    //        7  //扩展区只读
                                key = MasterKey.readManageSectorKey;
                                break;
                        default:
                                return ERR_PARAM;
                                break;
                }

                ret = RunDes( 0, (char *)inData, (char *)outData, 8, (char *)key, 16 );
                if( ret == 0 )
                {
                        return ERR_FALSE;
                }

                return ERR_OK;
        }

        switch( statInfo.scatteringFactor ) //密钥分散因子
        {
                case 0:                         //标准
                case 1:					//分散因子在之前已经分别计算出 20130426
                case 2:
                        memcpy( initData, statInfo.InData, 8 );
                        break;
                default:
                        return ERR_PARAM;
                        break;
        }

        switch( keyType )
        {
                case KEY_TYPE_INTERNAL:
                        key_type   = statInfo.keyType_Edition[0][0];
                        keyEdition = statInfo.keyType_Edition[0][1];
                        break;
                case KEY_TYPE_ISSU_READ:
                        key_type   = statInfo.keyType_Edition[1][0];
                        keyEdition = statInfo.keyType_Edition[1][1];
                        break;
                case KEY_TYPE_REQ:
                        key_type   = statInfo.keyType_Edition[2][0];
                        keyEdition = statInfo.keyType_Edition[2][1];
                        break;
                case KEY_TYPE_DEAL:
                        key_type   = statInfo.keyType_Edition[3][0];
                        keyEdition = statInfo.keyType_Edition[3][1];
                        break;
                case KEY_TYPE_ISSU:
                        key_type   = statInfo.keyType_Edition[4][0];
                        keyEdition = statInfo.keyType_Edition[4][1];
                        break;
                case KEY_TYPE_EXT_READ:
                        key_type   = statInfo.keyType_Edition[5][0];
                        keyEdition = statInfo.keyType_Edition[5][1];
                        break;
                case KEY_TYPE_EXT:
                        key_type   = statInfo.keyType_Edition[6][0];
                        keyEdition = statInfo.keyType_Edition[6][1];
                        break;
                default:
                        return ERR_PARAM;
                        break;
        }

        key_type |= 0x20;                                                               //加上分散级数

        memset(ioData, 0, sizeof(ioData));
        memcpy(ioData, statInfo.InData, sizeof(statInfo.InData));
        ret = PsamInitForDesCrypt( key_type, keyEdition, 0x08, 0x00, ioData, sizeof(ioData) );
        if( ret < ERR_OK )
        {
                return ERR_FALSE;
        }

        memset(ioData, 0, sizeof(ioData));
        ret = PsamDesCrypt( 0x00, 0x00, 0x08, 0x00, inData, sizeof(*inData) );
        if( ret < ERR_OK)
        {
                return ERR_FALSE;
        }

        outData = inData;

        return ERR_OK;

}

// surport cpu 20121019
int TFexternalAuthentication( unsigned char keyEdition, unsigned char keyIndex, unsigned char *data )
{
        unsigned char	cla	   = 0x00;
        unsigned char	ins	   = 0x82;

        unsigned char	apdu[128];
        memset( apdu, 0, 128 );

        unsigned char *addr = apdu;
        *addr = cla;
        addr++;

        *addr = ins;
        addr++;

        *addr = keyEdition;         //p1 密钥版本
        addr++;

        *addr = keyIndex;           //p2 密钥索引
        addr++;

        *addr = 8;                  //lc
        addr++;

        memcpy( addr, data, 8 );    // 加密后的随机数（认证数据）
        addr += 8;

        //APDU已准备好
        unsigned char	answerBuf[128];
        int				ret;

        switch( statInfo.readerType )
        {
                case WEDS_READER_CPU:
                        ret = wedsApduChannel2( apdu, 13, answerBuf, 128, 300 );
                        break;
                default:
                        return ERR_INVALIDREADER;
                        break;
        }

        if( ret == 2 ) //期望的返回字节数，SW1、SW2
        {
                ret = cpuErrCode( answerBuf );
                return ret;
        }else if( ret < 0 )
        {
                return ret;
        }else
        {
                return ERR_INVALID;                 //返回数据无效，无法解释
        }
}



static int checkPubInfo( PTpubInfo PpubInfo )
{
        if( PpubInfo->detailPoint > 15 )
        {
                return ERR_DETAIL_POINT;
        }

        if( PpubInfo->detailPoint != 0 && CCatalog.detailArray[PpubInfo->detailPoint] == 0 )
        {
                return ERR_DETAIL_POINT;
        }

        if( ( PpubInfo->validFlag != 0x35 ) && ( PpubInfo->validFlag != 0 ) )
        {
                return ERR_INVALID_FLAG;        //有效标志错误
        }
//	printf("hmdflag %02X\n",PpubInfo->blackFlag);
        if( ( PpubInfo->blackFlag != 1 ) && ( PpubInfo->blackFlag != 4 ) )
        {
                return ERR_BLACK_FLAG;          //黑名单标志非法
        }

        if( PpubInfo->dailyTotalPoint > 1 )
        {
                return ERR_DAILY_TOTAL_POINT;   //日累计指针非法
        }

        if( PpubInfo->walletPoint > 1 )
        {
                return ERR_WALLET_POINT;        //钱包指针非法
        }

        if( PpubInfo->subsidyPoint > 1 )
        {
                return ERR_SUBSIDY_POINT;       //补贴指针非法
        }

        return ERR_OK;
}


static int internalAuthentication()
{
        unsigned char data[8];
        int ret = 0, j = 0;
        int i=0;

        unsigned char random[4];

        for (j = 0; j < 2; j++)   //填充4字节伪随机数
        {
                *((short int *)(random + j*2)) = rand();
        }

        int len;

        unsigned char cla = 0x00;
        unsigned char ins = 0x88;

        unsigned char apdu[128];
        memset(apdu, 0, 128);

        unsigned char *addr = apdu;
        *addr = cla;
        addr++;

        *addr = ins;
        addr++;

        switch (statInfo.cpuCodeID)
        {
        case COSTF26:
                len = 10;

                *addr = 0; //p1 密钥版本
                addr++;

                *addr = 0x11; //20120801增加专用内部认证密钥
                addr++;

                *addr = 4; //lc
                addr++;
                memcpy(addr, random, 4);
                addr+= 4;

                break;
        case COSFM20:
        case COSTimeCOS_PBOC:  //20121228
        case COSTimeCOS_PBOC1: //20130812国电测试卡
                len = 14;

                *addr = 0; //p1 使用加密密钥进行加密运算
                addr++;

                *addr = statInfo.internalAuthKeyID;
                addr++;

                *addr = 8; //lc
                addr++;

                memcpy(addr, random, 4);
                addr+= 4;

                memset(addr, 0, 4);
                addr+= 4;

                break;
        default:
                return ERR_INVALIDCARDTYPE;
                break;

        }

        *addr = 0x08;

        //APDU已准备好
        unsigned char answerBuf[128];

        switch (statInfo.readerType)
        {
                case WEDS_READER_CPU:
                        ret = wedsApduChannel2(apdu, len, answerBuf, 128, 300);
                        break;
                default:
                        return ERR_INVALIDREADER;
                        break;
        }

        if ((ret == 10) && (answerBuf[8] == 0x90) && (answerBuf[9] == 0)) //期望的返回字节数，SW1、SW2
        {
                ;
        }
        else if (ret == 2) //期望的返回字节数，SW1、SW2
        {
                ret = cpuErrCode(answerBuf);
                if (ret == ERR_GET_RESPONSE) //返回63xx
                {
                        if (answerBuf[1] == 0x08) //8个字节内部认证的结果
                        {
                                ret = getResponse(answerBuf, 8);
                                if (ret < 0)
                                {
                                        return ret;
                                }
                                else
                                {
                                        if (ret != 8)
                                        {
                                                return ERR_INVALID; //返回数据无效，无法解释
                                        }
                                }
                        }
                        else
                        {
                                return ERR_INVALID; //返回数据无效，无法解释
                        }
                }
                else
                {
                        return ret;
                }
        }
        else if (ret < 0)
        {
                return ret;
        }
        else
        {
                return ERR_INVALID; //返回数据无效，无法解释
        }

        memset(data, 0, 8);
        memcpy(data, random, 4);

        //20130607调整，适应内部认证使用加密密钥或解密密钥（PSAM）
      //  if (statInfo.internalAuthKeyType)  //使用解密密钥
        if(0)
        {
                ret = getCpuAuthData(KEY_TYPE_INTERNAL, answerBuf, answerBuf); //20121213
        }
        else //使用加密密钥
        {
                ret = getCpuAuthData(KEY_TYPE_INTERNAL, data, data); //20121213
        }

        if (ret != ERR_OK)
        {
                return ret;
        }


        if (memcmp(data, answerBuf, 8))
        {
                return ERR_FALSE;
        }

        return ERR_OK;

}


static int ol_checkPubInfo( PTpubInfo PpubInfo )
{
        if( PpubInfo->cardStat > 6 )
        {
                return ERR_CARD_STAT;   //卡状态非法值
        }

        if( PpubInfo->dealDate > 1231 )  // mon*100+day
        {
                return ERR_DEAL_DATE;   //交易日期非法
        }

        return ERR_OK;
}

static int new_readPub( int keymode, unsigned char *key )
{
        unsigned char	keyType = pubInfo.keyType;
        int				ret, ret1, ret2;
        unsigned char	*addr, buf[32];

        TpubInfo		pubInfo1;
        TpubInfo		pubInfo2;

        memset( buf, 0, sizeof( buf ) );
printf("11 == %d, %d\n", CCatalog.pubArea[0] / 4, CCatalog.pubArea[0] % 4);
        addr   = buf;
        ret	   = readDataBlock2( keymode, key, CCatalog.pubArea[0] / 4, CCatalog.pubArea[0] % 4, 32, addr );
        if( ret != ERR_OK )
        {
                return ret;
        }

printf("22\n");


//dbgShowHexData( addr, 32, 1, '<', "new_readPub" );

// online version check
        if( CalcCRC16( addr, 16, 0 ) != 0 )
        {
                printf( "1\n" );
                ret1 = ERR_CRC;
        }else
        {
                printf( "2\n" );
                memset( &pubInfo1, 0, sizeof( pubInfo1 ) );
                pubInfo1.validFlag = *addr >> 7;                        // 1b,公共块有效标志。1：有效，0：无效
                pubInfo1.updateNo  = ( ( *addr >> 5 ) & 0x03 );         // 2b,更新序号
                pubInfo1.recNum	   = ( *addr & 0x1F );                  // 5b,记录数
                addr			  += 1;

                pubInfo1.cardStat  = *addr >> 4;                        // 4b,卡状态
                pubInfo1.mealNo	   = *addr & 0x0F;                      // 4b,餐别时段
                addr			  += 1;

                pubInfo1.walletAmt = HexToInt( addr, 3, 1 );   //  3B,卡余额，大端
                addr			  += 3;

                BcdToUInt(addr,& pubInfo1.dealDate, 2);
                addr += 2;

                pubInfo1.dayAmt	   = HexToUInt( addr, 2, 1 );  //2B,日累计额
                addr			  += 2;

                pubInfo1.mealAmt   = HexToUInt( addr, 2, 1 );  //2B,餐累计额
                addr			  += 2;

                pubInfo1.mealTimes = *addr >> 5;                        // 3b,餐累计次
                pubInfo1.dayTimes  = *addr & 0x1F;                      // 5b,日累计次
                addr			  += 1;

                pubInfo1.cardLsh   = HexToUInt( addr, 2, 1 );  // 2B,卡交易计数
                addr			  += 2;

                ret1 = ol_checkPubInfo( &pubInfo1 );
                if( ret1 != ERR_OK )
                {
                        printf( "3\n" );
                        return ret1;
                }
        }

        addr = buf + 16;
        if( CalcCRC16( addr, 16, 0 ) != 0 )
        {
                printf( "4\n" );
                ret2 = ERR_CRC;
        }else
        {
                printf( "5\n" );
                memset( &pubInfo2, 0, sizeof( pubInfo2 ) );
                pubInfo2.validFlag = *addr >> 7;                        // 1b,公共块有效标志。1：有效，0：无效
                pubInfo2.updateNo  = ( ( *addr >> 5 ) & 0x03 );         // 2b,更新序号
                pubInfo2.recNum	   = ( *addr & 0x1F );                  // 5b,记录数
                addr			  += 1;

                pubInfo2.cardStat  = *addr >> 4;                        // 4b,卡状态
                pubInfo2.mealNo	   = *addr & 0x0F;                      // 4b,餐别时段
                addr			  += 1;

                pubInfo2.walletAmt = HexToInt( addr, 3, 1 );   //  3B,卡余额，大端
                addr			  += 3;

                BcdToUInt(addr, &pubInfo2.dealDate, 2);
                addr			  += 2;

                pubInfo2.dayAmt	   = HexToUInt( addr, 2, 1 );  //2B,日累计额
                addr			  += 2;

                pubInfo2.mealAmt   = HexToUInt( addr, 2, 1 );  //2B,餐累计额
                addr			  += 2;

                pubInfo2.mealTimes = *addr >> 5;                        // 3b,餐累计次
                pubInfo2.dayTimes  = *addr & 0x1F;                      // 5b,日累计次
                addr			  += 1;

                pubInfo2.cardLsh   = HexToUInt( addr, 2, 1 );  // 2B,卡交易计数
                addr			  += 2;

                ret2 = ol_checkPubInfo( &pubInfo2 );
                if( ret2 != ERR_OK )
                {
                        printf( "6\n" );
                        return ret2;
                }
        }

        if( ret1 == ERR_CRC )
        {
                if( ret2 == ERR_OK ) //2通过校验
                {
                        if( pubInfo2.validFlag == 1 )
                        {
                                printf( "7\n" );
                                memcpy( &pubInfo, &pubInfo2, sizeof( pubInfo ) );
                                statInfo.currFlagPoint = 1;
                                pubInfo.keyType		   = keyType;
                                return ERR_NEW_CARD;
                        }else   //无效标志
                        {
                                printf( "8\n" );
                                statInfo.currFlagPoint = -1; //指针无效
                                return ERR_FLAG_INVALID;
                        }
                }else       //两个块均CRC错误
                {
                        printf( "9\n" );
                        goto CHECK_OLD;
                }
        }else           //OK
        {
                if( ret2 == ERR_CRC ) //1通过校验
                {
                        if( pubInfo1.validFlag == 1 )
                        {
                                printf( "10\n" );
                                memcpy( &pubInfo, &pubInfo1, sizeof( pubInfo ) );
                                statInfo.currFlagPoint = 0;
                                pubInfo.keyType		   = keyType;
                                return ERR_NEW_CARD;
                        }else //无效标志
                        {
                                printf( "11\n" );
                                statInfo.currFlagPoint = -1; //指针无效
                                return ERR_FLAG_INVALID;
                        }
                }
        }

//两个块均通过校验，需要判断标志
        if( pubInfo1.validFlag == 1 )
        {
                if( pubInfo2.validFlag != 1 ) //1有效
                {

                        memcpy( &pubInfo, &pubInfo1, sizeof( pubInfo ) );
                        statInfo.currFlagPoint = 0;
                        pubInfo.keyType		   = keyType;
printf( "12\n");
                        return ERR_NEW_CARD;
                }
        }else
        {
                if( pubInfo2.validFlag != 1 )
                {
                        printf( "13\n" );
                        statInfo.currFlagPoint = -1; //指针无效
                        return ERR_FLAG_INVALID;
                }else
                {
                        printf( "14\n" );
                        memcpy( &pubInfo, &pubInfo2, sizeof( pubInfo ) );
                        statInfo.currFlagPoint = 1;
                        pubInfo.keyType		   = keyType;
                        return ERR_NEW_CARD;
                }
        }

//需要判断序号
        if( pubInfo1.updateNo == ( pubInfo2.updateNo + 1 ) % 4 ) //1有效
        {
                memcpy( &pubInfo, &pubInfo1, sizeof( pubInfo ) );
                statInfo.currFlagPoint = 0;
                pubInfo.keyType		   = keyType;
printf( "15\n");
                return ERR_NEW_CARD;
        }else if( pubInfo2.updateNo == ( pubInfo1.updateNo + 1 ) % 4 ) //2有效
        {
                printf( "16\n" );
                memcpy( &pubInfo, &pubInfo2, sizeof( pubInfo ) );
                statInfo.currFlagPoint = 1;
                pubInfo.keyType		   = keyType;
                return ERR_NEW_CARD;
        }else
        {
                printf( "17\n" );
                statInfo.currFlagPoint = -1; //指针无效
                return ERR_FLAG_INVALID;
        }

// check if old version
CHECK_OLD:

        addr = buf;
        if( CalcCRC8( addr, 0 ) != 0 )
        {
                printf( "18\n" );
                ret1 = ERR_CRC;                                                 //CRC校验错误
        }else
        {
                printf( "19\n" );
                memset( &pubInfo1, 0, sizeof( pubInfo1 ) );
                pubInfo1.detailPoint   = HexToUInt( addr, 1, 1 );      //明细指针
                addr				  += 1;

                pubInfo1.totalCopies   = HexToUInt( addr, 2, 1 );      //累计次数
                addr				  += 2;

                pubInfo1.validFlag = HexToUInt( addr, 1, 1 );          //有效标志 0x35
                addr			  += 1;

                pubInfo1.totalSavingCopies = HexToUInt( addr, 2, 1 );  //累计充值次数
                addr					  += 2;

                pubInfo1.blackFlag = HexToUInt( addr, 1, 1 );          //黑名单标志
                addr			  += 1;

                pubInfo1.totalSavingAmt	   = HexToUInt( addr, 4, 1 );  //累计充值金额
                addr					  += 4;

                pubInfo1.dailyTotalPoint = *addr & 0x01;

                pubInfo1.subsidyPoint = ( *addr >> 1 ) & 0x01;

                pubInfo1.walletPoint = ( *addr >> 2 ) & 0x01;

                pubInfo1.copiesPoint = ( *addr >> 3 ) & 0x01;

                pubInfo1.linkagePoint = ( *addr >> 4 ) & 0x01;

                addr += 3;

                pubInfo1.updateNo  = HexToUInt( addr, 1, 1 ); //更新序号
                addr			  += 1;

                ret1 = checkPubInfo( &pubInfo1 );
                if( ret1 != ERR_OK )
                {
                        printf( "20\n" );
                        return ret1;
                }
        }

        addr = buf + 16;
        if( CalcCRC8( addr, 0 ) != 0 )
        {
                printf( "21\n" );
                ret2 = ERR_CRC;                                                 //CRC校验错误
        }else
        {
                printf( "22\n" );
                memset( &pubInfo2, 0, sizeof( pubInfo2 ) );
                pubInfo2.detailPoint   = HexToUInt( addr, 1, 1 );      //明细指针
                addr				  += 1;

                pubInfo2.totalCopies   = HexToUInt( addr, 2, 1 );      //累计次数
                addr				  += 2;

                pubInfo2.validFlag = HexToUInt( addr, 1, 1 );          //有效标志 0x35
                addr			  += 1;

                pubInfo2.totalSavingCopies = HexToUInt( addr, 2, 1 );  //累计充值次数
                addr					  += 2;

                pubInfo2.blackFlag = HexToUInt( addr, 1, 1 );          //黑名单标志
                addr			  += 1;

                pubInfo2.totalSavingAmt	   = HexToUInt( addr, 4, 1 );  //累计充值金额
                addr					  += 4;

                pubInfo2.dailyTotalPoint = *addr & 0x01;

                pubInfo2.subsidyPoint = ( *addr >> 1 ) & 0x01;

                pubInfo2.walletPoint = ( *addr >> 2 ) & 0x01;

                pubInfo2.copiesPoint = ( *addr >> 3 ) & 0x01;

                pubInfo2.linkagePoint = ( *addr >> 4 ) & 0x01;

                addr += 3;

                pubInfo2.updateNo  = HexToUInt( addr, 1, 1 ); //更新序号
                addr			  += 1;

                ret2 = checkPubInfo( &pubInfo2 );
                if( ret2 != ERR_OK )
                {
                        printf( "23\n" );
                        return ret2;
                }
        }

        if( ret1 == ERR_CRC )
        {
                if( ret2 == ERR_OK ) //2通过校验
                {
                        if( pubInfo2.validFlag == 0x35 )
                        {
                                printf( "24\n" );
                                memcpy( &pubInfo, &pubInfo2, sizeof( pubInfo ) );
                                statInfo.currFlagPoint = 1;
                                pubInfo.keyType		   = keyType;
                                return ERR_OLD_CARD;
                        }else   //无效标志
                        {
                                printf( "25\n" );
                                statInfo.currFlagPoint = -1; //指针无效
                                return ERR_FLAG_INVALID;
                        }
                }else       //两个块均CRC错误
                {
                        printf( "26\n" );
                        return ret1;
                }
        }else           //OK
        {
                if( ret2 == ERR_CRC ) //1通过校验
                {
                        if( pubInfo1.validFlag == 0x35 )
                        {
                                printf( "27\n" );
                                memcpy( &pubInfo, &pubInfo1, sizeof( pubInfo ) );
                                statInfo.currFlagPoint = 0;
                                pubInfo.keyType		   = keyType;
                                return ERR_OLD_CARD;
                        }else //无效标志
                        {
                                printf( "28\n" );
                                statInfo.currFlagPoint = -1; //指针无效
                                return ERR_FLAG_INVALID;
                        }
                }
        }

        //两个块均通过校验，需要判断标志
        if( pubInfo1.validFlag == 0x35 )
        {
                if( pubInfo2.validFlag != 0x35 ) //1有效
                {
                        printf( "29\n" );
                        memcpy( &pubInfo, &pubInfo1, sizeof( pubInfo ) );
                        statInfo.currFlagPoint = 0;
                        pubInfo.keyType		   = keyType;
                        return ERR_OLD_CARD;
                }
        }else
        {
                if( pubInfo2.validFlag != 0x35 )
                {
                        printf( "30\n" );
                        statInfo.currFlagPoint = -1; //指针无效
                        return ERR_FLAG_INVALID;
                }else
                {
                        printf( "31\n" );
                        memcpy( &pubInfo, &pubInfo2, sizeof( pubInfo ) );
                        statInfo.currFlagPoint = 1;
                        pubInfo.keyType		   = keyType;
                        return ERR_OLD_CARD;
                }
        }
        //需要判断序号
        if( pubInfo1.updateNo == ( pubInfo2.updateNo + 1 ) % 256 ) //1有效
        {
                printf( "32\n" );
                memcpy( &pubInfo, &pubInfo1, sizeof( pubInfo ) );
                statInfo.currFlagPoint = 0;
                pubInfo.keyType		   = keyType;
                return ERR_OLD_CARD;
        }else if( pubInfo2.updateNo == ( pubInfo1.updateNo + 1 ) % 256 ) //2有效
        {
                printf( "33\n" );
                memcpy( &pubInfo, &pubInfo2, sizeof( pubInfo ) );
                statInfo.currFlagPoint = 1;
                pubInfo.keyType		   = keyType;
                return ERR_OLD_CARD;
        }else
        {
                printf( "34\n" );
                statInfo.currFlagPoint = (char )-1; //指针无效
                return ERR_FLAG_INVALID;
        }
        printf( "35\n" );

        return ERR_FALSE;
}

// 校验公共块数据合法性
static int check_pub_data( )
{
        int			ret, point;
        TpubInfo	pubTmp;
        unsigned char	*addr = NULL;

        memcpy( &pubTmp, &pubInfo, sizeof( pubInfo ) );

        if(pubTmp.walletAmt > 0x7FFFFF || pubTmp.walletAmt < (-0x800000))
        {
                return ERR_OUT_BAN;
        }

        if(pubTmp.mealTimes > 7)
        {
                return ERR_OUT_BAN;
        }

        *addr = (pubTmp.mealTimes << 5) + pubTmp.dayTimes;
        addr++;

//	printf("d = %d\n", pubTmp.cardLsh);

        UIntToHex( pubTmp.cardLsh, addr, 2, 1 );
        addr += 2;

        CalcCRC16(pubTmp.blockBuf, 14, 1);

//dbgShowHexData(pubTmp.blockBuf,16,1,0, NULL);


        point = ( statInfo.currFlagPoint + 1 ) % 2;

        ret = writeDataBlock2(pubInfo.keyType, statInfo.dealKey, CCatalog.pubArea[point]/4, CCatalog.pubArea[point]%4, 16, pubTmp.blockBuf);
        if( ret != ERR_OK )
        {
                return ret;
        }

        pubInfo.updateNo = pubTmp.updateNo;
        statInfo.currFlagPoint = ( statInfo.currFlagPoint + 1 ) % 2;

// 置另外一块为无效
        pubTmp.validFlag   = 0;
        point = ( statInfo.currFlagPoint + 1 ) % 2;
        addr = pubTmp.blockBuf;
        *addr = ((pubTmp.validFlag << 7) & 0x80) + ((pubTmp.updateNo << 5) & 0x60) + (pubTmp.recNum & 0x1F);

        CalcCRC16(pubTmp.blockBuf, 14, 1);

//dbgShowHexData(pubTmp.blockBuf,16,1,0, NULL);

        ret = writeDataBlock2(pubInfo.keyType, statInfo.dealKey, CCatalog.pubArea[point]/4, CCatalog.pubArea[point]%4, 16, pubTmp.blockBuf);
        if( ret != ERR_OK )
        {
                return ERR_HALF_OK;  // 看作更新成功
        }

        return ERR_OK;
}


static int new_writePub( )
{
        int			ret, point;
        TpubInfo	pubTmp;
        unsigned char	*addr = NULL;

        memcpy( &pubTmp, &pubInfo, sizeof( pubInfo ) );
        addr = pubTmp.blockBuf;

        pubTmp.updateNo = ( pubTmp.updateNo + 1 ) % 4;
//printf("a = %d, %d, %d\n", pubTmp.validFlag,pubTmp.updateNo,pubTmp.recNum);
        *addr = ((pubTmp.validFlag << 7) & 0x80) + ((pubTmp.updateNo << 5) & 0x60) + (pubTmp.recNum & 0x1F);
        addr += 1;

//	printf("b = %d, %d\n", pubTmp.cardStat,pubTmp.mealNo);

        *addr = ((pubTmp.cardStat << 4) & 0xF0) + (pubTmp.mealNo & 0x0F);
        addr += 1;

//	printf("c = %d, %d, %d, %d\n", pubTmp.walletAmt,pubTmp.dealDate,pubTmp.dayAmt,pubTmp.mealAmt);

        IntToHex( pubTmp.walletAmt, addr, 3, 1 );	 // 大端模式
        addr += 3;

        UIntToBcd( pubTmp.dealDate, addr, 2 );
        addr += 2;

        UIntToHex( pubTmp.dayAmt, addr, 2, 1 );
        addr += 2;

        UIntToHex( pubTmp.mealAmt, addr, 2, 1 );
        addr += 2;

        *addr = (pubTmp.mealTimes << 5) + pubTmp.dayTimes;
        addr++;

//	printf("d = %d\n", pubTmp.cardLsh);

        UIntToHex( pubTmp.cardLsh, addr, 2, 1 );
        addr += 2;

        CalcCRC16(pubTmp.blockBuf, 14, 1);

//dbgShowHexData(pubTmp.blockBuf,16,1,0, NULL);


        point = ( statInfo.currFlagPoint + 1 ) % 2;

        ret = writeDataBlock2(pubInfo.keyType, statInfo.dealKey, CCatalog.pubArea[point]/4, CCatalog.pubArea[point]%4, 16, pubTmp.blockBuf);
        if( ret != ERR_OK )
        {
                return ret;
        }

        pubInfo.updateNo = pubTmp.updateNo;
        statInfo.currFlagPoint = ( statInfo.currFlagPoint + 1 ) % 2;

// 置另外一块为无效
        pubTmp.validFlag   = 0;
        point = ( statInfo.currFlagPoint + 1 ) % 2;
        addr = pubTmp.blockBuf;
        *addr = ((pubTmp.validFlag << 7) & 0x80) + ((pubTmp.updateNo << 5) & 0x60) + (pubTmp.recNum & 0x1F);

        CalcCRC16(pubTmp.blockBuf, 14, 1);

//dbgShowHexData(pubTmp.blockBuf,16,1,0, NULL);

        ret = writeDataBlock2(pubInfo.keyType, statInfo.dealKey, CCatalog.pubArea[point]/4, CCatalog.pubArea[point]%4, 16, pubTmp.blockBuf);
        if( ret != ERR_OK )
        {
                return ERR_HALF_OK;  // 看作更新成功
        }

        return ERR_OK;
}




static int writePwdInfo(PTpwdInfo PpwdInfo)
{
        int ret;

        unsigned char *addr = PpwdInfo->blockBuf;

        memset(PpwdInfo->blockBuf, 0, 16);

        if (PpwdInfo->name[11] != 0)
        {
                return ERR_PARAM;
        }

        unsigned int tmp;
        int flag = 0;
        int i;

        if (PpwdInfo->pwdValid) //密码有效
        {
                for (i = 0; i < 3; i++)
                {
                        tmp = PpwdInfo->pwd[i] >> 4;

                        if (flag == 0)
                        {
                                switch (tmp)
                                {
                                        case 0: case 1: case 2: case 3: case 4:
                                        case 5: case 6: case 7: case 8: case 9:
                                        break;
                                case 0x0F:
                                        flag = 1;
                                        //if (i < 3) //密码最少4位数字
                                        if (i < 2) //密码最少4位数字  20150626
                                        {
                                                PpwdInfo->pwdValid = 0;
                                                return ERR_PARAM;  //20151212
                                        }

                                        break;
                                default:
                                        PpwdInfo->pwdValid = 0;
                                        return ERR_PASSWORD_FORMAT;
                                        break;
                                }
                        }
                        else if (tmp != 0x0F)
                        {
                                PpwdInfo->pwdValid = 0;
                                return ERR_PASSWORD_FORMAT;  //20151212
                        }

                        tmp = PpwdInfo->pwd[i] & 0x0F;

                        if (flag == 0)
                        {
                                switch (tmp)
                                {
                                case 0: case 1: case 2: case 3: case 4:
                                case 5: case 6: case 7: case 8: case 9:
                                        break;
                                case 0x0F:
                                        flag = 1;
                                        if (i < 2) //密码最少4位数字
                                        {
                                                PpwdInfo->pwdValid = 0;
                                                return ERR_PARAM;  //20151212
                                        }

                                        break;
                                default:
                                        PpwdInfo->pwdValid = 0;
                                        return ERR_PASSWORD_FORMAT;  //20151212
                                        break;
                                }
                        }
                        else if (tmp != 0x0F)
                        {
                                PpwdInfo->pwdValid = 0;
                                return ERR_PASSWORD_FORMAT;  //20151212
                        }
                }
        }
        else
        {
                memset(PpwdInfo->pwd, 0xff, 3);
        }

        memcpy(addr, PpwdInfo->name, strlen((char *)PpwdInfo->name));
        addr+= 11;

        memcpy(addr, PpwdInfo->pwd, 3);

        addr = PpwdInfo->blockBuf;
        addr+= 6;

        unsigned char xor1[8] = {0xa8, 0x67, 0x65, 0x31, 0x9b, 0xe3, 0x11, 0x23};
        unsigned char key1[16] = {0x5c, 0x1c, 0x28, 0x78, 0x38, 0x34, 0x17, 0x9d,
                                                                0x66, 0x43, 0x12, 0x65, 0x90, 0x11, 0x42, 0x06};

        for (i = 0; i < 8; i++) //混杂
        {
                key1[2*i] = key1[2*i+1] ^ xor1[7-i];
                key1[2*i+1] = key1[2*i] ^ xor1[i];
        }

        for (i = 0; i < 8; i++)
        {
                addr[i] ^= xor1[i];
        }

        ret = RunTea(0, (char *)addr, (char *)addr, 8, (char *)key1, 16);
        if (ret != 1)
        {
                return ERR_FALSE;
        }

        addr = PpwdInfo->blockBuf;
        CalcCRC16(addr, 14, 1);

        return writeDataBlock2(pubInfo.keyType, statInfo.dealKey, CCatalog.walletArea[0]/4,
                                                                        CCatalog.walletArea[0]%4, 16, PpwdInfo->blockBuf);

}


static int readPwdInfo(PTpwdInfo PpwdInfo, int keymode, unsigned char *key)
{
        int ret, i;

        unsigned char *buf = PpwdInfo->blockBuf;
        unsigned char *addr = buf;

        memset(PpwdInfo, 0, sizeof(TpwdInfo));

        ret = readDataBlock2(keymode, key, CCatalog.walletArea[0]/4, CCatalog.walletArea[0]%4, 16, addr);
        if (ret != ERR_OK)
        {
                return ret;
        }

        if( CalcCRC16( addr, 16, 0 ) != 0 )
        {
                return ERR_CRC; //CRC校验错误
        }

        addr+= 6;
        unsigned char xor1[8] = {0xa8, 0x67, 0x65, 0x31, 0x9b, 0xe3, 0x11, 0x23};
        unsigned char key1[16] = {0x5c, 0x1c, 0x28, 0x78, 0x38, 0x34, 0x17, 0x9d,
                                                                0x66, 0x43, 0x12, 0x65, 0x90, 0x11, 0x42, 0x06};

        for (i = 0; i < 8; i++) //混杂
        {
                key1[2*i] = key1[2*i+1] ^ xor1[7-i];
                key1[2*i+1] = key1[2*i] ^ xor1[i];
        }

        ret = RunTea(1, (char *)addr, (char *)addr, 8, (char *)key1, 16);
        if (ret != 1)
        {
                return ERR_FALSE;
        }

        for (i = 0; i < 8; i++)
        {
                addr[i] ^= xor1[i];
        }

        addr = buf;
        memcpy(PpwdInfo->name, addr, 11);
        addr+= 11;

        memcpy(PpwdInfo->pwd, addr, 3);

        unsigned int tmp;
        int flag = 0;

        PpwdInfo->pwdValid = 1;
        for (i = 0; i < 3; i++)
        {
                tmp = PpwdInfo->pwd[i] >> 4;

                if (flag == 0)
                {
                        switch (tmp)
                        {
                                case 0: case 1: case 2: case 3: case 4:
                                case 5: case 6: case 7: case 8: case 9:
                                break;
                        case 0x0F:
                                flag = 1;
                                //if (i < 3) //密码最少4位数字
                                if (i < 2) //密码最少4位数字  20150626
                                {
                                        PpwdInfo->pwdValid = 0;
                                }

                                break;
                        default:
                                PpwdInfo->pwdValid = 0;
                                break;
                        }
                }
                else if (tmp != 0x0F)
                {
                        PpwdInfo->pwdValid = 0;
                }

                tmp = PpwdInfo->pwd[i] & 0x0F;

                if (flag == 0)
                {
                        switch (tmp)
                        {
                                case 0: case 1: case 2: case 3: case 4:
                                case 5: case 6: case 7: case 8: case 9:
                                break;
                        case 0x0F:
                                flag = 1;
                                if (i < 2) //密码最少4位数字
                                {
                                        PpwdInfo->pwdValid = 0;
                                }

                                break;
                        default:
                                PpwdInfo->pwdValid = 0;
                                break;
                        }
                }
                else if (tmp != 0x0F)
                {
                        PpwdInfo->pwdValid = 0;
                }
        }

        return ERR_OK;
}


// 卡片升级
// flag: 0-公共块两块均更新，1-其一已更新，仅更新第二块
static int update_card( int flag )
{
        PTpubInfo		tmppub = NULL;
        TpwdInfo 		pwdInfo;
        unsigned char	point  = 0;
        unsigned char	*addr  = NULL;
        int ret = -1;

// 有效状态均为有效，更新序号为0和1，卡状态为6和1
//	unsigned char buf[] =
//	{
//		0x80, 0x60, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//		0xA0, 0x10, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
//	};

// debug ,充值 1000
        unsigned char buf[] =
        {
                0x80, 0x60, 0x01, 0x86, 0xA0, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0xA0, 0x10, 0x01, 0x86, 0xA0, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00//,
//		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x34, 0x56, 0x00, 0x00
        };

        memset(&pwdInfo, 0, sizeof(pwdInfo));
        memcpy(pwdInfo.name, "Online Test", 11);
        memcpy(pwdInfo.pwd, "\x12\x34\x56", 3);
        pwdInfo.pwdValid = 1;

        addr = buf;
        CalcCRC16( addr, 14, 1 );
//leeDebugData(addr,16,16,1);
        addr += 16;
        CalcCRC16( addr, 14, 1 );
//	leeDebugData(addr,16,16,1);
//	addr += 16;
//	Card_Chb1.CalcCRC16( addr, 14, 1 );

// 写公共区
        if( flag == 0 )
        {
                addr	   = buf;
                point = ( statInfo.currFlagPoint + 1 ) % 2;
                ret	   = writeDataBlock2( pubInfo.keyType, statInfo.dealKey,
                                                       CCatalog.pubArea[point] / 4, CCatalog.pubArea[point] % 4,
                                                       16, addr );
                if( ret != ERR_OK )
                {
                        return ret;
                }

                if( point == 0 )
                {
                        point = 1;
                }else
                {
                        point = 0;
                }

                addr	  += 16;
                ret	   = writeDataBlock2( pubInfo.keyType, statInfo.dealKey,
                                                       CCatalog.pubArea[point] / 4, CCatalog.pubArea[point] % 4,
                                                       16, addr );
                if( ret != ERR_OK )
                {
                        return ret;
                }

                ret	   = writePwdInfo(&pwdInfo);
                if( ret != ERR_OK )
                {
                        return ret;
                }

        }
        else
        {
                addr	   = buf + 16;
                point = ( statInfo.currFlagPoint + 1 ) % 2;
                ret	   = writeDataBlock2( pubInfo.keyType, statInfo.dealKey,
                                                       CCatalog.pubArea[point] / 4, CCatalog.pubArea[point] % 4,
                                                       16, addr );
                if( ret != ERR_OK )
                {
                        return ret;
                }

                ret	   = writePwdInfo(&pwdInfo);
                if( ret != ERR_OK )
                {
                        return ret;
                }
        }

// 重构数据:第二块为有效块
        addr	   = buf + 16;
        tmppub = &pubInfo;
        tmppub->validFlag = *addr >> 7;                        // 1b,公共块有效标志。1：有效，0：无效
        tmppub->updateNo  = ( ( *addr >> 5 ) & 0x03 );         // 2b,更新序号
        tmppub->recNum	   = ( *addr & 0x1F );                  // 5b,记录数
        addr			  += 1;

        tmppub->cardStat  = *addr >> 4;                        // 4b,卡状态
        tmppub->mealNo	   = *addr & 0x0F;                      // 4b,餐别时段
        addr			  += 1;

        tmppub->walletAmt = HexToInt( addr, 3, 1 );   //  3B,卡余额，大端
        addr			  += 3;

        BcdToUInt(addr,& tmppub->dealDate, 2);
        addr += 2;

        tmppub->dayAmt	   = HexToUInt( addr, 2, 1 );  //2B,日累计额
        addr			  += 2;

        tmppub->mealAmt   = HexToUInt( addr, 2, 1 );  //2B,餐累计额
        addr			  += 2;

        tmppub->mealTimes = *addr >> 5;                        // 3b,餐累计次
        tmppub->dayTimes  = *addr & 0x1F;                      // 5b,日累计次
        addr			  += 1;

        tmppub->cardLsh   = HexToUInt( addr, 2, 1 );  // 2B,卡交易计数
        addr			  += 2;

        ret = ol_checkPubInfo( tmppub );
        if(ret != ERR_OK)
        {
                return ret;
        }

        statInfo.currFlagPoint = 1;

        return ret;

}


// 脱机卡片预读接口
// 上海酬勤 欣能cpu卡用做联机消费使用，卡片做内部认证；读取卡片逻辑卡号 20161025
 int deal_card_std( )
{
        int ret, i = 0;
        TCatalog tmpCatalog;
        TissuInfo tmpIssuInfo;
        unsigned char key[16];
        unsigned char	InData[8];

        ret = readCatalogInfo( &tmpCatalog, KEY_TYPE_A,MasterKey.readManageSectorKey );
        if( ret != ERR_OK )
        {
                return ret;
        }
        memcpy( &CCatalog, &tmpCatalog, sizeof( CCatalog ) );

        ret = readIssuInfo( &tmpIssuInfo, KEY_TYPE_A, MasterKey.readManageSectorKey );
        if( ret != ERR_OK )
        {
                return ret;
        }
        memcpy( &IssuInfo, &tmpIssuInfo, sizeof( IssuInfo ) );

        switch( statInfo.scatteringFactor )
        {
                case 0:
                        switch( statInfo.cardType )
                        {
                                case CARD_UIM:
                                        ret = UIntToHex( statInfo.card2, InData, 4, 1 );
                                        break;
                                default:
                                        ret = UIntToHex( statInfo.card2, InData, 4, 0 );
                                        break;
                        }

                        if( ret )
                        {
                                return ret;
                        }

                        ret = UIntToBcd( IssuInfo.issuSerialNo % 10000, InData + 4, 2 );
                        if( ret < 0 )
                        {
                                return ret;
                        }

                        ret = UIntToHex( IssuInfo.cardAuthCode / 0x10000, InData + 6, 2, 1 ); //大端
                        if( ret < 0 )
                        {
                                return ret;
                        }
                        break;

                case 1:                                                     //物理卡号作为分散因子 20130426 add
                        ret = UIntToHex( statInfo.card1 ^ 0x8592A0C7, InData, 4, 0 );
                        if( ret < 0 )
                        {
                                return ret;
                        }

                        ret = UIntToHex( statInfo.card1 ^ statInfo.card2 ^ 0xA3F39563, InData + 4, 4, 1 );
                        if( ret < 0 )
                        {
                                return ret;
                        }
                        break;

                case 2:                                                                 //密钥对卡片非接ATR的前8字节做分散 20130607 国电的握奇卡
                        memset( InData, 0xaa, 8 );
                        UIntToHex( statInfo.card2, InData + 4, 4, 0 );   //小端模式

                        if( statInfo.card1 )
                        {
                                UIntToHex( statInfo.card1, InData, 4, 0 );
                        }
                        break;

                default:
                        return ERR_PARAM;
                        break;

        }

        memcpy( statInfo.InData, InData, 8 ); // 计算卡片分散因子 statInfo.InData
        memset( key, 0, 16 );
        if( !( card_config.isPsam == 1 && statInfo.cardType == CARD_CPU ) )
        {
                ret =  calcKey( statInfo.InData, key );              //交易密钥（原为充值密钥）
                if( ret != ERR_OK)
                {
                        return ret;
                }
                memcpy( statInfo.dealKey, key, 16 );
                pubInfo.keyType = KEY_TYPE_B;              // Bkey
        }

        if( statInfo.cardType == CARD_CPU )   //CPU卡内部认证 20121015
        {
                if( 0 == card_config.isPsam )
                {
                        unsigned char tmpData[8];

                        for( i = 0; i < 8; i++ )
                        {
                                tmpData[i] = ~statInfo.InData[i];
                        }

                        RunDes( 0, (char *)statInfo.InData, (char *)key, 8, (char *)card_config.interAuthKey, 16 );
                        RunDes( 0, (char *)tmpData, (char *)key + 8, 8, (char *)card_config.interAuthKey, 16 );

                        memcpy( (char *)statInfo.internalAuthKeybuf, (char *)key, 16 );
                }

                ret = internalAuthentication( );
                if( ret != ERR_OK )
                {
                        return ret;
                }
        }

        CCatalog.pubReadFlag = 1;

// IssuInfo.issuSerialNo 即为逻辑卡号

        return ERR_OK;

}


static int ol_deal_card( )
{
        int ret, i = 0;
        TCatalog tmpCatalog;
        TissuInfo tmpIssuInfo;
        unsigned char key[16];
        unsigned char	InData[8];

        ret = readCatalogInfo( &tmpCatalog, KEY_TYPE_A,MasterKey.readManageSectorKey );
        if( ret != ERR_OK )
        {
                return ret;
        }
        memcpy( &CCatalog, &tmpCatalog, sizeof( CCatalog ) );

        ret = readIssuInfo( &tmpIssuInfo, KEY_TYPE_A, MasterKey.readManageSectorKey );
        if( ret != ERR_OK )
        {
                return ret;
        }
        memcpy( &IssuInfo, &tmpIssuInfo, sizeof( IssuInfo ) );
        switch( statInfo.scatteringFactor )
        {
                case 0:
                        switch( statInfo.cardType )
                        {
                                case CARD_UIM:
                                        ret = UIntToHex( statInfo.card2, InData, 4, 1 );
                                        break;
                                default:
                                        ret = UIntToHex( statInfo.card2, InData, 4, 0 );
                                        break;
                        }

                        if( ret )
                        {
                                return ret;
                        }

                        ret = UIntToBcd( IssuInfo.issuSerialNo % 10000, InData + 4, 2 );
                        if( ret < 0 )
                        {
                                return ret;
                        }

                        ret = UIntToHex( IssuInfo.cardAuthCode / 0x10000, InData + 6, 2, 1 ); //大端
                        if( ret < 0 )
                        {
                                return ret;
                        }
                        break;

                case 1:                                                     //物理卡号作为分散因子 20130426 add
                        ret = UIntToHex( statInfo.card1 ^ 0x8592A0C7, InData, 4, 0 );
                        if( ret < 0 )
                        {
                                return ret;
                        }

                        ret = UIntToHex( statInfo.card1 ^ statInfo.card2 ^ 0xA3F39563, InData + 4, 4, 1 );
                        if( ret < 0 )
                        {
                                return ret;
                        }
                        break;

                case 2:                                                                 //密钥对卡片非接ATR的前8字节做分散 20130607 国电的握奇卡
                        memset( InData, 0xaa, 8 );
                        UIntToHex( statInfo.card2, InData + 4, 4, 0 );   //小端模式

                        if( statInfo.card1 )
                        {
                                UIntToHex( statInfo.card1, InData, 4, 0 );
                        }
                        break;

                default:
                        return ERR_PARAM;
                        break;

        }
        memcpy( statInfo.InData, InData, 8 ); // 计算卡片分散因子 statInfo.InData
        memset( key, 0, 16 );
        if( !( card_config.isPsam == 1 && statInfo.cardType == CARD_CPU ) )
        {
                ret =  calcKey( statInfo.InData, key );              //交易密钥（原为充值密钥）
                if( ret != ERR_OK)
                {
                        return ret;
                }
//		dbgShowHexData(statInfo.InData,8,1,0,"InData");
//		dbgShowHexData(key,6,1,'<',"calcKey");
                memcpy( statInfo.dealKey, key, 16 );
                pubInfo.keyType = KEY_TYPE_B;              // Bkey
        }
        if( statInfo.cardType == CARD_CPU )   //CPU卡内部认证 20121015
        {
                if( 0 == card_config.isPsam )
                {
                        unsigned char tmpData[8];

                        for( i = 0; i < 8; i++ )
                        {
                                tmpData[i] = ~statInfo.InData[i];
                        }

                        RunDes( 0, (char *)statInfo.InData, (char *)key, 8, (char *)MasterKey.interAuthKey, 16 );
                        RunDes( 0, (char *)tmpData, (char *)key + 8, 8, (char *)MasterKey.interAuthKey, 16 );

                        memcpy( (char *)statInfo.internalAuthKeybuf, (char *)key, 16 );
                }

                ret = internalAuthentication( );
                if( ret != ERR_OK )
                {
                        return ret;
                }
        }
// just for debug : recover to old card
#if 0
        ret = initPublicInfoSector( CCatalog.pubSectorNo );
        return ERR_FALSE; // must false
#endif

        ret = new_readPub( KEY_TYPE_B, statInfo.dealKey );
        if( ret == ERR_NEW_CARD )
        {
                // 状态为升级中，则继续升级操作
                if( pubInfo.cardStat == 6 )
                {
                        ret = update_card( 1 );
                }else // 新卡结构，正常处理
                {
                        printf( "New Card\n" );
                        goto OK;
                }
        }else if( ret == ERR_OLD_CARD )
        {
                // 读取卡片余额
                ret = ol_readWallet( &pubInfo.walletAmt, pubInfo.keyType, statInfo.dealKey );
                if( ret != ERR_OK )
                {
                        return ret;
                }

                // 读取个人密码,同步密码 ???
//		retCode = readBaseInfo( );
//		if( retCode != ERR_OK )
//		{
//			return retCode;
//		}

                // 上传余额到平台
//		printf( "amt = %d\n", Card_Chb1.pubInfo.walletAmt );

/*
                此处增加通信 SDK层增加
*/

                // 卡升级
                ret = update_card( 0 );
                if( ret != ERR_OK )
                {
                        return ret;
                }

        }
        else
        {
                return ret;
        }

OK:

#if 0	// just for debug
        if(pubInfo.recNum == 27)
        {
                pubInfo.recNum = 0;
        }
#endif

        ret = readPwdInfo(&pwdInfo, pubInfo.keyType, statInfo.dealKey);
        if( ret != ERR_OK )
        {
                return ret;
        }

//printf("name = %s\n", Card_Chb1.pwdInfo.name);
//leeDebugData(Card_Chb1.pwdInfo.pwd,3,3,2);

        CCatalog.pubReadFlag = 1;                                                                                                                            //已读标志
        return ERR_OK;

}


// 2.4G 手机卡操作相关
static int uimAuth( int sectorno, int keymode, unsigned char *key )
{
        unsigned char SendBuf[] = { 0x82, 0X0B, 0x40, 0x08, 0xff, 0xff, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

        if( ( sectorno < 0 ) || ( sectorno > 15 ) )
        {
                return ERR_PARAM;
        }

        SendBuf[4] = sectorno;      //扇区号

        if( keymode == 1 )          //A密钥
        {
                SendBuf[5] = 1;         //密钥类型 00：B, 01：A
        }else if( keymode == 2 )    //B密钥
        {
                SendBuf[5] = 0;
        }else
        {
                return ERR_PARAM;
        }

        memcpy( SendBuf + 7, key, 6 ); //填充密钥

        unsigned char	RecvBuf[265];
        int len = card_data_sr( statInfo.comfd, SendBuf, RecvBuf, 0, 200 );
        if( len == 6 )
        {
                if( ( RecvBuf[0] & 0xFC ) == 0x90 )
                {
                        if( ( RecvBuf[len - 4] == 0x90 ) && ( RecvBuf[len - 3] == 0 ) )
                        {
                                return ERR_OK;
                        }else if( ( RecvBuf[len - 4] == 0x9c ) && ( RecvBuf[len - 3] == 0x03 ) )
                        {
                                return ERR_NOCARD;
                        }else
                        {
                                return ERR_FALSE;
                        }
                }else
                {
                        return ERR_INVALID;
                }
        }else if( len > 0 )
        {
                return ERR_INVALID;
        }else
        {
                return len;
        }

        //return ERR_FALSE;
}


static int uimReadBlock( char blockno, unsigned char *data )
{
        unsigned char SendBuf[] = { 0x82, 0X05, 0x40, 0x02, 0x01, 0xff, 0x00, 0x00, 0x00 };
        SendBuf[5] = blockno % 4; //块号

        unsigned char	RecvBuf[265];
        int len = card_data_sr( statInfo.comfd, SendBuf, RecvBuf, 0, 200 );
        if( ( len == 6 ) || ( len == 22 ) )
        {
                if( ( RecvBuf[0] & 0xFC ) == 0x90 )
                {
                        if( ( RecvBuf[len - 4] == 0x90 ) && ( RecvBuf[len - 3] == 0 ) )
                        {
                                memcpy( data, RecvBuf + 2, 16 );
                                statInfo.accessMsec = GetTickCount( );
                                return ERR_OK;
                        }else if( ( RecvBuf[len - 4] == 0x9c ) && ( RecvBuf[len - 3] == 0x03 ) )
                        {
                                return ERR_NOCARD;
                        }else
                        {
                                return ERR_FALSE;
                        }
                }else
                {
                        return ERR_INVALID;
                }
        }else if( len > 0 )
        {
                return ERR_INVALID;
        }else
        {
                return len;
        }
}


static int uimWriteBlock( char blockno, unsigned char *data )
{
        unsigned char SendBuf[] = { 0x82, 0X15, 0x40, 0x02, 0x02, 0xff, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        SendBuf[5] = blockno % 4; //块号
        memcpy( SendBuf + 7, data, 16 );

        unsigned char	RecvBuf[265];
        int len = card_data_sr( statInfo.comfd, SendBuf, RecvBuf, 0, 200 );
        if( len == 6 )
        {
                if( ( RecvBuf[0] & 0xFC ) == 0x90 )
                {
                        if( ( RecvBuf[len - 4] == 0x90 ) && ( RecvBuf[len - 3] == 0 ) )
                        {
                                statInfo.accessMsec = GetTickCount( );
                                return ERR_OK;
                        }else if( ( RecvBuf[len - 4] == 0x9c ) && ( RecvBuf[len - 3] == 0x03 ) )
                        {
                                return ERR_NOCARD;
                        }else
                        {
                                return ERR_FALSE;
                        }
                }else
                {
                        return ERR_INVALID;
                }
        }else if( len > 0 )
        {
                return ERR_INVALID;
        }else
        {
                return len;
        }

        //return ERR_FALSE;
}



// m_hcom: 串口操作句柄
static int uimHalt(int m_hcom)
{
        unsigned char	SendBuf[] = { 0x82, 0X05, 0x40, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00 };
        unsigned char	RecvBuf[265];

        int	len = card_data_sr( m_hcom, SendBuf, RecvBuf, 0, 200 );
        if( len == 6 )
        {
                if( ( RecvBuf[0] & 0xFC ) == 0x90 )
                {
                        if( ( RecvBuf[len - 4] == 0x90 ) && ( RecvBuf[len - 3] == 0 ) )
                        {
                                return ERR_OK;
                        }else if( ( RecvBuf[len - 4] == 0x9a ) && ( RecvBuf[len - 3] == 0x03 ) )
                        {
                                return ERR_COMM;
                        }else
                        {
                                return ERR_FALSE;
                        }
                }else
                {
                        return ERR_INVALID;
                }
        }else if( len > 0 )
        {
                return ERR_INVALID;
        }else
        {
                return len;
        }
}

static int uimSetMifareAppID( int m_hcom, char *data )
{
        unsigned char	SendBuf[21];
        unsigned char	parambuf1[] = { 0x82, 0x11, 0x40, 0x0C, 0x04, 0x00, 0x0C };
        unsigned char	RecvBuf[265];

        if( ( strlen( data ) < 6 ) || ( strlen( data ) > 12 ) ) //20130815
        {
                return ERR_PARAM;
        }

        memset( SendBuf, 0, 21 );
        memcpy( SendBuf, parambuf1, 7 );
        memcpy( (char *)SendBuf + 7, data, strlen( data ) );

        int	len = card_data_sr( m_hcom, SendBuf, RecvBuf, 0, 200 );
        if( len == 6 )
        {
                if( ( RecvBuf[0] & 0xFC ) == 0x90 )
                {
                        if( ( RecvBuf[len - 4] == 0x90 ) && ( RecvBuf[len - 3] == 0 ) )
                        {
                                return ERR_OK;
                        }else if( ( RecvBuf[len - 4] == 0x9a ) && ( RecvBuf[len - 3] == 0x24 ) )
                        {
                                return ERR_FUNCLOCK; //功能被锁
                        }else
                        {
                                return ERR_FALSE;
                        }
                }else
                {
                        return ERR_INVALID;
                }
        }else if( len > 0 )
        {
                return ERR_INVALID;
        }else
        {
                return len;
        }
}

static int uimMessageSwitch( int m_hcom, int on_off )
{
        unsigned char	SendBuf[9];
        unsigned char	parambuf1[] = { 0x82, 0x05, 0x90, 0xB0, 0x19, 0x00, 0x00 };
        unsigned char	RecvBuf[265];

        memset( SendBuf, 0, 9 );
        memcpy( SendBuf, parambuf1, 7 );

        if( on_off )
        {
                SendBuf[5] = 1;
        }

        int	len = card_data_sr( m_hcom, SendBuf, RecvBuf, 0, 200 );
        if( len == 7 )
        {
                if( ( RecvBuf[0] & 0xFC ) == 0x90 )
                {
                        if( ( RecvBuf[len - 4] == 0x90 ) && ( RecvBuf[len - 3] == 0 ) )
                        {
                                return ERR_OK;
                        }else if( ( RecvBuf[len - 4] == 0x9a ) && ( RecvBuf[len - 3] == 0x03 ) )
                        {
                                return ERR_COMM; //
                        }else
                        {
                                return ERR_FALSE;
                        }
                }else
                {
                        return ERR_INVALID;
                }
        }else if( len > 0 )
        {
                return ERR_INVALID;
        }else
        {
                return len;
        }
}


static int uimSelectAccounter(int m_hcom)
{
        unsigned char	SendBuf[25];
        unsigned char	parambuf1[] = { 0x82, 0x15, 0xA0, 0xA4, 0x04, 0x00, 0x10 };
        unsigned char	RecvBuf[265];

        memset( SendBuf, 0, 25 );
        memcpy( SendBuf, parambuf1, 7 );
        memcpy( (char *)SendBuf + 7, "Accounter App", 13 );

        int	len = card_data_sr( m_hcom, SendBuf, RecvBuf, 0, 200 );
        if( len == 6 )
        {
                if( ( RecvBuf[0] & 0xFC ) == 0x90 )
                {
                        if( ( RecvBuf[len - 4] == 0x90 ) && ( RecvBuf[len - 3] == 0 ) )
                        {
                                return ERR_OK;
                        }else if( ( RecvBuf[len - 4] == 0x9a ) && ( RecvBuf[len - 3] == 0x03 ) )
                        {
                                return ERR_COMM; //
                        }else
                        {
                                return ERR_FALSE;
                        }
                }else
                {
                        return ERR_INVALID;
                }
        }else if( len > 0 )
        {
                return ERR_INVALID;
        }else
        {
                return len;
        }
}


static int yooAuth( int sectorno, int keymode, unsigned char *key )
{
        unsigned char SendBuf[] = { 0x5A, 0x20, 0x00, 0x0B, 0x41, 0x08, 0xFF, 0xFF, 0x06, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00 };

        if( ( sectorno < 0 ) || ( sectorno > 15 ) )
        {
                return ERR_PARAM;
        }

        SendBuf[6] = sectorno;      //扇区号

        if( keymode == 1 )          //A密钥
        {
                SendBuf[7] = 1;         //密钥类型 00：B, 01：A
        }else if( keymode == 2 )    //B密钥
        {
                SendBuf[7] = 0;
        }else
        {
                return ERR_PARAM;
        }

        int				ret;
        unsigned int	card1	   = statInfo.card1;
        unsigned int	card2	   = statInfo.card2;
        unsigned int	cartType   = statInfo.cardType;

        if( ( card1 == 0 ) && ( card2 == 0 ) || ( cartType != CARD_USIM ) )
        {
                return ERR_NOCARD; //未寻卡，做无卡处理 20120625
        }
#if 1
        if( statInfo.accessMsec + statInfo.authDelayMsec < GetTickCount( ) )
        {
                ret = card_active( statInfo.readerType, NULL );
                if( ret != CARD_USIM )
                {
                        return ret;
                }
        } //20120625 解决操作间隔超过给定时间（最大350MS)重新寻卡的问题
#endif
        memcpy( SendBuf + 9, key, 6 ); //填充密钥
        unsigned char	RecvBuf[265];

        int len = card_data_sr( statInfo.comfd, SendBuf, RecvBuf, 0, 200 );
        if( len == 9 )
        {
                if( ( RecvBuf[4] == 0x90 ) && ( RecvBuf[5] == 0 ) && ( RecvBuf[6] == 0 ) )
                {
                        return ERR_OK;
                }else if( RecvBuf[6] == 0x05 )
                {
                        return ERR_NOCARD;
                }

                return ERR_FALSE;
        }else if( len < 0 )
        {
                return len;
        }
        return ERR_INVALID;
}

static int yooReadBlock( char blockno, unsigned char *data )
{
        unsigned char SendBuf[] = { 0x5A, 0x20, 0x00, 0x05, 0x41, 0x02, 0x01, 0xFF, 0x10, 0x00, 0x00 };
        SendBuf[7] = blockno % 4;   //块号

        unsigned char	RecvBuf[265];
        int len = card_data_sr( statInfo.comfd, SendBuf, RecvBuf, 0, 200 );
        if( ( len == 25 ) )         //收到数据
        {
                if( ( RecvBuf[20] == 0x90 ) && ( RecvBuf[21] == 0 ) && ( RecvBuf[22] == 0 ) )
                {
                        memcpy( data, RecvBuf + 4, 16 );
                        statInfo.accessMsec = GetTickCount( );
                        return ERR_OK;
                }else
                {
                        return ERR_OPERR;
                }
        }else if( len == 9 )        //未收到数据
        {
                if( RecvBuf[6] == 0x05 )
                {
                        return ERR_NOCARD;
                }else
                {
                        return ERR_OPERR;   //暂时这样处理
                }
        }else if( len < 0 )
        {
                return len;
        }

        return ERR_INVALID;
}

static int yooWriteBlock( char blockno, unsigned char *data )
{
        unsigned char SendBuf[] = { 0x5A, 0x20, 0x00, 0x15, 0x41, 0x02, 0x02, 0xFF, 0x10, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00 };
        SendBuf[7] = blockno % 4; //块号
        memcpy( SendBuf + 9, data, 16 );

        unsigned char	RecvBuf[265];
        int len = card_data_sr( statInfo.comfd, SendBuf, RecvBuf, 0, 200 );
        if( len == 9 )
        {
                if( ( RecvBuf[4] == 0x90 ) && ( RecvBuf[5] == 0 ) && ( RecvBuf[6] == 0 ) )
                {
                        statInfo.accessMsec = GetTickCount( );
                        return ERR_OK;
                }else if( RecvBuf[6] == 0x05 )
                {
                        return ERR_NOCARD;
                }else
                {
                        return ERR_FALSE;
                }
        }else if( len > 0 )
        {
                return ERR_INVALID;
        }else
        {
                return len;
        }
}


#if 0
#endif

// 脱机消费接口

//预读卡片
static int ol_preReadCard( )
{
        int ret, i = 0;
        TCatalog tmpCatalog;
        TissuInfo tmpIssuInfo;
        unsigned char key[16];
        unsigned char	InData[8];

        ret = readCatalogInfo( &tmpCatalog, KEY_TYPE_A,MasterKey.readManageSectorKey );
        if( ret != ERR_OK )
        {
                return ret;
        }
        memcpy( &CCatalog, &tmpCatalog, sizeof( CCatalog ) );

        ret = readIssuInfo( &tmpIssuInfo, KEY_TYPE_A, MasterKey.readManageSectorKey );
        if( ret != ERR_OK )
        {
                return ret;
        }
        memcpy( &IssuInfo, &tmpIssuInfo, sizeof( IssuInfo ) );

        switch( statInfo.scatteringFactor )
        {
                case 0:
                        switch( statInfo.cardType )
                        {
                                case CARD_UIM:
                                        ret = UIntToHex( statInfo.card2, InData, 4, 1 );
                                        break;
                                default:
                                        ret = UIntToHex( statInfo.card2, InData, 4, 0 );
                                        break;
                        }

                        if( ret )
                        {
                                return ret;
                        }

                        ret = UIntToBcd( IssuInfo.issuSerialNo % 10000, InData + 4, 2 );
                        if( ret < 0 )
                        {
                                return ret;
                        }

                        ret = UIntToHex( IssuInfo.cardAuthCode / 0x10000, InData + 6, 2, 1 ); //大端
                        if( ret < 0 )
                        {
                                return ret;
                        }
                        break;

                case 1:                                                     //物理卡号作为分散因子 20130426 add
                        ret = UIntToHex( statInfo.card1 ^ 0x8592A0C7, InData, 4, 0 );
                        if( ret < 0 )
                        {
                                return ret;
                        }

                        ret = UIntToHex( statInfo.card1 ^ statInfo.card2 ^ 0xA3F39563, InData + 4, 4, 1 );
                        if( ret < 0 )
                        {
                                return ret;
                        }
                        break;

                case 2:                                                                 //密钥对卡片非接ATR的前8字节做分散 20130607 国电的握奇卡
                        memset( InData, 0xaa, 8 );
                        UIntToHex( statInfo.card2, InData + 4, 4, 0 );   //小端模式

                        if( statInfo.card1 )
                        {
                                UIntToHex( statInfo.card1, InData, 4, 0 );
                        }
                        break;

                default:
                        return ERR_PARAM;
                        break;

        }

        memcpy( statInfo.InData, InData, 8 ); // 计算卡片分散因子 statInfo.InData
        memset( key, 0, 16 );
        if( !( card_config.isPsam == 1 && statInfo.cardType == CARD_CPU ) )
        {
                ret =  calcKey( statInfo.InData, key );              //交易密钥（原为充值密钥）
                if( ret != ERR_OK)
                {
                        return ret;
                }
//dbgShowHexData(statInfo.InData,8,1,0,"InData");
//dbgShowHexData(key,6,1,'<',"calcKey");
                memcpy( statInfo.dealKey, key, 16 );
                pubInfo.keyType = KEY_TYPE_B;              // Bkey
        }

        if( statInfo.cardType == CARD_CPU )   //CPU卡内部认证 20121015
        {
                if( 0 == card_config.isPsam )
                {
                        unsigned char tmpData[8];

                        for( i = 0; i < 8; i++ )
                        {
                                tmpData[i] = ~statInfo.InData[i];
                        }

                        RunDes( 0, (char *)statInfo.InData, (char *)key, 8, (char *)MasterKey.interAuthKey, 16 );
                        RunDes( 0, (char *)tmpData, (char *)key + 8, 8, (char *)MasterKey.interAuthKey, 16 );

                        memcpy( (char *)statInfo.internalAuthKeybuf, (char *)key, 16 );
                }

                ret = internalAuthentication( );
                if( ret != ERR_OK )
                {
                        return ret;
                }
        }


//printf("name = %s\n", Card_Chb1.pwdInfo.name);
//leeDebugData(Card_Chb1.pwdInfo.pwd,3,3,2);

        CCatalog.pubReadFlag = 1;                                                                                                                            //已读标志
        return ERR_OK;

}

// 读取脱机消费钱包
static int ol_readWallet( int *walletAmt, int keymode, unsigned char *key )
{
        int ret, i = 0;

        Twallet			wallet1;
        unsigned char	*addr = wallet1.blockBuf; //buf;

        memset( addr, 0, 16 );

        ret = readDataBlock2( keymode, key, CCatalog.walletArea[( pubInfo.walletPoint + 1 ) % 2] / 4, CCatalog.walletArea[( pubInfo.walletPoint + 1 ) % 2] % 4, 16, addr );
        if( ret != ERR_OK )
        {
                return ret;
        }

        if( CalcCRC8( addr, 0 ) != 0 )
        {
                return ERR_CRC; //CRC校验错误
        }

        unsigned char chkXor = 0;
        for( i = 0; i < 14; i++ )
        {
                chkXor ^= *( addr + i );
        }

        if( chkXor != *( addr + 14 ) )
        {
                return ERR_CRC;
        }

        wallet1.walletAmt  = HexToInt( addr, 4, 1 );       //钱包余额	4B（单位：分）
        addr			  += 4;

        wallet1.savingAmt  = HexToInt( addr, 3, 1 );       //充值金额	3B
        addr			  += 3;

        wallet1.dealType   = HexToUInt( addr, 1, 1 );      //交易类型	1B
        addr			  += 1;

        wallet1.savingDate = HexToUInt( addr, 4, 1 );      //充值日期时间（相对2000年经过的秒数）
        addr			  += 4;

        wallet1.savingSerialNo = HexToUInt( addr, 2, 1 );  //充值序号 2B
        addr				  += 2;

        *walletAmt = wallet1.walletAmt;

        return ERR_OK;
}


// 读取脱机消费累计
static int ol_readSumInfo(PTsumInfo PsumInfo, int keymode, unsigned char *key)
{
        int ret;
        struct TTime time1;
        unsigned char *addr = PsumInfo->blockBuf;

        memset(PsumInfo, 0, sizeof(TsumInfo));

        ret = readDataBlock2(keymode, key, CCatalog.sumArea[(pubInfo.dailyTotalPoint+1)%2]/4, CCatalog.sumArea[(pubInfo.dailyTotalPoint+1)%2]%4, 16, addr);
        if (ret != ERR_OK)
        {
                return ret;
        }

        if (CalcCRC8(addr, 0) != 0)
        {
                return ERR_CRC; //CRC校验错误
        }

        PsumInfo->totalConsumeAmt = HexToUInt(addr, 3, 1); //日累计消费金额（3B，HEX，单位：分）
        addr += 3;

        unsigned char timebuf[4];
        ret = BcdToHex(addr, timebuf, 3); //日累计消费日期（3B，YYMMDD）
        if (ret != 3)
        {
                return ERR_TIMEFORMAT; //日期时间格式错误
        }

        time1.year = 2000 + timebuf[0];
        time1.month = timebuf[1];
        time1.day = timebuf[2];
        time1.hour = 0;
        time1.minute = 0;
        time1.second = 0;

        ret = TimeToUInt(&time1, &PsumInfo->consumeDate, 2000, 1);
        if (ret != ERR_OK)
        {
                return ERR_TIMEFORMAT; //日期时间格式错误
        }
        addr += 3;

        PsumInfo->totalQty = HexToUInt(addr, 2, 1); //日累计消费次数（2B，HEX）
        addr += 2;

        PsumInfo->currHourSession = HexToUInt(addr, 1, 1); //时段号（1B，HEX）
        addr += 1;

        if (PsumInfo->currHourSession > 5)
        {
                return ERR_HOUR_SESSION; //时段号超限
        }

        PsumInfo->hourSessionAmt = HexToUInt(addr, 3, 1); //时段累计金额（3B，HEX，单位：分）
        addr += 3;

        PsumInfo->hourSessionCopies = HexToUInt(addr, 2, 1); //时段消费次数（2B，HEX）
        addr += 2;

        PsumInfo->sbsFlag = *addr;	// fuglee 20110908 补贴标志

        return ERR_OK;

}


// 读取脱机消费补贴
static int ol_readSubsidyInfo(PTsubsidyInfo PsubsidyInfo, int keymode, unsigned char *key)
{
        int ret;
        unsigned char *addr = PsubsidyInfo->blockBuf;

        memset(PsubsidyInfo, 0, sizeof(TsubsidyInfo));
        ret = readDataBlock2(keymode, key, CCatalog.subsidyArea[(pubInfo.subsidyPoint+1)%2]/4, CCatalog.subsidyArea[(pubInfo.subsidyPoint+1)%2]%4, 16, addr);
        if (ret != ERR_OK)
        {
                return ret;
        }

        if (CalcCRC8(addr, 0) != 0)
        {
                return ERR_CRC; //CRC校验错误
        }

        PsubsidyInfo->subsidySerialNo = HexToUInt(addr, 2, 1); //内部补助批号（2B，HEX）
        addr += 2;

        unsigned char timebuf[4];
        ret = BcdToHex(addr, timebuf, 3); //补助领取时间（3B，YYMMDD）
        if (ret != 3)
        {
                return ERR_TIMEFORMAT; //日期时间格式错误
        }

        struct TTime time1;
        time1.year = 2000 + timebuf[0];
        time1.month = timebuf[1];
        time1.day = timebuf[2];
        time1.hour = 0;
        time1.minute = 0;
        time1.second = 0;

        ret = TimeToUInt(&time1, &PsubsidyInfo->writeCardDate, 2000, 1);
        if (ret != ERR_OK)
        {
                return ERR_TIMEFORMAT; //日期时间格式错误
        }
        addr += 3;

        ret = BcdToHex(addr, timebuf, 3); //有效截至日期（3B，YYMMDD）
        if (ret != 3)
        {
                return ERR_TIMEFORMAT; //日期时间格式错误
        }

        time1.year = 2000 + timebuf[0];
        time1.month = timebuf[1];
        time1.day = timebuf[2];
        time1.hour = 0;
        time1.minute = 0;
        time1.second = 0;

        addr += 3;

// ---- 余额修改为有符号数
        PsubsidyInfo->subsidyAmt = HexToInt(addr, 3, 1); //本次领取金额（3B，单位：分）
        addr += 3;

        PsubsidyInfo->balance = HexToInt(addr, 3, 1); //补贴余额（3B，单位：分）
        addr += 3;

// fuglee 2012-06-09 有效截止时间
        PsubsidyInfo->endTime = *addr;	// 0~143
        if (PsubsidyInfo->endTime > 143)
        {
                PsubsidyInfo->endTime = 0;
        }

        time1.second = PsubsidyInfo->endTime * 600;
        ret = TimeToUInt(&time1, &PsubsidyInfo->endDate, 2000, 1);
        if (ret != ERR_OK)
        {
                return ERR_TIMEFORMAT; //日期时间格式错误
        }

        return ERR_OK;

}

// 读取脱机基本信息
static int ol_readBaseInfo(PTbaseInfo PbaseInfo, int keymode, unsigned char *key)
{
        int ret;
        unsigned char *addr = PbaseInfo->blockBuf;

        memset(PbaseInfo, 0, sizeof(TbaseInfo));

        ret = readDataBlock2(keymode, key, CCatalog.baseArea[0]/4, CCatalog.baseArea[0]%4, 32, addr);
        if (ret != ERR_OK)
        {
                return ret;
        }

        if (CalcCRC8(addr+16, 0) != 0)
        {
                return ERR_CRC;
        }

        memcpy(PbaseInfo->employeeCode, addr, 16);
        addr+= 16;

        unsigned char currCardNO[4]; //计算个人密码使用

        switch (statInfo.cardType)
        {
        case CARD_UIM:
                UIntToHex(statInfo.card2, currCardNO, 4, 1); //卡号应该为大端模式
                break;
        default:
                UIntToHex(statInfo.card2, currCardNO, 4, 0); //卡号应该为小端模式
                break;
        }

        *addr ^= currCardNO[0];
        *(addr+1) ^= currCardNO[1];
        *(addr+2) ^= currCardNO[2];

        ret = BcdToUInt(addr, &PbaseInfo->password, 3); //3BCD,与卡片物理序号前3字节异或
        if (ret != ERR_OK)
        {
                *addr ^= currCardNO[0];
                *(addr+1) ^= currCardNO[1];
                *(addr+2) ^= currCardNO[2];

                return ERR_PASSWORD_FORMAT; //密码格式错误;
        }
        addr += 3;

        PbaseInfo->passStat = HexToUInt(addr, 1, 1); //1B,HEX, 密码开关 01:开启 02:关闭密码功能
        if ((PbaseInfo->passStat != 1) && (PbaseInfo->passStat != 2))
        {
                return ERR_PASSWORD_STAT; //密码状态无效
        }
        addr += 1;

        PbaseInfo->cardEdition = HexToUInt(addr, 1, 1); //1B,hex,卡结构版本
        addr += 1;

        PbaseInfo->groupID = HexToUInt(addr, 1, 1); //1B,HEX,权限组别
        addr += 1;

        PbaseInfo->onceConsumeLimitation = HexToUInt(addr, 3, 1); //3B,HEX,单次消费密码限额，单位：分
        addr += 3;

        PbaseInfo->dailyConsumeLimitation = HexToUInt(addr, 3, 1); //3B,HEX,日累计消费密码限额，单位：分
        addr += 3;

        PbaseInfo->dailyConsumeCopies = HexToUInt(addr, 2, 1); //2B,HEX,日累计密码限次
        addr += 2;

        return ERR_OK;
}


// 读取脱机明细
static int ol_readDetailByPoint(PTdetailInfo PdetailInfo, int point, int keymode, unsigned char *key)
{
        int ret;
        unsigned char *addr = PdetailInfo->blockBuf;

        memset(PdetailInfo, 0, sizeof(TdetailInfo));

        if (point > 14)
        {
                return ERR_DETAIL_POINT; //明细指针非法
        }

        if (CCatalog.detailArray[point] == 0)
        {
                return ERR_DETAIL_POINT; //明细指针非法
        }

        ret = readDataBlock2(keymode, key, CCatalog.detailArray[point]/4, CCatalog.detailArray[point]%4, 16, addr);
        if (ret != ERR_OK)
        {
                return ret;
        }

        PdetailInfo->dealTime = HexToUInt(addr, 4, 1); //交易时间，相对于2000-01-01 00：00：00经过的秒数（4B，HEX）
        addr += 4;

        PdetailInfo->balanceBeforeDeal = HexToInt(addr, 4, 1); //交易前余额，单位：分（4B，HEX）
        addr += 4;

        PdetailInfo->dealAmt = HexToInt(addr, 3, 1); //交易金额，单位：分（3B，HEX）
        addr += 3;

        PdetailInfo->dealType = HexToUInt(addr, 1, 1); //交易类型，1B，HEX，01：消费，02：充值，04：补贴，08：提现 16：补贴清零，32：冲正
        addr += 1;

        ret = BcdToUInt(addr, &PdetailInfo->terminalNo, 4);
        if (ret != ERR_OK)
        {
                return ERR_DATA_FORMAT; //数据格式错误;
        }

        return ERR_OK;
}



/*
**
** 函数功能: 脱机消费卡片操作统一接口，如查询、消费等
**
** @in param
**			in:输入参数
**			out:输出参数
** @return
**			参考具体返回码
**
** @other
**			卡片相关操作最终均调用该接口，通过操作码区分不同的操作
*/

int pit_tuoji_main(_IN_PARAM in, _OUT_PARAM *out)
{
        int ret = -1, i = 0, j = 0, k = 0;
        unsigned int val, num, count;
        unsigned char *addr = NULL, cpdata[8][16];
        struct 	tm *tp = NULL;
        time_t curTimes, timep;
        _CARD_DATA CARD_DATA;

        if(out == NULL)
        {
                return ERR_PARAM;
        }
        ret = do_after_get_card(in);
        if( ERR_OK != ret )
        {
                return ret;
        }

        ret = ol_preReadCard( );
        if( ERR_OK != ret )
        {
                return ret;
        }

        curTimes = in.curTimes - TIME20000101;
        ret = checkCard( curTimes );
        if( ret != ERR_OK )
        {
                return ret;
        }

        if( IssuInfo.cardType != 0 )		// 是否为用户卡
        {
                return ERR_FALSE;
        }

        if( IssuInfo.CardNo != in.logcard)		// 是否为用户卡
        {
                return ERR_FALSE;
        }


        if (in.cmd == TUOJI_QUERY)	// 脱机查询
        {

        }
        else if(in.cmd == TUOJI_TRADE)	// 脱机消费
        {

        }
        else
        {
                return ERR_PARAM;
        }

        return ERR_FALSE;

}





#if 0
#endif

static const char IP_Table[64] = // initial permutation IP
{
        58, 50, 42, 34, 26, 18, 10, 2, 60, 52, 44, 36, 28, 20, 12, 4,
        62, 54, 46, 38, 30, 22, 14, 6, 64, 56, 48, 40, 32, 24, 16, 8,
        57, 49, 41, 33, 25, 17,  9, 1, 59, 51, 43, 35, 27, 19, 11, 3,
        61, 53, 45, 37, 29, 21, 13, 5, 63, 55, 47, 39, 31, 23, 15, 7
};

const char IPR_Table[64] = // final permutation IP^-1
{
        40, 8, 48, 16, 56, 24, 64, 32, 39, 7, 47, 15, 55, 23, 63, 31,
        38, 6, 46, 14, 54, 22, 62, 30, 37, 5, 45, 13, 53, 21, 61, 29,
        36, 4, 44, 12, 52, 20, 60, 28, 35, 3, 43, 11, 51, 19, 59, 27,
        34, 2, 42, 10, 50, 18, 58, 26, 33, 1, 41,  9, 49, 17, 57, 25
};

static const char E_Table[48] = // expansion operation matrix
{
        32,  1,  2,  3,  4,  5,  4,  5,  6,  7,  8,  9,
        8,  9, 10, 11, 12, 13, 12, 13, 14, 15, 16, 17,
        16, 17, 18, 19, 20, 21, 20, 21, 22, 23, 24, 25,
        24, 25, 26, 27, 28, 29, 28, 29, 30, 31, 32,  1
};

static const char P_Table[32] = // 32-bit permutation function P used on the output of the S-boxes
{
        16, 7, 20, 21, 29, 12, 28, 17, 1,  15, 23, 26, 5,  18, 31, 10,
        2,  8, 24, 14, 32, 27, 3,  9,  19, 13, 30, 6,  22, 11, 4,  25
};

static const char PC1_Table[56] = // permuted choice table (key)
{
        57, 49, 41, 33, 25, 17,  9,  1, 58, 50, 42, 34, 26, 18,
        10,  2, 59, 51, 43, 35, 27, 19, 11,  3, 60, 52, 44, 36,
        63, 55, 47, 39, 31, 23, 15,  7, 62, 54, 46, 38, 30, 22,
        14,  6, 61, 53, 45, 37, 29, 21, 13,  5, 28, 20, 12,  4
};

static const char PC2_Table[48] = // permuted choice key (table)
{
        14, 17, 11, 24,  1,  5,  3, 28, 15,  6, 21, 10,
        23, 19, 12,  4, 26,  8, 16,  7, 27, 20, 13,  2,
        41, 52, 31, 37, 47, 55, 30, 40, 51, 45, 33, 48,
        44, 49, 39, 56, 34, 53, 46, 42, 50, 36, 29, 32
};

static const char LOOP_Table[16] = // number left rotations of pc1
{
        1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1
};

static const char S_Box[8][4][16] = // The (in)famous S-boxes
{
        // S1
        {
                {14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7},
                {0,  15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8},
                {4,   1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0},
                {15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13}
        },
        // S2
        {
                {15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10},
                {3,  13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5},
                {0,  14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15},
                {13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9}
        },
        // S3
        {
                {10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8},
                {13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1},
                {13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7},
                {1,  10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12}
        },
        // S4
        {
                {7,  13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15},
                {13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9},
                {10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4},
                {3,  15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14}
        },
        // S5
        {
                {2,  12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9},
                {14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6},
                {4,   2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14},
                {11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3}
        },
        // S6
        {
                {12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11},
                {10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8},
                {9,  14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6},
                {4,   3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13}
        },
        // S7
        {
                {4,  11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1},
                {13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6},
                {1,   4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2},
                {6,  11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12}
        },
        // S8
        {
                {13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7},
                {1,  15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2},
                {7,  11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8},
                {2,   1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11}
        }
};


static void ByteToBit(char *Out, const char *In, int bits);
static void BitToByte(char *Out, const char *In, int bits);
static void RotateL(char *In, int len, int loop);
static void Xor(char *InA, const char *InB, int len);
static void Transform(char *Out, char *In, const char *Table, int len);
static void S_func(char Out[], char In[]);
static void F_func(char In[], const char Ki[]);
static void TEA_decrypt(unsigned long *v, unsigned long *k);
static void TEA_encrypt(unsigned long *v, unsigned long *k);
static void BasicDes(char Out[], char In[], const PSubKey pSubKey, char Type);
static void SetSubKey(PSubKey pSubKey, const char Key[]);








//  函 数 名 称:	ByteToBit
//  功 能 描 述:	把BYTE转化为Bit流
//  参 数 说 明:	Out:	输出的Bit流[in][out]
//				    In:	输入的BYTE流[in]
//				    bits:	Bit流的长度[in]
static void ByteToBit(char *Out, const char *In, int bits)
{
    int i=0;

    for(i=0; i<bits; ++i)
        {
        Out[i] = ((In[i>>3]>>(7 - (i&7))) & 1);
        }
}

//  函 数 名 称:	BitToByte
//  功 能 描 述:	把Bit转化为Byte流
//  参 数 说 明:	Out:	输出的BYTE流[in][out]
//					In:		输入的Bit流[in]
//					bits:	Bit流的长度[in]
static void BitToByte(char *Out, const char *In, int bits)
{
    int i=0;
    memset(Out, 0, bits>>3);

        for(i=0; i<bits; ++i)
        {
        Out[i>>3] |= In[i]<<((7 - i)&7);
        }
}

//  函 数 名 称:	RotateL
//  功 能 描 述:	把BIT流按位向左迭代
//  参 数 说 明:	In:		输入的Bit流[in]
//					len:	Bit流的长度[in]
//					loop:	向左迭代的长度
static void RotateL(char *In, int len, int loop)
{
        char Tmp[256];

    memcpy(Tmp, In, loop);
    memcpy(In, In+loop, len-loop);
    memcpy(In+len-loop, Tmp, loop);
}

//  函 数 名 称:	Xor
//  功 能 描 述:	把两个Bit流进行异或
//  参 数 说 明:	InA:	输入的Bit流[in][out]
//					InB:	输入的Bit流[in]
//					loop:	Bit流的长度
static void Xor(char *InA, const char *InB, int len)
{
    int i=0;
    for(i=0; i<len; ++i) InA[i] ^= InB[i];
}

//  函 数 名 称:	Transform
//  功 能 描 述：	把两个Bit流按表进行位转化
//  参 数 说 明：	Out:	输出的Bit流[out]
//				In:		输入的Bit流[in]
//				Table:	转化需要的表指针
//				len:	转化表的长度
static void Transform(char *Out, char *In, const char *Table, int len)
{
        char Tmp[256];
    int i=0;

    for (i=0; i<len; ++i)
        {
                Tmp[i] = In[ Table[i]-1 ];
        }

    memcpy(Out, Tmp, len);
}

//  函 数 名 称:	S_func
//  功 能 描 述:	实现数据加密S BOX模块
//  参 数 说 明:	Out:	输出的32Bit[out]
//					In:		输入的48Bit[in]
static void S_func(char Out[], char In[])
{
    unsigned char i=0;
    unsigned char j=0;
    unsigned char k=0;
    int l=0;

    for(i=0; i<8; ++i,In+=6,Out+=4)
        {
                j = (In[0]<<1) + In[5];
        k = (In[1]<<3) + (In[2]<<2) + (In[3]<<1) + In[4];	//组织SID下标

                for(l=0; l<4; ++l)								//把相应4bit赋值
                {
                        Out[l] = (S_Box[i][j][k]>>(3 - l)) & 1;
                }
    }
}

//  函 数 名 称:	F_func
//  功 能 描 述:	实现数据加密到输出P
//  参 数 说 明:	Out:	输出的32Bit[out]
//					In:		输入的48Bit[in]
static void F_func(char In[], const char Ki[])
{
    char MR[48];
    Transform(MR, In, E_Table, 48);
    Xor(MR, Ki, 48);
    S_func(In, MR);
    Transform(In, In, P_Table, 32);
}

//计算并填充子密钥到SubKey数据中

static void SetSubKey(PSubKey pSubKey, const char Key[])
{
        char K[64], *KL=&K[0], *KR=&K[28];
    int i=0;

    ByteToBit(K, Key, 64);
    Transform(K, K, PC1_Table, 56);

    for (i=0; i<16; ++i)
    {
        RotateL(KL, 28, LOOP_Table[i]);
        RotateL(KR, 28, LOOP_Table[i]);
        Transform((*pSubKey)[i], K, PC2_Table, 48);
    }
}

// Des单元运算
static void BasicDes(char Out[], char In[], const PSubKey pSubKey, char Type)
{
    char M[64], tmp[32], *Li=&M[0], *Ri=&M[32];
    int i=0;
    ByteToBit(M, In, 64);
    Transform(M, M, IP_Table, 64);

    if (Type == ENCRYPT )
        {
        for (i=0; i<16; ++i)
                {
            memcpy(tmp, Ri, 32);		        //Ri[i-1] 保存
            F_func(Ri, (*pSubKey)[i]);	        //Ri[i-1]经过转化和SBox输出为P
            Xor(Ri, Li, 32);		            //Ri[i] = P XOR Li[i-1]
            memcpy(Li, tmp, 32);		        //Li[i] = Ri[i-1]
        }
    }
    else
    {
        for (i=15; i>=0; --i)
        {
            memcpy(tmp, Ri, 32);		        //Ri[i-1] 保存
            F_func(Ri, (*pSubKey)[i]);	        //Ri[i-1] 经过转化和SBox输出为P
            Xor(Ri, Li, 32);		            //Ri[i] = P XOR Li[i-1]
            memcpy(Li, tmp, 32);		        //Li[i] = Ri[i-1]
        }
    }

        RotateL(M,64,32);				            //Ri与Li换位重组M
    Transform(M, M, IPR_Table, 64);		        //最后结果进行转化
    BitToByte(Out, M, 64);				        //组织成字符
}

static void TEA_encrypt(unsigned long *v, unsigned long *k)
{
        unsigned long y=v[0], z=v[1], sum=0, i;         /* set up */
        unsigned long delta=0x9e3779b9;                 /* a key schedule constant */
        unsigned long a=k[0], b=k[1], c=k[2], d=k[3];   /* cache key */

        for (i=0; i < 32; i++)  /* basic cycle start */
        {
                sum += delta;
                y += ((z<<4) + a) ^ (z + sum) ^ ((z>>5) + b);
                z += ((y<<4) + c) ^ (y + sum) ^ ((y>>5) + d);/* end cycle */
        }

        v[0]=y;
        v[1]=z;
}

static void TEA_decrypt(unsigned long *v, unsigned long *k)
{
        unsigned long y=v[0], z=v[1], sum=0xC6EF3720, i; /* set up */
        unsigned long delta=0x9e3779b9;                  /* a key schedule constant */
        unsigned long a=k[0], b=k[1], c=k[2], d=k[3];    /* cache key */

        for(i=0; i<32; i++)           /* basic cycle start */
        {
                z -= ((y<<4) + c) ^ (y + sum) ^ ((y>>5) + d);
                y -= ((z<<4) + a) ^ (z + sum) ^ ((z>>5) + b);
                sum -= delta;                                /* end cycle */
        }

        v[0]=y;
        v[1]=z;
}

//  函 数 名 称:	RunTea
//  功 能 描 述:	执行TEA算法对文本加解密
//  参 数 说 明:	bType	:类型：加密ENCRYPT，解密DECRYPT
//					bMode	:模式：忽略，目的为兼容des算法函数的参数格式
//					In		:待加密串指针
//					Out		:待输出串指针
//					datalen	:待加密串的长度，同时Out的缓冲区大小应大于或者等于datalen 必须为8的整数倍，否则返回错误
//					Key		:密钥：16字节
//					keylen	:密钥长度：忽略，内部使用16字节
//
//  返回值 说明：	bool	:是否加密成功 1:成功，0：失败

//20130104
int RunTea(char bType, char *In, char *Out, unsigned int datalen, const char *Key, const unsigned char keylen)
{
        int i;
        static char bMode = ECB;

        if ((In == NULL) || (Out == NULL) || (datalen == 0) || ((datalen % 8) != 0) || (Key == NULL))
        {
                return 0;
        }

        if (bMode == ECB)	//ECB模式
        {
                memcpy(Out, In, datalen);

                switch (bType)
                {
                case 0: //加密
                        while (datalen > 0)
                        {
                                TEA_encrypt((unsigned long *)Out, (unsigned long *)Key);
                                Out+= 8;
                                datalen-= 8;
                        }
                        break;
                case 1: //解密
                        while (datalen > 0)
                        {
                                TEA_decrypt((unsigned long *)Out, (unsigned long *)Key);
                                Out+= 8;
                                datalen-= 8;
                        }
                        break;
                default:
                        return 0;
                        break;
                }
        }
        else //CBC模式
        {
                char	cvec[8]	=	{0};	//扭转向量
                //char	cvin[8]	=	"";     //中间变量

                switch (bType)
                {
                case 0: //加密
                        while (datalen > 0)
                        {
                                for (i = 0; i < 8; i++)
                                {
                                        Out[i]	=	In[i] ^ cvec[i];
                                }

                                TEA_encrypt((unsigned long *)Out, (unsigned long *)Key);
                                memcpy(cvec, Out, 8);

                                In+= 8;
                                Out+= 8;
                                datalen-= 8;
                        }
                        break;
                case 1: //解密
                        memcpy(Out, In, datalen);

                        while (datalen > 0)
                        {
                                TEA_decrypt((unsigned long *)Out, (unsigned long *)Key);

                                for (i = 0; i < 8; i++)
                                {
                                        Out[i]	=	Out[i] ^ cvec[i];
                                }

                                memcpy(cvec, In, 8);
                                In+= 8;
                                Out+= 8;
                                datalen-= 8;
                        }
                        break;
                default:
                        return 0;
                        break;
                }
        }

        return 1;
}


//  函 数 名 称:	RunDes
//  功 能 描 述:	执行DES算法对文本加解密
//  参 数 说 明:	bType	:类型：加密ENCRYPT，解密DECRYPT
//					bMode	:模式：ECB,CBC
//					In		:待加密串指针
//					Out		:待输出串指针
//					datalen	:待加密串的长度，同时Out的缓冲区大小应大于或者等于datalen 必须为8的整数倍，否则返回错误
//					Key		:密钥(可为8位,16位,24位)支持3密钥
//					keylen	:密钥长度，多出24位部分将被自动裁减
//
//  返回值 说明：	bool	:是否加密成功 1:成功，0：失败
int RunDes(char bType, char *In, char *Out, unsigned int datalen, const char *Key, const unsigned char keylen)
{
        char EncryptKey[24] = {0x00};
        int DesNum = -1;
        static char bMode = ECB;
        char m_SubKey[3][16][48]; //分密钥


    int  i=0;
    int  j=0;
        int  k;  //20130201
    unsigned char nKey=0;

        //判断输入合法性

        if(!(In && Out && Key && datalen && keylen>=8)) return 0;

        //只处理8的整数倍，不足长度自己填充

        if(datalen & 0x00000007) return 0;

        //构造并生成SubKeys

        nKey	=	(keylen>>3)>3 ? 3: (keylen>>3);

        //是否需要重新生成子密钥
        //if (!((DesNum == nKey) && memcmp(EncryptKey, Key, nKey<<3))) //add gqhua 20110113
        if (!((DesNum == nKey) && !memcmp(EncryptKey, Key, nKey<<3))) //add gqhua 20110528
        {
                DesNum = nKey;
                memcpy(EncryptKey, Key, nKey<<3);

                for (i = 0; i < nKey; i++)
                {
                        SetSubKey(&m_SubKey[i],&Key[i<<3]);
                }
        }

        if (bMode == ECB)	//ECB模式
        {
                if (nKey	==	1)	//单Key
                {
                        for(i=0,j=datalen>>3;i<j;++i,Out+=8,In+=8)
                        {
                                BasicDes(Out,In,&m_SubKey[0],bType);
                        }
                }
                else
                if(nKey == 2)	//3DES 2Key
                {
                        for(i=0,j=datalen>>3;i<j;++i,Out+=8,In+=8)
                        {
                                BasicDes(Out,In,&m_SubKey[0],bType);
                                BasicDes(Out,Out,&m_SubKey[1],!bType);
                                BasicDes(Out,Out,&m_SubKey[0],bType);
                        }
                }
                else			//3DES 3Key
                {
                        for(i=0,j=datalen>>3;i<j;++i,Out+=8,In+=8)
                        {
//				char	Prd[8]	=	"";
                                BasicDes(Out,In,&m_SubKey[0],bType);
                                BasicDes(Out,Out,&m_SubKey[1],!bType);
                                BasicDes(Out,Out,&m_SubKey[2],bType);
                        }
                }
        }
        else				//CBC模式
        {
                char	cvec[8]	=	{0};	//扭转向量
                char	cvin[8]	=	""; //中间变量

                if (nKey == 1)	//单Key
                {
                        for(i=0,j=datalen>>3;i<j;++i,Out+=8,In+=8)
                        {
                if (bType==0)//加密
                {
                    for(k=0;k<8;++k)		//将输入与扭转变量异或
                    {
                        cvin[k]	=	In[k] ^ cvec[k];
                    }
                    BasicDes(Out,cvin,&m_SubKey[0],bType);
                    memcpy(cvec,Out,8);			//将输出设定为扭转变量
                }
                else
                {
                    BasicDes(Out,In,&m_SubKey[0],bType);
                    for(k=0;k<8;++k)		//将输出与扭转变量异或
                    {
                        Out[k]	=	Out[k] ^ cvec[k];
                    }
                    memcpy(cvec,In,8);			//将输入设定为扭转变量
                }
                        }
                }
                else if (nKey == 2)	//3DES CBC 2Key
                {
                        for(i=0,j=datalen>>3;i<j;++i,Out+=8,In+=8)
                        {
                if (bType==0)//加密
                {
                    for(k=0;k<8;++k)		//将输入与扭转变量异或
                    {
                        cvin[k]	=	In[k] ^ cvec[k];
                    }
                    BasicDes(Out,cvin,&m_SubKey[0],bType);
                    BasicDes(Out,Out,&m_SubKey[1],!bType);
                    BasicDes(Out,Out,&m_SubKey[0],bType);
                    memcpy(cvec,Out,8);			//将输出设定为扭转变量
                }
                else //解密
                {
                    BasicDes(Out,In,&m_SubKey[0],bType);
                    BasicDes(Out,Out,&m_SubKey[1],!bType);
                    BasicDes(Out,Out,&m_SubKey[0],bType);
                    for(k=0;k<8;++k)		//将输出与扭转变量异或
                    {
                        Out[k]	=	Out[k] ^ cvec[k];
                    }
                    memcpy(cvec,In,8);			//将输入设定为扭转变量
                }
                        }
                }
                else			//3DES CBC 3Key
                {
                        for( i=0,j=datalen>>3;i<j;++i,Out+=8,In+=8)
                        {
                if (bType==0)//加密)
                {
                    for(k=0;k<8;++k)		//将输入与扭转变量异或
                    {
                        cvin[k]	=	In[k] ^ cvec[k];
                    }

                    BasicDes(Out,cvin,&m_SubKey[0],bType);
                    BasicDes(Out,Out,&m_SubKey[1],!bType);
                    BasicDes(Out,Out,&m_SubKey[2],bType);

                    memcpy(cvec,Out,8);			//将输出设定为扭转变量
                }
                else
                {
                    BasicDes(Out,In,&m_SubKey[2],bType);
                    BasicDes(Out,Out,&m_SubKey[1],!bType);
                    BasicDes(Out,Out,&m_SubKey[0],bType);

                    for(k=0;k<8;++k)		//将输出与扭转变量异或
                    {
                        Out[k]	=	Out[k] ^ cvec[k];
                    }

                    memcpy(cvec,In,8);			//将输入设定为扭转变量
                }
                        }
                }
        }

        return 1;
}


int xn_init(int uart_port)
{
    _IN_PARAM pm;
    memset(&pm,0,sizeof(pm));
    pm.isPsam=1;        //启用PSAM卡
    pm.com=uart_port;   //保存串口信息
    strcpy(pm.path,"note/archives/sys.ini");
    sdk_get_comfd(uart_port);
    pit_init_sysini(pm);
    pit_init_param(pm);
    pit_init_card_cfg();
    initPsamCard( );
}



// 20170513
// ispsam: 是否启用psam卡, 1-启用
// psam 启用禁用改变或sys.ini更新后，调用该接口
static int isdowedscpuinit = 0;
int weds_cpu_init(int ispsam)
{
    _IN_PARAM pm;
    memset(&pm,0,sizeof(pm));
	
    pm.isPsam = ispsam;        //是否启用PSAM卡
    strcpy(pm.path, "note/archives/sys.ini");

    pit_init_sysini(pm);
    pit_init_param(pm);
    pit_init_card_cfg();

	if(pm.isPsam == 1)
	{
		initPsamCard( );
	}    

	isdowedscpuinit = 1;
}

// 威尔标准消费CPU卡，内部认证通过后，返回卡号
int read_cpu_data_xn(int uart_port,char *no)
{
	int ret = 0;
	unsigned char recv_buf[128],cardno[128];
	static int init_flag=0;
	_IN_PARAM in;
	_OUT_PARAM out;

	if (isdowedscpuinit == 0)	// 兼容酬勤
	{
		weds_cpu_init(1);
	}

	memset(recv_buf,0,sizeof(recv_buf));
	ret=device_recv_data(uart_port,cardno);
	if(!(ret==CARD_CPU||ret==CARD_IC))
	{
	return ERROR;
	}
	memset(&in,0,sizeof(in));
	memset(&out,0,sizeof(out));

	in.cardlx = CARD_CPU;
	in.reader = WEDS_READER_CPU;
	in.com = uart_port;
	in.cardlen = 4;
	in.curTimes = time(NULL);
	strcpy(in.phycard, cardno);
	ret = pit_card_main_std(in, &out);
	if(ret==ERR_OK)
	{
		//返回物理卡号，逻辑卡号
		sprintf(no,"%s,%08d",in.phycard,IssuInfo.issuSerialNo);
		printf("%s\n",no);
		return 8;
	}
	 
    return 0;

}



#endif




//深圳智天行ztx-g20远距离射频卡

#define PRESET_VALUE 0xFFFF
#define POLYNOMIAL  0x8408
unsigned int uiCrc16Cal(unsigned char const  * pucY, unsigned char ucX)
{
    unsigned char ucI,ucJ;
    unsigned short int  uiCrcValue = PRESET_VALUE;

    for(ucI = 0; ucI < ucX; ucI++)
    {
        uiCrcValue = uiCrcValue ^ *(pucY + ucI);
        for(ucJ = 0; ucJ < 8; ucJ++)
        {
            if(uiCrcValue & 0x0001)
            {
                uiCrcValue = (uiCrcValue >> 1) ^ POLYNOMIAL;
            }
            else
            {
                uiCrcValue = (uiCrcValue >> 1);
            }
        }
    }
    return uiCrcValue;
}

int ztx_g20_crc16(unsigned char *data)
{
    unsigned int ret=0,lsb,msb;
    ret=uiCrc16Cal(data,data[0]-2+1);
    lsb=ret&0xff;
    msb=(ret&0xff00)>>8;
    if((lsb==data[data[0]-1])&&(msb==data[data[0]]))
    return 0;
    return -1;
}


int ztx_g20_crc16_set(unsigned char *data)
{
    unsigned int ret=0,lsb,msb;
    ret=uiCrc16Cal(data,data[0]-2+1);
    lsb=ret&0xff;
    msb=(ret&0xff00)>>8;
    data[data[0]-1]=lsb;
    data[data[0]]=msb;
}

#define HASHTABLESIZE   256

typedef struct _NODE
{
    char  data[32];
    unsigned long clock;
    struct _NODE* next;
}NODE;

typedef struct _HASH_TABLE
{
    NODE* value[HASHTABLESIZE];
}HASH_TABLE;

HASH_TABLE* cardhashtable=NULL;
HASH_TABLE* create_hash_table()
{
    HASH_TABLE* pHashTbl = (HASH_TABLE*)malloc(sizeof(HASH_TABLE));
    memset(pHashTbl, 0, sizeof(HASH_TABLE));
    return pHashTbl;
}


NODE* find_data_in_hash(HASH_TABLE* pHashTbl, char * data)
{
    NODE* pNode;
    unsigned int hash;
    if(NULL ==  pHashTbl)
        return NULL;
      hash=FNVHash1(data,strlen(data));
    if(NULL == (pNode = pHashTbl->value[hash % HASHTABLESIZE]))
        return NULL;
    while(pNode){
        if(strcmp(data,pNode->data)==0)
            return pNode;
        pNode = pNode->next;
    }
    return NULL;
}


int  insert_data_into_hash(HASH_TABLE* pHashTbl, char * data)
{
    NODE* pNode;
    unsigned int hash;
    if(NULL == pHashTbl)
        return FALSE;

        hash=FNVHash1(data,strlen(data));
    if(NULL == pHashTbl->value[hash % HASHTABLESIZE]){
        pNode = (NODE*)malloc(sizeof(NODE));
        memset(pNode, 0, sizeof(NODE));
        strcpy(pNode->data,data);
        pHashTbl->value[hash % HASHTABLESIZE] = pNode;
        pNode->clock=GetTickCount();
        return TRUE;
    }
pNode=find_data_in_hash(pHashTbl, data);
    if(NULL != pNode)
    {
        pNode->clock=GetTickCount();
        return FALSE;
    }

    pNode = pHashTbl->value[hash % HASHTABLESIZE];
    while(NULL != pNode->next)
        pNode = pNode->next;

    pNode->next = (NODE*)malloc(sizeof(NODE));
    memset(pNode->next, 0, sizeof(NODE));
    strcpy(pNode->next->data,data);
    pNode->clock=GetTickCount();
    return TRUE;
}


int delete_data_from_hash(HASH_TABLE* pHashTbl, char *data)
{
    NODE* pHead;
    NODE* pNode;
    unsigned int hash;
    hash=FNVHash1(data,strlen(data));
    if(NULL == pHashTbl || NULL == pHashTbl->value[hash % HASHTABLESIZE])
        return FALSE;

    if(NULL == (pNode = find_data_in_hash(pHashTbl, data)))
        return FALSE;

    if(pNode == pHashTbl->value[hash % HASHTABLESIZE]){
        pHashTbl->value[hash % HASHTABLESIZE] = pNode->next;
        goto final;
    }

    pHead = pHashTbl->value[hash % HASHTABLESIZE];
    while(pNode != pHead ->next)
        pHead = pHead->next;
    pHead->next = pNode->next;

final:
    free(pNode);
    return TRUE;
}


int clear_hash_by_clock(HASH_TABLE* pHashTbl,unsigned int msec)
{
int i=0,j=0;
NODE* pHead;
NODE* pNode;
for(i=0;i<HASHTABLESIZE;i++)
{
  pHead=pHashTbl->value[i];
  pNode=pHead;
  while(pNode)
  {
    if(abs(GetTickCount( )-pNode->clock)>msec)
    {
      if(pNode==pHashTbl->value[i])
      {
        pHashTbl->value[i]=pHashTbl->value[i]->next;
        pHead=pHashTbl->value[i];
      }else {
        pHead=pNode->next;
      }
      free(pNode);
      pNode=NULL;
    }
    if(pHead)
     pNode=pHead->next;
  }
}
}

int find_all_data_from_hash(HASH_TABLE* pHashTbl,char *data)
{
int i=0,j=0,flag=0;
NODE* pHead;
NODE* pNode;
for(i=0;i<HASHTABLESIZE;i++)
{
  pHead=pHashTbl->value[i];
  pNode=pHead;
  while(pNode)
  {
  if(flag==0)
   {strcat(data,pNode->data);
    flag++;
   }else
   {
    strcat(data,",");
    strcat(data,pNode->data);
   }
  }
}
}
unsigned int ztx_clear_outtime=5000;
int ztx_g20_set_outtime(unsigned int msec)
{
    ztx_clear_outtime=msec;
}

int read_ztx_g20(int fd,char *value)
{
    //serial_send_data
    //serial_recv_data
   // unsigned char data[1024]={0x2D,0x00,0x01,0x01,0x03,0x0C,0xE2,0x00,0x34,0x12,0x01,0x2A,0x17,0x00,0x00,0x46,0x7B,0xF9,0x0C,0xE2,0x00,0x34,0x12,0x01,0x38,0xFD,0x00,0x02,0xC3,0x6E,0x63,0x0C,0xE2,0x00,0x34,0x12,0x01,0x2F,0x17,0x00,0x00,0x46,0x7B,0xA9,0x5D,0xE0};
    unsigned char data[1024]={0x11,0x00,0xEE,0x00,0xE2,0x00,0x34,0x12,0x01,0x30,0xFD,0x00,0x04,0x82,0xB5,0x63,0x0A,0xF8};
    unsigned char senddata[32]={0x06,0x00,0x01,0x00,0x06,0xFA,0x34};
    char *ptr;
   char cardno[36];
   int ret=0,len=0,i=0,num=0,firstcard=1,count=0;
   int PACKAGELEN=0;
   //int ADDR=1;
   int CMD=2;
   int STATUS=3;
   int NUM=4;
   static int first=1;
   if(first)
   {
   //    printf("%s 1\n",__func__);
       cardhashtable=create_hash_table();
       first=0;
   }
   ret=serial_recv_data(fd,data,sizeof(data));
   len=ret;
//   printf("%s1,fd=%d,%d\n",__func__,fd,ret);
   //find package
   while(ret>0&&ret-i>=data[i])

   {
       if(ztx_g20_crc16(data+i)==0)
       {
           //printf("%s 3 %d %d %d\n",__func__,i,data[i],data[i+CMD]);
           switch(data[i+CMD])
           {
           case 0x01:
                switch(data[i+STATUS])
               {
               case 0x01:
               case 0x02:
               case 0x03:
               case 0x04:
                   //获取数据包内的多个卡号
                   num=data[i+NUM];
                   ptr=&data[i+STATUS+2];
                   while(num--)
                   {
                       len=*ptr++;
                       memset(cardno,0,sizeof(cardno));
                       hex2string(ptr,cardno,len);
                       //cardno[len*2]=0;
                       ptr+=len;
                       if(find_data_in_hash(cardhashtable,cardno)==NULL)
                       {
                           if(firstcard)
                           {
                               strcat(value,cardno);
                               firstcard=0;
                           }else {
                               strcat(value,",");
                               strcat(value,cardno);
                           }
                           count++;
                          // printf("cardno len=%d,%s\n",len,cardno);
                       }
                       insert_data_into_hash(cardhashtable,cardno);


                   }
               case 0xFB:break;
               }
                break;
           case 0xEE:
                ptr=data+i+STATUS+1;
                len=data[i+PACKAGELEN]-5;
               // printf("%s2 i=%d,len=%d\n",__func__,i,data[i+PACKAGELEN]);
                hex2string(ptr,cardno,len);

                if(find_data_in_hash(cardhashtable,cardno)==NULL)
                {
                    if(firstcard)
                    {
                        strcat(value,cardno);
                        firstcard=0;
                    }else {
                        strcat(value,",");
                        strcat(value,cardno);
                    }
                    count++;
                   // printf("cardno len=%d,%s\n",len,cardno);
                }
                insert_data_into_hash(cardhashtable,cardno);


                break;
           default:
                break;
           }

           i+=data[i]+1;
           continue;
       }
       i++;
   }

  // find_all_data_from_hash(cardhashtable,value);
   clear_hash_by_clock(cardhashtable,ztx_clear_outtime);
   serial_send_data(fd,senddata,7);
   return count;
}








int leeDebugData( unsigned char *blkData, unsigned int dataLen, unsigned int lineLen, unsigned int sendRecvFlag )
{
	unsigned int i = 0;

	lineLen = lineLen > dataLen ? dataLen : lineLen;

	if( sendRecvFlag == 0 )
	{
		printf( "\n************* debug *************\n" );
	}else if( sendRecvFlag == 1 )
	{
		printf( "\n--->" );
	}else
	{
		printf( "\n<---" );
	}

	for( i = 0; i < dataLen; i++ )
	{
		if( i && i % lineLen == 0 )
		{
			printf( "\n" );
		}

		printf( "%02X ", blkData[i] );
	}

	//if (sendRecvFlag == 0)
	{
		printf( "\n" );
	}

	return 0;
}





























