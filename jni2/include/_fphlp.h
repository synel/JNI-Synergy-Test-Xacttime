#ifndef __FPHLP_H__
#define __FPHLP_H__

#define	CMOSTYPE_OV7648		0
#define	CMOSTYPE_HV7131R	1
#define	CMOSTYPE_EB6048		2

#define FPDATASIZE 1404

extern int gSensorType;
extern int gRegMax;
extern FPINFO* gMatchData;
/*
 * a char array indicating the validity of a finger print position
 * (index into the gMatchData array that stores the finger print data)
 * gValidFile[pos]=0 means that position's fp data is not valid or empty, ready for enroll new fp data
 * gValidFile[pos]=1 means that position's fp data is valid and the FPINFO data struct is stored at
 * gMatchData[pos].
 *
 */
extern BYTE* gValidFile;
extern FPINFO gFeature;

void hlpErrorPrint(int nError);
long hlpOpen(char* szLibSuffix, int nSensorType);
long hlpClose();

long hlpGetEnrollCount();
long hlpDelete(long nID, long nFingerNum);
long hlpDeleteID(long nID);
long hlpDeleteAll();
long hlpSearchID(long* pnID);
long hlpSearchFingerNum(long nID, long* pnFingerNum);
long hlpCheckID(long nID);
long hlpCheckFingerNum(long nID, long nFingerNum);
long hlpCheckManager(long nID, long nFingerNum);
long hlpCheckManagerID(long nID, long* pnFingerNum);

long hlpCheckAdjustSensor();

long hlpEnrollPrepare(long nID, long nFingerNum, long Manager);
long hlpEnrollEnd(long nID, long nFingerNum, long Manager);

long hlpIdentify(long* pnID, long* pnFingerNum);
long hlpVerify(long nID, long nFingerNum);

long hlpTemplateGetFromImage(void* pTemplate);
long hlpTemplateIdentify(long* pnID, long* pnFingerNum, void* pTemplate);
long hlpTemplateVerify(long nID, long nFingerNum, void* pTemplate);
long hlpGetQualityScore(long* pnScore, long nID, long nFingerNum);

BOOL hlpSaveImageToFile(char* strImageFileName, void* pImage, int cx, int cy);
BOOL hlpLoadTemplateFromFile(char* strTemplFileName, void* pTemplate);
int hlpLoadCompressedTemplateFromFile(char* strCompressedTemplFileName, void* pTemplate);
BOOL hlpSaveTemplateToFile(char* strTemplFileName, void* pTemplate);

int hlpLoadFingersDB(char* fpath);
int hlpGetFpDataOne(long nID,long FingerNum,unsigned char *str);
int hlpSetFpDataOne(long nID, long FingerNum, unsigned char* decodedTemplate);
int hlpLoadFpDataFile(char *nID,long FingerNum,char *FileName);
DWORD * hlpIdIterator();

#endif /*__FPHLP_H__*/
