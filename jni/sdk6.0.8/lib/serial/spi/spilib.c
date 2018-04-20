/**
 * @chinese
 * @file   spilib.c
 * @author 胡俊远
 * @date   Wed Jul 13 10:00:13 2011
 *
 * @brief  SPI操作接口模块
 * @endchinese
 *
 *
 * @english
 * @file   spilib.c
 * @author Hu Junyuan
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
//#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <unistd.h>
#include <stdint.h>

#include "../../public/public.h"
#include "spilib.h"

#if defined _2410
#include "spi_s3c2410.h"
#elif defined _2416

#include "spi_s3c2416.h"
#endif

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
int spi_open(int port, int rate)
{
    int fd = 0;

#if defined __2410
    fd = s3c2410_spi_open();
#elif defined _2416
    fd = s3c2416_spi_open();
#endif

    if(fd<0)
    {
        //plog("spi open error\n");
        return ERROR;
    }
    serial_fd[port] = fd;
    spi_clear(port);
    /*
    serial_set_work_mode(port,workmode);

    if((address <= 0) || (address >= 255))
    {
        address=1;
    }
    serial_set_address(port,address);
*/
    return OK;
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
int spi_close(int port)
{
    int retval = 0;

#if defined _2410
    retval = s3c2410_spi_close(serial_fd[port]);
#elif defined _2416
    retval = s3c2416_spi_close(serial_fd[port]);
#endif

    if(retval<0)
        return ERROR;

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
int spi_recv_data(int port, char *data, int len)
{
    int length=0;
#if defined __2410
    length = s3c2410_spi_recv_data(serial_fd[port],data,len);
#elif defined _2416
    length = s3c2416_spi_recv_data(serial_fd[port],data,len);
#endif
   //printf("length:%d\n",length);
    return length;
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
int spi_send_data(int port, char *data, int len)
{
    int retval;

    if(NULL==data || len<0 || serial_fd[port]<0)
    {
        return ERROR;
    }
#if defined _2416
    retval = s3c2416_spi_write_data(serial_fd[port],data,len);
#endif
//    retval=write(fd, data, len);

    if(retval<0)
    {
        //plog("spi send data error!\r\n");
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
int spi_set_max_speed(int port, unsigned long * max_speed_ptr)
{
    int ret;

    if(NULL == max_speed_ptr || serial_fd[port] < 0)
    {
        //plog("The params is error in %s\n", __func__);
        return ERROR;
    }

    ret = ioctl(serial_fd[port], SPI_IOC_WR_MAX_SPEED_HZ, max_speed_ptr);
    if(ret < 0)
    {
        //plog("set spi max speed failed in %s\n", __func__);
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
int spi_get_max_speed(int port, unsigned long * max_speed_ptr)
{
    int ret;

    if(NULL == max_speed_ptr || serial_fd[port] < 0)
    {
        //plog("The params is error in %s\n", __func__);
        return ERROR;
    }

    ret = ioctl(serial_fd[port], SPI_IOC_RD_MAX_SPEED_HZ, max_speed_ptr);
    if(ret < 0)
    {
        //plog("get spi max speed failed in %s\n", __func__);
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
int spi_set_write_mode(int port, unsigned char * mode_ptr)
{
    int ret;

    if(serial_fd[port] < 0 || NULL == mode_ptr)
    {
        //plog("The params is error in %s\n", __func__);
        return ERROR;
    }

    ret = ioctl(serial_fd[port], SPI_IOC_WR_MODE, mode_ptr);
    if(ret < 0)
    {
        //plog("set write mode failed in %s\n", __func__);
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
int spi_set_read_mode(int port, unsigned char * mode_ptr)
{
    int ret;

    if(serial_fd[port] < 0 || NULL == mode_ptr)
    {
        //plog("The params is error in %s\n", __func__);
        return ERROR;
    }

    ret = ioctl(serial_fd[port], SPI_IOC_RD_MODE, mode_ptr);
    if(ret < 0)
    {
        //plog("set read mode failed in %s\n", __func__);
        return ERROR;
    }

    return OK;
}

/**
 * @chinese
 * 清空spi设备
 * 连续获得10次"00"字节，表示已经清空完毕
 *
 * @param fd 打开文件描述符
 *
 * @return 成功-SUCCESS，失败-ERROR
 * @endchinese
 *
 * @english
 * clear spi device
 *
 * @param fd open file descirptor
 *
 * @return Success-SUCCESS，Fail-ERROR
 * @endenglish
 *
 */
int spi_clear(int port)
{
    char byte;
    int len=0,i=0;
    int num=0;

    if(serial_fd[port]<0)
        return ERROR;
    while(1)
    {
        /*最多读取512字节，防止出现卡死现象*/
        num++;
        if(num>512)
            break;

        len=spi_recv_data(port,&byte,1);
        if(byte==0)
        {
            i++;
        }
        else
        {
            i=0;
        }

        if(i>10)
        {
            return OK;
        }
    }
    return OK;
}
