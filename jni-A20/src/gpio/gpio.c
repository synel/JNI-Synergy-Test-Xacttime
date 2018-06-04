/**
 * @chinese
 * @file   gpio.c
 * @author 刘训， lish
 * @date   2014-06-06 16:37:56
 *
 * @brief gpio设备接口模块 线程池文件没有使用，暂保留。
 * @endchinese
 *
 * @english
 * @file   gpio.c
 * @author Liu Xun， lish
 * @date   2014-06-06 16:37:56
 *
 * @brief gpio device module
 * @endenglish
 */
// 2014-5-16 所有的GPIO的操作秒级-->毫秒级 李绍辉
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include "gpio.h"
#include "public.h"
#include "inifile.h"
#include "config.h"
#include "stringchar.h"
#include "debug.h"
#include "device_protocol.h"
#include "serial_devices.h"

#include "../serial/device_protocol.h"
#include "../card/uhf.h"

#ifdef _DM365
#include "gpio_dm365.h"
#endif

#ifdef _ARM_2410
#include "gpio_s3c2410.h"
#endif

#ifdef _ARM_2416
#include "gpio_s3c2416.h"
#endif

#ifdef _AM335X
#include "gpio_am335x.h"
#endif

#ifdef _ANDROID
#include "gpio_android.h"
#endif

// GPIO 各管脚信息和执行状态
Gpio_pin_info gpio_pin_info[GPIO_MAX];

// GPIO 文件句柄
int				gpio_fd = -1;
// gpio 互斥锁
pthread_mutex_t gpio_mutex		   = PTHREAD_MUTEX_INITIALIZER;
pthread_t		Watch_Thread_ID	   = 0;
volatile int	last_exec_time	   = 0;
volatile int	thread_cancel_flag = 0, gpio_thread_pause = 0;
int				udisk_using;
//pauseTime pause time
void set_gpio_thread_pause( int pauseTime )
{
	gpio_thread_pause = pauseTime;
}

/*
 * GPO看守线程
 */
void *Thread_Watch_GPO( )
{
	int i			   = 0;
	int scan_count	   = 0;
	int pulse_state	   = 0;

	/*线程脱离主线程*/
	Watch_Thread_ID = pthread_self( );
	pthread_detach( Watch_Thread_ID );
	/*设置线程的取消类型*/
	pthread_setcancelstate( PTHREAD_CANCEL_ENABLE, NULL );
	pthread_setcanceltype( PTHREAD_CANCEL_ASYNCHRONOUS, NULL );

	thread_cancel_flag = 0;
	while( TRUE )
	{
		if( gpio_thread_pause )
		{
			sleep( gpio_thread_pause );
		}
		pthread_mutex_lock( &gpio_mutex );
		for( i = 0; i < GPIO_MAX; i++ )
		{
			switch( gpio_pin_info[i].operation )
			{
				case GPO_None:
					break;
				case GPO_On:
					if( ( GetTickCount( ) - gpio_pin_info[i].operation_time )
					    >= 0 )
					{
						set_gpio_value( gpio_pin_info[i].pin_num,
						                gpio_pin_info[i].value );
						gpio_pin_info[i].current_state	   = GPO_On;
						gpio_pin_info[i].operation		   = GPO_None;
						gpio_pin_info[i].operation_time	   = 0;
					}
					break;
				case GPO_Off:
					if( ( GetTickCount( ) - gpio_pin_info[i].operation_time )
					    >= 0 )
					{
						set_gpio_value( gpio_pin_info[i].pin_num,
						                abs( 1 - gpio_pin_info[i].value ) );
						gpio_pin_info[i].current_state	   = GPO_Off;
						gpio_pin_info[i].operation		   = GPO_None;
						gpio_pin_info[i].operation_time	   = 0;
					}
					break;
				case GPO_Pulse:
					if( ( GetTickCount( ) - gpio_pin_info[i].operation_time )
					    >= 0 )
					{
						set_gpio_value( gpio_pin_info[i].pin_num,
						                abs( 1 - gpio_pin_info[i].value ) );
						gpio_pin_info[i].current_state	   = GPO_Off;
						gpio_pin_info[i].operation		   = GPO_None;
						gpio_pin_info[i].operation_time	   = 0;
					}else
					{
						if( scan_count % ( gpio_pin_info[i].interval / 10 ) == 0 )
						{
							set_gpio_value( gpio_pin_info[i].pin_num,
							                pulse_state );
						}
					}
					break;
				default:
					break;
			}
		}

// 判断门禁扩展板延时状态	20180314

		for(i = 0; i < 5; i++)
		{//printf("in thread1-------- = index=%d, value=%d, delay=%d, time=%d\n", i, mj_stat[i][0], mj_stat[i][1], mj_stat[i][2]);
			if(mj_stat[i][1] > 0)
			{
				if(  GetTickCount( ) - mj_stat[i][2] >= mj_stat[i][1] )
				{//printf("in thread = index=%d, value=%d, delay=%d, time=%d\n", i, mj_stat[i][0], mj_stat[i][1], mj_stat[i][2]);
					if(mj_stat[i][0] == 0)
					{
						mj_ext_set(i+1, 1);
					}
					else
					{
						mj_ext_set(i+1, 0);
					}
					
					mj_stat[i][1] = 0;					
				}
			}
		}
		
		pthread_mutex_unlock( &gpio_mutex );
		scan_count	   = ( scan_count + 1 ) % 1000;
		pulse_state	   = ( pulse_state + 1 ) % 2;
		last_exec_time = GetTickCount( );
		if( thread_cancel_flag )
		{
			break;
		}
		usleep( 10000 ); // 10毫秒
	}
	pthread_exit( 0 );
}

