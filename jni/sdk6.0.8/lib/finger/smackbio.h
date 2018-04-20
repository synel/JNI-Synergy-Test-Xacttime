/*
 * smackbio.h
 *
 *  Created on: 2014-5-6
 *      Author: aduo
 */

#ifndef SMACKBIO_H_
#define SMACKBIO_H_

#include "finger.h"

#define	FPDATASIZE		1404

//#define	KIT_REG_MAX		10000

// Function Code
#define		FP_GETVERSION		0	//Get version and some license information.
#define		FP_OPEN			1	//Initialize the engine.
#define		FP_CLOSE			2	//Terminate the engine.
#define		FP_INDEXDBALLOC		3	//Allocate index database.
#define		FP_INDEXDBFREE		4	//Free index database.
#define		FP_FPDBALLOC			5	//Allocate template database.
#define		FP_FPDBFREE			6	//Free template database.

#define		FP_ENROLLSTART		21	//Start an enrollment.
#define		FP_ENROLLNTH256		22	//Process a fingerprint image for an enrollment.
#define		FP_ENROLLNTHFPDATA		23	//Process a fingerprint image for an enrollment.
#define		FP_ENROLLMERGE		24	//Merge results of three calls of FP_ENROLLNTH* and generate a template for an enrollment.
#define		FP_PROCESSIMAGE256		31	//Process a fingerprint image and generate a template, should only use for matching (transmission), not for enrollment.
#define		FP_IDENTIFYIMAGE256		41	//Identify a fingerprint image from the database (1: N).
#define		FP_IDENTIFYFPDATA		42	//Identify a template from the database (1: N).

#define		FP_VERIFYIMAGE256		51	//Verify a fingerprint image with an enrolled template (1: 1).
#define		FP_VERIFYFPDATA		52	//Verify a template with an enrolled template (1: 1).
#define		FP_CMOSINIT			61	//Initialize (or reset) the sensor.
#define		FP_CMOSCHECKADJUST		62	//Check and adjust the consistency of the sensor.
#define		FP_ISPRESSFINGER		63	//Check whether a finger placed on the sensor.
#define		FP_CAPTUREFINGER		64	//Capture a fingerprint image, 256x256.
#define		FP_CAPTUREONEFRAME		65	//Capture live image, 640x480.

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


#define SB_FP__FIRSTIMAGE	engFirstImage
#define SB_FP__LIVEIMAGE	engTemp
#define SB_FP__256IMAGE		engImage256

#define U32    unsigned int
#define U16    unsigned short
#define S32    int
#define S16    short int
#define U8     unsigned char
#define	TRUE        1
#define	FALSE       0

#define VOID        void
#define SHORT       short





#define	CMOSTYPE_OV7648		0
#define	CMOSTYPE_HV7131R	1
#define CMOSTYPE_EB6048	2
#define CMOSTYPE_OV7670	3
#define CMOSTYPE_CEN931	0

enum
{
	HLP_ERR_CANNOT_LOAD_LIB = -101,
	HLP_ERR_CANNOT_LOAD_DB = -102,
	HLP_ERR_ID = -103,
	HLP_ERR_OVER = -104,
};

extern void*		engTemp;
extern int			engTempSize;
extern engcmos*		engCmosInitResult;
extern void*		engImage256;
extern void*		engFirstImage;

//extern int (*fp)( int FuncNo, long Param1, long Param2, long Param3 , long Param4, long Param5);
int _InitFp(int sensor_type,char *file_name);
int _Enroll( char * nID ,int FingerNum,char *tpath,char *dpath );
int _LoadFpData(char *nID,int FingerNum,char *FileName);
long _FpDataOneToNMatch(char *FileName);
long _OneToNMatch(char *tpath);
long _OneToOneMatch(char *nID,char *tpath);
int _DeleteFpOne(char *nID, int FingerNum);
int _DeleteFpAll(void);
int __get_enroll_count();
int _UninitFp(void);

#endif /* SMACKBIO_H_ */
