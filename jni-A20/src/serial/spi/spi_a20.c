/*
 * spi_a20.c
 *
 *  Created on: 2014-8-7
 *      Author: aduo
 */


#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <unistd.h>
#include <stdint.h>

#include "spi_a20.h"
#include "config.h"
#include "debug.h"


//#define BAUDRATE_SPI 10000
#define BAUDRATE_SPI 1000000



/**
 * @chinese
 * 打开spi设备
 *
 * @param path 设备路径
 *
 * @return 成功-文件描述符，失败-ERROR
 * @endchinese
 *
 * @english
 * open spi device
 *
 * @param path path of spi
 *
 * @return Success-fd，Fail-ERROR
 * @endenglish
 *
 */
int a20_spi_open()
{
    int fd = 0;
	unsigned char mode = SPI_CPHA,lsb;		// 20170516 lfg set mode to SPI_CPHA
    unsigned long baudrate_spi = BAUDRATE_SPI;
    unsigned char bits = 8;
    fd=open("/dev/spidev2.0", O_RDWR);

    if(fd<0)
    {
        printf("open spi error!\r\n");
        return ERROR;
    }

    a20_spi_set_bits(fd, &bits);
    a20_spi_set_write_mode(fd, &mode);
    a20_spi_set_max_speed(fd, &baudrate_spi);
//    a20_spi_get_bits(fd, &bits);
//    a20_spi_set_read_mode(fd, &mode);
//    a20_spi_get_max_speed(fd, &baudrate_spi);
    ioctl(fd, SPI_IOC_RD_LSB_FIRST, &lsb);
    return fd;
}

/**
 * @chinese
 * 关闭spi设备
 *
 * @param fd 打开文件描述符
 *
 * @return 成功-文件描述符，失败-ERROR
 * @endchinese
 *
 * @english
 * close spi device
 *
 * @param fd open file descirptor
 *
 * @return Success-fd，Fail-ERROR
 * @endenglish
 *
 */
int a20_spi_close(int fd)
{

	printf("close spi \r\n");
    if(fd<0)
        return ERROR;

    if(close(fd)<0)
    {
        printf("close spi error!\r\n");
        return ERROR;
    }

    return SUCCESS;
}

/**
 * @chinese
 * 读取数据
 *
 * @param fd 已打开的SPI
 * @param data 存放接收到的数据
 * @param len 要读取的数据长度
 *
 * @return 成功-读取到的串口数据长度, 0-无数据，失败-ERROR
 * @endchinese
 *
 * @english
 * read data
 *
 * @param fd spi opened
 * @param data save data received from uart
 * @param len lenth of data to receive from uart
 *
 * @return Success-data lenth of received，0-no data, Fail-ERROR
 * @endenglish
 */
int a20_spi_recv_data(int fd, char *data, int len)
{
    char tx[1];
    //    char rx[1] = {0};
    int ret=0;
    struct spi_ioc_transfer tr;

    if(data == NULL || len<0)
        return ERROR;

        ret=a20_spi_read_data(fd, data, len);
        if(ret<1 || data == NULL)
        {
            //pthread_mutex_unlock(&spi_mutex);
            return ERROR;
        }

    return len;
}


/**
 * @chinese
 * 发送数据
 *
 * @param fd 已打开的串口
 * @param data 要发送的数据
 * @param len 数据长度
 *
 * @return 成功-已发送长度，失败-ERROR
 * @endchinese
 *
 * @english
 * send data
 *
 * @param fd spi opened
 * @param data data to send
 * @param len lenth of data
 *
 * @return Sucess-lenth of data sent,fail-ERROR
 * @endenglish
 */
int a20_spi_send_data(int fd, char *data, int len)
{
    //    char tx[1];
    int ret=0;


    if(data == NULL || len<0)
        return ERROR;


    if(ret<1)
    {
        ret=write(fd, data, len);
        if(ret<1 || strlen(data) <= 0)
        {
            //pthread_mutex_unlock(&spi_mutex);
            return ERROR;
        }
    }
    //pthread_mutex_unlock(&spi_mutex);
    return len;
}

//读取spi
int a20_spi_read_data(int fd, char *data, int len)
{
    int retval=0;
	unsigned char		buf[32];

    if(fd<=0)
    {
        return ERROR;
    }
    //pthread_mutex_lock(&spi_mutex);
    retval=read(fd,data,len);
  //  printf("rec spi number=%d\n",retval);
   /* if(retval <= 0)
    {
        //pthread_mutex_unlock(&spi_mutex);
        return ERROR;
    }*/
    //pthread_mutex_unlock(&spi_mutex);
    return retval;
}

