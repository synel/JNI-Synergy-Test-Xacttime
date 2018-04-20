#include "_precomp.h"
#include "_fp.h"
#define hlp_printf printf

/************************************************************************/
/* Copyright (C) 2004-2009 Beijing Smackbio Technology Co., Ltd         */
/* Helper Source File for Engine Library                                */
/* Do not change this file.                                             */
/************************************************************************/

//////////////////////////////////////////////////////////////////////////
//
// for developers
//
void*		engTemp;
int			engTempSize;
engcmos*	engCmosInitResult;
void*		engImage256;
void*		engFirstImage;

int engLastError = FP_ERR_SUCCESS;

void SB_FP__PRINTLASTERROR()
{
	char* str = "eng: unknown error\r\n";

	switch (engLastError)
	{
	case FP_ERR_SUCCESS:
		str = "eng: success\r\n";
		break;
	case FP_ERR_PARAM:
		str = "eng: parameter error\r\n";
		break;
	case FP_ERR_NOT_ENROLLED_POS:
		str = "eng: not enrolled position\r\n";
		break;
	case FP_ERR_BAD_FINGER:
		str = "eng: bad fingerprint\r\n";
		break;
	case FP_ERR_MERGE:
		str = "eng: cannot merge three fingerprints\r\n";
		break;
	case FP_ERR_IDENTIFY:
		str = "eng: cannot identify\r\n";
		break;
	case FP_ERR_VERIFY:
		str = "eng: cannot verify\r\n";
		break;
	case FP_ERR_SENSOR:
		str = "eng: sensor error\r\n";
		break;
	case FP_ERR_NOT_PRESSED:
		str = "eng: not pressed\r\n";
		break;
	case DEV_ERR:
		str = "eng: device error\r\n";
		break;
	default:
		break;
	}

	printf(str);
}

//////////////////////////////////////////////////////////////////////////
//
// Global variables
//
pfnENGINE fnSB_fp = NULL;

DWORD gFpReleaseDate;
DWORD gFpMaximum;
DWORD gFpVersion;

//////////////////////////////////////////////////////////////////////////
//
// To the library
//
int SB_fp(int FuncNo, long Param1, long Param2, long Param3, long Param4, long Param5)
{
	//hlp_printf("%s: called \n",__func__);
	return fnSB_fp(FuncNo, Param1, Param2, Param3, Param4, Param5);
}

//////////////////////////////////////////////////////////////////////////
// Function : SB_FP_GETVERSION
// Input    :
// Output   :
//     pdwReleaseDate - The release date (read-only) of the library, for example, 0x20090515.
//     pdwLibCapacity - The capacity (read-only) of fingerprint template database, 2000.
// Return   :
//     Version Number, for example, 0x0360
//
DWORD SB_FP_GETVERSION(OUT OPTIONAL DWORD *pdwReleaseDate, OUT OPTIONAL DWORD *pdwLibCapacity)
{
	DWORD dwVersion;
	DWORD dwReleaseDate;
	DWORD dwLibCapacity;

	dwVersion = SB_fp(FP_GETVERSION, (long)&dwReleaseDate, (long)&dwLibCapacity, 0UL, 0UL, 0UL);
	if (pdwReleaseDate) *pdwReleaseDate = dwReleaseDate;
	if (pdwLibCapacity) *pdwLibCapacity = dwLibCapacity;

	return dwVersion;
}

//////////////////////////////////////////////////////////////////////////
// Function : SB_FP_OPEN
// Input    :
//     nSensorType - sensor type
// Output   :
//     pIndexDB - The pointer of pointer of index database.
//     pFingerprintDB - The pointer of pointer of fingerprint template database.
// Return   :
//     FP_ERR_SUCCESS 
//     DEV_ERR - sensor is trouble, or Crypto-Memory is trouble.
//
int SB_FP_OPEN(IN int nSensorType, OUT OPTIONAL BYTE **pIndexDB, OUT OPTIONAL BYTE **pFingerprintDB)
{
	long params[5];
	hlp_printf("%s: called \n",__func__);
	params[0] = nSensorType; //reserved for sensor type
	params[1] = 0; //reserved
	params[2] = 0; //reserved
	params[3] = 0; //reserved
	params[4] = 0; //reserved

	engLastError = SB_fp(FP_OPEN, (long)params, (long)pIndexDB, (long)pFingerprintDB, 0, 0);

	engTemp				= (void*)params[0];		//engTemp, temporary buffer used by the engine.
	engTempSize			= (int)params[1];		//engTemp Size, always is greater than 640x480 bytes.
	engCmosInitResult	= (engcmos*)params[2];	//engCmosInitResult, used for debug.
	engImage256			= (void*)params[3];		//engImg256, converted fingerprint image (256x256), used for displaying.
	engFirstImage		= (void*)params[4];		//engFirstImage, first captured image (640x480), used for debug.

	return engLastError;
}

