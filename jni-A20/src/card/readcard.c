/**
 * @chinese
 * @file   readcard.c
 * @author 刘训
 * @date   Thu Jul 14 16:46:20 2011
 *
 * @brief weds协议卡头操作模块，读写扇区操作
 * @endchinese
 *
 * @english
 * @file   readcard.c
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
#include <ctype.h>
#include "readcard.h"
#include "public.h"
#include "config.h"
#include "device_protocol.h"
#include "serial.h"
#include "debug.h"
#include "../datetime/date_time.h"

extern int	track_number;       /**< 设置磁条卡磁道数 */
int			treaty_start, treaty_end, treaty_len, data_start, data_len;


/**
 * @chinese
 * 设置读取磁条卡序列号使用的磁道数
 *
 * @param track 设置磁道值(取值范围(0-2))
 *
 * @return
 * @endchinese
 *
 * @english
 * set track number of Magnetic stripe card
 *
 * @param track number
 *
 * @return none
 * @endenglish
 *
 */
void set_track_number( int track )
{
	track_number = track;
}

/**
 * @chinese
 * 使用weds协议读取扇区内容
 * @param uart_port 串口号
 * @param beg_sectors 开始扇区号 (0-15)
 * @param end_sectors 结束扇区号(0-15)
 * @param beg_block 开始扇块号 (0-3)
 * @param end_block 结束扇块号(0-3)
 * @param keymode 0X01:A-key,0X02:B-key
 * @param key 密钥值（32Byte字节）
 * @param value 返回读取到的扇区内容
 *
 * @return 成功返回读取到扇区内容的字节数
                失败返回FALSE
 * @endchinese
 *
 * @english
 * read sector of MF1
 *
 *
 * @return
 * @endenglish
 *
 */
int mf1_read_sectors( int uart_port, int beg_sectors, int end_sectors, int beg_block,
                      int end_block, char keymode, char *key, char *value )
{
	int				i, n, j, flag = 1, len = 0;
	unsigned char	buf[2048];

	if( beg_sectors < 0 || end_sectors > 39 || beg_block < 0 || end_block > 14 ) //S70卡
	{
		return ERROR;
	}
//    printf("SDK read value=====%d,%d,%d,%d,%d,%d,%s,%02X\n",uart_port,beg_sectors,end_sectors,beg_block,end_block,keymode,key,value[0]);

	memset( buf, 0, sizeof( buf ) );
	for( n = beg_sectors; n <= end_sectors; n++ )
	{
		for( j = beg_block; j <= end_block; )
		{
			if( n == 0 && j == 0 )
			{
				j++; continue;
			}
			if( flag )
			{
				if( mf1_recv_data( uart_port, n, j, (char *)&buf[len], keymode, key ) < 0 )
				{
//                  printf("=====0\n");
					return ERROR;
				}

				flag = 0;
				j++;
			}else
			{
				if( mf1_recv_data( uart_port, n, j, (char *)&buf[len], keymode, NULL ) < 0 )
				{
					return ERROR;
				}
				flag = 0;
				j++;
			}
			len = len + 16;
		}
	}
	for( i = 0; i < len; i++ )
	{
		value[i] = (int)buf[i];
	}
//    printf("=====1=====%d\n",len);
	return len;
}

int mf1_read_sectors_bk( int uart_port, int beg_sectors, int end_sectors, int beg_block,
                         int end_block, char keymode, char *key, char *value, int outtime )
{
	int				i, n, j, flag = 1, len = 0;
	unsigned char	buf[2048];

	if( beg_sectors < 0 || end_sectors > 39 || beg_block < 0 || end_block > 14 ) //S70卡
	{
		return ERROR;
	}

	memset( buf, 0, sizeof( buf ) );
	for( n = beg_sectors; n <= end_sectors; n++ )
	{
		for( j = beg_block; j <= end_block; )
		{
			if( n == 0 && j == 0 )
			{
				j++; continue;
			}
			if( flag )
			{
				if( mf1_recv_data_bk( uart_port, n, j, (char *)&buf[len], keymode, key, outtime ) < 0 )
				{
					return ERROR;
				}

				flag = 0;
				j++;
			}else
			{
				if( mf1_recv_data_bk( uart_port, n, j, (char *)&buf[len], keymode, NULL, outtime ) < 0 )
				{
					return ERROR;
				}
				flag = 0;
				j++;
			}
			len = len + 16;
		}
	}
	for( i = 0; i < len; i++ )
	{
		value[i] = (int)buf[i];
	}
	return len;
}

