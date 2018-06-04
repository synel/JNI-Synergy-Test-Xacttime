/*
 * LV3095.c
 *
 *  Created on: 2014-6-3
 *      Author: Frank
 */
#include "LV1000.h"
#include "public.h"
#include "config.h"
#include "serial.h"
#include "debug.h"
#include "gpio.h"
#include "LV3095.h"
#include "uartlib.h"


pthread_mutex_t    LV3095_mutex = PTHREAD_MUTEX_INITIALIZER;
int Lv3095_Uart_port = 3;
int Scan_Sign = 0;
/*
    方法：读取串口数据
    参数：uart_port，串口号；
    *value，串口返回的数据；
    返回值：成功-- 1 失败-- -1
*/
int read_LV3095_card(int uart_port,char *value)
{
    int len_total=0;
    int retval=0;
    if (Scan_Sign == 1)
    {
        pthread_mutex_lock(&LV3095_mutex);
        while(1)
        {
            retval = serial_recv_onebyte(uart_port,(char*)value+len_total);
            if(retval != 1 ||
            		(value[len_total-1] == 0x0D &&
            				value[len_total] == 0x0A))
            {
            	serial_clear(uart_port);
            	cut_rn(value);
            	break;
            }
            len_total += retval;
        }
        pthread_mutex_unlock(&LV3095_mutex);

    }
    else
    {
    	serial_clear(uart_port);
    }
    return TRUE;
}

int open_Lv3095_light()
{
	int ret = -1;
    unsigned int crc = 0;
    unsigned char send_buf[16];
    GPO_OnOff(Lv3095_Uart_port, 1);
    // 设置模式
    memset(send_buf, 0, sizeof(send_buf));
    send_buf[0] = 0x7E;
    send_buf[1] = 0x00;
    send_buf[2] = 0x08;
    send_buf[3] = 0x01;
    send_buf[4] = 0x00;
    send_buf[5] = 0x00;
    send_buf[6] = 0x14;
    crc = crc_cal_by_bit(send_buf + 2, 5);
    send_buf[7] = (crc >> 8) & 0xFF;
    send_buf[8] = crc & 0xFF;
    printf("CRC1: %02X %02X\n",send_buf[7],send_buf[8]);
    ret = set_LV3095_card(Lv3095_Uart_port, (char *)send_buf, 9);

    // 保存到eeprom
    memset(send_buf, 0, sizeof(send_buf));
    send_buf[0] = 0x7E;
    send_buf[1] = 0x00;
    send_buf[2] = 0x09;
    send_buf[3] = 0x01;
    send_buf[4] = 0x00;
    send_buf[5] = 0x00;
    send_buf[6] = 0x00;
    crc = crc_cal_by_bit(send_buf + 2, 5);
    send_buf[7] = (crc >> 8) & 0xFF;
    send_buf[8] = crc & 0xFF;
    printf("CRC4: %02X %02X\n",send_buf[7],send_buf[8]);
    ret = set_LV3095_card(Lv3095_Uart_port, (char *)send_buf, 9);

    GPO_OnOff(Lv3095_Uart_port, 0);
    usleep(50000);
    GPO_OnOff(Lv3095_Uart_port, 1);
    Scan_Sign = 1;
    return ret;
}