/**
 * @chinese
 * 发送数据
 *
 * @param fd 已打开的串口
 * @param data 要发送的数据
 * @param len 数据长度
 *
 * @return 成功-已发送长度，失败-ERROR
 * @endchinese
 *
 * @english
 * send data
 *
 * @param fd spi opened
 * @param data data to send
 * @param len lenth of data
 *
 * @return Sucess-lenth of data sent,fail-ERROR
 * @endenglish
 */
int a20_spi_write_data(int fd, char *data, int len)
{
    int retval;

    if(NULL==data || len<0 || fd<0)
    {
        return ERROR;
    }
    //pthread_mutex_lock(&spi_mutex);
    retval=write(fd, data, len);
    //pthread_mutex_unlock(&spi_mutex);
    if(retval<0)
    {
        printf("spi send data error!\r\n");
        return ERROR;
    }

    return retval;
}

/**
 * 设置spi最大速度
 *
 * @param fd 打开spi文件描述符
 * @param max_speed_ptr 最大速度
 *
 * @return TRUE:            成功
     ERROR:            失败
 */
int a20_spi_set_max_speed(int fd, unsigned long * max_speed_ptr)
{
    int ret;

    if(NULL == max_speed_ptr || fd < 0)
    {
        printf("The params is error in %s\n", __func__);
        return ERROR;
    }

    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, max_speed_ptr);
    if(ret < 0)
    {
        printf("set spi max speed failed in %s\n", __func__);
        return ERROR;
    }

    return SUCCESS;
}

/**
 * 获得spi最大速度
 *
 * @param fd 打开spi文件描述符
 * @param max_speed_ptr 输出最大速度
 *
 * @return TRUE:            成功
     ERROR:            失败
 */
int a20_spi_get_max_speed(int fd, unsigned long * max_speed_ptr)
{
    int ret;

    if(NULL == max_speed_ptr || fd < 0)
    {
        printf("The params is error in %s\n", __func__);
        return ERROR;
    }

    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, max_speed_ptr);
    if(ret < 0)
    {
        printf("get spi max speed failed in %s\n", __func__);
        return ERROR;
    }

    return SUCCESS;
}

/**
 * 设置spi读取模式
 *
 * @param fd 打开spi文件描述符
 * @param mode_ptr 返回设置模式值
 *
 * @return TRUE:            成功
     ERROR:            失败
 */
int a20_spi_set_write_mode(int fd, unsigned char * mode_ptr)
{
    int ret;

    if(fd < 0 || NULL == mode_ptr)
    {
        printf("The params is error in %s\n", __func__);
        return ERROR;
    }

    ret = ioctl(fd, SPI_IOC_WR_MODE, mode_ptr);
    if(ret < 0)
    {
        printf("set write mode failed in %s\n", __func__);
        return ERROR;
    }

    return SUCCESS;
}
/**
 * 设置spi写入模式
 *
 * @param fd 打开spi文件描述符
 * @param mode_ptr 返回设置模式值
 *
 * @return  TRUE:            成功
     ERROR:            失败
 */
int a20_spi_set_read_mode(int fd, unsigned char * mode_ptr)
{
    int ret;

    if(fd < 0 || NULL == mode_ptr)
    {
        printf("The params is error in %s\n", __func__);
        return ERROR;
    }

    ret = ioctl(fd, SPI_IOC_RD_MODE, mode_ptr);
    if(ret < 0)
    {
        printf("set read mode failed in %s\n", __func__);
        return ERROR;
    }

    return SUCCESS;
}



/**
 * 设置SPI通信的字长
 *
 * @param fd 打开spi文件描述符
 * @param bits_ptr 返回设置模式值
 *
 * @return TRUE:            成功
     ERROR:            失败
 */
int a20_spi_set_bits(int fd, unsigned char * bits_ptr)
{
    int ret;

    if(fd < 0 || NULL == bits_ptr)
    {
        printf("The params is error in %s\n", __func__);
        return ERROR;
    }

    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, bits_ptr);
    if(ret < 0)
    {
        printf("set write bits failed in %s\n", __func__);
        return ERROR;
    }

    return SUCCESS;
}
/**
 * 获取SPI通信的字长
 *
 * @param fd 打开spi文件描述符
 * @param bits_ptr 返回设置模式值
 *
 * @return  TRUE:            成功
     ERROR:            失败
 */
int a20_spi_get_bits(int fd, unsigned char * bits_ptr)
{
    int ret;

    if(fd < 0 || NULL == bits_ptr)
    {
        printf("The params is error in %s\n", __func__);
        return ERROR;
    }

    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, bits_ptr);
    if(ret < 0)
    {
        printf("set read bits failed in %s\n", __func__);
        return ERROR;
    }

    return SUCCESS;
}
