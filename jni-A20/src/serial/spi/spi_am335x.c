/*
 * spi_am335x.c
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

#include "spi_am335x.h"
#include "config.h"
#include "debug.h"

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
int am335x_spi_open()
{
    int fd = 0;
    unsigned char mode = 0,lsb;
    unsigned long baudrate_spi = 200000;
    unsigned char bits = 8;

    fd=open("/dev/spidev1.0", O_RDWR);

    if(fd<0)
    {
        printf("open spi error!\r\n");
        return ERROR;
    }

    am335x_spi_set_bits(fd, &bits);
    am335x_spi_set_write_mode(fd, &mode);
    am335x_spi_set_max_speed(fd, &baudrate_spi);
    am335x_spi_get_bits(fd, &bits);
    am335x_spi_set_read_mode(fd, &mode);
    am335x_spi_get_max_speed(fd, &baudrate_spi);
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
int am335x_spi_close(int fd)
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
int am335x_spi_recv_data(int fd, char *data, int len)
{
    char tx[1];
    //    char rx[1] = {0};
    int ret=0;
    struct spi_ioc_transfer tr;

    if(data == NULL || len<0)
        return ERROR;

    //pthread_mutex_lock(&spi_mutex);
    tr.tx_buf = (unsigned long)tx;
    tr.rx_buf = (unsigned long)data;
    tr.len = sizeof(tx);
    tr.delay_usecs = 0;
    tr.speed_hz = 200000;
    tr.bits_per_word = 8;
    ret=ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if(ret<1 || data == NULL)
    {
        ret=am335x_spi_read_data(fd, data, len);
        if(ret<1 || data == NULL)
        {
            //pthread_mutex_unlock(&spi_mutex);
            return ERROR;
        }
    }
    //pthread_mutex_unlock(&spi_mutex);//lxy
    //    for (ret = 0; ret<sizeof(tx); ret++)
    //    {
    //        data[ret]=rx[ret];
    //    }

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
int am335x_spi_send_data(int fd, char *data, int len)
{
    //    char tx[1];
    char rx[1] = {0};
    int ret=0;
    struct spi_ioc_transfer tr;

    if(data == NULL || len<0)
        return ERROR;

    //pthread_mutex_lock(&spi_mutex);
    tr.tx_buf = (unsigned long)data;
    tr.rx_buf = (unsigned long)rx;
    tr.len = len;
    tr.delay_usecs = 0;
    tr.speed_hz = 100000;
    tr.bits_per_word = 8;


    ret=ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if(ret<1 || strlen(data)<=0)
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
int am335x_spi_read_data(int fd, char *data, int len)
{
    int retval=0;
	unsigned char		buf[32];
    int num=0,i=0;

    if(fd<=0)
    {
        return ERROR;
    }
    //pthread_mutex_lock(&spi_mutex);
    for(i=0;i<len;i++)
    {
        usleep(60);
        num=read(fd,data+retval,1);
        if(num<=0)
            break;
        else retval+=1;
    }

    //retval=read(fd,data,len);
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
int am335x_spi_write_data(int fd, char *data, int len)
{
    int retval=0;
    int num=0,i=0;

    if(NULL==data || len<0 || fd<0)
    {
        return ERROR;
    }
    //pthread_mutex_lock(&spi_mutex);
    for(i=0;i<len;i++)
    {
        usleep(60);
        num=write(fd,data+retval,1);
        if(num<=0)
            break;
        else retval+=1;
    }
    //retval=write(fd, data, len);
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
int am335x_spi_set_max_speed(int fd, unsigned long * max_speed_ptr)
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
int am335x_spi_get_max_speed(int fd, unsigned long * max_speed_ptr)
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
int am335x_spi_set_write_mode(int fd, unsigned char * mode_ptr)
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
int am335x_spi_set_read_mode(int fd, unsigned char * mode_ptr)
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
int am335x_spi_set_bits(int fd, unsigned char * bits_ptr)
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
int am335x_spi_get_bits(int fd, unsigned char * bits_ptr)
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
