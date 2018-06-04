/**
 * @chinese
 * @file   serial_devices.c
 * @author 刘训
 * @date   Thu Oct 13 10:00:00 2011
 *
 * @brief  串行设备接口
 * @endchinese
 *
 * @english
 * @file   serial_devices.c
 * @author Liu Xun
 * @date   Thu Oct 13 10:00:00 2011
 *
 * @brief Serial devices interface
 * @endenglish
 */

#include <pthread.h>

#include <stdio.h>
//#include <execinfo.h>
#include "printer_devices.h"
#include "device_protocol.h"
#include "serial_devices.h"
#include "uartlib.h"
#include "spilib.h"
#include "printer_interface.h"
#include "public.h"
#include "file.h"
#include "serial.h"
#include "config.h"
#include "debug.h"
#include "wiegand/wiegand_input.h"
#include "finger/arm9_finger.h"
#include "card/dx-RF-SIM.h"
#include "card/LV1000.h"
#include "card/LV3000.h"
#include "card/LV3095.h"
#include "vena/vena_devices.h"
#include "sound/sound.h"
#include "gps/gps.h"
#include "card/readcard.h"
#include "card/card_sectors.h"
#include "485_finger.h"
#include "card/uhf.h"
#include "psam/nari_psamcpu.h"
#include "../test/test_devices.h"
#include "cardrule.h"
#include "test/test_devices.h"
#include "scanner/BarCodeScanner.h"
#include "serial_async.h"
#ifdef _ARM_A23
#include "../android/wedsa23.h"
#endif
int						terminal_type;
int enable_input_source=0;
struct serial_devices	*head_serial_devices;
TDATA					synel_tdata;

pthread_mutex_t			serial_mutex = PTHREAD_MUTEX_INITIALIZER;

//追加输入源
int add_devices_stack( char *devices_type, int port )
{
	struct serial_devices *p1, *p2;

	//分配结构体空间
	p1 = (struct serial_devices *)malloc( sizeof( struct serial_devices ) );
	if( p1 == NULL )
	{
		return ERROR;
	}
	memset( p1, 0, sizeof( struct serial_devices ) );
	//加进链表内
	if( head_serial_devices == NULL )
	{
		head_serial_devices = p1;
	}else
	{
		p2 = head_serial_devices;
		while( p2->next != NULL )
		{
			p2 = p2->next;
		}
		p2->next = p1;
	}
	p1->next = NULL;
	memset( p1->devices_type, 0, sizeof( p1->devices_type ) );
	strcpy( p1->devices_type, devices_type );
	p1->serial_port = port;
	return TRUE;
}

/**
 * @chinese
 * 初始化串口,添加weds协议
 *
 * @param uart_port
 * @param baudrate 串口波特率，取值范围[2400,4800,9600,19200,38400,57600,115200]
 * @param workmode 工作方式(0-客户端,1-服务器)
 * @param address 服务器地址,取值范围[0-255]
 *
 * @return Success 文件描述符.fail:FALSE
 * @endchinese
 *
 * @english
 * initialize serial port,add weds protocol
 *
 * @param uart_port
 * @param baudrate serial port baudrate，value range [2400,4800,9600,19200,38400,57600,115200]
 * @param workmode workmod(0-client,1-server)
 * @param address server ip address,value range [0-255]
 *
 * @return Success:fd.fail:FALSE
 * @endenglish
 *
 */
int init_uart_devices( int uart_port, int baudrate, char *devices_type )
{
	char	buf[128];
	int		address = 0X01, retval = ERROR;

	//自动化测试时，异常断电后，测试临时数据会遗留到设备内，造成数据冗余，为简化上层应用处理逻辑，在此删从掉临时文件。

	clear_test_temp_data( );
        printf("%s,%d,%d\n",__func__,uart_port,baudrate);
//	printf( "init uart:%d,%s\n", uart_port, devices_type );
	//加载卡号转换规则文件
	openCardRuleFile( cardRuleFileName );
	if( strcmp( devices_type, "WEDS_DEVICES" ) == 0 )
	{
            if( PLATFORM_TYPE == 1 || PLATFORM_TYPE == 2  || PLATFORM_TYPE == 3 || PLATFORM_TYPE == 4)	// lfg 20180224 add 4 for 335 psam
                retval = serial_init_async( uart_port, baudrate, SERVER, address );
            else
                retval = serial_init( uart_port, baudrate, SERVER, address );
		if( retval != ERROR )
		{
			return add_devices_stack( devices_type, uart_port );
		}
	}else if( strcmp( devices_type, "RS485" ) == 0 )
	{
		memset( buf, 0, sizeof( buf ) );
		read_terminal_number( buf );
		address = atoi( buf ) % 1000;
	}else if( strcmp( devices_type, "M300" ) == 0 )
	{
		return add_devices_stack( "M300", uart_port );
	}else if( strcmp( devices_type, "printer_devices" ) == 0 )
	{
		retval = init_printer_uart( uart_port, baudrate );
	}else if( strcmp( devices_type, "GPS" ) == 0 )
	{
		retval = init_gps( uart_port, baudrate );
	}else if( strcmp( devices_type, "Com_FP" ) == 0 )
	{
		retval = init_ComFingerPrinter( uart_port );
	}else if( strcmp( devices_type, "TgHfCard" ) == 0 )
	{
#ifndef _ARM_A23
		retval = hidCardDeviceInit( );
		if( retval > 0 )
		{
			return add_devices_stack( devices_type, uart_port );
		}
#else
		return -1;
#endif
	}
	else if( strcmp( devices_type, "QRCODE_E20" ) == 0 )
	{
            retval = serial_init( uart_port, 9600, SERVER, address );
            if( retval != ERROR )
            {
                return add_devices_stack( devices_type, uart_port );
            }
    }else if( strcmp( devices_type, "QRCODE_H2D" ) == 0 )
    {
        return add_devices_stack( devices_type, uart_port );
    }else if( strcmp( devices_type, "WEDS_MCU" ) == 0 )
    {
        if( PLATFORM_TYPE == 1 || PLATFORM_TYPE == 2  || PLATFORM_TYPE == 3)
            retval = serial_init_async( uart_port, baudrate, SERVER, address );
        else
            retval = serial_init( uart_port, baudrate, SERVER, address );
        if( retval != ERROR )
        {
            //添加初始化gpio
            init_mcu_port( uart_port );
            return add_devices_stack( "WEDS_DEVICES", uart_port );	// must use WEDS_DEVICES,do not use WEDS_MCU
        }
    }else
    {
        retval = serial_init( uart_port, baudrate, SERVER, address );
        if( retval != ERROR )
        {
            return add_devices_stack( devices_type, uart_port );
        }
    }
	//start_test();
	return retval;
}

/**
 * @chinese
 * 初始化串口,添加weds协议
 *
 * @param uart_port
 * @param baudrate 串口波特率，取值范围[2400,4800,9600,19200,38400,57600,115200]
 * @param workmode 工作方式(0-客户端,1-服务器)
 * @param address 服务器地址,取值范围[0-255]
 *
 * @return Success 文件描述符.fail:FALSE
 * @endchinese
 *
 * @english
 * initialize serial port,add weds protocol
 *
 * @param uart_port
 * @param baudrate serial port baudrate，value range [2400,4800,9600,19200,38400,57600,115200]
 * @param workmode workmod(0-client,1-server)
 * @param address server ip address,value range [0-255]
 *
 * @return Success:fd.fail:FALSE
 * @endenglish
 *
 */

//void dump_log(int signo)
//{
//    int j, nptrs;
// #define SIZE 100
//    void *buffer[100];
//    char **strings;

//    nptrs = backtrace(buffer, SIZE);
//    printf("backtrace() returned %d addresses\n", nptrs);

//    /* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
//       would produce similar output to the following: */

//    strings = backtrace_symbols(buffer, nptrs);
//    if (strings == NULL) {
//        perror("backtrace_symbols");
//       exit(0) ;
//    }

//    for (j = 0; j < nptrs; j++)
//        printf("%s\n", strings[j]);

//    free(strings);
//    exit(0);
//}

int init_spi_devices( int spi_port, char *devices_type )
{
	char	buf[128];
	int		address	   = 0X01, retval = ERROR;
	int		port	   = 10 + spi_port, rate = 115200;

	if( devices_type == NULL || strlen( devices_type ) == 0 )
	{
		return FALSE;
	}
        printf( "init spi:%d,%s\n", spi_port, devices_type );
	memset( buf, 0, sizeof( buf ) );
	//    read_terminal_number(buf);
	//    address = atoi(buf)%1000;

	if( strcmp( devices_type, "WEDS_DEVICES" ) == 0 )
	{
            if( PLATFORM_TYPE == 1 || PLATFORM_TYPE == 2  || PLATFORM_TYPE == 3)
                retval = serial_init_async( port, rate, SERVER, address );
            else
		retval = spi_open( port, rate, SERVER, address ); //serial_init(port,baudrate,SERVER,address);
		if( retval != ERROR )
		{
			return add_devices_stack( devices_type, port );
		}
	}
	serial_send_onebyte( 10, 0x00 );
	serial_send_onebyte( 10, 0x00 );
	return TRUE;
}