/**
 * @chinese
 * 使用weds协议写入扇区内容
 * @param uart_port 串口号
 * @param beg_sectors 开始扇区号 (0-15)
 * @param end_sectors 结束扇区号(0-15)
 * @param beg_block 开始扇块号 (0-3)
 * @param end_block 结束扇块号(0-3)
 * @param keymode 0X01:A-key,0X02:B-key
 * @param key 密钥值
 * @param value 写入扇区的内容
 *
 * @return 成功返回TRUE
                失败返回FALSE
 * @endchinese
 *
 * @english
 * write sector of MF1
 *
 *
 * @return
 * @endenglish
 *
 */
int mf1_write_sectors( int uart_port, int beg_sectors, int end_sectors, int beg_block,
                       int end_block, char keymode, char *key, char *value )
{
	int				n, j, flag = 1, len = 0;
	char			tmp[2048];
	unsigned char	buf[26];

	if( beg_sectors < 0 || end_sectors > 15 || beg_block < 0 || end_block > 2 )
	{
		return ERROR;
	}
//printf("SDK input value=====%d,%d,%d,%d,%d,%d,%s,%02X\n",uart_port,beg_sectors,end_sectors,beg_block,end_block,keymode,key,value[0]);
//    strncpy(tmp,value,1024);
	memcpy( tmp, value, 1024 );

	for( n = beg_sectors; n <= end_sectors; n++ )
	{
		for( j = beg_block; j <= end_block; )
		{
			if( n == 0 && j == 0 )
			{
				j++; continue;
			}
			memset( buf, 0, sizeof( buf ) );
			memcpy( buf, &tmp[len], 16 );
			if( flag )
			{
				if( mf1_send_data( uart_port, n, j, (char *)buf, keymode, key ) < 0 )
				{
					return ERROR;
				}

				flag = 0;
				j++;
			}else
			{
				if( mf1_send_data( uart_port, n, j, (char *)buf, keymode, NULL ) < 0 )
				{
					return ERROR;
				}
				plog( "\nwrite right\n" );
				flag = 0;
				j++;
			}
			len = len + 16;
		}
	}
	return SUCCESS;
}

/**
 * @chinese
 * 解析ID卡格式,‘LINEAR’ format  description
        EXAMPLE:
        //linear(13 Decimal char)
        D0  D1 |  D2  D3  D4  D5  D6  D7  D8  D9           D=4 bits
        8 bits          32 bits
        000-255           000..00   -  4294967295          =13 digits (characters).
        3 decimal                      10 decimal
    digits                                 digits
    00000100 00010000110110000011110111101011 (40 bits)
    ‘LINEAR’ decoding:   0040282607083 (13 decimal digits )
 * @param cardsno (入口参数)原始卡号
 * @param param (出口参数)解析后的卡号
 *
 * @return 成功返回TRUE
                失败返回FALSE
 * @endchinese
 *
 * @english
 * analyze ID number
 *
 * @param cardsno original card number
 * @param param new card number
 *
 * @return
 * @endenglish
 *
 */
int card_linearhandle( char *cardsno, char*param )
{
	int					len = 0;
	char				tmpsno[128];
	unsigned long long	cardno = 0, cardno1;

	if( cardsno == NULL || strlen( cardsno ) == 0 )
	{
		return FALSE;
	}
	len = strlen( cardsno );
	memcpy( tmpsno, cardsno, 2 );
	cardno1	   = strtoull( tmpsno, NULL, 16 );
	cardno	   = strtoull( &cardsno[2], NULL, 16 );
	sprintf( param, "%03llu%010llu", cardno1, cardno );
	return TRUE;
}

/**
 * @chinese
 **解析ID卡格式,
 *		EXAMPLE:
        //syner card no.(14 Decimal char)
        b0  b1  b2 |  b3  b4  b5 | b6 ……b38 | b39                    b=1 bit
           0-7           0-7
         octal                octal                    =14 digits (characters).
        EXAMPLE  for  the following bit stream :
        0000010000010000110110000011110111101011 (40 bits)
        ‘SYNEL’    decoding:  35763066020200 (14 octal digits )
 * @param cardsno (入口参数)原始卡号
 * @param param (出口参数)解析后的卡号
 *
 * @return 成功返回TRUE
                失败返回FALSE
 * @endchinese
 *
 * @english
 * analyze ID number
 *
 * @param cardsno original card number
 * @param param new card number
 *
 * @return
 * @endenglish
 *
 */
int card_synelhandle( char *cardsno, char*param )
{
	char				sno[128];
	int					i;
	unsigned long long	cardno = 0;

	if( cardsno == NULL || strlen( cardsno ) == 0 )
	{
		return FALSE;
	}

	cardno = strtoull( cardsno, NULL, 16 );
	memset( sno, 0, sizeof( sno ) );
	sprintf( sno, "%014llo", cardno );

	for( i = 0; i < 14; i++ )
	{
		param[i] = sno[13 - i];
	}
	return TRUE;
}

