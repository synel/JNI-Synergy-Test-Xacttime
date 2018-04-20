/**
 * @chinese
 * @file   spi_s3c2410.c
 * @author 刘训
 * @date   Wed Jul 13 10:00:13 2011
 *
 * @brief  SPI操作接口模块
 * @endchinese
 *
 *
 * @english
 * @file   spi_s3c2410.c
 * @author liu xun
 * @date   Wed Jul 13 10:00:13 2011
 *
 * @brief  SPI module
 * @endenglish
 *
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

#include "spi_s3c2410.h"
#include "spilib.h"

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
int s3c2410_spi_open()
{
    int fd = 0;

    fd=open("/dev/spidev1.0", O_RDWR);

    if(fd<0)
    {
        plog("open spi error!\r\n");
        return ERROR;
    }

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
int s3c2410_spi_close(int fd)
{
    if(fd<0)
        return ERROR;

    if(close(fd)<0)
    {
        plog("close spi error!\r\n");
        return ERROR;
    }

    return OK;
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
int s3c2410_spi_recv_data(int fd, char *data, int len)
{
    char tx[1];
    char rx[1] = {0};
    int ret=0;

    if(data == NULL || len<0)
        return ERROR;

    struct spi_ioc_transfer tr=
    {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)data,
        .len = sizeof(tx),
        .delay_usecs = 0,
        .speed_hz = 1000000,
        .bits_per_word = 8,
    };

    ret=ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if(ret<1 || strlen(data)<=0)
    {
        ret=ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
        if(ret<1 || strlen(data) <= 0)
            return ERROR;
    }

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
int s3c2410_spi_send_data(int fd, char *data, int len)
{
    int retval;

    if(NULL==data || len<0 || fd<0)
    {
        return ERROR;
    }

    retval=write(fd, data, len);

    if(retval<0)
    {
        plog("spi send data error!\r\n");
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
int s3c2410_spi_set_max_speed(int fd, unsigned long * max_speed_ptr)
{
    int ret;

    if(NULL == max_speed_ptr || fd < 0)
    {
        plog("The params is error in %s\n", __func__);
        return ERROR;
    }

    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, max_speed_ptr);
    if(ret < 0)
    {
        plog("set spi max speed failed in %s\n", __func__);
        return ERROR;
    }

    return OK;
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
int s3c2410_spi_get_max_speed(int fd, unsigned long * max_speed_ptr)
{
    int ret;

    if(NULL == max_speed_ptr || fd < 0)
    {
        plog("The params is error in %s\n", __func__);
        return ERROR;
    }

    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, max_speed_ptr);
    if(ret < 0)
    {
        plog("get spi max speed failed in %s\n", __func__);
        return ERROR;
    }

    return OK;
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
int s3c2410_spi_set_write_mode(int fd, unsigned char * mode_ptr)
{
    int ret;

    if(fd < 0 || NULL == mode_ptr)
    {
        plog("The params is error in %s\n", __func__);
        return ERROR;
    }

    ret = ioctl(fd, SPI_IOC_WR_MODE, mode_ptr);
    if(ret < 0)
    {
        plog("set write mode failed in %s\n", __func__);
        return ERROR;
    }

    return OK;
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
int s3c2410_spi_set_read_mode(int fd, unsigned char * mode_ptr)
{
    int ret;

    if(fd < 0 || NULL == mode_ptr)
    {
        plog("The params is error in %s\n", __func__);
        return ERROR;
    }

    ret = ioctl(fd, SPI_IOC_RD_MODE, mode_ptr);
    if(ret < 0)
    {
        plog("set read mode failed in %s\n", __func__);
        return ERROR;
    }

    return OK;
}

