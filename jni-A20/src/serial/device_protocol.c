/**
 * @chinese
 * @file   device_protocol.c
 * @author 刘训
 * @date   Thu Oct 13 10:00:00 2011
 *
 * @brief  设备通信协议处理模块
 * @endchinese
 *
 * @english
 * @file   device_protocol.c
 * @author Liu Xun
 * @date   Thu Oct 13 10:00:00 2011
 *
 * @brief  protocol between device handling module
 * @endenglish
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <stdlib.h>

#include "card_sectors.h"
#include "cardinfo.h"
#include "wiegand_input.h"
#include "device_protocol.h"
#include "serial.h"
#include "config.h"
#include "stringchar.h"
#include "debug.h"
#include "serial_devices.h"
#include "serial_async.h"
//#include "../android/wedsa23.h"


int						track_number = 0;   /**< 磁条卡磁道数 */
char					card_number [64];   /**< 卡号 */

extern pthread_mutex_t	gpio_mutex;

_RK3288_DATABUF			*RK3288_HEAD   = NULL;
unsigned int			RK3288cnt	   = 0;

int	RK3288_51port = 10; // gpio, wg，psam等单片机通信 使用的端口号，初始化时作为全局参数赋值，供后续使用 20170817



/**
 *
 * @chinese
 * 根据weds协议,读取串口数据
 *
 * @param port 串口号
 * @param value 读取到的串口数据
 *
 * @return 成功-指令,失败-ERROR
 * @endchinese
 *
 * @english
 * According to weds protocol,read data from serial
 *
 * @param port uart port number
 * @param value data of reading uart port
 *
 * @return success-instruction,fail-ERROR
 * @endenglish
 *
 */
int tongdao_itemnum = 0;
int  tongdao_id( )
{
	return tongdao_itemnum;
}