/**
 * @chinese
 * 从右截取字符串
 * @param sno 原字符串,该参数作为截取后的字符串返回
 * @param rightnum 截取长度
 *
 * @return
 * @endchinese
 *
 * @english
 * cut out string from the right
 *
 * @param sno string
 * @param rightnum length to cut
 *
 * @return
 * @endenglish
 *
 */
void right( char * sno, int rightnum )
{
	char	tmpsno[1024];
	int		len, j;
	char	*p1;

	if( sno == NULL || rightnum < 0 )
	{
		return;
	}
	memset( tmpsno, 0, sizeof( tmpsno ) );
	len = strlen( sno );
	strncpy( tmpsno, sno, 1024 );
	p1 = tmpsno;
	j  = len - rightnum;
	if( j < 0 )
	{
		return;
	}
	p1 += j;
	if( len > 1024 )
	{
		len = 1024;
	}
	memset( sno, 0, len );
	strncpy( sno, p1, len );
	return;
}

/**
 * @chinese
 * 截取字符串最后的'\0'操作
 * @param src 原字符串,该参数作为截取后的字符串返回
 *
 * @return
 * @endchinese
 *
 * @english
 * cut out '\0' end of string
 *
 * @param src string
 *
 * @return
 * @endenglish
 *
 */
void cutzero( char *src )
{
	char	*ptr, *str;
	int		flag = 0;

	ptr = str = src;
	while( *str )
	{
		if( ( flag ) || ( *str != '0' ) )
		{
			*ptr++ = *str++;
			flag   = 1;
		}else
		{
			str++;
		}
	}
	*ptr = '\0';
}

/**
 *
 **通过所设置的协议读取串口数据
 * @param uart_port 使用的串口号
 * @param value 通过串口读取到的有效数据
 *
 * @return 成功 1,失败-(-1)
 */
int read_text_card( int uart_port, char *value )
{
	char *p;
	p = value;
	memset( value, 0, sizeof( value ) );
	/* 接收数据 */
	if( serial_recv_data( uart_port, (unsigned char *)p, treaty_len ) < 0 )
	{
		serial_clear( (TCOM)uart_port );
		return ERROR;
	}
	if( p[0] != treaty_start )
	{
		return BEGINERROR;
	}
	if( p[treaty_len - 1] != treaty_end )
	{
		return ENDERROR;
	}
	memcpy( value, &p[data_start], data_len );
	value[data_len] = '\0';
	serial_clear( (TCOM)uart_port ); //清空串口缓存
	return SUCCESS;
}

/**
 * @chinese
 * 设置textcars规约格式
 * head:0 data:1-10 verify bit:11 end:12
 * @param t_start 协议起始位
 * @param t_end 协议结束位
 * @param t_len 协议长度
 * @param d_start 有效数据起始位
 * @param d_len 有效数据长度
 * @return 1 成功 -1 失败
 * @endchinese
 */
int set_textcard_mode( int t_start, int t_end, int t_len, int d_start, int d_len )
{
	if( ( d_start + d_len - 1 ) > ( t_len - 2 ) || d_start < 1 )
	{
		treaty_start   = 2;
		treaty_end	   = 3;
		treaty_len	   = 13;
		data_start	   = 1;
		data_len	   = 10;
		return FALSE;
	}
	treaty_start   = t_start;
	treaty_end	   = t_end;
	treaty_len	   = t_len;
	data_start	   = d_start;
	data_len	   = d_len;
	return TRUE;
}

/*
 * 主动寻cpu卡
 * 1 = success 0 = fail
 */
int active_homing_card( int uart_port, char *value )
{
	int				ret = 0;
	char			data[256];
	struct timeval	oldtimer, newtimer;

	//寻卡指令
	memset( data, 0, sizeof( data ) );
	data[0]	   = 0X95;
	ret		   = device_send_data( uart_port, 0X01, SEARCH_CARD, 0X01, data, 1 );

	gettimeofday( &oldtimer, NULL );
	gettimeofday( &newtimer, NULL );
	while( 1 )
	{
		memset( data, 0, sizeof( data ) );
		ret = device_recv_data( uart_port, data );

		if( ret > 0 )
		{
//			printf( "card no. %s card type %02x\n", data, ret );

			if( ret != CPU_CARDNO )
			{
				ret = 0;
				break;
			} else
			{
				memmove( value, data, strlen( data ) );
				ret = 1;
				break;
			}
		}

		gettimeofday( &newtimer, NULL );
		if( ( newtimer.tv_sec - oldtimer.tv_sec ) * 1000
		    + ( newtimer.tv_usec - oldtimer.tv_usec ) / 1000 > 300 ) //ms
		{
//			printf( "ret %d\n", ret );
//			printf( "*******timeout********** \n" );
			return 0;
		}
	}

	return ret;
}

