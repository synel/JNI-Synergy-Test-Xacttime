#ifndef __SMACKBIOFP_H__
#define __SMACKBIOFP_H__
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/time.h>
#include <unistd.h>
//#include <dlfcn.h>
#include "../public/public.h"
#include "../_precomp.h"

#include "smackbio.h"
#include "supremainc.h"

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned long BOOL;


/************************************************************************/
/* bitmap                                                               */
/************************************************************************/
//===========

typedef struct tagBITMAPFILEHEADER
{
	WORD bfType;
	DWORD bfSize;
	WORD bfReserved1;
	WORD bfReserved2;
	DWORD bfOffBits;
} BITMAPFILEHEADER;//14

typedef struct tagBITMAPINFOHEADER
{
	DWORD biSize;
	long biWidth;
	long biHeight;
	WORD biPlanes;
	WORD biBitCount;
	DWORD biCompression;
	DWORD biSizeImage;
	long biXPelsPerMeter;
	long biYPelsPerMeter;
	DWORD biClrUsed;
	DWORD biClrImportant;
} BITMAPINFOHEADER;//40

typedef struct tagRGBQUAD
{
	BYTE rgbBlue;
	BYTE rgbGreen;
	BYTE rgbRed;
	BYTE rgbReserved;
} RGBQUAD;//4

typedef struct tagBITMAPINFO
{
	BITMAPINFOHEADER bmiHeader;
	RGBQUAD bmiColors[1];
} BITMAPINFO;


/*========================================================================
                    Fingerprint Equipment
==========================================================================*/
typedef struct _fp_INFO
{
	int sensortype;	//sensor type
	int fpmath;		//loading fingerpirnt type,1:1 loading,1;N loading.0-1:N,1-1:1
	int (*performBeforScan)(int ScanNum);
	int (*performAfterScan)(int scanNum);//fingerprint picture show，cal back function
	char sysfinger[64];
	char fingerport[64],fingertype[64],fingerbtl[64],fingerbright[64];
}FPDEVINFO;

extern FPDEVINFO fpdevinfo;
extern char *fingerfilesuffix;
extern  int fp_enable;		//启动是否成功

void write_bitmap(char *imgout, unsigned char *buffer, int nWidth, int nHeight);
long ExposeGet(int nSensorType);
int	SaveTempData( long nID, long nFingerNum ,char *dpath);

int getfingernum();
unsigned char *GetFpDataOne(char *nID,int FingerNum);
int setFpData(char *nID,int FingerNum,unsigned char *str);
long FpDataOneToNMatch(char *FileName);
int LoadFpData(char *nID,int FingerNum,char *FileName);


int InitFp(FPDEVINFO *fpdev_info);
int LoadFingerTemplate(char *nID,char *fpath);

int  Enroll( char *nID ,int FingerNum,char *tpath,char *dpath );

long OneToNMatch(char *tpath);
long OneToOneMatch(char *nID,char *tpath);

int  DeleteFpOne(char *nID, int FingerNum);
int DeleteFPFlash(char *nID, int FingerNum,char *dpath);
int  DeleteFpOneAll(char *nID);
int DeleteFpOneAllFALSE(char *nID,char *dpath);
int  DeleteFpAll(void);

int _get_enroll_count();
FILE * GetFpList();

int UninitFp(void);
#endif