int device_recv_data( int port, char *value )
{
	static int		closebt = 0, deg=0;
	TDATA			tdata;
	int				len	   = 0;
	unsigned char	*p	   = NULL;
	int				i	   = 0, j = 1;
        int				retval = FALSE;
	char			data[16];
	_RK3288_DATABUF *RK3288_TMP = NULL, *RK3288_CUR = NULL;
       // LOGI("%s pid=%lu\n",__func__, pthread_self());

	memset( &tdata, 0, sizeof( tdata ) );
    retval = device_recv_RK3288_buf_data( &tdata );
	if( retval != TRUE )
	{	
		retval = serial_package_processor( port, &tdata, RECV_PACKAGE );
		if( retval < 0 )
		{
			return retval;
		}
	}

	memset( value, 0, sizeof( value ) );
	len				   = (int)tdata.nbytes;
	p				   = tdata.user_data;
	tongdao_itemnum	   = tdata.itemnum;

       // printf("%s,%d len=%d,%02X,%02X,%02X,%02X,%02X\n",__func__,port,len,tdata.instruction,p[0],p[1],p[2],p[3]);
	switch( tdata.instruction )
	{
		case READHIDCARD:
			if( (int)tdata.user_data[0] == 0x00 && (int)tdata.user_data[1] == 0x00 )
			{
				strcpy( value, "CARDERROR" );
				return -10;
				break;
			}else if( (int)tdata.user_data[0] == 0x00 && (int)tdata.user_data[1] == 0x01 )
			{
				strcpy( value, "EFFIACCYERROR" );
				return -11;
				break;
			}else if( (int)tdata.user_data[0] == 0x00 )
			{
				return -5;
			}

//        value[0]=len;
//        memcpy(value+1,tdata.user_data,len);

			for( i = 0; i < len; i++ )
			{
				sprintf( value + i * 2, "%02X", tdata.user_data[i] );
			}
			printf( "HID ... = %d, %s\n", len, value );
			break;
		case READPF: // iclass
			if( (int)tdata.itemnum == 0x00 )
			{
				if( (int)tdata.user_data[0] == 0x00 )
				{
					return -13;
					break;
				}else if( (int)tdata.user_data[0] == 0x01 )
				{
					return -14;
					break;
				}else
				{
					return -5;
					break;
				}
			}

			for( i = 0; i < len; i++ )
			{
				sprintf( value + i * 2, "%02X", tdata.user_data[i] );
			}
			break;

		/*键盘*/
		case KEYBOARD:
			if( (int)( *p ) == 16 )
			{
				sprintf( value, "%d", 0 );
			} else
			{
				sprintf( value, "%d", p[0] );
			}
			switch( terminal_type )
			{
				case F_TYPE:
					if( atoi( value ) == 14 )
					{
						memset( value, 0, sizeof( value ) );
						sprintf( value, "%d", 28 );
					}
					break;
			}
			break;

		case RK3288_GPIO_OUT:   // gpio 输出应答
		case RK3288_GPIO_IN:    // IO 输入状态变化
		case WIEGAND_CARD:      // wgout 应答
		case PSAMCMD:		// 透传指令应答 20170922
			if( RK3288cnt >= RK3288_MAXCNT )
			{
				freeRK3288Buf( );
			}

			pthread_mutex_lock( &gpio_mutex );
			RK3288_TMP = (_RK3288_DATABUF *)malloc( sizeof( _RK3288_DATABUF ) );
			if( RK3288_TMP != NULL )
			{
				memset( RK3288_TMP, 0, sizeof( _RK3288_DATABUF ) );
				memcpy( &RK3288_TMP->tdata, &tdata, sizeof( tdata ) );
				if( !RK3288_HEAD )
				{
					RK3288_HEAD		   = RK3288_TMP;
					RK3288_TMP->next   = NULL;
				}else // 放到链表尾
				{
					RK3288_CUR = RK3288_HEAD;
					while( RK3288_CUR )
					{
						if( RK3288_CUR->next == NULL )
						{
							RK3288_CUR->next = RK3288_TMP;
							break;
						}
						RK3288_CUR = RK3288_CUR->next;
					}
				}
				RK3288cnt++;
			}

			if( tdata.instruction == RK3288_GPIO_IN ) // 接收到GPIO输入状态，要立刻应答单片机,否则单片机会在50ms超时或重发
			{
				bzero( data, sizeof( data ) );
				data[0] = tdata.user_data[0];
				device_send_data( port, 0X01, RK3288_GPIO_IN, 0X01, data, 1 );
			}

			//LOGI( "case RK3288_GPIO_IN: put in RK3288buf, now cnt = %d, cmd=%02X, %02X, %02X\n", RK3288cnt, tdata.instruction, tdata.user_data[0],tdata.user_data[1] );
			pthread_mutex_unlock( &gpio_mutex );
			return 0;
			break;
		case READEMCARD /*读取id卡序列号*/:
			if( len == 1 )
			{
				if( (int)tdata.user_data[0] == 0x00 )
				{
					strcpy( value, "DECODERROR" );
					return -8;
					break;
				}else if( (int)tdata.user_data[0] == 0x01 )
				{
					strcpy( value, "CARDERROR" );
					return -9;
					break;
				}else
				{
					return -5;
				}
			}
			hex2string( (char *)p, value, len );
			break;
		/*读取MF1 S50卡序列号*/
		case MF1_S50_CARDNO:
			if( len == 1 )
			{
				if( (int)tdata.user_data[0] == 0x00 )
				{
					strcpy( value, "DECODERROR" );
					return -6;
					break;
				}else if( (int)tdata.user_data[0] == 0x01 )
				{
					strcpy( value, "CARDERROR" );
					return -7;
					break;
				}else
				{
					return -5;
				}
			}
			hex2string( (char *)p, value, len );
			strcpy( card_number, value );
			//清空读取扇区内容的控制变量
			cardHexToUInt( len, tdata.user_data );
			//设置卡片和卡头类型
			setCardType( tdata.instruction, WEDS_READER );

			break;
		/*读取MF1 S70卡序列号*/
		case MF1_S70_CARDNO:
			if( len == 1 )
			{
				if( (int)tdata.user_data[0] == 0x00 )
				{
					strcpy( value, "DECODERROR" );
					break;
				}else if( (int)tdata.user_data[0] == 0x01 )
				{
					strcpy( value, "CARDERROR" );
					break;
				}
			}
			hex2string( (char *)p, value, len );
			strcpy( card_number, value );
			//清空读取扇区内容的控制变量
			cardHexToUInt( len, tdata.user_data );
			//设置卡片和卡头类型
			setCardType( tdata.instruction, WEDS_READER );

			break;
		case CPU_CARDNO:
			hex2string( (char *)p, value, len );
			break;
		case TRANSIT:
			hex2string( (char *)p, value, len );
			break;
		/*读取磁条卡序列号*/
                case READMAGNETIC:
			if( track_number == 0 ) // 任何轨都返 20170526
			{
				;
			}     else if( (int)tdata.address != track_number )
			{
				break;
			}
//		printf("22\n");

			//读卡错误
			if( (int)tdata.nbytes == 1 )
                        {       //printf("33\n");
				if( (int)tdata.user_data[0] == 0x00 )
                                {   //printf("44\n");
					strcpy( value, "DECODERROR" );
					return -4;
					break;
				}else if( (int)tdata.user_data[0] == 0x01 )
                                {   //printf("55\n");
					strcpy( value, "CARDERROR" );
					return -1;
					break;
				}else
                                {
					return -5;
				}
			}
			i = 0;
			while( isxdigit( tdata.user_data[i] ) )
			{
				value[i] = (int)tdata.user_data[i];
				i++;
                        } //printf("66\n");
			break;
		/*读取条码卡序列号*/
		case READBARCODE:
			len = len - 1;
			if( len >= 0 )
			{
				if( (int)tdata.user_data[0] == 0x00 && (int)tdata.user_data[1] == 0x00 )
				{
					strcpy( value, "DECODERROR" );
					return -2;
					break;
				}else if( (int)tdata.user_data[0] == 0x00 && (int)tdata.user_data[1] == 0x01 )
				{
					strcpy( value, "CARDERROR" );
					return -3;
					break;
				}

				for( i = 0; i < len; i++ )
				{
					value[i] = (int)tdata.user_data[i + 1];
				}
			}else
			{
				strcpy( value, "ERROR" );
			}
			break;
		/*身份证*/
		case ID_CARDNO:
			if( len == 1 )
			{
				if( (int)tdata.user_data[0] == 0x00 )
				{
					strcpy( value, "DECODERROR" );
					break;
				}else if( (int)tdata.user_data[0] == 0x01 )
				{
					strcpy( value, "CARDERROR" );
					break;
				}
			}
			hex2string( (char *)p, value, len );
			break;

		case WIEGANDINPUT:
			retval = resolve_wiegand_input( (char *)p, value );
			deg++;
//			LOGI( "wg data %d ===%s\n", deg, value );
			//printf( "wg data2 ===%s\n", value );
			break;
		/*电池电量信息*/
		case BATTERY_INFO:


			/*
			        150: High voltage alarm
			        110:Low voltage alarm
			        160:Chip failure
			        120:No battery
			        130:Battery charging
			        140:Battery ful
			        0~~100:battery level %
			 */
#ifndef _ARM_A23
			value[0] = p[0];
//printf("ppppppppp = %02x\n", value[0]);
#else       //android 没有无符合类型，只能支持-128-127;
			switch( p[0] )
			{
				case 110: value[0] = 101; break;
				case 120: value[0] = 102; break;
				case 130: value[0] = 103; break;
				case 140: value[0] = 104; break;
				case 150: value[0] = 105; break;
				case 160: value[0] = 106; break;
				case 170: value[0] = 107; break;
				case 180: value[0] = 108; break;
				default: value[0]  = p[0]; break;
			}
#endif
			if( value[0] <= 160 && value[0] >= 10 )
			{
				//根据机型发送告知单片机是否发送电池电量情况
//            printf("terminal_type.... = %d\n", terminal_type);
				switch( terminal_type )
				{
					case K_TYPE:
//				printf("xxxxxxxxxxxxxxxxxxx\n");
//				if(closebt)
					{
						retval	   = send_close_battery_info( "BATTERY_INFO" );
						closebt	   = 1;
					}

					break;
				}
				if( retval < 0 )
				{
					printf( "send_close_battery_info error!!\n" );
				}
			}
			return tdata.instruction;
			break;

		case LIGHT_SENSE:	// 20180115 光感
		case HUMAN_SENSE:	// 20180115 人体感应
//			value[0] = p[0];
//			hex2string( (char *)p, value, len );
			memset(value, 0, sizeof(value));
			sprintf(value, "%d", p[0]);
			break;
			
		case CONTROL_P:     // mcu 发出关机指令 0x0F	,应用层可取消
		case CONTROL_PL:    // 应用层不可取消	0x1F
		case REBOOT_CMD:    // mcu返回重启信息，将在5s后重启	0x0E,应用层可取消掉
		case LIGHT_CMD:     // 背光控制指令	0x0D
			value[0] = p[0];
			break;

		/*错误反馈信息*/
		case FEEDBACK:
			value[0] = p[0];
			break;
		case GET_VERSION:
			value[0] = tdata.instruction;
			memcpy( value + 1, tdata.user_data, len );
			return SUCCESS;
			break;
		case SPI_TEST:
			value[0]   = p[0];
			value[1]   = p[1];
			break;
		case SEARCH_CARD:
			//printf("card type %02x\n",p[0]);
			//printf("card len %d\n",len);
			if( p[0] == 0x04 )
			{
				tdata.instruction = 0x95;
				hex2string( (char *)p + 1, value, 4 );
			}else
			{
				tdata.instruction = p[0];
				hex2string( (char *)p + 1, value, len - 1 );
			}
			break;
		case SEARCH_CARD_IC:
			tdata.instruction = 0x94;
			if( len > 1 )
			{
				hex2string( (char *)p + 1, value, len - 1 );
			}

		case UPDATEUARTCARD: // 0x3A 需要升级
			memcpy( &synel_tdata, &tdata, sizeof( tdata ) );
//			memcpy( value, &tdata, sizeof( tdata ) );				// 此处如果传入value的长度<tdata类型，调用后会导致溢出 20171120		
			memcpy( value, tdata.user_data, len );		// 20180207 lfg , 用于接口获取信息 int getUartCardType( int uartPort, char *outValue, int oSize )
			break;

		default:
			memcpy( value, tdata.user_data, len );
			break;
	}
	/*添加上此处以后可能会造成按键丢数，不添加的时候会造成操作快，读数据迟钝的显现,可以开到应用层处理*/
//	if( port < SPI_COM )
//	{
//		serial_clear( port );
//	}
	return tdata.instruction;
}