/*
 *    检查GPO的看守线程
 */
void Check_Thread( )
{
	if( GetTickCount( ) - last_exec_time > 50000 )
	{
		thread_cancel_flag = 1;
		usleep( 40000 );
		pthread_create( &Watch_Thread_ID,
		                NULL,
		                (void*)Thread_Watch_GPO,
		                NULL );
	}
}

/**
 * @chinese
 * 初始化gpio设备
 * @param con_value GPIO管脚控制定义值
 * @param def_value GPIO管脚控制默认值
 *
 * @return 成功-TRUE,失败-FALSE
 * @endchinese
 *
 * @english
 * initialize gpio device
 * @param con_value  define value of controlling GPIO pin
 * @param def_value default value of controlling GPIO pin
 *
 * @return success-TRUE,fail-FALSE
 * @endenglish
 */
int init_gpio_devices( long con_value, int def_value )
{
	int i = 0;
#ifdef _ARM_2410
	gpio_fd = api_init_gpio( con_value, def_value );
#endif

#ifdef _DM365
	gpio_fd = open_gpio( );
#endif

#ifdef _ARM_2416
	gpio_fd = gpio_init_s3c2416( );
#endif

#ifdef _AM335X
	gpio_fd = gpio_init_am335x( );
#endif

	if( gpio_fd <= 0 )
	{
		perror( "init_gpio_devices" );
		return FALSE;
	}
	for( i = 0; i < GPIO_MAX; i++ )
	{
		memset( &gpio_pin_info[i], 0, sizeof( Gpio_pin_info ) );
	}
	pthread_mutex_init( &gpio_mutex, NULL );
	pthread_create( &Watch_Thread_ID,
	                NULL,
	                (void*)Thread_Watch_GPO,
	                NULL );
	return TRUE;
}

/**
 * @chinese
 * 初始化gpio管脚
 *
 * @param type GPIO管脚链接设备类型
 * @param value GPIO管脚使能使电平
 * @param pin_num GPIO管脚控制定义值
 *
 * @return 成功-TRUE,失败-FALSE
 * @endchinese
 *
 * @english
 * init gpio
 *
 * @param type gpio device type
 * @param value enable level
 * @param pin_num pin num
 *
 * @return success-TRUE,fail-FALSE
 * @endenglish
 *
 */