//读取串行设备数据
int read_devices_data( char *input_source, char *value, char* value_type )
{
	char					outCardNumber[1024];
	char					*spca = ",";
	char					input_type[24][32], *ptr = NULL;
	int						weds_devices   = -1, i = 0, j = 0, retval = 0, n = 0;
	int						len			   = 0;
	struct serial_devices	*p1;
	static int				finger_time		   = 0;
	static int				com_finger_time	   = 0;
	unsigned char ch = 0;
#ifdef _ARM_A23
        if(enable_input_source)
            return -1;
#endif
        //LOGI( "begin read_devices_data\n" );
	len = strlen( input_source );
	if( input_source == NULL || len == 0 )
	{
		return -1;
	}
	memset( outCardNumber, 0, sizeof( outCardNumber ) );
	memset( input_type, 0, sizeof( input_type ) );
	ptr = input_source;
	while( len >= 0 )
	{
		i = strcspn( ptr, spca );
		if( i >= 32 )
		{
			continue;
		}
		if( j >= 24 )
		{
			break;
		}
#ifndef _ARM_2410
		if( strncmp( ptr, "WG26", i ) == 0 )
		{
			if( weds_devices == -1 )
			{
				weds_devices = j;
				j++;
			}
			strcpy( input_type[weds_devices], "WEDS_DEVICES" );
		}
#endif
		else
		{
			strncpy( input_type[j], ptr, i );
			j++;
		}
		ptr	  += ( i + 1 );
		len	  -= ( i + 1 );                                 //剩余字符串长度。
	}
	for( i = 0; i < j; i++ )
	{
		if( strcmp( input_type[i], "WEDS_DEVICES" ) == 0 )  //读取weds协议下的所有设备
		{
			p1 = head_serial_devices;
			while( p1 )
			{
				if( strcmp( p1->devices_type, "WEDS_DEVICES" ) != 0 )
				{
					p1 = p1->next;
					continue;
				}
                                // LOGI("%s 1,%d\n",__func__,p1->serial_port);
#ifndef _ARM_A23
				pthread_mutex_lock( &serial_mutex );
                                if( PLATFORM_TYPE == 1 || PLATFORM_TYPE == 2  || PLATFORM_TYPE == 3 || PLATFORM_TYPE == 4)
                                retval = device_recv_data_async( p1->serial_port, value );
                                else
				retval = device_recv_data( p1->serial_port, value );
				pthread_mutex_unlock( &serial_mutex );
#else
                                if( PLATFORM_TYPE == 1 || PLATFORM_TYPE == 2  || PLATFORM_TYPE == 3 || PLATFORM_TYPE == 4)
                                retval = device_recv_data_async( p1->serial_port, value );
                                else
				retval = device_recv_data( p1->serial_port, value );
#endif
                               // LOGI("%s 2 %d\n",__func__,retval);

				if( retval > 0 )
				{
					//  printftime("");
					//printf("%s,%02X,%s\n",__func__,retval,value);
				}
				switch( retval )
				{
					case KEYBOARD:
						strcpy( value_type, "KB" );
						return p1->serial_port;
					case READMAGNETIC:
						strcpy( value_type, "MG" );
						//卡规则处理
						cardRuleAnalysis( value, outCardNumber );
						memset( value, 0, strlen( value ) );
						strcpy( value, outCardNumber );
						return p1->serial_port;
					case READBARCODE:
						strcpy( value_type, "BC" );
						return p1->serial_port;
					case MF1_S50_CARDNO:
					case MF1_S70_CARDNO:
						strcpy( value_type, "MF1" );
						//卡规则处理
						cardRuleAnalysis( value, outCardNumber );
						memset( value, 0, strlen( value ) );
						strcpy( value, outCardNumber );						
						
//					LeeLog(value, NULL, 0, 0);	
						if(strlen(value) != 8)		// 长度错误
						{
//							LeeLog("[this is invaild card: len err !=8]", NULL, 0, 0);	
							return -1;
						}
						for(n = 0; n < 8; n++)		// 非法字符
						{
							ch = *(value + n); 
							if(!((ch >= '0' && ch <= '9') || (ch >='A' && ch <= 'F')))
							{
//								LeeLog("[this is invaild card: err char]", NULL, 0, 0);	
								return -1;
							}
						} 

						return p1->serial_port;

					case READPF:
						strcpy( value_type, "ICLASS" );
						return p1->serial_port;
						break;
					case READHIDCARD:
						strcpy( value_type, "HID" );
						return p1->serial_port;
						break;

					case READEMCARD:
						strcpy( value_type, "PROX" );
						//卡规则处理
						cardRuleAnalysis( value, outCardNumber );
						memset( value, 0, strlen( value ) );
						strcpy( value, outCardNumber );
						return p1->serial_port;
					case ID_CARDNO:
						strcpy( value_type, "ID_CARD" );
						//卡规则处理
						cardRuleAnalysis( value, outCardNumber );
						memset( value, 0, strlen( value ) );
						strcpy( value, outCardNumber );
						return p1->serial_port;
					case BATTERY_INFO:
						strcpy( value_type, "BT" );
						return p1->serial_port;
						
					case LIGHT_SENSE:	// 20180115 光感
						strcpy( value_type, "LIGHT_SENSE" );
						return p1->serial_port;						
						
					case HUMAN_SENSE:	// 20180115 人体感应
						strcpy( value_type, "HUMAN_SENSE" );
						return p1->serial_port;
						
					case WIEGAND_CARD:
						strcpy( value_type, "WG26" );
						                                //卡规则处理
						cardRuleAnalysis( value, outCardNumber );
						memset( value, 0, strlen( value ) );
						strcpy( value, outCardNumber );
						return p1->serial_port;
					case WIEGANDINPUT:
						strcpy( value_type, "WEDS_WG" );
						                                //卡规则处理
						cardRuleAnalysis( value, outCardNumber );
						memset( value, 0, strlen( value ) );
						strcpy( value, outCardNumber );
						return p1->serial_port;
//				case CONTROL_PL:	 // 低电压关机
					case CONTROL_P:                     // 0F, 1F要区分
						if( value[0] == 0x0F )          // 关机
						{
							strcpy( value_type, "CLOSE_P" );
						}else if( value[0] == 0x1F )    // 低电压关机
						{
							strcpy( value_type, "CONTROL_PL" );
						}
						return p1->serial_port;
					case LIGHT_CMD:                     // 背光开启关闭
						strcpy( value_type, "LIGHT_CMD" );
						return p1->serial_port;
					case REBOOT_CMD:                    // 20170620
						strcpy( value_type, "REBOOT_CMD" );
						return p1->serial_port;
					case SPI_TEST:
						strcpy( value_type, "SPI_TEST" );
						return p1->serial_port;
					case CPU_CARDNO:
						strcpy( value_type, "CPU_CARD" );
						//卡规则处理
						cardRuleAnalysis( value, outCardNumber );
						memset( value, 0, strlen( value ) );
						strcpy( value, outCardNumber );
						return p1->serial_port;
				}

				p1 = p1->next;
			}
		}else if( strcmp( input_type[i], "FI" ) == 0 )
		{
#ifndef _ARM_A23
			finger_time += 1;
			if( get_sound_background( ) == 1 )
			{
				continue;
			}
			if( finger_time >= 3 ) //add by lxy 2013-5-28 三个周期检测一次指纹
#endif
			{
				finger_time = 0;
#ifndef _ARM_A23
				retval = OneToNMatch( "/tmp/zw.bmp" );
#else
				retval = OneToNMatch( "/mnt/obb/tmp/zw.bmp" );
#endif
				if( retval >= 0 )
				{
					sprintf( value, "%d", retval );
				} else if( retval == -5 )
				{
					strcpy( value, "-5" );
				} else
				{
					continue;
				}
				strcpy( value_type, "FI" );
				return 0;
			}
		}else if( strcmp( input_type[i], "Com_FP" ) == 0 ) // 串口指纹仪
		{
			if( get_sound_background( ) == 1 )
			{
				continue;
			}
			com_finger_time	   = 0;
			retval			   = Com_FP_One2NMatch( );
			if( retval >= 0 )
			{
				sprintf( value, "%d", retval );
			} else if( retval == -5 )
			{
				strcpy( value, "-5" );
			} else
			{
				continue;
			}
			strcpy( value_type, "Com_FP" );
			return 10;
		}else if( strcmp( input_type[i], "FV" ) == 0 )
		{
			if( get_sound_background( ) == 1 )
			{
				continue;
			}
			retval = OneToOneMatch( "", 0, "tmp/zw.jpg" );
			if( retval >= 0 )
			{
				strcpy( value, "T" );
			} else if( retval == -5 )
			{
				strcpy( value, "F" );
			} else
			{
				continue;
			}
			strcpy( value_type, "FV" );
			return 0;
		}else if( strcmp( input_type[i], "WG26" ) == 0 )
		{
			;
		}else
		{
			p1 = head_serial_devices;
			while( p1 )
                        {
				if( strcmp( p1->devices_type, input_type[i] ) == 0 )
				{
					if( strcmp( p1->devices_type, "M300" ) == 0 )
					{
						retval = read_M300_card( p1->serial_port, value );
					}else if( strcmp( p1->devices_type, "LV_1000" ) == 0 )
					{
						retval = read_LV1000_card( p1->serial_port, value );
					}else if( strcmp( p1->devices_type, "LV_3000" ) == 0 )
					{
						retval = read_LV3000_card( p1->serial_port, value );
					}else if( strcmp( p1->devices_type, "LV_3095" ) == 0 )
					{
						retval = read_LV3095_card( p1->serial_port, value );
					}else if( strcmp( p1->devices_type, "RFID_YDGM" ) == 0 )
					{
						retval = read_24G_card( p1->serial_port, value );
						//                        if (retval == 1)
						//                        {
						//                          printf ("Read:  %d\n",gmjs_24G_read(p1->serial_port));
						//                          printf ("Write: %d\n",gmjs_24G_write(p1->serial_port, 0x09));
						//                        }
					}else if( strcmp( p1->devices_type, "RFID_0201A" ) == 0 )
					{
						retval = read_RFID_SIM_0201A( p1->serial_port, value );
					}else if( strcmp( p1->devices_type, "TgHfCard" ) == 0 )
					{
						retval = readHighFrequency( value );
						if( retval == TRUE )
						{
							strcpy( value_type, p1->devices_type );
						}
					}else if( strcmp( p1->devices_type, "ZTXHfCard" ) == 0 )    // 至天行 20161125
					{
						retval = readHighFrequency_szztx( p1->serial_port, value );
					}else if( strcmp( p1->devices_type, "RFID_0100D" ) == 0 )
					{
						serial_clear( p1->serial_port );                        //清空串口缓存
						retval = read_RFID_SIM_0100D( p1->serial_port, value );
					}else if( strcmp( p1->devices_type, "XDF_CARD" ) == 0 )
					{
						retval = read_xdf_card( p1->serial_port, value );
					}else if( strcmp( p1->devices_type, "TEXT_CARD" ) == 0 )
					{
						retval = read_text_card( p1->serial_port, value );
					}else if( strcmp( p1->devices_type, "VENA_DEVICE" ) == 0 )
					{
						retval = read_vena_device( p1->serial_port, value );
					}else if( strcmp( p1->devices_type, "RFID_ZKXL" ) == 0 )
					{
						retval = read_zkxl_card( p1->serial_port, value );
					}else if( strcmp( p1->devices_type, "RFID_24G" ) == 0 )
					{
						retval = read_24G_card_app( p1->serial_port, value );
					}else if( strcmp( p1->devices_type, "XZX_RFID_24G" ) == 0 )  // 20160331新中新
					{
						retval = read_24G_card_xzx( p1->serial_port, value );
					}else if( strcmp( p1->devices_type, "ZY_RFID_24G" ) == 0 )
					{
						retval = zy_24G_card_app( p1->serial_port, value );
					}else if( strcmp( p1->devices_type, "DX_CPU" ) == 0 )
					{
						retval = device_recv_data( p1->serial_port, value );
						if( retval == CPU_CARDNO )
						{
							retval = read_dx_cpu_card_app( p1->serial_port, value );
							serial_clear( p1->serial_port );
						}
					}else if( strcmp( p1->devices_type, "NR_CPU" ) == 0 )
					{
						char cpu_card[16];
						retval = device_recv_data( p1->serial_port, value );
						if( retval == CPU_CARDNO )
						{
							memset( cpu_card, 0, sizeof( cpu_card ) );
							memcpy( cpu_card, value, strlen( value ) );
							memset( value, 0, sizeof( value ) );
							retval = nanrui_psam_verification( p1->serial_port, value );
							//retval= nanrui_verification(p1->serial_port);
							if( retval == TRUE )
							{
								memset( value, 0, strlen( value ) );
								memcpy( value, cpu_card, strlen( cpu_card ) );
							}
							serial_clear( p1->serial_port );
						}
					}else if( strcmp( p1->devices_type, "NR_CPU2" ) == 0 )		// 20180227 lfg 补充，原定制在2016.1.23，由于版本未同步而丢失
					{
						char cpu_card[16];
						retval = device_recv_data( p1->serial_port, value );
						if( retval == CPU_CARDNO )
						{
							memset( cpu_card, 0, sizeof( cpu_card ) );
							memcpy( cpu_card, value, strlen( value ) );
							memset( value, 0, sizeof( value ) );
							retval = nanrui_psam_verification2( p1->serial_port, value );
							//retval= nanrui_verification(p1->serial_port);
							if( retval == TRUE )
							{
								memset( value, 0, strlen( value ) );
								memcpy( value, cpu_card, strlen( cpu_card ) );
							}
							serial_clear( p1->serial_port );
						}
					}else if( strcmp( p1->devices_type, "RFID_UHF" ) == 0 )     //add by aduo 2014.5.6
					{
						retval = read_uhf_card( p1->serial_port, value );
					}else if( strcmp( p1->devices_type, "RFID_HR915M" ) == 0 )  //add by lfg 2017.4.20
					{ //                        retval = read_uhf_card(p1->serial_port,value);
						retval = FALSE;
						if( HR915MGetCard_TimeMode( value ) > 0 )
						{
							retval = TRUE;
						}
					}else if( strcmp( p1->devices_type, "Mj_Ext" ) == 0 )	// 20180307 广西大学 消防联动 门禁扩展支持
					{
						retval = mj_ext_event( p1->serial_port, value);
					}else if( strcmp( p1->devices_type, "KIMMA" ) == 0 )        //add by lfg 2017.6.22
					{
						retval = JmListenEvent( p1->serial_port, value );
//						printf("vvvvvv = %d, str = %s\n", retval, value);
						if( retval == 0 )
						{
							strcpy( value_type, p1->devices_type );
							return p1->serial_port;
						}
					}else if( strcmp( p1->devices_type, "ISO_15693" ) == 0 )        //add by lfg 2017.8.28
					{
						retval = read_iso_15693card( p1->serial_port, value );
//						printf("vvvvvv = %d, str = %s\n", retval, value);
						if( retval == 0 )
						{
							strcpy( value_type, p1->devices_type );
							return p1->serial_port;
						}
					}
#ifndef _ARM_A23
					else if( strcmp( p1->devices_type, "ID2_INFO" ) == 0 )
					{
						retval = read_ID2_card( p1->serial_port, value );
					}
#endif
					else if( strcmp( p1->devices_type, "NJ_CITIZEN" ) == 0 )
					{
						retval = read_nj_citizen_card( p1->serial_port, value );
						//serial_clear(p1->serial_port);
					}else if( strcmp( p1->devices_type, "HZ_CITIZEN" ) == 0 )
					{
						serial_clear( p1->serial_port );
						retval = read_hz_citizen_card( p1->serial_port, value );
						serial_clear( p1->serial_port );
					}else if( strcmp( p1->devices_type, "HIRF_CARD" ) == 0 )
					{
						retval = read_mj_card( p1->serial_port, value );
					}else if( strcmp( p1->devices_type, "HIRF_24G_CARD" ) == 0 )
					{
						retval = read_hirf_24g_card( p1->serial_port, value );
					}else if( strcmp( p1->devices_type, "XZX_CPU_XTDX" ) == 0 )
					{
						retval = read_cpu_data_xiangtan( p1->serial_port, value );
					}else if( strcmp( p1->devices_type, "ZGRB_CPU" ) == 0 )
					{
						retval = read_cpu_data_zhongguangruibo( p1->serial_port, value );
                    }else if( strcmp( p1->devices_type, "LeiS_CPU" ) == 0 )
					{
						retval = read_cpu_data_beijingleisen_appleapy( p1->serial_port, value );
                    }else if( strcmp( p1->devices_type, "WEDS_CPU" ) == 0 )
					{
						retval = read_cpu_data_weds( p1->serial_port, value );
                    }else if( strcmp( p1->devices_type, "QRCODE_E20" ) == 0 )
                    {
						retval = read_qrcode_E20( p1->serial_port, value, 1024 );
		            }else if( strcmp( p1->devices_type, "QRCODE_H2D" ) == 0 )
		            {
		                retval = readEventKey(value );
		            }else if(strcmp(p1->devices_type,"JXBS3001")==0)
		            {
		                retval= read_JXBS3001(p1->serial_port,value);
		            }
					if( retval == TRUE )
					{
						strcpy( value_type, p1->devices_type );
						//卡规则处理
						cardRuleAnalysis( value, outCardNumber );
						memset( value, 0, strlen( value ) );
						strcpy( value, outCardNumber );
						return p1->serial_port;
					}
				}
				p1 = p1->next;
			}
		}
	} //end for
	return -1;
}