int send_and_recv_apdu( int uart_port, char *send, char *resp )
{
	struct timeval	oldtimer, newtimer;
	int				oldsec;
	int				ret_code   = 0;
	int				len		   = 0;
	char			ask[256];

	if( !send )
	{
		return ERROR;
	}

	memset( ask, 0, sizeof( ask ) );
	len = string2hex( send, ask, strlen( send ) );

	ret_code = device_send_data( uart_port, 0x01, TRANSIT, 0x01, ask, len );
	if( ret_code != SUCCESS )
	{
		return ERROR;
	}

	gettimeofday( &oldtimer, NULL );
	oldsec = ( oldtimer.tv_sec % 10000 ) * 1000 + oldtimer.tv_usec / 1000;

	while( 1 )
	{
		gettimeofday( &newtimer, NULL );
		if( abs(
		        ( ( newtimer.tv_sec % 10000 ) * 1000 + newtimer.tv_usec / 1000 )
		        - oldsec ) > 300 )
		{
			return ERROR;
		}

		ret_code = device_recv_data( uart_port, resp );
		if( ret_code == TRANSIT )
		{
			break;
		}else
		{
			return ERROR;
		}
	}

	return SUCCESS;
}

int read_xbt_card( int uart_port, char *value )
{
	FILE	*fp = NULL;
	char	key[20], buf[20], code[20], code_value[20], des_buf[20], tmp[20], *p = NULL, *p1 = NULL, *p_code = NULL;
	char	*p_code1   = NULL;
	int		len		   = 0, sector = 0, start = 0, end = 0, code_start = 0, code_end = 0;
	fp = fopen( "key.ini", "r" );
	if( fp == NULL )
	{
		return ERROR;
	}
	memset( buf, 0, sizeof( buf ) );
	memset( des_buf, 0, sizeof( des_buf ) );
	fgets( buf, sizeof( buf ), fp );
	cut_rn( buf );
	if( strlen( buf ) == 1 )
	{
		des_buf[0] = buf[0] - '0';
	}else
	{
		gsmString2Bytes( buf, des_buf, strlen( buf ) ); //even
	}
	sector = ( des_buf[0] >> 4 ) * 10 + des_buf[0] & 0X0F;
//	printf("sector=%d\n",sector);

	memset( buf, 0, sizeof( buf ) );
	memset( key, 0, sizeof( key ) );
	fgets( buf, sizeof( buf ), fp );
	cut_rn( buf );
	memcpy( key, buf, strlen( buf ) );
//	printf("key=%s\n",key);

	memset( buf, 0, sizeof( buf ) );
	memset( code, 0, sizeof( code ) );
	fgets( buf, sizeof( buf ), fp );
	cut_rn( buf );
	memcpy( code, buf, strlen( buf ) );
//	printf("code=%s\n",code);

	memset( buf, 0, sizeof( buf ) );
	memset( des_buf, 0, sizeof( des_buf ) );
	fgets( buf, sizeof( buf ), fp );
	cut_rn( buf );
//	printf("buf1=%s\n",buf);
	p_code = strtok( buf, "," );
	if( strlen( p_code ) == 1 )
	{
		des_buf[0] = *p_code - '0';
	}else
	{
		gsmString2Bytes( p_code, des_buf, strlen( p_code ) ); //even
	}
	code_start = ( des_buf[0] >> 4 ) * 10 + des_buf[0] & 0X0F;
	memset( des_buf, 0, sizeof( des_buf ) );
	p_code1 = strtok( NULL, "," );
	if( strlen( buf ) == 1 )
	{
		des_buf[0] = *p_code1 - '0';
	}else
	{
		gsmString2Bytes( p_code1, des_buf, strlen( p_code1 ) ); //even
	}
	code_end = ( des_buf[0] >> 4 ) * 10 + des_buf[0] & 0X0F;

//	printf("code len=%d,%d\n",code_start,code_end);
	memset( buf, 0, sizeof( buf ) );
	memset( des_buf, 0, sizeof( des_buf ) );
	fgets( buf, sizeof( buf ), fp );
	cut_rn( buf );
//	printf("buf=%s\n",buf);
	p = strtok( buf, "," );
	if( strlen( p ) == 1 )
	{
		des_buf[0] = *p - '0';
	}else
	{
		gsmString2Bytes( p, des_buf, strlen( p ) ); //even
	}
	start = ( des_buf[0] >> 4 ) * 10 + des_buf[0] & 0X0F;
	memset( des_buf, 0, sizeof( des_buf ) );
	p1 = strtok( NULL, "," );
	if( strlen( buf ) == 1 )
	{
		des_buf[0] = *p1 - '0';
	}else
	{
		gsmString2Bytes( p1, des_buf, strlen( p1 ) ); //even
	}
	end = ( des_buf[0] >> 4 ) * 10 + des_buf[0] & 0X0F;

	memset( des_buf, 0, sizeof( des_buf ) );
	fclose( fp );
	len = mf1_read_sectors( uart_port, sector, sector, 0, 0, 0X01, key, des_buf );
	if( len < 0 )
	{
		return ERROR;                               //ILLEGAL CARD
	}

	memset( tmp, 0, sizeof( tmp ) );
	memset( code_value, 0, sizeof( code_value ) );
	memcpy( tmp, &des_buf[code_start], code_end );
	gsmBytes2String( tmp, code_value, code_end );   //even

	if( strcmp( code, code_value ) != 0 )
	{
		return ERROR;                               //ILLEGAL CARD
	}

	memset( tmp, 0, sizeof( tmp ) );
	memcpy( tmp, &des_buf[start], end );
	gsmBytes2String( tmp, value, end );             //even

//	printf("=======%s,%d\n",value,strlen(tmp));
	return SUCCESS;
}

