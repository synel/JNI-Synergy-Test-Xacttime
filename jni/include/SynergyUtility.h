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
                     GPIO memmap Set
 ==========================================================================*/
 int _memory_setup(uint32_t** gpio);
 int _memory_close(uint32_t** gpio);

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

#ifdef __cplusplus
}
#endif
#endif /* JNI_INCLUDE_SYNERGYUTILITY_H_ */