//设置键盘背光时间
int set_keypad_backlight_time( char *devices_type, int value )
{
	int						retval = 0;
	char					data[4];
	int						data1 = 0, data2 = 0;
	struct serial_devices	*p1;

	if( devices_type == NULL || value > 255 || value < 0 || strcmp( devices_type, "BACKLIGHT_T" ) != 0 )
	{
		return -1;
	}

	memset( data, 0, sizeof( data ) );
	p1 = head_serial_devices;
	while( p1 )
	{
		if( p1->serial_port < 10 || strcmp( p1->devices_type, "WEDS_DEVICES" ) != 0 )
		{
			p1 = p1->next;
			continue;
		}
		data1  = value % 16;
		data2  = value / 16;
		if( data1 > 9 )
		{
			data1 = 0X0A + ( data1 - 10 );
		}
		if( data2 > 9 )
		{
			data2 = 0X0A + ( data2 - 10 );
		}
		data[0]	   = ( data2 << 4 ) + data1;
		retval	   = device_send_data( p1->serial_port, 0X01, BACKLIGHT_T, 0X01, data, 1 );
		p1		   = p1->next;
	}

	return retval;
}

//设置logo背光时间


/*
    value:00 关闭背光
          0xFF 背光常亮
          1-0xFE 背光打开时间 单位：秒
 */
