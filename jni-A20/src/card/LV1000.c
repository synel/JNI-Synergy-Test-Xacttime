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
#include "LV1000.h"
#include "public.h"
#include "config.h"
#include "serial.h"
#include "debug.h"
#include "gpio.h"

int baud_back=9600;
/**
 *进入设置命令状态
 * @param uart_port 串口号
 * @return SUCCESS 成功；ERROR 失败
 */
int init_LV1000_card(int uart_port)
{
	int retval =0,nbytes=0;
	char recv_data[128];

	if(uart_port<0)
	{
		return ERROR;
	}

	memset(recv_data,0,sizeof(recv_data));
	//进入设置状态 发送指令
	retval = serial_send_data(uart_port,INPUT_SEND, strlen(INPUT_SEND));
	if(retval == ERROR)
	{
		return ERROR;
	}

	//进入设置状态 接收返回指令
	nbytes = strlen(INPUT_RECV);
	retval = serial_recv_data(uart_port,(unsigned char *)recv_data,nbytes);
	if(retval != nbytes || strcmp(recv_data,INPUT_RECV) != 0)
	{
		return ERROR;
	}
	return SUCCESS;
}
/**
 *退出设置命令状态
 * @param uart_port 串口号
 * @return SUCCESS 成功；ERROR 失败
 */
int quit_LV1000_card(int uart_port)
{
	int retval =0,nbytes=0;
	char recv_data[128];

	if(uart_port<0)
	{
		return ERROR;
	}

	memset(recv_data,0,sizeof(recv_data));
	//进入设置状态 发送指令
	retval = serial_send_data(uart_port,SIGNOUT_SEND, strlen(SIGNOUT_SEND));
	if(retval == ERROR)
	{
		return ERROR;
	}

	//进入设置状态 接收返回指令
	nbytes = strlen(SIGNOUT_RECV);
	retval = serial_recv_data(uart_port,(unsigned char *)recv_data,nbytes);
	if(retval != nbytes || strcmp(recv_data,SIGNOUT_RECV) != 0)
	{
		return ERROR;
	}
	return SUCCESS;
}
/**
 *设置lv1000读头属性
 * @param uart_port 串口号
 * @param value 设置值
 * @return SUCCESS 成功；ERROR 失败
 */
int set_LV1000_card(int uart_port,char *value)
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
        if(abs((int)difftime(time(NULL),recv_time)) > 2)  //秒为单位
        {            
            return ERROR;
        }
        retval = read_LV1000_card((int)uart_port,(char*)recv_data+len_total);
        if(retval == ERROR)   /* receive begin */
        {
            continue;
        }
        time(&recv_time);
        len_total += retval;

        memset(cmd_data,0,sizeof(cmd_data));

        if(recv_data[0] == '!') //读取成功
        {
            sprintf(cmd_data,"!%s;",value);
            return SUCCESS;

        }
        else if(recv_data[0] == '?')
        {
        	return ERROR;
        }
        else
        {
            return ERROR;
        }
        memset(recv_data,0,sizeof(recv_data));
        len_total = 0;
    }
}
/**
 *
 *通过RF_SIM协议读取串口数据
 * @param uart_port 使用的串口号
 * @param framet_ype 协议类型
 * @param value 通过串口读取到的有效数据
 *
 * @return 成功-读取的有效数据长度,失败-(-1)
 */
int read_LV1000_card(int uart_port,char *value)
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
 * @chinese
 * 设置LV1000识读模式
 * @param mode 模式（1～5）
 * mode1 手动识读模式，设置模式后，假如要按键触发读码，需要在采集到按键后调用Lv1000_off();Lv1000_on();这两步来触发读码。读码成功后，红外灭。
 * mode2 自动识读模式，设置模式后，假如要按键触发读码，需要在采集到按键后调用Lv1000_off();Lv1000_on();这两步来触发读码。该模式会一直读码，直到再次调用Lv1000_off();Lv1000_on();这两步。
 * mode3 间歇识读模式，设置模式后，自动开始间歇读码，该模式已经被设置成不会停止，会一直间歇读码。
 * mode4 感应识读模式，设置模式后，设备会自动感应外界光线变化，感应到光线变化后就会开启红外，直到读码成功后才会灭掉红外灯。
 * mode5 连续识读模式，设置模式后，设备会自动开始读码。
 * @param port_number 串口号
 * @param data1 读码时间长度（0～15）
 * @param data2 间歇时长（1～15）
 * @param psame 是否允许重复识读同一条码（1允许，0不允许）
 * @param restart 是否允许重新计时（1允许，0不允许）
 * @return 1 成功 -1 失败
 * @endchinese
 *
 * @english
 * set LV1000 read mode
 * @param mode mode of LV1000（1～5）
 * @param port_number serial port number
 * @param data1 read time（0～15）
 * @param data2 interval time（1～15）
 * @param psame read the same data（1 yep，0 nope）
 * @param restart restart timer（1 yep，0 nope）
 * @return 1 success -1 error
 * @endenglish
 */