int init_gpio_value( int gpio_type, int pin_num, int value )
{
	int gpio_index = gpio_type;

	if( gpio_fd <= 0 || gpio_index < 0 || gpio_index > GPIO_MAX )
	{
		return FALSE;
	}

	// printf("init_gpio_value%d,%d,%d\n",gpio_index,pin_num,value);
	gpio_pin_info[gpio_index].activate		   = 1;
	gpio_pin_info[gpio_index].pin_num		   = pin_num;
	gpio_pin_info[gpio_index].value			   = value;
	gpio_pin_info[gpio_index].current_state	   = GPO_None;
	gpio_pin_info[gpio_index].operation		   = GPO_None;
	gpio_pin_info[gpio_index].operation_time   = 0;
	gpio_pin_info[gpio_index].interval		   = 100;

	return TRUE;
}

/**
 * @chinese
 * 设置gpio位使能
 *
 * @param value GPIO管脚使能使电平,必须是控制开启功能的电平
 * @param pin_num  GPIO管脚控制定义值
 *
 * @return 成功-TRUE,失败-FALSE
 * @endchinese
 *
 * @english
 * set gpio enable
 *
 * @param value gpio enable
 * @param pin_num  defining value of controlling gpio pin
 *
 * @return success-TRUE,fail-FALSE
 * @endenglish
 *
 */
int set_gpio_value( int pin_num, int value )
{
	int fd	   = gpio_fd;
	int flag   = FALSE;

	if( fd <= 0 )
	{
		return FALSE;
	}
#ifdef _ARM_2410
	flag = api_gpio_write( fd, pin_num, value );
#endif

#ifdef _DM365
	flag = set_pin_state( fd, pin_num, value );
#endif

#ifdef _ARM_2416
	flag = gpio_write_s3c2416( fd, pin_num, value );
#endif

#ifdef _AM335X
	flag = gpio_write_am335x( fd, pin_num, value );
#endif

	plog( "GPIO ---- %d  ===== %d \n", pin_num, value );
	return flag;
}


/**
 * @chinese
 * 打开GPIO链接设备操作
 *
 * @param Millisecond 操作时长
 * @param devices_type 操作设备类型 *
 *
 * @return 成功-TRUE,失败-FALSE
 * @endchinese
 *
 * @english
 * turn on gpio
 *
 * @param Millisecond time
 * @param devices_type device type
 *
 * @return success-TRUE,fail-FALSE
 * @endenglish
 *
 */
int turn_on_gpio( int millisecond, int devices_type )
{
	int gpio_index = devices_type;

	if( devices_type > GPIO_MAX
	    || devices_type < 0
	    || gpio_pin_info[gpio_index].activate == 0 )
	{
		return FALSE;
	}
	Check_Thread( );
	pthread_mutex_lock( &gpio_mutex );
	/*执行开启动作*/
	set_gpio_value( gpio_pin_info[gpio_index].pin_num,
	                gpio_pin_info[gpio_index].value );
	if( millisecond == 0 )
	{
		gpio_pin_info[gpio_index].current_state	   = GPO_On;
		gpio_pin_info[gpio_index].operation		   = GPO_None;
	}else
	{
		gpio_pin_info[gpio_index].current_state	   = GPO_On;
		gpio_pin_info[gpio_index].operation		   = GPO_Off;
		gpio_pin_info[gpio_index].operation_time   = GetTickCount( ) + millisecond;
	}
	pthread_mutex_unlock( &gpio_mutex );
	return TRUE;
}

/**
 * @chinese
 * 关闭GPIO链接设备操作
 *
 * @param Millisecond 操作时长
 * @param devices_type 操作设备类型 *
 *
 * @return 成功-TRUE,失败-FALSE
 * @endchinese
 *
 * @english
 * close gpio device
 *
 * @param Millisecond time
 * @param devices_type device type
 *
 * @return success-TRUE,fail-FALSE
 * @endenglish
 *
 */