int close_Lv3095_light()
{
	int ret = -1;
    unsigned int crc = 0;
    unsigned char send_buf[16];
    // 设置模式
    GPO_OnOff(Lv3095_Uart_port, 0);
    memset(send_buf, 0, sizeof(send_buf));
    send_buf[0] = 0x7E;
    send_buf[1] = 0x00;
    send_buf[2] = 0x08;
    send_buf[3] = 0x01;
    send_buf[4] = 0x00;
    send_buf[5] = 0x00;
    send_buf[6] = 0x00;
    crc = crc_cal_by_bit(send_buf + 2, 5);
    send_buf[7] = (crc >> 8) & 0xFF;
    send_buf[8] = crc & 0xFF;
    printf("CRC1: %02X %02X\n",send_buf[7],send_buf[8]);
    ret = set_LV3095_card(Lv3095_Uart_port, (char *)send_buf, 9);

    // 保存到eeprom
    memset(send_buf, 0, sizeof(send_buf));
    send_buf[0] = 0x7E;
    send_buf[1] = 0x00;
    send_buf[2] = 0x09;
    send_buf[3] = 0x01;
    send_buf[4] = 0x00;
    send_buf[5] = 0x00;
    send_buf[6] = 0x00;
    crc = crc_cal_by_bit(send_buf + 2, 5);
    send_buf[7] = (crc >> 8) & 0xFF;
    send_buf[8] = crc & 0xFF;
    printf("CRC4: %02X %02X\n",send_buf[7],send_buf[8]);
    ret = set_LV3095_card(Lv3095_Uart_port, (char *)send_buf, 9);

    GPO_OnOff(Lv3095_Uart_port, 0);
    usleep(50000);
    GPO_OnOff(Lv3095_Uart_port, 1);
    Scan_Sign = 0;
    return ret;
}

/**
 *设置lv3095读头属性
 * @param uart_port 串口号
 * @param value 设置值
 * @return SUCCESS 成功；ERROR 失败
 */
int set_LV3095_card(int uart_port,char *value ,int send_len)
{
    int retval =0,nbytes=0;
    int len_total = 0;
    char rev_buf[32];
    time_t recv_time;//记录最后收数据时间

    pthread_mutex_lock(&LV3095_mutex);
    if(uart_port<0 || value == NULL)
    {
    	goto error;
    }

    //发送设置指令
    nbytes = send_len;
    retval = serial_send_data(uart_port, value, nbytes);
    if(retval != nbytes)
    {
    	goto error;
    }
    memset(rev_buf,0,sizeof(rev_buf));
    time(&recv_time);
    while(1)
    {
        if(abs((int)difftime(time(NULL),recv_time)) > 1)  //秒为单位
        {
        	goto error;
        }
        while(1)
        {
        	retval = uart_recv_data(serial_fd[uart_port],(char*)(rev_buf + len_total),1);
            if(retval != 1 || len_total > 9)
            {
            	break;
            }
            len_total += retval;
        }

        if (len_total)
        {
        	memcpy(value, rev_buf, sizeof(rev_buf));

			if(rev_buf[0] == 0x02 && rev_buf[2] == 0x00) //读取成功
			{
				goto success;
			}
			else
			{
				goto error;
			}
        }
    }

error:
	pthread_mutex_unlock(&LV3095_mutex);
    return ERROR;
success:
	pthread_mutex_unlock(&LV3095_mutex);
	return SUCCESS;
}

 /*
    计算CRC_CCITT 校验值（2 bytes）。
    计算的范围：Types、Lens、Address、Datas
    计算的方法为 CRC_CCITT，特征多项式：X16+X12+X5+1，
    即多项式系数为 0x1021，初始值为全 0，
    对于单个字节来说最高位先计算，
    不需要取反直接输出。
 */
unsigned int crc_cal_by_bit(unsigned char* ptr, unsigned int len)
{
    unsigned int crc = 0;
    unsigned int i = 0x80;
    while(len-- != 0)
    {
        for(i = 0x80; i != 0; i /= 2)
        {
            crc *= 2;
            //上一位 CRC 乘 2 后，若首位是 1，则除以 0x11021
            if((crc&0x10000) !=0)
                crc ^= 0x11021;
            //如果本位是 1，那么 CRC = 上一位的 CRC + 本位/CRC_CCITT
            if((*ptr&i) != 0)
                crc ^= 0x1021;
        }
        ptr++;
    }
    return crc;
}