//////////////////////////////////////////////////////////////////////////
// Function : SB_FP_CLOSE
// Input    :
// Output   :
// Return   : FP_ERR_SUCCESS
//
int SB_FP_CLOSE()
{
	engLastError = SB_fp(FP_CLOSE, 0, 0, 0, 0, 0);
	return engLastError;
}

//////////////////////////////////////////////////////////////////////////
// Function : SB_FP_INDEXDBALLOC
// Input    :
// Output   :
// Return   : pointer of index database
//
BYTE* SB_FP_INDEXDBALLOC()
{
	return (BYTE*)SB_fp(FP_INDEXDBALLOC, 0, 0, 0, 0, 0);
}

//////////////////////////////////////////////////////////////////////////
// Function : SB_FP_INDEXDBFREE
// Input    :
// Output   :
// Return   :
//
void SB_FP_INDEXDBFREE()
{
	SB_fp(FP_INDEXDBFREE, 0, 0, 0, 0, 0);
}

//////////////////////////////////////////////////////////////////////////
// Function : SB_FP_FPDBALLOC
// Input    :
// Output   :
// Return   : pointer of fingerprint template database
//
FPINFO* SB_FP_FPDBALLOC()
{
	return (FPINFO*)SB_fp(FP_FPDBALLOC, 0, 0, 0, 0, 0);
}

//////////////////////////////////////////////////////////////////////////
// Function : SB_FP_FPDBFREE
// Input    :
// Output   :
// Return   :
//
void SB_FP_FPDBFREE()
{
	SB_fp(FP_FPDBFREE, 0, 0, 0, 0, 0);
}

//////////////////////////////////////////////////////////////////////////
// Function : SB_FP_ENROLLSTART
// Input    :
// Output   :
// Return   : FP_ERR_SUCCESS
//
int SB_FP_ENROLLSTART()
{
	engLastError = SB_fp(FP_ENROLLSTART, 0, 0, 0, 0, 0);
	return engLastError;
}

//////////////////////////////////////////////////////////////////////////
// Function : SB_FP_ENROLLNTH256
// Input    :
//     nStep - Must one of 1, 2, 3. Indicates the step of an enrollment.
// Output   :
// Return   :
//     FP_ERR_SUCCESS
//     FP_ERR_PARAM
//     FP_ERR_BAD_FINGER
//     > 0	: The position of doubled fingerprint template + 1.
//
int SB_FP_ENROLLNTH256(IN int nStep)
{
	engLastError = SB_fp(FP_ENROLLNTH256, (long)nStep, 0, 0, 0, 0);
	return engLastError;
}

//////////////////////////////////////////////////////////////////////////
// Function : SB_FP_ENROLLNTHFPDATA
// Input    :
//     pTemplate - a template
//     nStep - Must one of 1, 2, 3. Indicates the step of an enrollment.
// Output   :
// Return   :
//     FP_ERR_SUCCESS
//     FP_ERR_PARAM
//     FP_ERR_BAD_FINGER
//     > 0	: The position of doubled fingerprint template + 1.
//
int	SB_FP_ENROLLNTHFPDATA(IN FPINFO *pTemplate, IN int nStep)
{
	engLastError = SB_fp(FP_ENROLLNTHFPDATA, (long)pTemplate, (long)nStep, 0, 0, 0);
	return engLastError;
}

//////////////////////////////////////////////////////////////////////////
// Function : SB_FP_ENROLLMERGE
// Input    :
// Output   :
//     pTemplate - The merged fingerprint template, used for an enrollment.
// Return   :
//     FP_ERR_SUCCESS
//     FP_ERR_PARAM
//     FP_ERR_MERGE
//
int SB_FP_ENROLLMERGE(OUT FPINFO *pTemplate)
{
	engLastError = SB_fp(FP_ENROLLMERGE, (long)pTemplate, 0, 0, 0, 0);
	return engLastError;
}

//////////////////////////////////////////////////////////////////////////
// Function : SB_FP_PROCESSIMAGE256
// Input    :
// Output   :
//     pTemplate - The fingerprint template, used for Anti-Pass, do not use for an enrollment.
// Return   :
//     FP_ERR_SUCCESS
//     FP_ERR_PARAM
//     FP_ERR_BAD_FINGER
//
int	SB_FP_PROCESSIMAGE256(OUT FPINFO *pTemplate)
{
	engLastError = SB_fp(FP_PROCESSIMAGE256, (long)pTemplate, 0, 0, 0, 0);
	return engLastError;
}