int turn_off_gpio( int millisecond, int devices_type )
{
	int gpio_index = devices_type;

	if( devices_type > GPIO_MAX ||
	    devices_type < 0 ||
	    gpio_pin_info[gpio_index].activate == 0 )
	{
		return FALSE;
	}
	Check_Thread( );
	pthread_mutex_lock( &gpio_mutex );
	/*执行开启动作*/
	set_gpio_value( gpio_pin_info[gpio_index].pin_num,
	                abs( 1 - gpio_pin_info[gpio_index].value ) );
	if( millisecond == 0 )
	{
		gpio_pin_info[gpio_index].current_state	   = GPO_Off;
		gpio_pin_info[gpio_index].operation		   = GPO_None;
	}else
	{
		gpio_pin_info[gpio_index].current_state	   = GPO_Off;
		gpio_pin_info[gpio_index].operation		   = GPO_On;
		gpio_pin_info[gpio_index].operation_time   = GetTickCount( ) + millisecond;
	}
	pthread_mutex_unlock( &gpio_mutex );
	return TRUE;
}

/**
 * @chinese
 * 控制GPIO链接设备闪烁/脉冲操作
 *
 * @param millisecond 操作时长
 * @param devices_type 操作设备类型 *
 *
 * @return 成功-TRUE,失败-FALSE
 * @endchinese
 *
 * @english
 * gpio device shine
 *
 * @param millisecond time
 * @param devices_type device type
 *
 * @return success-TRUE,fail-FALSE
 * @endenglish
 *
 */
int pulse_gpio( int millisecond, int interval, int devices_type )
{
	int gpio_index = devices_type;

	if( devices_type > GPIO_MAX ||
	    devices_type < 0 ||
	    gpio_pin_info[gpio_index].activate == 0 )
	{
		return FALSE;
	}

	if( millisecond == 0 || interval == 0 )
	{
		return FALSE;
	}
	Check_Thread( );
	pthread_mutex_lock( &gpio_mutex );
	/*执行开启动作*/
	gpio_pin_info[gpio_index].current_state	   = GPO_On;
	gpio_pin_info[gpio_index].operation		   = GPO_Pulse;
	gpio_pin_info[gpio_index].operation_time   = GetTickCount( ) + millisecond;
	gpio_pin_info[gpio_index].interval		   = interval;
	pthread_mutex_unlock( &gpio_mutex );
	return TRUE;
}

/**
 *
 * @chinese
 * 操作gpio管脚设备
 * @param opt 操作类型(C-关闭,S-开启,B/P-脉冲/闪烁）
 * @param millisecond 操作时长
 * @param pin_num 操作设备类型
 * @endchinese
 *
 * @english
 * operate device of gpio pin define
 * @param opt operate type(C-close,S-start,B/P-pulse/blink）
 * @param millisecond time of operating gpio
 * @param value  gpio enable
 * @param pin_num defining pin_num of controlling gpio pin
 * @endenglish
 * @return
 */
int option_gpio( char opt, int millisecond, int devices_type )
{
	int fd = gpio_fd;

	if( fd <= 0 )
	{
		return FALSE;
	}
	plog( "++++option_gpio:%c,Millisecond:%d,pin_num:%d\n", opt, millisecond, devices_type );
	switch( tolower( opt ) )
	{
		case 's':   //开启一段时间
			turn_on_gpio( millisecond, devices_type );
			break;
		case 'b':
		case 'p':   //闪烁一段时间
			pulse_gpio( millisecond, 100, devices_type );
			break;
		case 'c':   //关闭一段时间
			turn_off_gpio( millisecond, devices_type );
			break;
		default: break;
	}
	plog( "end option gpio\n" );
	return TRUE;
}

//操作闪烁
int option_gpio_pulse( int millisecond, int interval, int devices_type )
{
	return pulse_gpio( millisecond, interval, devices_type );
}

/**
 * @chinese
 * 获得GPIO管脚定义的输入点的状态
 *
 * @param value GPIO管脚使能使电平
 * @param pin_num GPIO管脚控制定义值
 *
 * @return 成功-TRUE,失败-FALSE
 * @endchinese
 *
 * @english
 * get gpio state
 *
 * @param value enable level
 * @param pin_num pin num
 *
 * @return success-TRUE,fail-FALSE
 * @endenglish
 *
 */