/**
 * @chinese
 * 设置LV3095识读模式
 * @param mode 模式（0～3）
 * mode0 手动识读模式，设置模式后，假如要按键触发读码，需要在采集到按键后调用Lv1000_off();Lv1000_on();这两步来触发读码。读码成功后，红外灭。
 * mode1 命令模式，设置模式后，设备自动开始感应光线读码；
 * mode2 连续识读模式，设置模式后，假如要按键触发读码，需要在采集到按键后调用Lv1000_off();Lv1000_on();Lv1000_off();这三步来触发读码。该模式会一直读码，直到再次调用Lv1000_off();Lv1000_on();Lv1000_off();这三步。
 * mode3 感应模式。
 * @return 1 成功 -1 失败
 * @endchinese
 */


int set_LV3095_mode(int mode, int port_number)
{
	int ret=-1;
    unsigned int crc = 0;
    unsigned char send_buf[16];

    Lv3095_Uart_port = port_number;
    // 设置模式
    GPO_OnOff(port_number, 1);
    pthread_mutex_init(&LV3095_mutex,NULL);
    memset(send_buf, 0, sizeof(send_buf));
    send_buf[0] = 0x7E;
    send_buf[1] = 0x00;
    send_buf[2] = 0x08;
    send_buf[3] = 0x01;
    send_buf[4] = 0x00;
    send_buf[5] = 0x00;
    send_buf[6] = 0x00;
    crc = crc_cal_by_bit(send_buf + 2, 5);
    send_buf[7] = (crc >> 8) & 0xFF;
    send_buf[8] = crc & 0xFF;
    printf("CRC1: %02X %02X\n",send_buf[7],send_buf[8]);
    ret = set_LV3095_card(port_number,(char *)send_buf, 9);
    if (ret != 1) return ERROR;

    // 设置识别间隔
    send_buf[0] = 0x7E;
    send_buf[1] = 0x00;
    send_buf[2] = 0x08;
    send_buf[3] = 0x01;
    send_buf[4] = 0x00;
    send_buf[5] = 0x05;
    send_buf[6] = 0x32;
    crc = crc_cal_by_bit(send_buf + 2, 5);
    send_buf[7] = (crc >> 8) & 0xFF;
    send_buf[8] = crc & 0xFF;
    printf("CRC2: %02X %02X\n",send_buf[7],send_buf[8]);
    ret = set_LV3095_card(port_number, (char *)send_buf, 9);
    if (ret != 1) return ERROR;

    // 设置结束标识
     send_buf[0] = 0x7E;
     send_buf[1] = 0x00;
     send_buf[2] = 0x08;
     send_buf[3] = 0x01;
     send_buf[4] = 0x00;
     send_buf[5] = 0x60;
     send_buf[6] = 0xB1;
     crc = crc_cal_by_bit(send_buf + 2, 5);
     send_buf[7] = (crc >> 8) & 0xFF;
     send_buf[8] = crc & 0xFF;
     printf("CRC3: %02X %02X\n",send_buf[7],send_buf[8]);
     ret = set_LV3095_card(port_number, (char *)send_buf, 9);
     if (ret != 1) return ERROR;

    // 保存到eeprom
    memset(send_buf, 0, sizeof(send_buf));
    send_buf[0] = 0x7E;
    send_buf[1] = 0x00;
    send_buf[2] = 0x09;
    send_buf[3] = 0x01;
    send_buf[4] = 0x00;
    send_buf[5] = 0x00;
    send_buf[6] = 0x00;
    crc = crc_cal_by_bit(send_buf + 2, 5);
    send_buf[7] = (crc >> 8) & 0xFF;
    send_buf[8] = crc & 0xFF;
    printf("CRC4: %02X %02X\n",send_buf[7],send_buf[8]);
    ret = set_LV3095_card(port_number, (char *)send_buf, 9);
    GPO_OnOff(port_number, 0);
    usleep(50000);
    GPO_OnOff(port_number, 1);
    return ret;
}
