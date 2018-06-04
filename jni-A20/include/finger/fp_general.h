#ifndef _FP_GENERAL_H_
#define _FP_GENERAL_H_
#include "_precomp.h"
#define	FPDATASIZE		1404

typedef	int (*performBeforScan)(int ScanNum); /**< 回调函数,传给当前录入的次数，可以用来控制播放提示声音 */
typedef	int (*performAfterScan)(int scanNum); /**<回调函数,传给当前录入的次数，可以用来控制显示指纹图像  */

int _save_template(int nID, int nFingerNum, void* gFeature);

#endif
