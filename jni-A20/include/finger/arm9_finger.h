/**
 * @file   arm9_finger.h
 * @author 刘训
 * @date   Wed Jul 13 09:41:03 2011
 *
 * @brief
 *
 *
 */
#ifndef __ARM9_FINGER_H__
#define __ARM9_FINGER_H__

#include "public.h"
#include "_precomp.h"
#include "_fp.h"
#define	FPDATASIZE		1404
extern int fp_enable;
extern FPINFO* gMatchData;
extern BYTE* gValidFile;
static int PROBEMODE=0;     //指纹传感器扩展探测模式 ，0 无此功能，1有此功能 有扩展探测功能时，先使用次功能，获取图像。
int _get_enroll_count();
void _delete_all();
void _delete(int pos);
int _get_empty_pos();
int _get_pos(long nID, long nFingerNum);
int _get_posID(long nID);
long hlpEnrollPrepare(long nID, long nFingerNum, long Manager);
void _settings_get(int nSensorType, DWORD* pdwMechanical, DWORD* pdwExpose);
int _save_template(int nID, int nFingerNum);
long hlpEnrollEnd(long nID, long nFingerNum, long Manager);

void _tick_stat();
int _tick_end();

BOOL _capture_finger(int nMilliSecondTimeout);
long hlpIdentify(long* pnID, long* pnFingerNum);
BOOL hlpSaveImageToFile(char* strImageFileName, void* pImage, int nHeight, int nWidth);
void hlpErrorPrint(int nError);

/************************************************************************/
/* Interfaces                                                           */
/************************************************************************/
typedef	int (*performBeforScan)(int ScanNum); /**< 回调函数,传给当前录入的次数，可以用来控制播放提示声音 */
typedef	int (*performAfterScan)(int scanNum); /**<回调函数,传给当前录入的次数，可以用来控制显示指纹图像  */

int InitFp(int sensortype,char *fp_path);
int LoadAllFinger(char *fpath);
int LoadFingerTemplate(char *nID,char *finger_argv,char *fpath);
int LoadFpData(char *nID,int FingerNum,char *FileName);
//int Enroll( char * nID ,int FingerNum,char *tpath,char *dpath);
int Enroll( char * nID ,int FingerNum,char *tpath,char *dpath,
				performBeforScan formBeforScan,performAfterScan formAfterScan );
long OneToNMatch(char *tpath);
long OneToOneMatch(char *nID,int FingerNum,char *tpath);
int hlpOneToOneMatch(char *nID,char *finger_argv,char *fppath,char *tpath);
int DeleteFpOne(char *nID, int FingerNum);
int hlpDeleteFlash(char* nID, int FingerNum,char *dpath);
int hlpDeleteFpIdFlash(char* nID,char *dpath);
long hlpDelete(char* nID, long nFingerNum);
int hlpDeleteID(char* nID);
int hlpDeleteAll(void);
int hlpClose();
int hlpGetRegMax();
int hlpGetEnrollCount();
int load_all_finger_by_wdda(char *filename, char *dir, int id_index, int finger_index);
unsigned char *GetFpDataOne(char *nID,int FingerNum);
FILE * GetFpList();
int ImageToTemplate(char *path);
void ImageTemplateMode(int flag);
BOOL valid_capture_finger(int nMilliSecondTimeout);
int finger_one_to_one_match(int );
int read_s70_finger(int port_number);

int finger_probe();
int SetFingerProbeMode(int mode);

#define CMOSTYPE_OP5 0x33

#endif
