#ifndef __SERIAL_ASYNC_H
#define __SERIAL_ASYNC_H
#include <pthread.h>
#include "serial.h"
#define DEVICE_DATA_ARRAY 10      //串口缓冲区应用

//
int (*serial_async_func[256])(int com, TDATA* data);


struct virtual_device_data{                     //虚拟设备
    int num,over;
    int head,end;                               //循环缓存区头尾指针。
    unsigned char instruction;                  //指令，用于区别虚拟设备类型。
    TDATA device_array[DEVICE_DATA_ARRAY];      //虚拟设备缓存数据
    struct virtual_device_data *next;
};


struct _serial_data {
    TCOM com;                                       //串口号
    pthread_mutex_t serial_lock;                    //串口操作线程锁
    pthread_mutex_t serial_data_lock;               //串口数据操作线程锁
    struct virtual_device_data *device_data;        //每个串口支持的虚拟设备，
};


struct _serial_data serial_data_array[MAXCOM];            //串口数据


void * serial_monitor();                    //



int device_recv_data_async( int port, char *value );
//初始化串口的操作信息。
int serial_init_async( int com, int baudrate, int workmode, int address );
int serial_package_processor_async( TCOM com, TDATA *tdata, ACTION action );

int set_async_func(unsigned char instruction,int (*serial_async_fun)(int com, TDATA*data));


#endif