int get_import_state( int devices_type )
{
	int fd	   = gpio_fd;
	int status = -1, thread_num = 0;

	if( fd <= 0 )
	{
		return FALSE;
	}
	plog( "begin get import state\n" );
	if( devices_type > GPRS_RESET || devices_type < 0 )
	{
		return FALSE;
	}
	thread_num = devices_type;
	pthread_mutex_lock( &gpio_mutex );

#ifdef _ARM_2410
	status = api_gpio_read( fd, gpio_pin_info[thread_num].pin_num );
#endif

#ifdef _DM365
	status = get_pin_state( fd, gpio_pin_info[thread_num].pin_num );
#endif

#ifdef _ARM_2416
	status = gpio_read_s3c2416( fd, gpio_pin_info[thread_num].pin_num );
#endif

#ifdef _AM335X
	status = gpio_read_am335x( fd, gpio_pin_info[thread_num].pin_num );
#endif

	pthread_mutex_unlock( &gpio_mutex );
	plog( "eng get import state:%d\n", status );
	if( status < 0 )
	{
		return FALSE;
	}

	if( status == gpio_pin_info[thread_num].value )
	{
		return TRUE;
	}

	return FALSE;
}




/**
 * @chinese
 * 关闭GPIO设备
 *
 * @return Success TRUE, Failure FALSE
 * @endchinese
 *
 * @english
 * close GPIO device
 *
 * @return Success TRUE, Failure FALSE
 * @endenglish
 *
 *
 */
int close_gpio( void )
{
	int fd	   = gpio_fd;
	int flag   = 0;

#ifdef _ARM_2410
	flag = api_gpio_close( fd );
#endif

#ifdef __DM365
	flag = api_close_gpio( fd );
#endif

#ifdef _ARM_2416
	flag = gpio_close_s3c2416( fd );
#endif
	if( flag == FALSE )
	{
		return FALSE;
	}
	gpio_fd = 0;
	return TRUE;
}

/*
    GPO 开关
    参数: IO_Port GPO口
    ll_switch 0-开 1-关
 */
int GPO_OnOff( int IO_Port, int ll_switch )
{
	int ret = -1;

	if( gpio_pin_info[IO_Port].activate == 0 )
	{
		return FALSE;
	}
	pthread_mutex_lock( &gpio_mutex );
	ret = set_gpio_value( gpio_pin_info[IO_Port].pin_num,
	                      abs( ll_switch - gpio_pin_info[IO_Port].value ) );
	pthread_mutex_unlock( &gpio_mutex );
	return ret;
}

/**
 * @chinese
 * 通过GPIO管脚定义控制gprs关闭操作
 *
 * @return 成功-TRUE,失败-FALSE
 * @endchinese
 *
 * @english
 * close gprs
 *
 * @return success-TRUE,fail-FALSE
 * @endenglish
 *
 */
int gprs_off( )
{
	return GPO_OnOff( GPRS_RESET, 1 );
}

/**
 * @chinese
 * 通过GPIO管脚定义控制gprs开启操作
 *
 * @return 成功-TRUE,失败-FALSE
 * @endchinese
 *
 * @english
 * open gprs
 *
 * @return success-TRUE,fail-FALSE
 * @endenglish
 *
 */
int gprs_on( )
{
	return GPO_OnOff( GPRS_RESET, 0 );;
}

/**
 * @chinese
 * 通过GPIO管脚定义控制摄像头开启操作
 *
 * @return 成功-TRUE,失败-FALSE
 * @endchinese
 *
 * @english
 * close camera
 *
 * @return success-TRUE,fail-FALSE
 * @endenglish
 *
 */
int camera_off( )
{
	return GPO_OnOff( CAMERA, 1 );
}

/**
 * @chinese
 * 通过GPIO管脚定义控制摄像头/关闭操作
 *
 * @return 成功-TRUE,失败-FALSE
 * @endchinese
 *
 * @english
 * open camera
 *
 * @return success-TRUE,fail-FALSE
 * @endenglish
 *
 */
int camera_on( )
{
	return GPO_OnOff( CAMERA, 0 );
}

/**
 * @chinese
 * 通过GPIO管脚定义控制打印机开启操作
 *
 * @return 成功-TRUE,失败-FALSE
 * @endchinese
 *
 * @english
 * close printer
 *
 * @return success-TRUE,fail-FALSE
 * @endenglish
 *
 */
