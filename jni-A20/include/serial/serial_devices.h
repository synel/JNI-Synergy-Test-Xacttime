#ifndef SERIAL_DEVICES_H
#define SERIAL_DEVICES_H


#include <pthread.h>
#include "serial.h"


extern TDATA synel_tdata;

/*卡头类型*/
typedef enum __SERIAL_DEVICES_MODE {
    UART_TYPE,
    SPI_TYPE,
    M300,
    XDF_CARD,
    RFID_0100D,   //电信
    RFID_0201A,   //电信 只读序列号
    RFID_YDGM,    //移动 国民技术 读头
    RFID_24G,	//移动2.4G卡头
    LV_1000,    //lv条码卡
    LV_3000,
    TEXT_CARD,
    VENA_DEVICE,
    RFID_ZKXL    //联通 中科讯联 读头
} TSERIAL_DEVICES_MODE;

/*终端机类型*/
typedef enum __TERMINAL_MODE {
    HANDSET_TYPE=0, //手持机
    I_TYPE,   //I型机器
    K_TYPE,  //K型机器
    F_TYPE, //F型机器
    H_TYPE, //H型机器
    S_TYPE, //S型机器
    V_TYPE, //F型机器
    C_TYPE, //消费C型机器
    MAC_TERMINAL_TYPE
} TTERMINAL_MODE;


extern pthread_mutex_t serial_mutex;


struct serial_devices{
    char devices_type[24];    //串行口接的设备类型
    int serial_port;
    struct serial_devices *next;
};
union serial_model{
	int model_name;//终端类型（0-手持机，1-I型机，2-K型机）
};
extern struct serial_devices *head_serial_devices;
extern int terminal_type;
typedef enum __CARDPLATFROM{
    MA805=0X01,
    STM32F030=0X02,
    STM32F103=0X03,
    UBOOTASK=0X06
}CARDPLATFROM;

typedef enum __CARDDEVICESTYPE{
    STANDARDIC=0X01,
    STANDARDID=0X02,
    STANDARDF=0X03,
    STANDARDH=0X04,
    STANDARDK=0X05,
    STANDARDI=0X06,
    STANDARDV=0X07,
    STANDARDPOS=0X08,
    STANDARDHANDSET=0X09,
}CARDDEVICESTYPE;
int add_devices_stack(char *devices_type,int port);
int init_uart_devices(int uart_port, int baudrate,char *devices_type);
int init_spi_devices(int spi_port,char *devices_type);
int read_devices_data(char *devices_type,char *value,char* value_type);
int set_keypad_backlight_time(char *devices_type, int value);
int send_close_machine_info(char *devices_type, int value);
int send_reboot_machine_info(char *devices_type, int value);
int get_uart_devices_version(int uart_port,char *out_value);
int send_close_battery_info(char *devices_type);
int get_spi_devices_version(int spi_port,char *out_value);
int init_sys_model(int model);
int ask_power_request();

//设置logo背光时间
int set_logo_backlight_time( int value);
int set_logo_backlight_time_V( int value );

//获取WEDS——WG 卡号
//内部测试用
int get_uart_wg_data(int uart_port,char *out_value);
int get_spi_battery(int spi_port,char *out_value);




int check_mcu_update(void);
int synel_update_mcu(int uartPort,char *file,int mode);



typedef enum __UPDATE_MCU_MODE {
    auto_update=0,
    active_update
}UPDATE_MCU_MODE;

//置位后，关闭输入有源
extern int enable_input_source;

#endif // SERIAL_DEVICES_H