// 2017-08-09
int rk3288_device_recv_data( int port, TDATA *otdata )
{
	static int		closebt = 0;
	TDATA			tdata;
	int				len			   = 0;
	unsigned char	*p			   = NULL;
	int				i			   = 0, j = 1;
	int				retval		   = 0;
	_RK3288_DATABUF *RK3288_TMP	   = NULL;

	memset( &tdata, 0, sizeof( tdata ) );
	retval = serial_package_processor( port, &tdata, RECV_PACKAGE );
	if( retval < 0 )
	{
		return retval;
	}

	memcpy( otdata, &tdata, sizeof( tdata ) );
	return tdata.instruction;
}

// 缓冲区中是否有需要的数据
int device_recv_RK3288_buf_data( TDATA *otdata )
{
	int				ret			   = -1, flag = 0;
	_RK3288_DATABUF *RK3288_TMP	   = NULL, *RK3288_CUR = NULL;

	pthread_mutex_lock( &gpio_mutex );
	RK3288_TMP = RK3288_HEAD;
	RK3288_CUR = RK3288_TMP;
	while( RK3288_TMP )
	{
		switch( RK3288_TMP->tdata.instruction )
		{
			case RK3288_GPIO_OUT:   // gpio 输出应答
			case RK3288_GPIO_IN:    // IO 输入状态变化
			case WIEGAND_CARD:      // wgout 应答
			case PSAMCMD:		// 透传指令应答 20170922		
				// those type do not deal
				break;

			default:
				if( RK3288_TMP == RK3288_HEAD ) // 头单独处理
				{
					RK3288_HEAD = RK3288_TMP->next;
				}else
				{
					RK3288_CUR->next = RK3288_TMP->next;
				}
				memcpy( otdata, &( RK3288_TMP->tdata ), sizeof( TDATA ) );
				free( RK3288_TMP );
				RK3288cnt--;
				flag = 1;
//				printf("there is buf data in RK3288_buf, cmd = %02X\n", RK3288_TMP->tdata.instruction);
				goto END;
				break;

		}
		RK3288_CUR = RK3288_TMP;
		RK3288_TMP = RK3288_TMP->next;
	}
END:
	pthread_mutex_unlock( &gpio_mutex );
	if( flag == 1 )
	{
		return TRUE;
	}

	return FALSE;
}

