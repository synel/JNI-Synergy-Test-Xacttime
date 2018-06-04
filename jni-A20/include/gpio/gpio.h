/**
 * @file   gpio.h
 * @author lish
 * @date   Thu Jul  7 16:48:08 2011
 *
 * @brief
 *
 *
 */
#ifndef __GPIO_INTERFACE_H
#define __GPIO_INTERFACE_H
#include <pthread.h>
#include <signal.h>

#include "_precomp.h"

extern int udisk_using;

/****************************
以色列机器gpio驱动：pin define:

7	128	red-led
6	64	green-led
5	32	lcd-backlight
4	16	input-point
3	8	relay
2	4	camera
1	2	NC
0	1	NC
*****************************/

/** 枚举常量 */
enum {
    INPUT_1 = 0,	/**<输入点1  */
    INPUT_2,		/**<输入点2  */
    INPUT_3,		/**<输入点3  */
    OUT_1,		/**<输出点1  */
    OUT_2,		/**<输出点2  */
    RED_LED,		/**<红灯  */
    GREEN_LED,		/**<绿灯  */
    LCD,		/**<LCD设备  */
    CAMERA,		/**<摄像头设备  */
    GPRS_POWER,		/**<GPRS_POWER  用于模块开关机*/
    GPRS_RESET,		/**<GPRS_RESET  */
    PRINTER_POWER,	/**<打印机开启/关闭  */
    BARCODE_POWER,	/**<条码开启/关闭  */
    USB_POWER,  	/**<USB电源控制  1-外置卡 0-内置口*/
    GPRS_POWER_SUPPLY,	/**<GPRS 电源控制 用于模块电源开关*/
    WIFI_POWER,         /*内置WIFI 模块电源控制口*/
    SIDE_USB_POWER,
    BOTTOM_USB_POWER,
    SD_POWER,
    FP_LIGHT,
    TOUCH_POWER ,
    FINGER_POWER,           //指纹背光
    FINGER_PROBE,           //指纹检测   
    GPIO_MAX
};


// GPO的动作或状态
enum {
    GPO_None = 0,
    GPO_On,
    GPO_Off,
    GPO_Pulse,
    GPO_State_Count
};

// GPIO 设置值和运行状态
typedef struct
{
    int activate; // 激活状态 1-激活 0-未激活
    int value;    // 打开值
    int pin_num;  // 管脚值
    volatile int current_state; // 当前状态
    volatile int operation;  // 执行操作
    volatile int operation_time;  // 执行时间(毫秒)
    volatile int interval; // 脉冲间隔(毫秒)
} Gpio_pin_info;

extern int gpio_fd;

int init_gpio_devices(long con_value,int def_value);
int init_gpio_value(int gpio_type,int pin_num,int value);
int set_gpio_value(int pin_num,int value);
int turn_on_gpio(int sec,int devices_type);
int turn_off_gpio(int sec,int devices_type);
int pulse_gpio(int sec,int interval,int devices_type);
int option_gpio(char opt, int sec, int devices_type);
int option_gpio_pulse(int sec,int interval,int devices_type);
int get_import_state(int devices_type);
int close_gpio(void);
int GPO_OnOff(int IO_Port, int ll_switch);
int gprs_on();
int gprs_off();
int camera_off();
int camera_on();
int printer_on();
int printer_off();
int gps_off();
int gps_on();
int Lv1000_on();
int Lv1000_off();
int set_gpio_on(int devices_type);
int set_gpio_off(int devices_type);
void set_gpio_thread_pause(int time);
#endif