//////////////////////////////////////////////////////////////////////////
// Function : SB_FP_DECOMPRESSFPDATA
// Input    :
//     pCompressed - Compressed template data.
//     nCompressedLen - Length of compressed template data.
// Output   :
//     pTemplate - The fingerprint template, used for an enrollment or matching.
// Return   :
//     FP_ERR_SUCCESS
//     FP_ERR_PARAM
//
int	SB_FP_DECOMPRESSFPDATA(IN void* pCompressed, IN int nCompressedLen, OUT FPINFO *pTemplate)
{
	engLastError = SB_fp(FP_DECOMPRESSFPDATA, (long)pCompressed, (long)nCompressedLen, (long)pTemplate, 0, 0);
	return engLastError;
}

//////////////////////////////////////////////////////////////////////////
// Function : SB_FP_IDENTIFYIMAGE256
// Input    :
// Output   :
//     pbAdapted - The adapted status information (0 - not adapted, 1 - adapted)
// Return   :
//     >= 0 : The position of the matched template in fingerprint template database
//     FP_ERR_BAD_FINGER
//     FP_ERR_IDENTIFY
//
int	SB_FP_IDENTIFYIMAGE256(OUT OPTIONAL BOOL *pbAdapted)
{
	DWORD dwAdapted;

	engLastError = SB_fp(FP_IDENTIFYIMAGE256, (long)&dwAdapted, 0, 0, 0, 0);
	if (pbAdapted) *pbAdapted = (BOOL)dwAdapted;

	return engLastError;
}

//////////////////////////////////////////////////////////////////////////
// Function : SB_FP_IDENTIFYFPDATA
// Input    :
//     pTemplate - a fingerprint template for matching
// Output   :
//     pbAdapted - The adapted status information (0 - not adapted, 1 - adapted)
// Return   :
//     >= 0 : The position of the matched template in fingerprint template database
//     FP_ERR_PARAM
//     FP_ERR_BAD_FINGER
//     FP_ERR_IDENTIFY
//
int SB_FP_IDENTIFYFPDATA(IN FPINFO *pTemplate, OUT OPTIONAL BOOL *pbAdapted)
{
	DWORD dwAdapted;

	engLastError = SB_fp(FP_IDENTIFYFPDATA, (long)pTemplate, (long)&dwAdapted, 0, 0, 0);
	if (pbAdapted) *pbAdapted = (BOOL)dwAdapted;

	return engLastError;
}

//////////////////////////////////////////////////////////////////////////
// Function : SB_FP_VERIFYIMAGE256
// Input    :
//     nPos - The position of the template (to be verified) in fingerprint template database.
// Output   :
//     pbAdapted - The adapted status information (0 - not adapted, 1 - adapted)
// Return   :
//     FP_ERR_SUCCESS
//     FP_ERR_NOT_ENROLLED_POS
//     FP_ERR_BAD_FINGER
//     FP_ERR_VERIFY
//
int	SB_FP_VERIFYIMAGE256(IN int nPos, OUT OPTIONAL BOOL *pbAdapted)
{
	DWORD dwAdapted;

	engLastError = SB_fp(FP_VERIFYIMAGE256, (long)nPos, (long)&dwAdapted, 0, 0, 0);
	if (pbAdapted) *pbAdapted = (BOOL)dwAdapted;

	return engLastError;
}

//////////////////////////////////////////////////////////////////////////
// Function : SB_FP_VERIFYFPDATA
// Input    :
//     pTemplate - a fingerprint template for matching
//     nPos - The position of the template (to be verified) in fingerprint template database.
// Output   :
//     pbAdapted - The adapted status information (0 - not adapted, 1 - adapted)
// Return   :
//     FP_ERR_SUCCESS
//     FP_ERR_PARAM
//     FP_ERR_NOT_ENROLLED_POS
//     FP_ERR_BAD_FINGER
//     FP_ERR_VERIFY
//
int SB_FP_VERIFYFPDATA(IN FPINFO *pTemplate, IN int nPos, OUT OPTIONAL BOOL *pbAdapted)
{
	DWORD dwAdapted;

	engLastError = SB_fp(FP_VERIFYFPDATA, (long)pTemplate, (long)nPos, (long)&dwAdapted, 0, 0);
	if (pbAdapted) *pbAdapted = (BOOL)dwAdapted;

	return engLastError;
}

