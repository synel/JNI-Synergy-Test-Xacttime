#ifndef __CONFIG_H
#define __CONFIG_H

/**
 * @chinese
 * @file   config.h
 * @author 胡俊远
 * @date   Thu Oct 13 10:00:00 2011
 *
 * @brief  共用状态变量定义模块
 * @endchinese
 *
 * @english
 * @file   config.h
 * @author Hu Junyuan
 * @date   Thu Oct 13 10:00:00 2011
 *
 * @brief  general variable define module
 * @endenglish
 */

#include "stringchar.h"

#define TRUE	1   /**< 真值：是\r\n TRUE*/
#define FALSE	0   /**< 真值：否\r\n FALSE*/

#define ERROR	-1  /**< 失败\r\n ERROR*/
#define SUCCESS 1   /**< 成功\r\n SUCCESS*/

//#define FEED_WATCHDOG

#ifndef _ARM_A23
    #define WORKPATH ""
#endif
#endif
