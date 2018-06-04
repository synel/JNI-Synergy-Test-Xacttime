/*
 * gpio_am335x.c
 *
 *  Created on: 2014-8-8
 *      Author: aduo
 */


#include <assert.h>
#include "gpio_am335x.h"
#include "config.h"
#include "debug.h"
#include "_precomp.h"

/**
 * @chinese
 * 初始化gpio设备
 *
 * @return 成功-TRUE,失败-FALSE
 * @endchinese
 *
 * @english
 * initialize gpio device
 *
 * @return success-TRUE,fail-FALSE
 * @endenglish
 *
 *
 */
int gpio_init_am335x()
{
#ifdef _A20_UBUNTU
    int fd=1;
#else
    int fd=-1;
    char *path = "/dev/gpio_am335x";

    if(access(path,F_OK) == -1)
    {
        perror("open GPIO device error!\r\n");
        return -1;
    }
    if((fd = open(path, O_RDWR)) == -1)
    {
        perror("open GPIO device error!\r\n");
        return -1;
    }
#endif
    return fd;
}

/**
 * @chinese
 * 设置gpio位使能
 *
 * @param value GPIO管脚使能使电平
 * @param num  GPIO管脚控制定义值
 *
 * @return 成功-TRUE,失败-FALSE
 * @endchinese
 *
 * @english
 * set gpio enable
 *
 * @param value gpio enable
 * @param num  defining value of controlling gpio pin
 *
 * @return success-TRUE,fail-FALSE
 * @endenglish
 *
 */
int gpio_write_am335x(int fd, int pin ,int value)
{
    char buf[2] = {0};
//    printf("aaaaaaaaaaaaa\n");
#ifdef _A20_UBUNTU    
    int fp=-1;
    char path[128] ;
    static char config[20]={0,0,2,3,4,5,6,7,0,9,10,11,12,13,14,15,0,0,0,0};

    if(pin==9)
        return 0;
    if(pin>=20||pin<0)
        return -1;
    //配置GPIO口的方向，有些已经在启动时配置了，不需要在这里再配置一遍。
    if(config[pin])
    {
        config[pin]=0;
	sprintf(path,"bash /arm/lib/gpio/gpio%d.sh",pin);	// must use bash 20170601
	printf("gpio shell cmd = %s\n", path);		
        chmod(path,0x00777);
        system(path);
    }

    memset(path,0,sizeof(path));
    sprintf(path,"/dev/gpio/out%d",pin);
    if(access(path,F_OK) == -1)
    {
//        printf("open GPIO device error:%s!\r\n",path);
        return -1;
    }
//    printf("********open GPIO device error:%s,%d!\r\n",path,value);

    if((fp = open(path, O_RDWR)) == -1)
    {
        perror("open GPIO device error!\r\n");
        return -1;
    }
    if(fp <= 0)
    {
        return -1;
    }
//printf("-----------------gpio_write_s3c2416:%d,%d\n",pin,value);
    memset(buf,0,sizeof(buf));    
//    buf[0] = value;
    sprintf(buf,"%d",value);
    if (write(fp, buf, 1) < 0)
    {
        plog("gpio write error!\n");
        close(fp);
        return -1;
    }
    close(fp);
#else
    int para2,ret;

    if(fd <= 0)
    {
        return -1;
    }

    if (pin < 0 || pin > 11){
    	return -1;
    }
    para2 = 0xaa00;
    para2|=value;

    ret=ioctl(fd,pin | 0x8000,para2);
#endif
    return 1;
}

/**
 * @chinese
 * 获得GPIO管脚定义的输入点的状态
 * @param gpio_enable GPIO管脚使能使电平
 * @param num GPIO管脚控制定义值
 *
 * @return Success 状态值, Failure -1
 * @endchinese
 *
 * @english
 * get gpio state
 *
 * @param gpio_enable GPIO enable level
 * @param num GPIO pin num
 * @return Success state, Failure -1
 * @endenglish
 *
 *
 */
int gpio_read_am335x(int fd, int pin)
{
    char gpio_value[2] = {0};
#ifdef _A20_UBUNTU    
    int fp=-1;
    char path[128] ;

    memset(path,0,sizeof(path));
    sprintf(path,"/dev/gpio/in%d",pin);
    if(access(path,F_OK) == -1)
    {
       //printf("open GPIO device error:%s!\r\n",path);
        return -1;
    }
    if((fp = open(path, O_RDWR)) == -1)
    {
        //printf("open GPIO device error!\r\n");
        return -1;
    }


    if (read(fp, gpio_value, 1) < 0)
    {
        //printf("gpio read error!\r\n");
        close(fp);
        return -1;
    }
    close(fp);
    //printf("%s,%d\n",__func__,gpio_value[0]-'0');
    return gpio_value[0]-'0';
#else
    int para2,ret;

    if(fd<=0)
    {
        return -1;
    }

    if (pin < 0 || pin > 2){
    	return -1;
    }

    para2 = 0x5500;

    ret=ioctl(fd,pin | 0x8000,para2);
    return ret;
#endif

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
int gpio_close_am335x(int fd)
{
#ifdef _A20_UBUNTU
#else
    if(fd > 0 && close(fd) == -1)
    {
        perror("close gpio");
        return FALSE;
    }
#endif

    return TRUE;
}