int printer_off( )
{
	return GPO_OnOff( PRINTER_POWER, 1 );
}

/**
 * @chinese
 * 通过GPIO管脚定义控制打印机开启操作
 *
 * @return 成功-TRUE,失败-FALSE
 * @endchinese
 *
 * @english
 * open printer
 *
 * @return success-TRUE,fail-FALSE
 * @endenglish
 *
 */
int printer_on( )
{
	return GPO_OnOff( PRINTER_POWER, 0 );
}

/**
 * @chinese
 * 通过GPIO管脚定义控制LV1000开启操作
 *
 * @return 成功-TRUE,失败-FALSE
 * @endchinese
 *
 * @english
 * open LV1000
 *
 * @return success-TRUE,fail-FALSE
 * @endenglish
 *
 */
int Lv1000_on( )
{
	int ret		   = -1;
	int IO_Port	   = BARCODE_POWER;
	int ll_switch  = 0; // 0 - on 1 - off

	if( gpio_pin_info[IO_Port].activate == 0 )
	{
		return FALSE;
	}
	pthread_mutex_lock( &gpio_mutex );
	gpio_pin_info[BARCODE_POWER].value = ll_switch;
	ret								   = set_gpio_value( gpio_pin_info[IO_Port].pin_num,
	                                                     abs( ll_switch - gpio_pin_info[IO_Port].value ) );
	pthread_mutex_unlock( &gpio_mutex );
	sleep( 1 );
	return ret;
}

/**
 * @chinese
 * 通过GPIO管脚定义控制LV1000关闭操作
 *
 * @return 成功-TRUE,失败-FALSE
 * @endchinese
 *
 * @english
 * open LV1000
 *
 * @return success-TRUE,fail-FALSE
 * @endenglish
 *
 */
int Lv1000_off( )
{
	int ret		   = -1;
	int IO_Port	   = BARCODE_POWER;
	int ll_switch  = 1; // 0 - on 1 - off

	if( gpio_pin_info[IO_Port].activate == 0 )
	{
		return FALSE;
	}
	pthread_mutex_lock( &gpio_mutex );
	gpio_pin_info[BARCODE_POWER].value = ll_switch;
	ret								   = set_gpio_value( gpio_pin_info[IO_Port].pin_num,
	                                                     abs( ll_switch - gpio_pin_info[IO_Port].value ) );
	pthread_mutex_unlock( &gpio_mutex );
	sleep( 1 );
	return ret;
}

/**
 * @chinese
 * 通过GPIO管脚定义控制gpio开启操作
 *
 * @return 成功-TRUE,失败-FALSE
 * @endchinese
 *
 * @english
 * close printer
 *
 * @return success-TRUE,fail-FALSE
 * @endenglish
 *
 */
int set_gpio_off( int devices_type )
{
	return GPO_OnOff( devices_type, 1 );
}

/**
 * @chinese
 * 通过GPIO管脚定义控制gpio开启操作
 *
 * @return 成功-TRUE,失败-FALSE
 * @endchinese
 *
 * @english
 * open printer
 *
 * @return success-TRUE,fail-FALSE
 * @endenglish
 *
 */
int set_gpio_on( int devices_type )
{
	return GPO_OnOff( devices_type, 0 );
}

/**
 * @chinese
 * 通过GPIO管脚定义控制gps开启操作
 *
 * @return 成功-TRUE,失败-FALSE
 * @endchinese
 *
 * @english
 * close printer
 *
 * @return success-TRUE,fail-FALSE
 * @endenglish
 *
 */
int gps_off( )
{
	return GPO_OnOff( PRINTER_POWER, 1 );
}

/**
 * @chinese
 * 通过GPIO管脚定义控制gps开启操作
 *
 * @return 成功-TRUE,失败-FALSE
 * @endchinese
 *
 * @english
 * open printer
 *
 * @return success-TRUE,fail-FALSE
 * @endenglish
 *
 */
int gps_on( )
{
	return GPO_OnOff( PRINTER_POWER, 0 );
}