int set_LV1000_mode(int mode, int port_number, int data1, int data2, int psame,int restart,int baud)
{
	char *DATA[10]={DATA_ZERO,DATA_ONE,DATA_TWO,DATA_THREE,DATA_FOUR,DATA_FIVE,DATA_SIX,DATA_SEVEN,DATA_EIGHT,DATA_NINE};
	int ret,ret1,ret2,ret3,ret4,ret6,ret7,ret8;
	char *BAUD[9]={BAUD_1200,BAUD_2400,BAUD_4800,BAUD_9600,BAUD_14400,BAUD_19200,BAUD_38400,BAUD_57600,BAUD_115200};
	int num;

	if(mode<=0 || mode>5 || data1>15 || data2>15)
		return ERROR;
	if(data2 < 2)
		data2 = 2;
	switch(baud)
	{
		case 1200:	num=0;break;
		case 2400:	num=1;break;
		case 4800:	num=2;break;
		case 9600:	num=3;break;
		case 14400:	num=4;break;
		case 19200:	num=5;break;
		case 38400:	num=6;break;
		case 57600:	num=7;break;
		case 115200:	num=8;break;
		default:	num=3;break;
	}

	if(baud_back != baud)
	{	
		Lv1000_off();
		init_LV1000_card(port_number);
		ret=set_LV1000_card(port_number,BAUD[num]);
		quit_LV1000_card(port_number);
		baud_back = baud;
		return ret;
	}
	switch(mode)
	{
		case 1:
			Lv1000_off();
			init_LV1000_card(port_number);
			ret1=set_LV1000_card(port_number,MANUAL_READ_MODE);
			ret2=set_LV1000_card(port_number,TIME_READ_DATA);
			ret3=set_LV1000_card(port_number,DATA[data1/10]);
			ret4=set_LV1000_card(port_number,DATA[data1%10]);//设置读码时间为00，为无限长
			set_LV1000_card(port_number,SAVE_DATA_PARAM);
			set_LV1000_card(port_number,ADD_SUFFIX_NO);
			set_LV1000_card(port_number,BAUD[num]);
			quit_LV1000_card(port_number);
			ret = ret1&ret2&ret3&ret4;
			break;
		case 2:
			Lv1000_off();
			init_LV1000_card(port_number);
			ret1=set_LV1000_card(port_number,AUTO_READ_MODE);
			ret2=set_LV1000_card(port_number,TIME_READ_DATA);
			ret3=set_LV1000_card(port_number,DATA[data1/10]);
			ret4=set_LV1000_card(port_number,DATA[data1%10]);//设置读码时间为00，为无限长
			set_LV1000_card(port_number,SAVE_DATA_PARAM);
			//暂时只有模式2添加了后缀“，”
			set_LV1000_card(port_number,ADD_SUFFIX_YES);
			set_LV1000_card(port_number,SET_SUFFIX);
			set_LV1000_card(port_number,DATA_TWO);
			set_LV1000_card(port_number,DATA_C);
			set_LV1000_card(port_number,SAVE_DATA_PARAM);
			set_LV1000_card(port_number,BAUD[num]);
			if(psame==1)
				ret6=set_LV1000_card(port_number,AUTOMODE_READ_SAMEDATA_YES);
			else
				ret6=set_LV1000_card(port_number,AUTOMODE_READ_SAMEDATA_NO);
			if(restart==1)
				ret7=set_LV1000_card(port_number,AUTOMODE_RESTART_TIMER);
			else
				ret7=set_LV1000_card(port_number,AUTOMODE_RESTART_TIMER_NO);
			quit_LV1000_card(port_number);
			ret = ret1&ret2&ret3&ret4&ret6&ret7;
			break;
		case 3:
			Lv1000_off();
			init_LV1000_card(port_number);
			ret1=set_LV1000_card(port_number,INTERVAL_READ_MODE);
			ret2=set_LV1000_card(port_number,INTERVAL_TIME_READ_DATA);
			ret3=set_LV1000_card(port_number,DATA[data2/10]);
			ret4=set_LV1000_card(port_number,DATA[data2%10]);//设置读码时间为00，为无限长
			set_LV1000_card(port_number,SAVE_DATA_PARAM);
			set_LV1000_card(port_number,ADD_SUFFIX_NO);
			set_LV1000_card(port_number,BAUD[num]);
			quit_LV1000_card(port_number);
			Lv1000_off();
			ret = ret1&ret2&ret3&ret4;
			break;
		case 4:
			Lv1000_off();
			init_LV1000_card(port_number);
			ret1=set_LV1000_card(port_number,INDUCT_READ_MODE);//感应识读模式
			ret2=set_LV1000_card(port_number,TIME_READ_DATA);
			ret3=set_LV1000_card(port_number,DATA[data1/10]);
			ret4=set_LV1000_card(port_number,DATA[data1%10]);//设置读码时间为00，为无限长
			set_LV1000_card(port_number,SAVE_DATA_PARAM);
			ret6=set_LV1000_card(port_number,INTERVAL_TIME_READ_DATA);
			ret7=set_LV1000_card(port_number,DATA[data2/10]);
			ret8=set_LV1000_card(port_number,DATA[data2%10]);
			set_LV1000_card(port_number,SAVE_DATA_PARAM);
			set_LV1000_card(port_number,ADD_SUFFIX_NO);
			set_LV1000_card(port_number,BAUD[num]);
			quit_LV1000_card(port_number);
			Lv1000_off();
			ret = ret1&ret2&ret3&ret4&ret6&ret7&ret8;
			break;
		case 5:
			Lv1000_off();
			init_LV1000_card(port_number);
			ret1=set_LV1000_card(port_number,CONTINUOUS_READ_MODE);//连续识读模式
			ret2=set_LV1000_card(port_number,INTERVAL_TIME_READ_DATA);
			ret3=set_LV1000_card(port_number,DATA[data2/10]);
			ret4=set_LV1000_card(port_number,DATA[data2%10]);//设置读码时间为00，为无限长
			set_LV1000_card(port_number,SAVE_DATA_PARAM);
			set_LV1000_card(port_number,ADD_SUFFIX_NO);
			set_LV1000_card(port_number,BAUD[num]);
			quit_LV1000_card(port_number);
			Lv1000_off();
			ret = ret1&ret2&ret3&ret4;
			break;
		default:
			break;
	}
	return ret;
}