/**
 *
 * @chinese
 * 根据weds协议,发送数据
 *
 * @param port 串口号
 * @param address 地址
 * @param instruction 指令
 * @param itemnum 数据包数量
 * @param data 有效数据
 * @param data_len 有效数据长度
 *
 * @return 成功-SUCCESS,失败-ERROR
 * @endchinese
 *
 * @english
 * According to weds protocol,send data
 *
 * @param port serial port
 * @param address address
 * @param instruction instruction
 * @param itemnum item num
 * @param data data
 * @param data_len lenth of data
 *
 * @return success-SUCCESS,fail-ERROR
 * @endenglish
 *
 */
int device_send_data( int port, char address, char instruction, char itemnum, char * data, int data_len )
{
	TDATA tdata;
	memset( &tdata, 0, sizeof( tdata ) );

	tdata.address	   = address;
	tdata.instruction  = instruction;
	tdata.itemnum	   = itemnum;
	tdata.nbytes	   = sizeof( instruction ) + sizeof( itemnum ) + data_len;
	memcpy( tdata.user_data, data, data_len );
        if( PLATFORM_TYPE == 1 || PLATFORM_TYPE == 2  || PLATFORM_TYPE == 3 || PLATFORM_TYPE == 4){	// 20180224 lfg add type 4 for 335 psam
            return serial_package_processor_async( port, &tdata, SEND_PACKAGE );
        }
	return serial_package_processor( port, &tdata, SEND_PACKAGE );
}

