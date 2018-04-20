#ifndef __FP_H__
#define __FP_H__

/************************************************************************/
/* Copyright (C) 2004-2009 Beijing Smackbio Technology Co., Ltd         */
/* Helper Header File for Engine Library                                */
/* Do not change this file.                                             */
/************************************************************************/

/*========================================================================
                    Function Code
==========================================================================*/
#define		FP_GETVERSION			0
#define		FP_OPEN					1
#define		FP_CLOSE				2
#define		FP_INDEXDBALLOC			3
#define		FP_INDEXDBFREE			4
#define		FP_FPDBALLOC			5
#define		FP_FPDBFREE				6

#define		FP_ENROLLSTART			21
#define		FP_ENROLLNTH256			22
#define		FP_ENROLLNTHFPDATA		23
#define		FP_ENROLLMERGE			24

#define		FP_PROCESSIMAGE256		31
#define		FP_DECOMPRESSFPDATA		32

#define		FP_IDENTIFYIMAGE256		41
#define		FP_IDENTIFYFPDATA		42

#define		FP_VERIFYIMAGE256		51
#define		FP_VERIFYFPDATA			52
#define		FP_VERIFYIMAGE256_2		53
#define		FP_VERIFYFPDATA_2		54
#define		FP_QUALIFY_FPDATA		57

#define		FP_CMOSINIT				61
#define		FP_CMOSCHECKADJUST		62
#define		FP_ISPRESSFINGER		63
#define		FP_CAPTUREFINGER		64
#define		FP_CAPTUREONEFRAME		65

/*========================================================================
                    Error Code
==========================================================================*/
#define FP_ERR_SUCCESS					0
#define FP_ERR_PARAM					-1
#define FP_ERR_NOT_ENROLLED_POS			-2
#define FP_ERR_BAD_FINGER				-3
#define FP_ERR_MERGE					-4
#define FP_ERR_IDENTIFY					-5
#define FP_ERR_VERIFY					-6
#define FP_ERR_SENSOR					-7
#define FP_ERR_NOT_PRESSED				-8
#define DEV_ERR							-10

/*========================================================================
                    Fingerprint Template
==========================================================================*/
typedef struct _tag_FPINFO
{
	unsigned int    ID;		//unsigned long   lParam;
	unsigned char   Manager;//unsigned short  wParam;
	unsigned char   FingerNum;
	unsigned char   Valid;
	unsigned char   Reserved[1397];
} FPINFO, *P_FPINFO;// all size = 1404byte

/*========================================================================
                    for developers
==========================================================================*/
typedef struct _tag_engcmos
{
	int		left; //left start
	int		top; //top start
	int		br_first; //initial brightness
	int		br_last; //normal brightness
} engcmos;

extern void*		engTemp;
extern int			engTempSize;
extern engcmos*		engCmosInitResult;
extern void*		engImage256;
extern void*		engFirstImage;

#define SB_FP__FIRSTIMAGE	engFirstImage
#define SB_FP__LIVEIMAGE	engTemp
#define SB_FP__256IMAGE		engImage256

extern int engLastError;
void SB_FP__PRINTLASTERROR();

/*========================================================================
                    Helper Functions
==========================================================================*/
typedef int (*pfnENGINE)(int FuncNo, long Param1, long Param2, long Param3, long Param4, long Param5);
extern pfnENGINE fnSB_fp;

extern DWORD gFpReleaseDate;
extern DWORD gFpMaximum;
extern DWORD gFpVersion;

int SB_fp(int FuncNo, long Param1, long Param2, long Param3, long Param4, long Param5);

#define IN
#define OUT
#define INOUT
#define OPTIONAL

//////////////////////////////////////////////////////////////////////////
DWORD	SB_FP_GETVERSION(OUT OPTIONAL DWORD *pdwReleaseDate, OUT OPTIONAL DWORD *pdwLibCapacity);
int		SB_FP_OPEN(IN int nSensorType, OUT OPTIONAL BYTE **pIndexDB, OUT OPTIONAL BYTE **pFingerprintDB);
int		SB_FP_CLOSE();
BYTE*	SB_FP_INDEXDBALLOC();
void	SB_FP_INDEXDBFREE();
FPINFO*	SB_FP_FPDBALLOC();
void	SB_FP_FPDBFREE();
int		SB_FP_ENROLLSTART();
int		SB_FP_ENROLLNTH256(IN int nStep);
int		SB_FP_ENROLLNTHFPDATA(IN FPINFO *pTemplate, IN int nStep);
int		SB_FP_ENROLLMERGE(OUT FPINFO *pTemplate);
int		SB_FP_PROCESSIMAGE256(OUT FPINFO *pTemplate);
int		SB_FP_DECOMPRESSFPDATA(IN void* pCompressed, IN int nCompressedLen, OUT FPINFO *pTemplate);
int		SB_FP_IDENTIFYIMAGE256(OUT OPTIONAL BOOL *pbAdapted);
int		SB_FP_IDENTIFYFPDATA(IN FPINFO *pTemplate, OUT OPTIONAL BOOL *pbAdapted);
int		SB_FP_VERIFYIMAGE256(IN int nPos, OUT OPTIONAL BOOL *pbAdapted);
int		SB_FP_VERIFYFPDATA(IN FPINFO *pTemplate, IN int nPos, OUT OPTIONAL BOOL *pbAdapted);
int		SB_FP_VERIFYIMAGE256_2(IN FPINFO *pTemplate);
int		SB_FP_VERIFYFPDATA_2(IN FPINFO *pTemplate, IN FPINFO *pTemplateReg);
int		SB_FP_GET_QUALITY(IN FPINFO *pTemplate);

//////////////////////////////////////////////////////////////////////////
int		SB_FP_CMOSINIT(IN DWORD dwMechanical, IN DWORD dwExpose);
int		SB_FP_CMOSCHECKADJUST(OUT OPTIONAL DWORD* pdwMechanical, OUT OPTIONAL DWORD* pdwExpose);
int		SB_FP_ISPRESSFINGER();
int		SB_FP_CAPTUREFINGER();
int		SB_FP_CAPTUREONEFRAME();

#endif /*__FP_H__*/
