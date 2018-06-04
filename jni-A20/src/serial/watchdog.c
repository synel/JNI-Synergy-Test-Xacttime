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
#include "printer_devices.h"
#include "device_protocol.h"
#include "serial_devices.h"
#include "uartlib.h"
#include "spilib.h"
#include "watchdog.h"
#include "public.h"
#include "serial.h"
#include "config.h"
#include "debug.h"
#include "../serial/watchdog.h"

#include <stdio.h>


int watchdog_port=0,watchflag = 0;


//通过spi设备发送看门狗值
int send_watch_info(char value)
{
    int retval=ERROR;
    char data[12];

    memset(data,0,sizeof(data));
    data[0]=value;
    retval=device_send_data(watchdog_port, 0X01, WATCHDOG, 0X01, data, 1);

    return retval;
}


/**
 * @chinese
 * 初始化看门狗,必须在初始化spi程序以后在初始化
 *
 * @return Success 成功.失败:ERROR
 * @endchinese
 *
 * @english
 * initialize serial port,add weds protocol
 *
 *
 * @return Success:fd.fail:FALSE
 * @endenglish
 *
 */
int init_watchdog()
{
    int retval = 0;
    int ret=0;
    char data[2];
    pthread_t thread_id_watchdog=0;
    struct serial_devices *p1;
    p1 = head_serial_devices;
    while(p1)
    {
        if(p1->serial_port <10 || strcmp(p1->devices_type,"WEDS_DEVICES") != 0)
        {
            p1 = p1->next;
            continue;
        }
        else
        {
        	ret=1;
            break;
        }
    }
    if(ret==0)
    	return ERROR;
    watchdog_port = p1->serial_port;
    retval = pthread_create(&thread_id_watchdog, NULL,(void*)keep_watchdog_alive, NULL);
    if (retval != 0)
    {
        plog("pthread_1_create");
        return ERROR;
    }
    data[0]= 0XA1;
    device_send_data(watchdog_port, 0X01, 0xB0, 0X01, data, 1);
    device_send_data(watchdog_port, 0X01, 0xB0, 0X01, data, 1);
    return SUCCESS;
}
//存储看门狗喂狗日志到某文件中
int save_watchdog_record(char *dir, int value,long long num)
{
	FILE* file;
	struct tm*t1;
	time_t t;
	char buf[100];
	memset(buf,0,100);
	file= fopen(dir,"a+");
	if(file == NULL)
	{
		printf("failed to open FILE\n");
        return ERROR;
	}
	time(&t);
	t1= localtime(&t);
	sprintf(buf,"%d,%d-%d-%d %d:%d:%d\n",value,t1->tm_year+1900,t1->tm_mon+1,t1->tm_mday,t1->tm_hour,t1->tm_min,t1->tm_sec);
	fwrite(buf,1,strlen(buf),file);
	fflush(file);
	fclose(file);
    return SUCCESS;
}
//喂狗
char* keep_watchdog_alive()
{
	long long i=0;
    int value = 0x1E,ret=ERROR;
	struct tm*t1;
	time_t t;
	char buf[20];
    watchflag = 1;
    char dir[50];
    char *dir1="/weds/kq42/watchdog1.log";
	time(&t);
	t1= localtime(&t);
	sprintf(buf,"%d-%d-%d",t1->tm_year+1900,t1->tm_mon+1,t1->tm_mday);
	sprintf(dir,"/weds/kq42/watchdog%s",buf);
    while(watchflag == 1)
    {
        ret=send_watch_info(value);
        if(ret==ERROR)
        	save_watchdog_record(dir1,value,i);
        //save_watchdog_record(dir,value,i);
#ifdef DEBUG
        save_watchdog_record(dir,value,i);
#endif
        //sleep(value/2);
        i++;
        sleep(5);
    }
    value=0x00;
    send_watch_info(value);
#ifdef DEBUG
    save_watchdog_record(dir,value,i);
#endif
    watchflag =2;
    return NULL;
}

//关闭看门狗
int close_watchdog()
{
    time_t last_time = 0;

    watchflag = 0;

    time(&last_time);
    while(watchflag == 0)
    {
        if(abs((int)difftime(time(NULL),last_time)) > 15)
        {
            watchflag = 0;
            return ERROR;
        }
        sleep(2);
    }
    return SUCCESS;
}
//关闭看门狗
int close_watchdog_info()
{
	//static time_t time4=0;
	int retval = -1;
	//time_t time3;
	//time3 = time(NULL);
	//if(abs(difftime(time3,time4)) < WATCHDOG_FEEDTIME)
	//{
	//	printf("too fast for close watchdog!\n");
		//return SUCCESS;
	//}
	retval = feed_watchdog(0);
	//time4 = time(NULL);
	return retval;
}
/**
 * @chinese
 * 看门狗喂狗
 *
 * @param type 发送喂狗的模块类型
 * @param num 发送喂狗的模块需要等待的调用次数
 *
 * @return SUCCESS喂狗成功 ERROR喂狗失败
 * @endchinese
 *
 **/
int send_watchdog_info(int model)
{
#ifndef FEED_WATCHDOG
    return 1;
#endif

	static int model_step[8]={240,8,48,1,240,240,1,240};
	static int MODEL_COUNT = 240;
	static int model_num=0;
	static time_t time2=0;
	time_t time1;
    int retval=-1;

    struct tm*t1;
    time_t t;
	time(&t);
    char buf[20];
    char dir[50];
	t1= localtime(&t);
	sprintf(buf,"%d-%d-%d",t1->tm_year+1900,t1->tm_mon+1,t1->tm_mday);
	sprintf(dir,"/weds/kq42/watchdog%s",buf);

	model_num += model_step[model];
	if(model_num >= MODEL_COUNT)
	{
		model_num=0;
		time1 = time(NULL);
		if(abs(difftime(time1,time2)) < WATCHDOG_FEEDTIME)
		{
			//printf("too fast for watchdog===%d\n",model);
			retval =  SUCCESS;
		}
		else
		{
			retval = feed_watchdog(WATCHDOG_TIME);
			time2 = time(NULL);
			save_watchdog_record(dir,model,0);
		}
	}
	return retval;
}
//向单片机发送喂狗数据
int feed_watchdog(char value)
{
	int retval = ERROR,ret = 0;
	char data[4];
	struct serial_devices *p1;
	p1 = head_serial_devices;
	while(p1)
	{
		if(p1->serial_port < 10 || strcmp(p1->devices_type,"WEDS_DEVICES") != 0)
		{
			p1 = p1->next;
			continue;
		}
		else
		{
			ret = 1;
			break;
		}
	}
	if(ret == 0)
		return ERROR;
	watchdog_port = p1->serial_port;
	memset(data,0,sizeof(data));
	data[0] = value;
	retval = device_send_data(watchdog_port, 0X01, WATCHDOG, 0X01, data, 1);
	retval = device_send_data(watchdog_port, 0X01, WATCHDOG, 0X01, data, 1);
	return retval;
}

//孙工测试
int send_watchdog_info_test(int watch_time)
{
    int retval=0;
	retval=feed_watchdog(watch_time);
    return retval;
}