/**
 *
 * @chinese
 * 读取MF1扇区内容
 *
 * @param port 串口号
 * @param sector 扇区号
 * @param block 块号
 * @param data 返回读取到的扇区内容
 * @param mode 0X01:A-key,0X02:B-key
 * @param key 密钥值
 *
 * @return 成功-SUCCESS,失败-ERROR
 * @endchinese
 *
 * @english
 * read MF1 data
 *
 * @param port serial port
 * @param sector sector
 * @param block block
 * @param data data
 * @param mode 0X01:A-key,0X02:B-key
 * @param key key
 *
 * @return success-SUCCESS,fail-ERROR
 * @endenglish
 *
 */
int mf1_recv_data( int port, char sector, char block, char *data, char mode, char *key )
{
	return mf1_recv_data_bk( port, sector, block, data, mode, key, 1 );
}

int mf1_recv_data_bk( int port, char sector, char block, char *data, char mode, char *key, int outtime )
{
	TDATA			tdata;
	TDATA			answer;
	int				card_number_len	   = 0;
	int				key_len			   = 0;
	struct timeval	oldtimer, newtimer;
	int				oldsec;
	memset( (char *)&tdata, 0, sizeof( tdata ) );
	memset( (char *)&answer, 0, sizeof( answer ) );

	answer.address = 0X01;

	if( key == NULL )
	{
		answer.instruction = MF1_READ_UNKEY;
		answer.nbytes	   = 6 + 2;
	}else
	{
		answer.instruction = MF1_READ_KEY;
		answer.nbytes	   = 13 + 2;
	}
	answer.itemnum	   = 1;
	card_number_len	   = string2hex( card_number, (char *)&answer.user_data[0], strlen( card_number ) );

	answer.user_data[card_number_len]	   = sector;
	answer.user_data[card_number_len + 1]  = block;

	if( key != NULL )
	{
		answer.user_data[card_number_len + 2] = mode;
	}

	if( key != NULL )
	{
		key_len = string2hex( key, (char *)&answer.user_data[card_number_len + 3], strlen( key ) );
	}

	if( serial_package_processor( port, &answer, SEND_PACKAGE ) < 0 )
	{
		return ERROR;
	}

	gettimeofday( &oldtimer, NULL );
	oldsec = ( oldtimer.tv_sec % 10000 ) * 1000 + oldtimer.tv_usec / 1000;

	while( 1 )
	{
		gettimeofday( &newtimer, NULL );
		if( abs( ( ( newtimer.tv_sec % 10000 ) * 1000 + newtimer.tv_usec / 1000 ) - oldsec ) > outtime * 1000 ) //根据扇区内容多少来确定时间，读卡指纹未3000
		{
			return ERROR;
		}

		memset( &tdata, 0, sizeof( tdata ) );
		if( serial_package_processor( port, &tdata, RECV_PACKAGE ) == SUCCESS )
		{
			if( tdata.instruction == MF1_READ_KEY )
			{
				memcpy( data, tdata.user_data, 16 );
				return SUCCESS;
			}
		}
	}
	return ERROR;
}