int set_logo_backlight_time( int value )
{
	int						retval = 0;
	char					data[4];
	struct serial_devices	*p1;
	if( value > 255 || value < 0 )
	{
		return -1;
	}
	memset( data, 0, sizeof( data ) );
	p1 = head_serial_devices;
	while( p1 )
	{
		if( strcmp( p1->devices_type, "WEDS_DEVICES" ) != 0 )
		{
			p1 = p1->next;
			continue;
		}
		data[0]	   = value % 256;
		retval	   = device_send_data( p1->serial_port, 0X01, LOGOBACKLIGHT, 0X01, data, 1 );
		p1		   = p1->next;
	}

	return retval;
}

// 20180115 竖屏版本
int set_logo_backlight_time_V( int value )
{
	int						retval = 0;
	char					data[4];
	struct serial_devices	*p1;
	if( value > 255 || value < 0 )
	{
		return -1;
	}
	memset( data, 0, sizeof( data ) );
	p1 = head_serial_devices;
	while( p1 )
	{
		if( strcmp( p1->devices_type, "WEDS_DEVICES" ) != 0 )
		{
			p1 = p1->next;
			continue;
		}
		data[0]	   = value % 256;
		retval	   = device_send_data( p1->serial_port, 0X01, LOGOBACKLIGHT_V, 0X01, data, 1 );
		p1		   = p1->next;
	}

	return retval;
}


//通过串行设备发送关机-0XFF/取消-0X00关机数据
int send_close_machine_info( char *devices_type, int value )
{
	int						retval = 0;
	char					data[12];
	struct serial_devices	*p1;

	if( devices_type == NULL || value > 1 || value < 0 || strcmp( devices_type, "CLOSE_P" ) != 0 )
	{
		return -1;
	}

	memset( data, 0, sizeof( data ) );
	p1 = head_serial_devices;
	while( p1 )
	{
		if( strcmp( p1->devices_type, "WEDS_DEVICES" ) != 0 )
		{
			p1 = p1->next;
			continue;
		}

		if( value == 0 )    //取消
		{
			data[0] = 0X00;
		} else
		{
			data[0] = 0XFF;
		}
		retval = device_send_data( p1->serial_port, 0X01, CONTROL_P, 0X01, data, 1 );
		p1	   = p1->next;
	}

	return retval;
}

//通过串行设备发送重启-0XFF/取消重启-0X00数据
int send_reboot_machine_info( char *devices_type, int value )
{
	int						retval = 0;
	char					data[12];
	struct serial_devices	*p1;

	if( devices_type == NULL || value > 1 || value < 0 || strcmp( devices_type, "REBOOT_CMD" ) != 0 )
	{
		return -1;
	}

	memset( data, 0, sizeof( data ) );
	p1 = head_serial_devices;
	while( p1 )
	{
		if( strcmp( p1->devices_type, "WEDS_DEVICES" ) != 0 )
		{
			p1 = p1->next;
			continue;
		}

		if( value == 0 )    //取消
		{
			data[0] = 0X00;
		} else
		{
			data[0] = 0XFF;
		}
		retval = device_send_data( p1->serial_port, 0X01, REBOOT_CMD, 0X01, data, 1 );
		p1	   = p1->next;
	}

	return retval;
}


//20180115 通过串行设备设置光感设备上送光感数据的频率:最小单位1S，默认设置时间为5S
int send_light_sense_frequency( int value )
{
	int						retval = 0;
	char					data[12];
	struct serial_devices	*p1;

	memset( data, 0, sizeof( data ) );
	p1 = head_serial_devices;
	while( p1 )
	{
		if( strcmp( p1->devices_type, "WEDS_DEVICES" ) != 0 )
		{
			p1 = p1->next;
			continue;
		}

		data[0] = value;

		retval = device_send_data( p1->serial_port, 0X01, LIGHT_SENSE, 0X01, data, 1 );
		p1	   = p1->next;
	}

	return retval;
}

//20180115 通过串行设备设置人体感应设备上送感应数据的频率:最小单位1S，默认设置时间为2S
int send_human_sense_frequency( int value )
{
	int						retval = 0;
	char					data[12];
	struct serial_devices	*p1;

	memset( data, 0, sizeof( data ) );
	p1 = head_serial_devices;
	while( p1 )
	{
		if( strcmp( p1->devices_type, "WEDS_DEVICES" ) != 0 )
		{
			p1 = p1->next;
			continue;
		}

		data[0] = value;

		retval = device_send_data( p1->serial_port, 0X01, HUMAN_SENSE, 0X01, data, 1 );
		p1	   = p1->next;
	}

	return retval;
}



//初始化库在那个型号机器上使用
int init_sys_model( int model )
{
	if( model < 0 || model > MAC_TERMINAL_TYPE )
	{
		return ERROR;
	}

	terminal_type = model;

	return SUCCESS;
}

//通过串行设备发送FF通知停止发送电池电量
int send_close_battery_info( char *devices_type )
{
	int						retval = 0;
	char					data[12];
	struct serial_devices	*p1;

	if( devices_type == NULL || strcmp( devices_type, "BATTERY_INFO" ) != 0 )
	{
		return -1;
	}

	memset( data, 0, sizeof( data ) );
	p1 = head_serial_devices;
	while( p1 )
	{
		if( p1->serial_port < 10 || strcmp( p1->devices_type, "WEDS_DEVICES" ) != 0 )
		{
			p1 = p1->next;
			continue;
		}
		data[0]	   = 0XFF;
		retval	   = device_send_data( p1->serial_port, 0X01, BATTERY_INFO, 0X01, data, 1 );
		p1		   = p1->next;
	}
	return retval;
}

//发送电池 应当。当收到电池电量信息以后发送应当信息
int ask_power_request( )
{
	int						retval = 0;
	char					data[12];
	struct serial_devices	*p1;

	memset( data, 0, sizeof( data ) );
	p1 = head_serial_devices;
	while( p1 )
	{
		if( p1->serial_port < 10 || strcmp( p1->devices_type, "WEDS_DEVICES" ) != 0 )
		{
			p1 = p1->next;
			continue;
		}
		data[0]	   = 0X01;
		retval	   = device_send_data( p1->serial_port, 0X01, BATTERY_INFO, 0X01, data, 1 );
		p1		   = p1->next;
	}
	return retval;
}

