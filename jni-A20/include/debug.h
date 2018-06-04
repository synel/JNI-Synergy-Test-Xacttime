#ifndef __DEBUG_H
#define __DEBUG_H

/**
 * @chinese
 * @file   debug.h
 * @author 刘训
 * @date   Thu Oct 13 10:00:00 2011
 *
 * @brief  调试
 * @endchinese
 *
 * @english
 * @file   debug.h
 * @author lx
 * @date   Thu Oct 13 10:00:00 2011
 *
 * @brief  general variable define module
 * @endenglish
 */


//#undef DEBUG
/**是否开启打印语句*/
#define DEBUG
#ifdef DEBUG
//#include "../memwatch-2.71/memwatch.h"
    //#ifdef _ARM_A23
    //    #include "android/wedsa23.h"
//    #define plog LOGI
    //#else
        #define plog printf
    //#endif
#else
    #define plog printf_none
    //#define plog(format,args...) fprintf(stderr,format,args)
#endif

#endif