//////////////////////////////////////////////////////////////////////////
// Function : SB_FP_VERIFYIMAGE256_2
// Input    :
//     pTemplate - template data, will be matched with current captured image.
// Output   :
// Return   :
//     FP_ERR_SUCCESS
//     FP_ERR_PARAM
//     FP_ERR_BAD_FINGER
//     FP_ERR_VERIFY
//
int	SB_FP_VERIFYIMAGE256_2(IN FPINFO *pTemplate)
{
	engLastError = SB_fp(FP_VERIFYIMAGE256_2, (long)pTemplate, 0, 0, 0, 0); //does not adapting for CARD matching !!!!!

	return engLastError;
}

//////////////////////////////////////////////////////////////////////////
// Function : SB_FP_VERIFYFPDATA_2
// Input    :
//     pTemplate - template data, will be matched with second parameter.
//     pTemplateReg - template data, will be matched with first parameter.
// Output   :
// Return   :
//     FP_ERR_SUCCESS
//     FP_ERR_PARAM
//     FP_ERR_BAD_FINGER
//     FP_ERR_VERIFY
//
int SB_FP_VERIFYFPDATA_2(IN FPINFO *pTemplate, IN FPINFO *pTemplateReg)
{
	engLastError = SB_fp(FP_VERIFYFPDATA_2, (long)pTemplate, (long)pTemplateReg, 0, 0, 0); //does not adapting for CARD matching !!!!!

	return engLastError;
}

//////////////////////////////////////////////////////////////////////////
// Function : SB_FP_GET_QUALITY
// Input    :
//     pTemplate - template data to be evaluated.
// Output   :
// Return   :
//     quality score
int SB_FP_GET_QUALITY(IN FPINFO *pTemplate)
{
	engLastError = SB_fp(FP_QUALIFY_FPDATA, (long)pTemplate, 0, 0, 0, 0);

	return engLastError;
}

//////////////////////////////////////////////////////////////////////////
// Function : SB_FP_CMOSINIT
// Input    :
//     dwMechanical - The current mechanical parameter, if 0, then use standard value.
//     dwExpose - The current expose, if 0, then use standard value.
// Output   :
// Return   :
//     FP_ERR_SUCCESS
//     FP_ERR_SENSOR
//     
int SB_FP_CMOSINIT(IN DWORD dwMechanical, IN DWORD dwExpose)
{
	engLastError = SB_fp(FP_CMOSINIT, (long)dwMechanical, (long)dwExpose, 0, 0, 0);
	return engLastError;
}

//////////////////////////////////////////////////////////////////////////
// Function : SB_FP_CMOSCHECKADJUST
// Input    :
//     pdwMechanical - The calculated mechanical parameter value of the SENSOR, should be saved.
//     pdwExpose - The adjusted expose register value of the CMOS, should be saved.
// Output   :
// Return   :
//     FP_ERR_SUCCESS
//     FP_ERR_SENSOR
//
int SB_FP_CMOSCHECKADJUST(OUT OPTIONAL DWORD* pdwMechanical, OUT OPTIONAL DWORD* pdwExpose)
{
	DWORD dwMechanical;
	DWORD dwExpose;

	engLastError = SB_fp(FP_CMOSCHECKADJUST, (long)&dwMechanical, (long)&dwExpose, 0, 0, 0);
	
	if (pdwMechanical) *pdwMechanical = dwMechanical;
	if (pdwExpose) *pdwExpose = dwExpose;
	
	return engLastError;
}

//////////////////////////////////////////////////////////////////////////
// Function : SB_FP_ISPRESSFINGER
// Input    :
// Output   :
// Return   :
//     FP_ERR_SUCCESS
//     FP_ERR_SENSOR
//     FP_ERR_NOT_PRESSED
//
int SB_FP_ISPRESSFINGER()
{
	engLastError = SB_fp(FP_ISPRESSFINGER, 0, 0, 0, 0, 0);
	return engLastError;
}

//////////////////////////////////////////////////////////////////////////
// Function : SB_FP_CAPTUREFINGER
// Input    :
// Output   :
// Return   :
//     FP_ERR_SUCCESS
//     FP_ERR_SENSOR
//     FP_ERR_NOT_PRESSED
//
int SB_FP_CAPTUREFINGER()
{
	engLastError = SB_fp(FP_CAPTUREFINGER, 0, 0, 0, 0, 0);
	return engLastError;
}

//////////////////////////////////////////////////////////////////////////
// Function : SB_FP_CAPTUREONEFRAME
// Input    :
// Output   :
// Return   :
//     FP_ERR_SUCCESS
//     FP_ERR_SENSOR
//
int SB_FP_CAPTUREONEFRAME()
{
	engLastError = SB_fp(FP_CAPTUREONEFRAME, 0, 0, 0, 0, 0);
	return engLastError;
}