//获取weds串口设备版本号
int get_uart_devices_version( int uart_port, char *out_value )
{
	int						retval = 0, ret = 0;
	char					data[12], tmp[128];
	struct serial_devices	*p1;
	int						overtime = 0;
	struct timeval			tv1, tv2;

	if( out_value == NULL )
	{
		return ERROR;
	}

	memset( data, 0, sizeof( data ) );
	p1 = head_serial_devices;
	while( p1 )
	{
		if( p1->serial_port != uart_port || strcmp( p1->devices_type, "WEDS_DEVICES" ) != 0 )
		{
			p1 = p1->next;
			continue;
		}
		ret = 1;
		break;
	}
	if( ret == 0 )
	{
		return ERROR;
	}
	retval = device_send_data( p1->serial_port, 0X01, GET_VERSION, 0X01, data, 1 );
	if( retval < 0 )
	{
		return ERROR;
	}
	//读取返回值
	memset( tmp, 0, sizeof( tmp ) );
	gettimeofday( &tv1, NULL );
	gettimeofday( &tv2, NULL );
	overtime = tv1.tv_sec * 1000000 + tv1.tv_usec;
	while( 1 )
	{
		if( abs( ( tv2.tv_sec * 1000000 + tv2.tv_usec ) - overtime ) > 300000 )
		{
			return ERROR;
		}
		gettimeofday( &tv2, NULL );
		retval = device_recv_data( p1->serial_port, tmp );
		if( retval < 0 )   /* receive begin */
		{
			continue;
		}
		if( tmp[0] != GET_VERSION )
		{
			continue;
		}
		break;
	}
	memcpy( out_value, tmp + 1, sizeof( tmp ) );
	return retval;
}

//获取spi串口设备版本号
int get_spi_devices_version( int spi_port, char *out_value )
{
	int						retval = 0, port = 10 + spi_port;
	char					data[12], tmp[128];
	struct serial_devices	*p1;
	int						ret = 0, redoflag=0;

	int						overtime = 0;
	struct timeval			tv1, tv2;

	if( out_value == NULL )
	{
		return ERROR;
	}

	memset( data, 0, sizeof( data ) );
	p1 = head_serial_devices;
	while( p1 )
	{
		if( p1->serial_port != port || strcmp( p1->devices_type, "WEDS_DEVICES" ) != 0 )
		{
			p1 = p1->next;
			continue;
		}else
		{
			ret = 1;
			break;
		}
	}
	if( ret == 0 )
	{
		return ERROR;
	}
REDO:
	retval = device_send_data( p1->serial_port, 0X01, GET_VERSION, 0X01, data, 1 );
	if( retval < 0 )
	{
		return ERROR;
	}
	//读取返回值
	memset( tmp, 0, sizeof( tmp ) );
	gettimeofday( &tv1, NULL );
	gettimeofday( &tv2, NULL );
	overtime = tv1.tv_sec * 1000000 + tv1.tv_usec;
	while( 1 )
	{
		if( abs( ( tv2.tv_sec * 1000000 + tv2.tv_usec ) - overtime ) > 100000 )
		{
			printf( "out time spi\n" );
                        if(redoflag < 0)
			{
				redoflag++;
				printf("now redo agen=%d\n", redoflag);
				goto REDO;
			}
			return ERROR;
		}
		gettimeofday( &tv2, NULL );
		retval = device_recv_data( p1->serial_port, tmp );
		if( retval < 0 )   /* receive begin */
		{
			continue;
		}
		if( tmp[0] != GET_VERSION )
		{
			continue;
		}else
		{
			break;
		}
	}

	memcpy( out_value, tmp + 1, sizeof( tmp ) );
	return retval;
}

//获取spi串口设备版本号
int get_spi_battery( int spi_port, char *out_value )
{
	int						retval = 0, port = 10 + spi_port;
	char					data[12], tmp[128];
	struct serial_devices	*p1;
	int						ret = 0;

	int						overtime = 0;
	struct timeval			tv1, tv2;

	if( out_value == NULL )
	{
		return ERROR;
	}
	memset( data, 0, sizeof( data ) );
	p1 = head_serial_devices;
	while( p1 )
	{
		if( p1->serial_port != port || strcmp( p1->devices_type, "WEDS_DEVICES" ) != 0 )
		{
			p1 = p1->next;
			continue;
		}else
		{
			ret = 1;
			break;
		}
	}
	if( ret == 0 )
	{
		return ERROR;
	}
	//读取返回值
	memset( tmp, 0, sizeof( tmp ) );
	gettimeofday( &tv1, NULL );
	gettimeofday( &tv2, NULL );
	overtime = tv1.tv_sec * 1000000 + tv1.tv_usec;
	while( 1 )
	{
		if( abs( ( tv2.tv_sec * 1000000 + tv2.tv_usec ) - overtime ) > 30000000 )
		{
//			printf( "out time spi\n" );
			return ERROR;
		}
		gettimeofday( &tv2, NULL );
		retval = device_recv_data( p1->serial_port, tmp );
		if( retval < 0 )   /* receive begin */
		{
			continue;
		}
		if( retval != 0x8c )
		{
			continue;
		}else
		{
			break;
		}
	}

	//memcpy(out_value,tmp+1,sizeof(tmp));
	out_value[0] = tmp[0];
	return retval;
}

//获取WEDS——WG 卡号
//内部测试用
int get_uart_wg_data( int spi_port, char *out_value )
{
	int						retval = 0, port = spi_port;
	char					data[12], tmp[128];
	struct serial_devices	*p1;
	int						ret = 0;

	int						overtime = 0;
	struct timeval			tv1, tv2;
	if( out_value == NULL )
	{
		return ERROR;
	}
	memset( data, 0, sizeof( data ) );
	p1 = head_serial_devices;
	while( p1 )
	{
		if( p1->serial_port != port || strcmp( p1->devices_type, "WEDS_DEVICES" ) != 0 )
		{
			p1 = p1->next;
			continue;
		}else
		{
			ret = 1;
			break;
		}
	}
	if( ret == 0 )
	{
		return ERROR;
	}
	//读取返回值
	memset( tmp, 0, sizeof( tmp ) );
	gettimeofday( &tv1, NULL );
	gettimeofday( &tv2, NULL );
	overtime = tv1.tv_sec * 1000000 + tv1.tv_usec;
	while( 1 )
	{
		if( abs( ( tv2.tv_sec * 1000000 + tv2.tv_usec ) - overtime ) > 1000000 )
		{
			//printf("out time read -----------------------------------wg \n");
			return ERROR;
		}
		gettimeofday( &tv2, NULL );
		retval = device_recv_data( p1->serial_port, tmp );
		if( retval == WIEGANDINPUT )   /* receive begin */
		{
			break;
		}
	}

	memcpy( out_value, tmp, sizeof( tmp ) );
	return retval;
}

/*升级串口卡头*/
int printfUartData( char *data, int uSize )
{
	int i = 0;
	printf( "printfUartData:" );
	for( i = 0; i < uSize; i++ )
	{
		printf( "%02X ", data[i] );
	}
	printf( "\n" );
	return 0;
}

int updateUartRecv( int fd, char *pBuf, int pSize )
{
	int		i		   = 0;
	int		len_total  = 0, len_tmp = 0;
	time_t	optTime;

	time( &optTime );
	while( pSize > len_total )
	{
		if( abs( (int)difftime( time( NULL ), optTime ) ) > 1 )
		{
			return ERROR;
		}
		len_tmp = 0;

		len_tmp = uart_recv_data( fd, (char *)pBuf + len_total, pSize - len_total );
		if( len_tmp < 0 )
		{
			return ERROR;
		}else if( len_tmp == 0 )
		{
			continue;
		}
		len_total = len_total + len_tmp;
		time( &optTime );
	}

	return len_total;
	return 1;
}

int updateUartSend( int fd, char *pBuf, int pSize )
{
	int		len_total = 0, len_tmp = 0;

	time_t	optTime;

	time( &optTime );
	while( pSize > len_total )
	{
		if( abs( (int)difftime( time( NULL ), optTime ) ) > 5 )
		{
			return ERROR;
		}
		len_tmp	   = 0;
		len_tmp	   = uart_send_data( fd, pBuf + len_total, pSize - len_total );
		len_total  = len_total + len_tmp;
		time( &optTime );
	}

	return len_total;
}

