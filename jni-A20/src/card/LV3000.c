/*
 * LV3000.c
 *
 *  Created on: 2013-5-14
 *      Author: sherry
 */
#include "LV1000.h"
#include "general/public.h"
#include "general/config.h"
#include "serial/serial.h"
#include "debug.h"
#include "gpio/gpio.h"
#include "card/LV3000.h"
int read_LV3000_card(int uart_port,char *value)
{
    fd_set read_fd;
    struct timeval over_timer;
    int len_total=0;
    int retval=0;
    int fd = -1;

    fd = get_uard_fd(uart_port);
    if(fd<=0)
    {
        return ERROR;
    }

    over_timer.tv_sec = 0;
    over_timer.tv_usec = 0;
    FD_ZERO(&read_fd);
    FD_SET(fd,&read_fd);
    retval=select(fd+1, &read_fd, NULL, NULL, &over_timer);
    if(retval <= 0)
    {
        return ERROR;
    }
    memset(value,0,sizeof(value));
    while(1)
    {
        retval = serial_recv_onebyte((int)uart_port,(char*)value+len_total);
        if(retval != 1)   /* receive begin */
        {
            break;
        }
        len_total += retval;
    }
    return TRUE;
}
/**
 *设置lv3000读头属性
 * @param uart_port 串口号
 * @param value 设置值
 * @return SUCCESS 成功；ERROR 失败
 */
int set_LV3000_card(int uart_port,char *value)
{
    int retval =0,nbytes=0,len_total=0;
    char recv_data[128],cmd_data[128];
    time_t recv_time;//记录最后收数据时间

    if(uart_port<0 || value == NULL)
    {
        return ERROR;
    }
    //发送设置指令
    nbytes = strlen(value);
    retval = serial_send_data(uart_port,value, nbytes);
    if(retval != nbytes)
    {
        return ERROR;
    }

    memset(recv_data,0,sizeof(recv_data));
    time(&recv_time);
    while(1)
    {
        if(abs((int)difftime(time(NULL),recv_time)) > 1)  //秒为单位
        {
            return ERROR;
        }
        retval = read_LV3000_card((int)uart_port,recv_data+len_total);
        if(retval == ERROR)   /* receive begin */
        {
            continue;
        }
        time(&recv_time);
        len_total += retval;

        memset(cmd_data,0,sizeof(cmd_data));

        if(recv_data[0] == 0x06) //读取成功
        {
        	//printf("++++++++++%x\n",recv_data[0]);
            return SUCCESS;

        }
        else if(recv_data[0] == 0x15)
        {
        	return ERROR;
        }
        else
        {
            return ERROR;
        }
    }
    return ERROR;
}
/**
 * @chinese
 * 设置LV3000识读模式
 * @param mode 模式（1～3）
 * mode1 手动识读模式，设置模式后，假如要按键触发读码，需要在采集到按键后调用Lv1000_off();Lv1000_on();这两步来触发读码。读码成功后，红外灭。
 * mode2 自动识读模式，设置模式后，设备自动开始感应光线读码；
 * mode3 连续识读模式，设置模式后，假如要按键触发读码，需要在采集到按键后调用Lv1000_off();Lv1000_on();Lv1000_off();这三步来触发读码。该模式会一直读码，直到再次调用Lv1000_off();Lv1000_on();Lv1000_off();这三步。
 *
 * @return 1 成功 -1 失败
 * @endchinese
 */
int set_LV3000_mode(int mode, int port_number)
{
	int ret=-1,ret1=-1,ret2=-1,ret3=-1;
	switch(mode)
	{
		case 1:
			ret1=set_LV3000_card(port_number,"NLS0006010");
			ret2=set_LV3000_card(port_number,"NLS0302000");
			ret3=set_LV3000_card(port_number,"NLS0006000");
			ret = ret1&ret2&ret3;
			break;
		case 2:
			ret1=set_LV3000_card(port_number,"NLS0006010");
			ret2=set_LV3000_card(port_number,"NLS0302010");
			ret3=set_LV3000_card(port_number,"NLS0006000");
			ret = ret1&ret2&ret3;
			break;
		case 3:
			ret1=set_LV3000_card(port_number,"NLS0006010");
			ret2=set_LV3000_card(port_number,"NLS0302020");
			ret3=set_LV3000_card(port_number,"NLS0006000");
			ret = ret1&ret2&ret3;
			break;
	}
	return ret;
}