/*
 */

int active_read_card( int uart_port, char *value )
{
	int				ret = 0;
	char			data[256];
	struct timeval	oldtimer, newtimer;

	//寻卡指令
	memset( data, 0, sizeof( data ) );
	data[0]	   = 0XF1;
	ret		   = device_send_data( uart_port, 0X01, SEARCH_CARD_IC, 0X01, data, 1 );
	if( SUCCESS != ret )
	{
		return -1;
	}
	gettimeofday( &oldtimer, NULL );
	gettimeofday( &newtimer, NULL );
	while( 1 )
	{
		memset( data, 0, sizeof( data ) );
		ret = device_recv_data( uart_port, data );
		if( ret == 0x94 )
		{
			memmove( value, data, strlen( data ) );
			break;
		} else if( ret == 0 )
		{
			return -2;
			break;
		}

		gettimeofday( &newtimer, NULL );
		if( ( newtimer.tv_sec - oldtimer.tv_sec ) * 1000
		    + ( newtimer.tv_usec - oldtimer.tv_usec ) / 1000 > 100 )    //ms
		{              // printf("ret %d\n",ret);
			                                                            // printf("*******timeout********** \n");
			return -3;
		}
	}

	return TRUE;
}

//主动寻CPU 卡
int active_read_card_cpu( int uart_port, char *value )
{
	int				ret = 0;
	char			data[256];
	struct timeval	oldtimer, newtimer;

	//寻卡指令
	memset( data, 0, sizeof( data ) );
	data[0]	   = CPU_CARDNO;
	ret		   = device_send_data( uart_port, 0X01, SEARCH_CARD, 0X01, data, 1 );
	if( SUCCESS != ret )
	{
		return -1;
	}
	gettimeofday( &oldtimer, NULL );
	gettimeofday( &newtimer, NULL );
	while( 1 )
	{
		memset( data, 0, sizeof( data ) );
		ret = device_recv_data( uart_port, data );

		// printf("%s %d\n",__func__,ret);
		if( ret == CPU_CARDNO )
		{
			memmove( value, data, strlen( data ) );
			break;
		} else if( ret == 0 )
		{
			return -2;
			break;
		}

		gettimeofday( &newtimer, NULL );
		if( ( newtimer.tv_sec - oldtimer.tv_sec ) * 1000
		    + ( newtimer.tv_usec - oldtimer.tv_usec ) / 1000 > 100 )    //ms
		{              // printf("ret %d\n",ret);
			                                                            // printf("*******timeout********** \n");
			return -3;
		}
	}

	return TRUE;
}

int test_carder( int uart_port, char *value )
{
	int		ret = 0;
	char	cur[32];
	FILE	*file;
	ret = active_read_card( uart_port, value );
	if( ret == TRUE )
	{
		return ret;
	}
	file = fopen( "./test_data.log", "a" );
	if( file == NULL )
	{
		return ret;
	}
	memset( cur, 0, sizeof( cur ) );
	get_system_time2( cur );
	fprintf( file, "%,%d\n", cur, ret );
	fclose( file );
	return ret;
}