int checkUpdateFile( char *src, char *qBuf )
{
	FILE		*fp = NULL;
	char		buf[12];
	int			fileSize = 0, i = 0;
	struct stat filestat;

	fp = fopen( src, "r" );
	if( fp == NULL )
	{
		return -1;
	}

	fstat( fileno( fp ), &filestat );            //获取文件状态信息
	fileSize = filestat.st_size;
	if( fileSize <= 7 )
	{
		fclose( fp );
		return -1;
	}
	fseek( fp, fileSize - 7, SEEK_SET );
	memset( buf, 0, sizeof( buf ) );
	while( !feof( fp ) && !ferror( fp ) )
	{
		memset( buf, 0, sizeof( buf ) );
		fread( buf, 1, 7, fp );
                printfUartData( buf, 7 );
		for( i = 0; i < 5; i++ )
		{
			if( buf[i] != 0XA5 )
			{printf("nnnnnnnnnnn a5\n");
				if( fp )
				{
					fclose( fp );
				}
				return -1;
			}
		}
		if( buf[i] == qBuf[0] && buf[i + 1] == qBuf[1] )
		{
			if( fp )
			{
				fclose( fp );
			}
			return 1;
		}
	}

	if( fp )
	{
		fclose( fp );
	}
	return -1;
}

/*
   强制手动升级
   uart_port:串口号
   src：升级文件
   返回结果：
   1:成功
   -1：未知错误
   -2：升级设备串口未打开或者初始化类型不对
   -3:升级文件检测错误
   -4：未检测到设备
   -5:设备未响应
   -6:设备未就绪
   -7:升级文件发送失败
 */
int updateUartCard( int uartPort, char *src )
{
	int						fd		   = 0;
	int						len		   = 0, retval = 0;
	int						i		   = 0, flag = 0;
	int						baudRate   = 1200;
	char					sData[128], rData[128];
	char					jtype[6];
	time_t					optTime;
	FILE					*fp = NULL;
	char					buf[64];

	struct serial_devices	*p1;
	int						ret = 0;

	p1 = head_serial_devices;
	while( p1 )
	{
		if( p1->serial_port != uartPort || strcmp( p1->devices_type, "WEDS_DEVICES" ) != 0 )
		{
			p1 = p1->next;
			continue;
		}
		ret = 1;
		break;
	}
	if( ret == 0 )
	{
		return -2;
	}
	fd = serial_fd[uartPort];


//	if (fd > 0)
//	{
//		close(fd);
//		fd = -1;
//	}

//	fd = init_uart_devices( uartPort, 1200, "WEDS_DEVICES" );
//	serial_fd[uartPort] = fd;

	//使用低波特率探测
	if( uart_set_baudrate( fd, baudRate ) < 0 )
	{printf("aa\n");
		return -1;
	}

//	printf("bb\n");

	/*
	   fd =uart_open_and_setattr(port, baudRate, 8, "1", 'N');
	   if(fd<0)
	   {
	    return ERROR;
	   }
	 */

#if 1	
	memset( sData, 0, sizeof( sData ) );
	sData[0]   = 0X80;
	sData[1]   = 0X80;
	len		   = 2;
	flag	   = 0;
	time( &optTime );
	while( abs( (int)difftime( time( NULL ), optTime ) ) < 180 ) //s
	{
		retval = uart_send_data( fd, sData, len );
		if( retval != len )
		{//printf("send le = %d\n", retval);
			continue;
		}
		memset( rData, 0, sizeof( rData ) );
		retval = uart_recv_data( fd, rData, sizeof( rData ) );
		if( retval <= 0 )
		{//printf("send rev = %d\n", retval);
			continue;
		}
                printfUartData( rData, retval );
		for( i = 0; i < retval; i++ )
		{
			if( rData[i] == 0x69 )
			{
				flag = 1;
				break;
			}
		}
		if( flag == 1 )
		{
			break;
		}
	}
	
	if( flag == 0 )
	{printf("33\n");
		return -5;
	}
#endif	
#if 0
	memset( sData, 0, sizeof( sData ) );
	sData[0]   = 0x80;
	sData[1]   = 0x80;
	sData[2]   = 0x5A;
	sData[3]   = 0x69;
	len		   = 4;
	flag	   = 0;
	uart_send_data( fd, sData, len );
	printf( "get devices id\n" );
	sleep( 1 );
	//"连接成功,验证设备型号..."
	//取回型号
	time( &optTime );
	while( abs( (int)difftime( time( NULL ), optTime ) ) < 30 ) //s
	{
		memset( rData, 0, sizeof( rData ) );
		retval = uart_recv_data( fd, rData, sizeof( rData ) );
		if( retval <= 0 )
		{
			continue;
		}
                printfUartData( rData, retval );
		for( i = 0; i < retval; i++ )
		{
			jtype[jtype[5]++] = rData[i];
			if( jtype[5] >= 5 )
			{
				flag = 1;
				break;
			}
		}
		if( flag == 1 )
		{
			break;
		}
	}
	if( flag == 0 )
	{
		return -5;
	}
	printf( "set 115200 baudrate\n" );
	//发送波特率协商
	char jbtlqh[8] = { 0x5A, 0x00, 0x81, 0x00, 0xFA, 0xFF, 0x00, 0x69 };
	uart_send_data( fd, jbtlqh, 8 );
	sleep( 1 );
	//切换波特率
	//    uart_close (fd);
	baudRate = 115200;
	//    fd =uart_open_and_setattr(port, baudRate, 8, "1", 'N');;
	if( uart_set_baudrate( fd, baudRate ) < 0 )
	{
		return -1;
	}
	//发送准备信号
	char jbtlks[8] = { 0x5A, 0x01, 0x35, 0x00, 0x00, 0x00, 0x00, 0x69 };
	uart_send_data( fd, jbtlks, 8 );

	//等待就绪信号
	flag = 0;
	time( &optTime );
	while( abs( (int)difftime( time( NULL ), optTime ) ) < 30 ) //s
	{
		memset( rData, 0, sizeof( rData ) );
		retval = uart_recv_data( fd, rData, sizeof( rData ) );
		if( retval <= 0 )
		{
			continue;
		}
                printfUartData( rData, retval );
		if( rData[0] == 0x00 )
		{
			flag = 1;
			break; //就绪
		}
	}
	if( flag == 0 )
	{
		return -6;
	}
#endif	
	printf( "send data:%s\n", src );
	//发送文件数据
	char jhead[8] = { 0x5A, 0x02, 0x00, 0x00, 0xD7, 0x00, 0x00, 0x69 };
	fp = fopen( src, "r" );
	if( fp == NULL )
	{
		return -1;
	}
	i	   = 0;
	len	   = 0;
	memset( buf, 0, sizeof( buf ) );
	while( !feof( fp ) && !ferror( fp ) )
	{
		memset( buf, 0, sizeof( buf ) );
		fread( buf, 1, sizeof( buf ), fp );

		jhead[2]   = len & 0XFF;
		jhead[3]   = ( len >> 8 ) & 0XFF;
		jhead[4]   = serial_cal_sum( buf, 64 );
		//发送头标记
		retval = updateUartSend( fd, jhead, 8 );

		usleep( 100 );
		//发送数据
		retval = updateUartSend( fd, buf, 64 );

		//等待确认
		memset( rData, 0, sizeof( rData ) );
		retval = updateUartRecv( fd, rData, 1 );
		if( rData[0] != 0x00 )
		{
			fclose( fp );
			return -7;
		}
		len += 64;
	}
	fclose( fp );
	//发送结束包
	jhead[0]   = 0xff;
	jhead[1]   = 0xff;
	updateUartSend( fd, jhead, 2 );
	//    uart_close (fd);
	printf( "updata ok\n" );
	return 1;
}

/*
   自动升级
   uart_port:串口号
   src：升级文件
   返回结果：
   1:成功
   -1：未知错误
   -2：升级设备串口未打开或者初始化类型不对
   -3:升级文件检测错误
   -4：未检测到设备型号
 */

