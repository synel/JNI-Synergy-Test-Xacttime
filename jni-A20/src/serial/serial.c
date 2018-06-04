/**
 * @chinese
 * @file   serial.c
 * @author 刘训
 * @date   Thu Oct 13 10:00:00 2011
 *
 * @brief  串口传输协议处理模块
 * @endchinese
 *
 * @english
 * @file   serial.c
 * @author Liu Xun
 * @date   Thu Oct 13 10:00:00 2011
 *
 * @brief  serial transmitting protocol handling module
 * @endenglish
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include "serial.h"
#include "uartlib.h"
#ifndef _ARM_FROCK
#include "spilib.h"
#endif
#include "config.h"
#include "debug.h"
//#include "../android/wedsa23.h"

char		_begin_putch[MAXCOM], _begin_getch[MAXCOM];
static char _address[MAXCOM];
/*定义MAXCOM个串口的文件描述符*/
int			serial_fd[MAXCOM];

pthread_mutex_t uart_mutex		   = PTHREAD_MUTEX_INITIALIZER;

/**
 * @chinese
 * 协议处理器
 *
 * @param com 串口号
 * @param data 协议结构体
 * @param action 解析/制作协议包
 *
 * @return
 * @endchinese
 *
 * @english
 * protocol handler
 *
 * @param com com
 * @param data data
 * @param action analyze/make protocol package
 *
 * @endenglish
 *
 */
int serial_package_processor( TCOM com, TDATA *tdata, ACTION action )
{
	int			retval = 0;
	TPACKAGE	package;
	int			len = 0;




	if( action == RECV_PACKAGE )
	{
pthread_mutex_lock( &uart_mutex );	
//		LOGI("serial pid ==== %lu\n", pthread_self());
		retval = serial_recv_package( com, package, _begin_getch[com], ETX );
pthread_mutex_unlock( &uart_mutex );		
		if( retval < 0 )
		{
			return retval;
		}

		return serial_analyze_package( package, tdata );
	}else if( action == SEND_PACKAGE )
	{
		if( tdata == NULL )
		{
			return ERROR;
		}

		len = serial_make_package( package, tdata );
		if( len < 0 )
		{
			return ERROR;
		}
		retval = serial_send_package( com, package, len );
		return retval;
	}

	return ERROR;
}

