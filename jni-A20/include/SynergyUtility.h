/*
 * SynergyUtility.h
 *
 *  Created on: Nov 5, 2014
 *      Author: chaol
 */

#ifndef JNI_INCLUDE_SYNERGYUTILITY_H_
#define JNI_INCLUDE_SYNERGYUTILITY_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

 /*========================================================================
                     number to string conversion of any base
 ==========================================================================*/
 char* _ultostr(unsigned long num, char* str, int base);

 /*========================================================================
                     timing utility
 ==========================================================================*/
 void _tick_start();

 int _tick_end();

 void _tick_print(char* szPrompt, int nTime);

 void _initGpio();

extern int init_gpio_devices(long con_value,int def_value);
extern int init_gpio_value(int gpio_type,int pin_num,int value);
extern int set_gpio_value(int pin_num,int value);

#ifdef __cplusplus
}
#endif
#endif /* JNI_INCLUDE_SYNERGYUTILITY_H_ */