#ifdef _ARM_2416
int autoUpdateUartCard( int uart_port, char *src )
{
	int						retval = 0, ret = 0;
	char					data[12], tmp[128];
	struct serial_devices	*p1;
	int						overtime = 0;
	struct timeval			tv1, tv2;
	char					fileName[256], cBuf[128];

	if( access( src, F_OK ) != 0 )   //if the directory is not exit
	{
		return -3;
	}
	memset( fileName, 0, sizeof( fileName ) );
	strcpy( fileName, "/tmp/cardUpDate.bin" );
	retval = fileEncryptionAndDecryption( src, fileName );
	if( retval == -1 )
	{
		return -3;
	}

	memset( data, 0, sizeof( data ) );
	p1 = head_serial_devices;
	while( p1 )
	{
		if( p1->serial_port != uart_port || strcmp( p1->devices_type, "WEDS_DEVICES" ) != 0 )
		{
			p1 = p1->next;
			continue;
		}
		ret = 1;
		break;
	}
	if( ret == 0 )
	{
		return -2;
	}
	//    AA 01 03 3A 01 11 28 50 A5
	data[0]	   = 0X11;
	retval	   = device_send_data( p1->serial_port, 0X01, UPDATEUARTCARD, 0X01, data, 1 );
	if( retval < 0 )
	{
		return -1;
	}
//	printf( "read\n" );
	//读取返回值 AB 01 04 3A 01 01 01 3E 42 A5
	gettimeofday( &tv1, NULL );
	gettimeofday( &tv2, NULL );
	overtime = tv1.tv_sec * 1000000 + tv1.tv_usec;
	while( 1 )
	{
		if( abs( ( tv2.tv_sec * 1000000 + tv2.tv_usec ) - overtime ) > 6000000 )    //6s
		{
			return -4;
		}
		gettimeofday( &tv2, NULL );
		memset( tmp, 0, sizeof( tmp ) );
		retval = device_recv_data( p1->serial_port, tmp );
		if( retval < 0 )                                                            /* receive begin */
		{
			continue;
		}
//		printf( "start:%02X,%02X,%02X\n", retval, tmp[0], tmp[1] );
		if( retval == UPDATEUARTCARD )
		{
			//            break;
			switch( tmp[0] )
			{
				case MA805:
					retval = checkUpdateFile( fileName, tmp );
					if( retval == -1 )
					{
						return -3;
					}
//					printf( "update \n" );
					retval = updateUartCard( uart_port, fileName );
					return retval;
				case STM32F030:
					gettimeofday( &tv1, NULL );
					overtime = tv1.tv_sec * 1000000 + tv1.tv_usec;
					memset( cBuf, 0, sizeof( cBuf ) );
					cBuf[0]	   = tmp[0];
					cBuf[1]	   = tmp[1];
					break;
				case UBOOTASK:
					retval = update_mcu( p1->serial_port, fileName, cBuf );
					return retval;
			}
		}
	}
//	printf( "error\n" );
	return -1;
}
#else
int autoUpdateUartCard( int uart_port, char *src )
{
	int						retval = 0, ret = 0;
	char					data[12], tmp[128];
	struct serial_devices	*p1;
	int						overtime = 0;
	struct timeval			tv1, tv2;
	char					fileName[256], cBuf[128];

	if( access( src, F_OK ) != 0 )   //if the directory is not exit
	{
		return -3;
	}

	memset( data, 0, sizeof( data ) );
	p1 = head_serial_devices;
	while( p1 )
	{
		if( p1->serial_port != uart_port || strcmp( p1->devices_type, "WEDS_DEVICES" ) != 0 )
		{
			p1 = p1->next;
			continue;
		}
		ret = 1;
		break;
	}
	if( ret == 0 )
	{
		return -2;
	}
	//    AA 01 03 3A 01 11 28 50 A5
	data[0]	   = 0X11;
	retval	   = device_send_data( p1->serial_port, 0X01, UPDATEUARTCARD, 0X01, data, 1 );
	if( retval < 0 )
	{
		return -1;
	}
//	printf( "read\n" );
	//读取返回值 AB 01 04 3A 01 01 01 3E 42 A5
	gettimeofday( &tv1, NULL );
	gettimeofday( &tv2, NULL );
	overtime = tv1.tv_sec * 1000000 + tv1.tv_usec;
	while( 1 )
	{
		if( abs( ( tv2.tv_sec * 1000000 + tv2.tv_usec ) - overtime ) > 6000000 )    //6s
		{
			return -4;
		}
		gettimeofday( &tv2, NULL );
		memset( tmp, 0, sizeof( tmp ) );
		retval = device_recv_data( p1->serial_port, tmp );
		if( retval < 0 )                                                            /* receive begin */
		{
			continue;
		}
		printf( "start:%02X,%02X,%02X\n", retval, synel_tdata.user_data[0], synel_tdata.user_data[1]);	// do not use 'tmp'

		if( retval == UPDATEUARTCARD )
		{
			//            break;
			switch( synel_tdata.user_data[0] )
			{
				case MA805:
//					retval = checkUpdateFile( fileName, tmp );
//					if( retval == -1 )
//					{printf("xxxxxxxxxxx 4\n");
//						return -3;
//					}

					retval = updateUartCard( uart_port, src );
					return retval;
				case STM32F030:
					gettimeofday( &tv1, NULL );
					overtime = tv1.tv_sec * 1000000 + tv1.tv_usec;
					memset( cBuf, 0, sizeof( cBuf ) );
					cBuf[0]	   = synel_tdata.user_data[0];
					cBuf[1]	   = synel_tdata.user_data[1];
					break;
				case UBOOTASK:
					retval = update_mcu( p1->serial_port, src, cBuf );
					return retval;
			}
		}
	}
//	printf( "error\n" );
	return -1;
}

#endif

/*
   获取设备型号
   uartPort:串口号
   outValue：输出参数，获取到的设备型号
   oSize：输出参数大小
   1:成功
   -1：失败
 */
int getUartCardType( int uartPort, char *outValue, int oSize )
{
	int						retval = 0;
        char					data[12],tmp[256];
	struct serial_devices	*p1;
	int						ret = 0;

	int						overtime = 0;
	struct timeval			tv1, tv2;

	if( outValue == NULL )
	{
		return ERROR;
        }
        memset( data, 0, sizeof( data ) );
	p1 = head_serial_devices;
	while( p1 )
	{
		if( p1->serial_port != uartPort || strcmp( p1->devices_type, "WEDS_DEVICES" ) != 0 )
		{
			p1 = p1->next;
			continue;
		}else
		{
			ret = 1;
			break;
		}
        }
	if( ret == 0 )
	{
		return ERROR;
	}
	data[0]	   = 0X10;
	retval	   = device_send_data( p1->serial_port, 0X01, UPDATEUARTCARD, 0X01, data, 1 );
	if( retval < 0 )
	{
		return ERROR;
	}
	//读取返回值
        memset( &tmp, 0, sizeof( tmp ) );
	gettimeofday( &tv1, NULL );
	gettimeofday( &tv2, NULL );
	overtime = tv1.tv_sec * 1000000 + tv1.tv_usec;
	while( 1 )
	{
		if( abs( ( tv2.tv_sec * 1000000 + tv2.tv_usec ) - overtime ) > 500000 )
		{
//			printf( "out time spi\n" );
			return ERROR;
		}
		gettimeofday( &tv2, NULL );
		retval = device_recv_data( p1->serial_port, tmp );
		if( retval < 0 )   /* receive begin */
		{
			continue;
		}
		if( retval == UPDATEUARTCARD )
		{
                        memset( outValue, 0, oSize );
                        hex2string(tmp, outValue,2 );
			retval = 1;
			break;
		}
	}
        printf( "outValue:%s\n", outValue );
	return retval;
}

/*第二种卡头升级方案*/

int update_mcu_start( int uartPort, char *oBuf )
{
	int				retval = 0, stat = 0;
	struct timeval	oldtime, newtime;
	int				oldsec = 0, overtime = 6000;
	TDATA			value;
	char			data[32];
	int				times = 0;

	memset( data, 0, sizeof( data ) );
//	data[0] = 0x11;
REDO:
	times++;
	data[0]	   = 0x55;                                                              // just for synel
	retval	   = device_send_data( uartPort, 0X01, UPDATEUARTCARD, 0X01, data, 1 ); // just for synel
	if( retval < 0 )
	{
		return -1;
	}

	gettimeofday( &oldtime, NULL );
	oldsec = ( oldtime.tv_sec % 10000 ) * 1000 + oldtime.tv_usec / 1000;
	while( 1 )
	{
		gettimeofday( &newtime, NULL );
		if( abs( ( ( newtime.tv_sec % 10000 ) * 1000 + newtime.tv_usec / 1000 ) - oldsec ) > overtime )
		{
//			printf( "update timeout ...times = %d, over = %d \n", times, overtime );
			if( times < 3 )
			{
				goto REDO;
			}
			return FALSE;
		}
		usleep( 150 );
		memset( &value, 0, sizeof( value ) );
		retval = device_recv_data( uartPort, (char *)&value );		// 此处存在丢失串口数据的情况，暂不处理 20170810
		if( retval < 0 )   /* receive begin */
		{
			continue;
		}

		if( retval == UPDATEUARTCARD && value.user_data[0] == 0X06 )
		{
			if( stat == 0 )
			{
				return 1;
			} else
			{
				return 2;
			}
		}else if( retval == UPDATEUARTCARD && stat == 0 )
		{
			overtime = 6000;
			gettimeofday( &oldtime, NULL );
			stat = 1;
		}
	}
	return -1;
}