/**
 *
 * @chinese
 * 往ic卡扇区里写数据
 *
 * @param port 串口号
 * @param sector 扇区号
 * @param block 块号
 * @param data 返回读取到的扇区内容
 * @param mode 0X01:A-key,0X02:B-key
 * @param key 密钥值
 *
 * @return 成功-SUCCESS,失败-ERROR
 * @endchinese
 *
 * @english
 * write MF1 data
 *
 * @param port serial port
 * @param sector sector
 * @param block block
 * @param data data
 * @param mode 0X01:A-key,0X02:B-key
 * @param key key
 *
 * @return success-SUCCESS,fail-ERROR
 * @endenglish
 *
 */
int mf1_send_data( int port, char sector, char block, char *data, char mode, char *key )
{
	TDATA			tdata;
	TDATA			answer;
	int				card_number_len	   = 0;
	int				key_len			   = 0;
	struct timeval	oldtimer, newtimer;
	int				oldsec;

	memset( &answer, 0, sizeof( answer ) );

	answer.address = 0X01;

	if( key == NULL )
	{
		answer.instruction = MF1_WERITE_UNKEY;
		answer.nbytes	   = 24;
	}else
	{
		answer.instruction = MF1_WERITE_KEY;
		answer.nbytes	   = 31;
	}
	answer.itemnum = 1;

	card_number_len = string2hex( card_number, (char *)&answer.user_data, strlen( card_number ) );

	answer.user_data[card_number_len]	   = sector;
	answer.user_data[card_number_len + 1]  = block;

	if( key != NULL )
	{
		answer.user_data[card_number_len + 2] = mode;
	}

	if( key != NULL )
	{
		key_len = string2hex( key, (char *)&answer.user_data[card_number_len + 3], strlen( key ) );
	}

	memcpy( &answer.user_data[key_len + card_number_len + 3], data, 16 );

	if( serial_package_processor( port, &answer, SEND_PACKAGE ) < 0 )
	{
		return ERROR;
	}

	gettimeofday( &oldtimer, NULL );
	oldsec = ( oldtimer.tv_sec % 10000 ) * 1000 + oldtimer.tv_usec / 1000;

	while( 1 )
	{
		gettimeofday( &newtimer, NULL );
		if( abs( ( ( newtimer.tv_sec % 10000 ) * 1000 + newtimer.tv_usec / 1000 ) - oldsec ) > 300 )
		{
			return ERROR;
		}
		memset( &tdata, 0, sizeof( tdata ) );
		if( serial_package_processor( port, &tdata, RECV_PACKAGE ) == SUCCESS )
		{
			if( tdata.instruction == FEEDBACK && tdata.user_data[0] == FEEDBACK_SUCCESS )
			{
				return SUCCESS;
			}
		}
	}

	return ERROR;
}

/**
 *
 * @chinese
 * 重启
 *
 * @param port 串口号
 *
 * @return 成功-SUCCESS,失败-ERROR
 * @endchinese
 *
 * @english
 * reboot
 *
 * @param port serial port
 *
 * @return success-SUCCESS,fail-ERROR
 * @endenglish
 *
 */
int machine_reboot( int port )
{
	int		retval;
	char	data[1];
	char	value[1];
	int		loopnum = 3, i = 0, j = 0;

	while( i < loopnum )
	{
		i++;

		data[0] = REBOOT_P;
		memset( value, 0, sizeof( value ) );
		j = 0;

		retval = device_send_data( port, 0X01, CONTROL_P, 0X01, data, sizeof( data ) );
		if( retval < 0 )
		{
			continue;
		}

		usleep( 200000 );

		while( 1 )
		{
			retval = device_recv_data( port, value );
			if( retval == BEGINERROR )
			{
				usleep( 20000 );
				continue;
			}
			j++;
			if( j >= loopnum )
			{
				return ERROR;
			}
			break;
		}

		if( retval == ENDERROR || retval == ADDRERROR || retval == BCCERROR || retval == SUMERROR )
		{
			continue;
		}

		if( value[0] == FEEDBACK_SUCCESS )
		{
			return SUCCESS;
		}else if( value[0] == FEEDBACK_ERROR )
		{
			continue;
		}
	}
	return ERROR;
}