int _serial_package_processor_async( TCOM com, TDATA *tdata, ACTION action )
{
        int			retval = 0;
        TPACKAGE	package;
        int			len = 0;

        if( action == RECV_PACKAGE )
        {
                retval = serial_recv_package( com, package, _begin_getch[com], ETX );
                if( retval < 0 )
                {
                        return retval;
                }
                return serial_analyze_package( package, tdata );
        }else if( action == SEND_PACKAGE )
        {
                if( tdata == NULL )
                {
                        return ERROR;
                }

                len = serial_make_package( package, tdata );
                if( len < 0 )
                {
                        return ERROR;
                }
                retval = serial_send_package( com, package, len );
                return retval;
        }

        return ERROR;
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
int serial_init( int port, int baudrate, int workmode, int address )
{
	int fd = 0;
#ifdef _ARM_A23
	system( "su -c \"mkdir /mnt/obb/tmp\"" );
	system( "su -c \"chmod 777 /mnt/obb/tmp\"" );
#endif
	/*根据威尔终端机内部设备通讯规约，UART通讯模式115200/8/1/N方式*/
	fd = uart_open_and_setattr( port, baudrate, 8, "1", 'N' );

	if( fd < 0 )
	{
		return ERROR;
	}
	serial_fd[(TCOM)port] = fd;
	serial_clear( (TCOM)port );
	serial_set_work_mode( (TCOM)port, workmode );

	if( ( address < 0 ) || ( address > 255 ) )
	{
		address = 1;
	}

	serial_set_address( (TCOM)port, address );

	return serial_fd[(TCOM)port];
}

/**
 * @chinese
 * 设置工作方式
 *
 * @param com 串口号
 * @param work_mode 工作方式，取值为_TERMINAL 或 _SERVER
 * @endchinese
 *
 * @english
 * set working mode
 *
 * @param com serial port number
 * @param work_mode working mode,value maybe `_TERMINAL` or ` _SERVER`
 * @endenglish
 *
 */
void serial_set_work_mode( TCOM com, TWORK_MODE work_mode )
{
	switch( work_mode )
	{
		case TERMINAL:
			_begin_putch[com]  = STY;
			_begin_getch[com]  = STX;
			break;
		case SERVER:
			_begin_putch[com]  = STX;
			_begin_getch[com]  = STY;
			break;
		default:
			_begin_putch[com]  = STY; /* default : terminal mode */
			_begin_getch[com]  = STX;
			break;
	}
}

/**
   *取得工作方式
 *
 * @param com 串口号
 *
 * @return 当前工作方式
 */
TWORK_MODE serial_get_work_mode( TCOM com )
{
	return ( _begin_putch[com] == STX && _begin_getch[com] == STY ) ? SERVER : TERMINAL;
}

/**
 * @chinese
 * 设置通信地址
 *
 * @param com 串口号
 * @param address 通信地址
 * @note
    通信地址被规定了最小值和最大值，设置通信地址。通信地址用来标志一个接收者

 * @return 如果设置成功返回非零值，否则返回零值
 * @endchinese
 *
 * @english
 * set communication address
 *
 * @param com serial port number
 * @param address communication address
 * @note
    communication address was specified minimum value and maximum value.@n
    setting communication address to present a `receiver`.

 * @return If setting successfully,return non-zero,else return 0.
 * @endenglish
 *
 */
void serial_set_address( TCOM com, unsigned char address )
{
	if( address == SERVERADDR || address == BROADCASTADDR )
	{
		_address[com] = 0;
	}else
	{
		_address[com] = address;
	}
}

/**
 * @chinese
 * 取得通信地址,取得设置好的通信地址
 *
 * @param com 串口号

 * @return 返回通信地址
 * @endchinese
 *
 * @english
 * get address of communication
 *
 * @param com serial port number

 * @return communication address
 * @endenglish
 *
 */
char serial_get_address( TCOM com )
{
	return _address[com];
}

/**
 * @chinese
 * 根据协议，接收数据包
 *
 * @param com 串口号
 * @param package 存放接收到的数据
 * @param begin 开始标识符
 * @param end 结束标识符
 *
 * @return 成功-读取到的串口数据长度，失败-ERROR
 * @endchinese
 *
 * @english
 * read data from serial
 *
 * @param com com
 * @param package save data received from uart
 * @param begin begin
 * @param end end
 *
 * @return Success-data lenth of received，Fail-ERROR
 * @endenglish
 */

int serial_recv_package( TCOM com, TPACKAGE package, char begin, char end )
{
	int			i = 0;
	short int	nbytes;
	char		ch = 0, *p;


	if( package == NULL )
	{
		return ERROR;
	}

	p = package;

	while( 1 )
	{
		/*接收begin*/
		if( serial_recv_onebyte( com, &ch ) <= 0 )
		{
			return ERROR;
		}

		if( ch != 0 )
		{
                        plog( "begin : %02X\r\n", ch );
		}
		if( ch != begin )
		{
			if( i++ > 10 )
			{
				return BEGINERROR;
			}
			continue;
		}

//printf("<<--beg: %02X ", ch);
//LOGI("<<--beg: %02X ", ch);

		/*根据协议，这个字节是地址*/
		if( serial_recv_onebyte( com, &ch ) <= 0 )
                {
			return ERROR;
		}
//plog( "address : %02X,%02X,%02X\r\n", ch, serial_get_work_mode( com ), serial_get_address( com ) );
//LOGI( "address : %02X,%02X,%02X\n", ch, serial_get_work_mode( com ), serial_get_address( com ) );

		/*以终端方式工作时，检查是不是本机的地址；以服务器方式工作时，不检查地址*/
		if( serial_get_work_mode( com ) == TERMINAL && ch != serial_get_address( com ) )
		{
			return ADDRERROR;
		}else
		{
			break;
		}
	}

	/*把地址字节放入 package 的第一个字节*/
	*p++ = ch;
//printf("%02X ", ch);
//LOGI("<<%02X ", ch);

	/* 根据协议，这个字节是数据的字节数 */
	if( serial_recv_onebyte( com, &ch ) < 0 )
	{
		return ERROR;
	}
	/* 把数据字节数放入 package 的第二个字节 */
	*p++ = ch;
	plog( "len : %02X\r\n", ch );
//printf("%02X ", ch);
//LOGI("<<%02X ", ch);

	/* 加 2 是为了接收 BCC 和 SUM 校验位 */
	nbytes = (short int)ch + 2;
	if( nbytes > USERDATALEN )
	{
		return ERROR;
	}

	/* 接收数据 */
	if( serial_recv_data( com, (unsigned char *)p, nbytes ) < 0 )
	{
		return ERROR;
	}

	for( i = 0; i < nbytes; i++ )
	{
		if( i == 0 )
		{
			plog( "instruction : %02X\r\n", p[i] );
		}else if( i == 1 )
		{
			plog( "itemnum : %02X\r\n", p[i] );
		}else if( i < nbytes - 2 && i > 1 )
		{
			plog( "%02X ", p[i] );
		}else if( i == nbytes - 2 )
		{
			plog( "\r\nBCC : %02X", p[i] );
		}else if( i == nbytes - 1 )
		{
			plog( "\r\nSUM : %02X", p[i] );
		}

//printf("%02X ", p[i]);
//if(i > 0 && i%32 == 0) printf("\n");
//LOGI("<<%02X ", p[i]);

	}
	plog( "\r\n" );

	/* 根据协议现在取得的字节值应是 end */
	if( serial_recv_onebyte( com, &ch ) < 0 )
	{
		return ERROR;
	}
	plog( "end %02x\n", ch );
	if( ch != end )
	{
		return ENDERROR;
	}
	plog( "end : %02X\r\n", ch );

//printf("%02X\n", ch);
//LOGI("<<%02X\n", ch);

	return SUCCESS;
}

/**
 * @chinese
 * 根据协议，发送数据包
 *
 * @param com 串口号
 * @param package 要发送的数据
 * @param len 数据包长度
 *
 * @return 成功-SUCCESS，失败-ERROR
 * @endchinese
 *
 * @english
 * read data from serial
 *
 * @param com com
 * @param package data to send
 * @param len len of data
 *
 * @return Success-SUCCESS，Fail-ERROR
 * @endenglish
 */
int serial_send_package( TCOM com, TPACKAGE package, int len )
{
	int i = 0;

	if( package == NULL || len < 0 )
	{
		return ERROR;
	}
	plog( "com:%d\r\n", com );
	/*开始通讯*/
	if( serial_send_onebyte( com, _begin_putch[com] ) == ERROR )
	{
		return ERROR;
	}

	plog( "begin : %02X\r\n", _begin_putch[com] );

//printf("-->>beg: %02X ",_begin_putch[com]);
//LOGI("-->>beg: %02X ",_begin_putch[com]);

	/*发送协议包*/
	if( serial_send_data( com, (char *)package, len ) == ERROR )
	{
		return ERROR;
	}

	for( i = 0; i < len; i++ )
	{
		if( i == 0 )
		{
			plog( "address : %02X\r\n", package[i] );
		} else if( i == 1 )
		{
			plog( "len : %02X\r\n", package[i] );
		} else if( i == 2 )
		{
			plog( "instruction : %02X\r\n", package[i] );
		} else if( i == 3 )
		{
			plog( "itemnum : %02X\r\n", package[i] );
		} else if( i == len - 2 )
		{
			plog( "\r\nBCC : %02X\r\n", package[i] );
		} else if( i == len - 1 )
		{
			plog( "SUM : %02X\r\n", package[i] );
		} else
		{
			plog( "%02X ", package[i] );
		}

//printf("%02X ", package[i]);
//if(i > 0 && i%32 == 0) printf("\n");
//LOGI(">>%02X ", package[i]);

	}

	/*结束通讯*/
	if( serial_send_onebyte( com, ETX ) == ERROR )
	{
		return ERROR;
	}

//printf("%02X\n", ETX);
//LOGI(">>%02X\n", ETX);

	plog( "end:%02X\r\n", ETX );
	return SUCCESS;
}

/**
 * @chinese
 * 分析数据包
 *
 * @param package 接收到的网络数据包
 * @param data 指向将要存放已接收数据的结构的指针
 *
 * @note
 * 它首先对 package 指向的数据包的尾部处理，执行下面的操作： @n
 *   1:计算出 bcc 校验，与 package 中的 bcc 对比，看是否相等  @n
 *   2:计算出 sum 校验，与 package 中的 sum 对比，看是否相等 @n
 *   3:如果上面二步执行正确，对参数指向的变量赋以相应的值。 @n
 *
 * @return 成功-SUCCESS，失败-ERROR
 *
 * @endchinese
 *
 * @english
 * analyze package
 *
 * @param package package
 * @param data saving package
 *
 * @return Success-SUCCESS，Fail-ERROR
 * @endenglish
 */
int serial_analyze_package( TPACKAGE package, TDATA *tdata )
{
	unsigned char *p = (unsigned char *)package;

	if( p[p[P_LENGTH] + 2] != serial_cal_bcc( p, p[P_LENGTH] + 2 ) )
	{
		return BCCERROR;
	}

	if( p[p[P_LENGTH] + 3] != serial_cal_sum( p, p[P_LENGTH] + 2 ) )
	{
		return SUMERROR;
	}

	/*向tdata指向的结构中的各个域赋以相应的值*/
	tdata->instruction = p[P_INSTRUCTION];
	tdata->address	   = p[P_ADDRESS];
	tdata->nbytes	   = p[P_LENGTH] - 2;
	tdata->itemnum	   = p[P_ITEMNUM];
	memcpy( tdata->user_data, p + 4, p[P_LENGTH] - 2 );
	return SUCCESS;
}

/**
 * @chinese
 * 制作数据包
 *
 * @param package 网络数据包
 * @param data 协议结构体
 *
 * @return Success-SUCCESS，Fail-BCCERROR|SUMERROR
 * @endchinese
 *
 * @english
 * make package
 *
 * @param package package
 * @param data TDATA
 *
 * @return Success-SUCCESS，Fail-BCCERROR|SUMERROR
 * @endenglish
 */
int serial_make_package( TPACKAGE package, TDATA *tdata )
{
	unsigned char	*p = (unsigned char *)package;
	int				len;

	p[P_ADDRESS]	   = tdata->address;
	p[P_INSTRUCTION]   = tdata->instruction;
	if( tdata->nbytes == 0 )
	{
		/*没有数据*/
		p[P_ITEMNUM] = tdata->itemnum = 0;
		memset( p + P_ITEMNUM + 1, 0, MAXLENGTH - 4 );
		p[P_LENGTH] = 2;
	}else
	{
		p[P_ITEMNUM] = tdata->itemnum;
		memcpy( p + P_ITEMNUM + 1, tdata->user_data, tdata->nbytes - 2 );
		p[P_LENGTH] = tdata->nbytes;
	}

	p[p[P_LENGTH] + 2] = serial_cal_bcc( p, p[P_LENGTH] + 2 );
	p[p[P_LENGTH] + 3] = serial_cal_sum( p, p[P_LENGTH] + 2 );

	len = 2 + 2 + tdata->nbytes;
	return len;
}

/**
 * @chinese
 * 计算异或校验
 *
 * @param data 指向数据块的指针
 * @param nbytes 数据块中数据的字节数
 *
 * @note
 * 计算公式：BCC = DATA(0)^DATA(1)^DATA(2)^...^DATA(nbytes - 1)；
 *
 * @return 成功-计算出的异或校验值，失败(如果 data 为空指针或 nbytes 不大于 0)-ERROR
 * @endchinese
 *
 * @english
 * calculate bcc
 *
 * @param data data
 * @param nbytes lenth of data
 *
 * @return Success-bcc，Fail-ERROR
 * @endenglish
 */
unsigned char serial_cal_bcc( unsigned char *data, short int nbytes )
{
	short int		i	   = 0;
	unsigned char	bcc	   = 0, *p = data;

	if( data == NULL || nbytes <= 0 )
	{
		return ERROR;
	}

	while( i < nbytes )
	{
		bcc = bcc ^ ( p[i++] & 0xFF );
	}

	return bcc;
}

/**
 * @chinese
 * 计算和校验
 *
 * @param data 指向数据块的指针
 * @param nbytes 数据块中数据的字节数
 *
 * @note
 * SUM = LOWBATE(DATA(0)+DATA(1)+DATA(2)+...+DATA(nbytes - 1))；其中，LOWBATE 表示的计算是取数值的最低字节
 *
 * @return 成功-计算出的和校验值，失败(如果 data 为空指针或 nbytes 不大于 0)-ERROR
 * @endchinese
 *
 * @english
 * calculate sum
 *
 * @param data data
 * @param nbytes lenth of data
 *
 * @return Success-bcc，Fail-ERROR
 * @endenglish
 */
unsigned char serial_cal_sum( unsigned char *data, short int nbytes )
{
	short int			i	   = 0;
	unsigned char		*p	   = data;
	unsigned long int	sum	   = 0UL;

	if( data == NULL || nbytes <= 0 )
	{
		return ERROR;
	}

	while( i < nbytes )
	{
		sum += ( p[i++] & 0xFF );
	}
	/*Only get the minimum bytes*/
	return (unsigned char)( sum & (unsigned long int)0xFF );
}

/**
 * @chinese
 * 通过串口读取数据
 *
 * @param com 串口号
 * @param data 存放接收到的数据
 * @param len 要读取的数据长度
 *
 * @return 成功-读取到的串口数据长度，失败-ERROR
 * @endchinese
 *
 * @english
 * read data from serial
 *
 * @param com com
 * @param data save data received from uart
 * @param len lenth of data to receive from uart
 *
 * @return Success-data lenth of received，Fail-ERROR
 * @endenglish
 */
int serial_recv_data( TCOM com, unsigned char *data, int len )
{
	int i		   = 0;
	int len_total  = 0, len_tmp = 0;
	if( serial_fd[com] <= 0 )
	{
		return ERROR;
	}
	while( len > len_total )
	{
		len_tmp = 0;
		if( is_uart( com ) )
		{
			len_tmp = uart_recv_data( serial_fd[com], (char *)data + len_total, len - len_total );
			if( len_tmp < 0 )
			{
				return len_total;
				return ERROR;
			}else if( len_tmp == 0 )
			{
				if( ( i++ ) > 5 ) //10 lxy 2013-3-13 text_card 卡
				{
					return len_total;
					//return ERROR;
				}
			}
		}
#ifndef _ARM_FROCK
		else
		{
			/*注意：DM365驱动只支持每次取1个字节*/
			len_tmp = spi_recv_data( serial_fd[com], (char *)data + len_total, 1 );
			if( len_tmp < 0 )
			{
				return ERROR;
			}
		}
#endif
		len_total = len_total + len_tmp;
	}
#if 0
	if( len_total > 0 && len_total < 48 )
	{
		printf( "<<-- " );
		for( i = 0; i < len_total; i++ )
		{
			printf( "%02X ", data[i] );
			if( i > 0 && i % 32 == 0 )
			{
				printf( "\n" );
			}
		}
		printf( "\n" );
	}
#endif
	return len_total;
}

/**
 * @chinese
 * 通过串口读取数据
 *
 * @param com 串口号
 * @param data 存放接收到的数据
 *
 * @return 成功-读取到的串口数据长度，失败-ERROR
 * @endchinese
 *
 * @english
 * read data from serial
 *
 * @param com com
 * @param data save data received from uart
 *
 * @return Success-data lenth of received，Fail-ERROR
 * @endenglish
 */
int serial_recv_data_all( TCOM com, unsigned char *data )
{
	int i		   = 0;
	int len_total  = 0, len_tmp = 0;

	if( serial_fd[com] <= 0 )
	{
		return ERROR;
	}

	while( len_total <= 4096 )
	{
		len_tmp = 0;
		if( is_uart( com ) )
		{
			len_tmp = uart_recv_data( serial_fd[com], (char *)data + len_total, 1 );
			if( len_tmp <= 0 )
			{
				return len_total;
			}
		}
#ifndef _ARM_FROCK
		else
		{
			/*注意：DM365驱动只支持每次取1个字节*/
			len_tmp = spi_recv_data( serial_fd[com], (char *)data + len_total, 1 );
			if( len_tmp < 0 )
			{
				return ERROR;
			}
		}
#endif
		len_total = len_total + len_tmp;
	}

	return len_total;
}

/**
 * @chinese
 * 通过串口读取一个字节数据
 *
 * @param com 串口号
 * @param data 存放接收到的数据
 * @param len 要读取的数据长度
 *
 * @return 成功-读取到的串口数据长度, 0-无数据，失败-ERROR
 * @endchinese
 *
 * @english
 * read one byte data from serial
 *
 * @param com com
 * @param data save data received from uart
 * @param len lenth of data to receive from uart
 *
 * @return Success-data lenth of received，0-no data, Fail-ERROR
 * @endenglish
 */
int serial_recv_onebyte( TCOM com, char * byte )
{
	int len = 0;

	if( serial_fd[com] <= 0 )
	{
		return ERROR;
	}

	if( is_uart( com ) )
	{
		len = uart_recv_data( serial_fd[com], byte, 1 );
	}
#ifndef _ARM_FROCK
	else
	{
		len = spi_recv_data( serial_fd[com], byte, 1 );
	}
#endif
	return len;
}

/**
 * @chinese
 * 通过串口发送数据
 *
 * @param com 串口号
 * @param data 发送的数据
 * @param len 要发送的数据长度
 *
 * @return 成功-发送的数据长度，失败-ERROR
 * @endchinese
 *
 * @english
 * send data from serial
 *
 * @param com com
 * @param data save data received from uart
 * @param len lenth of data to receive from uart
 *
 * @return Success-data lenth of sent，Fail-ERROR
 * @endenglish
 */
int serial_send_data( TCOM com, char *data, int len )
{
	int len_total = 0, len_tmp = 0;

	if( serial_fd[com] <= 0 )
	{
		return ERROR;
	}

	while( len > len_total )
	{
		len_tmp = 0;
		if( is_uart( com ) )
		{
			len_tmp = uart_send_data( serial_fd[com], data + len_total, len - len_total );
		}
#ifndef _ARM_FROCK
		else
		{
			len_tmp = spi_send_data( serial_fd[com], data + len_total, 1 );
			usleep( 15 );
		}
#endif
		len_total = len_total + len_tmp;
	}

	return len_total;
}

/**
 * @chinese
 * 通过串口发送一字节
 *
 * @param com 串口号
 * @param data 发送的数据
 * @param len 要发送的数据长度
 *
 * @return 成功-发送的数据长度，失败-ERROR
 * @endchinese
 *
 * @english
 * send one byte from serial
 *
 * @param com com
 * @param data save data received from uart
 * @param len lenth of data to receive from uart
 *
 * @return Success-data lenth of sent，Fail-ERROR
 * @endenglish
 */
int serial_send_onebyte( TCOM com, char byte )
{
	int len = 0;

	if( serial_fd[com] <= 0 )
	{
		return ERROR;
	}

	if( is_uart( com ) )
	{
		len = uart_send_data( serial_fd[com], &byte, 1 );
	}
#ifndef _ARM_FROCK
	else
	{
		len = spi_send_data( serial_fd[com], &byte, 1 );
	}
#endif
	return len;
}

/**
 * @chinese
 * 清空串口缓存数据
 *
 * @param com 串口号
 *
 * @return 成功-SUCCESS，失败-ERROR
 * @endchinese
 *
 * @english
 * clear serial port
 *
 * @param com com
 *
 * @return Success-SUCCESS，Fail-ERROR
 * @endenglish
 *
 */
int serial_clear( TCOM com )
{
	if( serial_fd[com] <= 0 )
	{
		return ERROR;
	}

	if( is_uart( com ) )
	{
//printf("uart clear xxxxxxxxxxxxxxxx = %d\n", com);	
		return uart_clear( serial_fd[com] );
	}
#ifndef _ARM_FROCK
	else
	{
		return spi_clear( serial_fd[com] );
	}
#endif
	return ERROR;
}

/**
 * @chinese
 * 关闭串口
 *
 * @param com 串口号
 *
 * @return 成功-SUCCESS，失败-ERROR
 * @endchinese
 *
 * @english
 * close serial port
 *
 * @param com com
 *
 * @return Success-SUCCESS，Fail-ERROR
 * @endenglish
 *
 */
int serial_close( TCOM com )
{
	if( serial_fd[com] <= 0 )
	{
		return ERROR;
	}

	if( is_uart( com ) )
	{
		return uart_close( serial_fd[com] );
	}
#ifndef _ARM_FROCK
	else
	{
		return spi_close( serial_fd[com] );
	}
#endif
	return ERROR;
}

/**
 * @chinese
 * 检测是否是UART
 *
 * @param com 串口号
 *
 * @return 是-TRUE，不是-FALSE
 * @endchinese
 *
 * @english
 * check if device is uart
 *
 * @param com com
 *
 * @return yes-TRUE，no-FALSE
 * @endenglish
 *
 */
int is_uart( TCOM com )
{
	if( com < SPI_COM )
	{
		return TRUE;
	}else
	{
		return FALSE;
	}
}

/**
 * @chinese
 * 检测是否是SPI
 *
 * @param com 串口号
 *
 * @return 是-TRUE，不是-FALSE
 * @endchinese
 *
 * @english
 * check if device is SPI
 *
 * @param com com
 *
 * @return yes-TRUE，no-FALSE
 * @endenglish
 *
 */
int is_spi( TCOM com )
{
	if( com < SPI_COM )
	{
		return FALSE;
	}else
	{
		return TRUE;
	}
}

//获得串口句柄
int get_uard_fd( TCOM com )
{
	return serial_fd[com];
}

//检测串口是否有数据
int check_uart_cache( TCOM com )
{
	fd_set			recv_fs;
	struct timeval	tv;
	int				retval = 0, sockfd = -1;

	sockfd = serial_fd[com];
	if( sockfd < 0 )
	{
		return ERROR;
	}

	FD_ZERO( &recv_fs );
	FD_SET( sockfd, &recv_fs );

	tv.tv_sec  = 0;
	tv.tv_usec = 0;

	retval = select( sockfd + 1, &recv_fs, NULL, NULL, &tv );
	/*报错*/
	if( retval < 0 )
	{
		return ERROR;
	}
	/*socket的状态没有发生变化*/
	else if( retval == 0 )
	{
		return FALSE;
	}else
	{
		return TRUE;
	}
}