/*
   int update_mcu_end()
   {
    int uartport=serialport3 + 10;
    _TData data;
    _TData asker;
    struct timeval oldtime,newtime;
    int oldsec=0,overtime=300;


    //send command start update
    memset(&data, 0, sizeof(data));
    data.address = 0X01;
    data.nbytes = 0X00;
    data.instruction = 0x3B;
    data.itemnum = 0x01;
    data.user_data[0] = 0x0;

    if (_put_data(uartport, &data) != SUCCESS)
        return FALSE;

    gettimeofday(&oldtime,NULL);
    oldsec=(oldtime.tv_sec % 10000) *1000+oldtime.tv_usec/1000;
    while(1){
        gettimeofday(&newtime,NULL);
        if (abs(((newtime.tv_sec % 10000) *1000+newtime.tv_usec/1000)-oldsec)>overtime)
        {
            return FALSE;//there is no card in 200ms
        }
        memset(&asker,0,sizeof(asker));
        if (_get_data(uartport,&asker)==SUCCESS)	//read data in block
        {
            printf("update_mcu_start instruction:%02X\n",asker.instruction);
            if(asker.instruction==0X00 && asker.user_data[0]==0X00)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
   }
 */
int update_mcu_senddata( int uartPort, char *value, int size, int endflag )
{
	TDATA	data;
	int		retval = 0;

	//send command start update
	memset( &data, 0, sizeof( data ) );
	data.address = 0X01;
	if( endflag == 0 )
	{
		data.nbytes		   = size;
		data.instruction   = 0x3A;
	}else
	{
		data.nbytes		   = 0x82;
		data.instruction   = 0x3B;
	}
	data.itemnum = 0x01;
	memcpy( data.user_data, value, size );
	retval = device_send_data( uartPort, 0X01, data.instruction, 0X01, data.user_data, data.nbytes );
	if( retval == ERROR )
	{
		return FALSE;
	}

	return TRUE;
}

//download file to mcu
int update_mcu_download( int uartPort, char *value, int size, int endflag )
{
	int				retval = 0;
//    char asker[1024];
	TDATA			asker;
//	memcpy(&synel_tdata, &tdata, sizeof(tdata));
	struct timeval	oldtime, newtime;
	int				oldsec = 0, newsec = 0, exitsec = 0, againtime = 2000, exittime = 10000;
	//int i;

	retval = update_mcu_senddata( uartPort, value, size, endflag ); //send data

	gettimeofday( &oldtime, NULL );
	oldsec	   = ( oldtime.tv_sec % 10000 ) * 1000 + oldtime.tv_usec / 1000;
	exitsec	   = oldsec;
	while( 1 )
	{
		gettimeofday( &newtime, NULL );
		newsec = ( ( newtime.tv_sec % 10000 ) * 1000 + newtime.tv_usec / 1000 );
		if( abs( newsec - oldsec ) > againtime )
		{
			printf( "re send ,2 out----------------------------------\n" );
			oldsec = newsec;
			retval = update_mcu_senddata( uartPort, value, size, endflag ); //send data
		}else if( abs( newsec - exitsec ) > exittime )                      //10s exit
		{
			return 2;
		}
		memset( &asker, 0, sizeof( asker ) );
		usleep( 150 );
		retval = device_recv_data( uartPort, (char*)&asker );
		if( retval < 0 )                                                    /* receive begin */
		{                                                                   //printf("recv err 11, continue\n");
			continue;
		}

		if( retval == UPDATEUARTCARD && asker.user_data[0] == 0X00 )
		{                                                                   printf("recv ok ....\n");
			break;
		}else if( retval == UPDATEUARTCARD && asker.user_data[0] == 0X01 )
		{
                        printf( "recv redo = %02X\n", retval );
			oldsec = newsec;
			retval = update_mcu_senddata( uartPort, value, size, endflag ); //send data
		}
	}
	return TRUE;
}

/*
   自动升级
   uartPort:串口号
   file：升级文件
   返回结果：
   1:成功
   -1：未知错误
   -2：升级设备串口未打开或者初始化类型不对
   -3:升级文件检测错误
   -4：未检测到设备型号
   -7:升级文件发送失败
 */
int update_mcu( int uartPort, char *fileName, char *cBuf )
{
	unsigned long	filesize   = 0, sendsize = 0;
	int				retval	   = 0;
	unsigned int	CMAXLEN	   = 128, nbytes;
	char			buf[CMAXLEN];
	FILE			*ffp;
	struct stat		f_stat;
	//    char fileName[128],tmp[128];

//	printf( "update_mcu-1:%s\n", fileName );


	/*
	   memset(fileName,0,sizeof(fileName));
	   strcpy(fileName,"/tmp/cardUpDate.bin");

	   retval = fileEncryptionAndDecryption(file,fileName);
	   if(retval == -1)
	   {
	    return -3;
	   }

	   memset(tmp,0,sizeof(tmp));
	   retval = update_mcu_start(uartPort,tmp);
	   if(retval <= 0)
	    return -2;
	 */
//	printf( "begin update%02X,%02X\n", cBuf[0], cBuf[1] );
	if( cBuf[0] != 0X00 && cBuf[1] != 0X00 )
	{
		retval = checkUpdateFile( fileName, cBuf );
		if( retval == -1 )
		{
			return -4;
		}
	}

	//
	if( stat( fileName, &f_stat ) < 0 )
	{
		return -3;
	}

	filesize = (unsigned long)f_stat.st_size;
	if( filesize == 0 )
	{
		return -3;
	}

	if( ( ffp = fopen( fileName, "rb" ) ) == NULL )
	{
		return -3;
	}
	while( !feof( ffp ) && !ferror( ffp ) )
	{
		memset( buf, 0, sizeof( buf ) );
		nbytes	   = fread( buf, sizeof( char ), CMAXLEN, ffp );
		sendsize  += nbytes;
		if( ( sendsize ) >= filesize || nbytes < CMAXLEN ) //file end
		{
			retval = update_mcu_download( uartPort, buf, nbytes, 1 );
		} else
		{
			retval = update_mcu_download( uartPort, buf, nbytes, 0 );
		}

		if( retval == 2 ) //overtime
		{
			fclose( ffp );
			return -7;
		}
	}
	printf( "ok:%d\n", sendsize );
	//    update_mcu_end();
	fclose( ffp );
	return 1;
}

// 检查是否需要固件升级
int check_mcu_update( void )
{
	if( synel_tdata.instruction == UPDATEUARTCARD &&
	    synel_tdata.user_data[0] == 0X06 )
	{
		return TRUE;
	}
	return FALSE;
}

/*
   update keyboad program
   uartPort: uart or spi port
   file : upload file
   mode :0-auto update,1-Manual update
 */
int synel_update_mcu( int uartPort, char *file, int mode )
{
	unsigned long	filesize   = 0, sendsize = 0;
	int				retval	   = 0;
	unsigned int	pkindex	   = 0;
	unsigned int	CMAXLEN	   = 128, nbytes;
	char			buf[CMAXLEN + 2];   // 数据内容前增加2Btyes长度，高字节在前
	char			obuf[CMAXLEN];
	FILE			*ffp;
	struct stat		f_stat;

	if( access( file, F_OK ) != 0 )     //if the directory is not exit
	{
		return FALSE;
	}

	if( stat( file, &f_stat ) < 0 )
	{
		return FALSE;
	}

	filesize = (unsigned long)f_stat.st_size;
	if( filesize == 0 )
	{
		return FALSE;
	}

	if( mode == active_update )
	{
		retval = update_mcu_start( uartPort, obuf );
		if( retval != 1 && retval != 2 )
		{
			return FALSE;
		}
	}

	if( ( ffp = fopen( file, "rb" ) ) == NULL )
	{
		return FALSE;
	}

	while( !feof( ffp ) && !ferror( ffp ) )
	{
		memset( buf, 0, sizeof( buf ) );
		nbytes	   = fread( buf + 2, sizeof( char ), CMAXLEN, ffp );
		sendsize  += nbytes;
		pkindex++;
//		printf( "pkindex ===== %d\n", pkindex );
		buf[0] = pkindex >> 8;
		buf[1] = ( pkindex & 0xFF );
		if( ( sendsize ) >= filesize || nbytes < CMAXLEN ) //file end
		{
			retval = update_mcu_download( uartPort, buf, nbytes + 2, 1 );
		} else
		{
			retval = update_mcu_download( uartPort, buf, nbytes + 2, 0 );
		}

		if( retval == 2 ) //overtime
		{
//			printf( "time out fail\n" );
			fclose( ffp );
			return FALSE;
		}
		if( sendsize >= filesize )
		{
			break;
		}
	}

	fclose( ffp );
	return TRUE;
}



// 20170817 
int init_mcu_port(int uartport)
{
	RK3288_51port = uartport;
}


int init_platform_type(int type)
{
    if(type>=PLATFORM_TYPE_MAX)
        PLATFORM_TYPE=PLATFORM_TYPE_MAX;
    else
	PLATFORM_TYPE = type;
}
