// psam卡透传指令: send and recv
int rk3288_psam_apdu_data( unsigned char* data, int bytes, unsigned char *obuf )
{
	int						ret = 0, flag = 0;
	TDATA					tdata;
	_RK3288_DATABUF			*RK3288_TMP = NULL, *RK3288_CUR = NULL;
	struct timeval			oldtimer, newtimer;
	struct serial_devices	*p1 = NULL;

	if( RK3288_51port < 0 )
	{
		return -1;
	}

	ret = device_send_data( RK3288_51port, 0X01, PSAMCMD, 0X01, data, bytes );
	gettimeofday( &oldtimer, NULL );
	gettimeofday( &newtimer, NULL );
	while( 1 )
	{
		memset( data, 0, sizeof( data ) );
		ret = rk3288_device_recv_data( RK3288_51port, &tdata );
		if( ret > 0 )
		{//printf("psam ret = %02x\n", ret);		
			if( ret == PSAMCMD )
			{
OK:
				obuf[0] = tdata.nbytes; // 首字节要赋值为长度
				memcpy( obuf + 1, tdata.user_data, tdata.nbytes );
				return tdata.nbytes;
			}else
			{
				if( RK3288cnt >= RK3288_MAXCNT )
				{
					freeRK3288Buf( );
				}

				if( RK3288cnt < RK3288_MAXCNT )
				{
					pthread_mutex_lock( &gpio_mutex );
					RK3288_TMP = (_RK3288_DATABUF *)malloc( sizeof( _RK3288_DATABUF ) );
					if( RK3288_TMP != NULL )
					{
						memset( RK3288_TMP, 0, sizeof( _RK3288_DATABUF ) );
						memcpy( &RK3288_TMP->tdata, &tdata, sizeof( tdata ) );
						if( !RK3288_HEAD )
						{
							RK3288_HEAD		   = RK3288_TMP;
							RK3288_TMP->next   = NULL;
						}else
						{
							RK3288_CUR = RK3288_HEAD;
							while( RK3288_CUR )
							{
								if( RK3288_CUR->next == NULL )
								{
									RK3288_CUR->next = RK3288_TMP;
									break;
								}
								RK3288_CUR = RK3288_CUR->next;
							}
						}
						RK3288cnt++;
//						printf("rk3288_psam_apdu_data: put in RK3288buf, now cnt = %d\n", RK3288cnt);
					}
					pthread_mutex_unlock( &gpio_mutex );
				}
			}
		}
		else // 尝试从缓存中读取结果
		{
//		printf("now rk3288_psam_apdu_data read from buf^^^^^^^\n");
			pthread_mutex_lock( &gpio_mutex );
			RK3288_TMP = RK3288_HEAD;
			RK3288_CUR = RK3288_TMP;
			while( RK3288_TMP )
			{
				if( RK3288_TMP->tdata.instruction == PSAMCMD )
				{
					if( RK3288_TMP == RK3288_HEAD ) // 头单独处理
					{
						RK3288_HEAD = RK3288_TMP->next;
					}else
					{
						RK3288_CUR->next = RK3288_TMP->next;
					}
					free( RK3288_TMP );
					RK3288cnt--;
//					printf("rk3288_psam_apdu_data : get respose for RK3288 buf\n");
					flag = 1;
					break;
				}
				RK3288_CUR = RK3288_TMP;
				RK3288_TMP = RK3288_TMP->next;
			}
			pthread_mutex_unlock( &gpio_mutex );
			if( flag == 1 )
			{
				goto OK;
			}
		}

		gettimeofday( &newtimer, NULL );
		if( ( newtimer.tv_sec - oldtimer.tv_sec ) * 1000
		    + ( newtimer.tv_usec - oldtimer.tv_usec ) / 1000 > 800 ) // 800 ms, 部分运算耗时较长
		{
//			printf("now rk3288_psam_apdu_data time out ~~~~~~~~~~~~\n");
			return -1;
		}
	}

	return -1;
}

void freeRK3288Buf( )
{
	_RK3288_DATABUF *RK3288_CUR = NULL;
//printf("now free buf\n");
	pthread_mutex_lock( &gpio_mutex );

	RK3288_CUR = RK3288_HEAD;
	while( RK3288_CUR )
	{
		RK3288_CUR = RK3288_CUR->next;
		free( RK3288_HEAD );
		RK3288_HEAD = RK3288_CUR;
	}

	RK3288cnt = 0;

	pthread_mutex_unlock( &gpio_mutex );
}

