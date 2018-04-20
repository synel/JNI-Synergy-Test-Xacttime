/*
 * smackbio.c
 *
 *  Created on: 2014-5-6
 *      Author: aduo
 */

#include "smackbio.h"
#include "../_precomp.h"

int (*fp)( int FuncNo, long Param1, long Param2, long Param3, long Param4, long Param5 );

FPINFO* gMatchData;
BYTE* gValidFile;
FPINFO gFeature;
void *vhfp;

void*		engTemp;
int			engTempSize;
engcmos*	engCmosInitResult;
void*		engImage256;
void*		engFirstImage;

int gRegMax = 1;

/*--------------------------------------------------------------------------*
@Function			:ErrorProc - error
@Include      		: "finger_s10.h"
@Description			: nErr£ºError No.
@Return Value		: Success 0,Failure-1.
*---------------------------------------------------------------------------*/
int ErrorProc(int nErr)
{
	char* str ;
printf("nErr=%d\n",nErr);
return 0;
	switch (nErr)
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
	case HLP_ERR_CANNOT_LOAD_LIB:
		str = "hlp: could not load library\r\n";
		break;
	case HLP_ERR_CANNOT_LOAD_DB:
		str = "hlp: could not load database\r\n";
		break;
	case HLP_ERR_ID:
		str = "hlp: invalid ID or Finger number\r\n";
		break;
	case HLP_ERR_OVER:
		str = "hlp: database overflow\r\n";
		break;
	default:
		str = "hlp: unknown error\r\n";
		break;
	}
	printf(str);
  	return 0;
}

/***************************************************************/
int __get_enroll_count()
{
	int i,nCount = 0;

	for (i=0; i<gRegMax; i++)
	{
		if(gValidFile[i] != 0)
			nCount++;
	}

	return nCount;
}

int _get_empty_pos()
{
	int i;

	for (i=0; i<gRegMax; i++)
	{
		if (gValidFile[i] == 0)
			break;
	}

	return i;
}
//get the positon of fingerprint template
int _get_pos(long nID, long nFingerNum)
{
	int i;

	if (nFingerNum < 0 || nFingerNum > 255) return gRegMax;
	printf("gRegMax %d\r\n",gRegMax);

	for (i=0; i < gRegMax; i++)
	{
		if (gValidFile[i] != 0 && gMatchData[i].ID == (DWORD)nID && gMatchData[i].FingerNum == nFingerNum)
			break;
	}

	return i;
}
//////////////////////////////////////////////////////////////////////////
long hlpEnrollPrepare(long nID, long nFingerNum, long Manager)
{
	int nPos;

	if (nFingerNum < 0 || nFingerNum > 255) return HLP_ERR_ID;
	if (Manager < 0 || Manager > 255) return HLP_ERR_ID;

	nPos = _get_pos(nID, nFingerNum);
	if (nPos != gRegMax)
		return HLP_ERR_ID;

	nPos = _get_empty_pos();
	if (nPos == gRegMax)
		return HLP_ERR_OVER;

	return nPos;
}
/***************************************************************/

//////////////////////////////////////////////////////////////////////////
// Function : SB_FP_ENROLLNTH256
// Input    :
//     nStep - Must one of 1, 2, 3. Indicates the step of an enrollment.
// Output   :
// Return   :
//     FP_ERR_SUCCESS
//     -1 FP_ERR_PARAM
//     -3 FP_ERR_BAD_FINGER
//     > 0	: The position of doubled fingerprint template + 1.
//
int SB_FP_ENROLLNTH256(IN int nStep)
{
	int Error=0;

	Error = fp(FP_ENROLLNTH256, (long)nStep, 0, 0, 0, 0);
	return Error;
}

int EnrollState=0;
/*--------------------------------------------------------------------------*
@Function			: Enroll - enroll fingerprint
@Include      		: "finger_s10.h"
@Description			: nID£ºStaff_No.
				FingerNum : finger_No.
				tpath : path where to store fingerprint image
				dpath : path where to store of template
@Return Value		:
			 1£ºEnter success£»
			 0: Alignment success£¬ show the image.
			-1£ºTimeout to quit.
			-2£ºAlignment error
			-3£ºSaving fingerprint image error
			-4£ºSynthesis fingerprint error
			-5£ºSaving finger template error
*---------------------------------------------------------------------------*/
int _Enroll( char * nID ,int FingerNum,char *tpath,char *dpath )
{
	long i,j=0;
	long nRet;
	if(_get_enroll_count() == gRegMax )
	{
//		printf("Database overflow\n");
		return -1;
	}
	nRet = fp(FP_ENROLLSTART, 0, 0, 0 ,0,0); //enroll start
	if(nRet != 0)
	{
		ErrorProc( nRet );
		return -2;
	}

	for(i=1;i<4;i++)
	{
		if(fpdevinfo.performBeforScan)
			fpdevinfo.performBeforScan(i);

		for(j=0;j<40;j++)
		{
			nRet = fp(FP_CAPTUREFINGER, 0, 0, 0, 0, 0);	//get fingerprint image
			if(nRet==0)	break;
		}
		if(j==40)
		{
//			printf("Time Out\n");
			return -3;  //timeout,have not read fingerprint data
		}

		if(tpath){
		      write_bitmap(tpath,(unsigned char*)SB_FP__256IMAGE,256,256);
		}
		if(fpdevinfo.performAfterScan)
				fpdevinfo.performAfterScan(i);

		nRet=SB_FP_ENROLLNTH256(i);//fp(FP_ENROLLNTH256, (long)i, 0, 0, 0, 0);
		if(nRet < 0)
		{
			ErrorProc( nRet );
			return -4;
//			i--;
//			continue;
		}
		else if(nRet > 0)
		{
//                        i=0;
//                        continue;
			return -5;
		}

		if(i<3)
		{
			while(fp(FP_ISPRESSFINGER, 0, 0, 0, 0, 0) == 0);
		}
	}

//	nid = atol(nID)*10+FingerNum;
	if(SaveTempData((long)atol(nID),(long)FingerNum,dpath) ==TRUE)
		return TRUE;
	else
		return -6;
}

/*--------------------------------------------------------------------------*
@Function		: OneToOneMatch - fingerprint match
@Include      	: "finger_s10.h"
@Description		: nID£ºStaff_No.
			tpath : path to store fingerprint image
			waiting press on FPU£¬get fingerprint data£¬OneToOne match
@Return Value	: Success staff_No.*10+finger_No.;
			Read Failure -1;
			Database empty -2;
			Comparision Failure 0 or -103;
*---------------------------------------------------------------------------*/
long _OneToOneMatch(char *nID,char *tpath)
{
	int nRet=0,FingerNum=0;
	long nPos,anpos;
	DWORD dwAdapted;

	if( fp_enable == 0||nID==NULL)	return -1;

	if (_get_enroll_count() == 0)
	{
//		printf("Database empty\n");
		return -2;
	}

	nRet = fp(FP_CAPTUREFINGER, 0, 0, 0,0,0);
	if(nRet < 0)
	{
		return -1;
	}
      if(tpath)
        {
		write_bitmap(tpath, (unsigned char*)SB_FP__256IMAGE, 256, 256);
        }
	//for(FingerNum=0;FingerNum<=10;FingerNum++)
	for(FingerNum=0;FingerNum<10;FingerNum++)
	{
		anpos= atol(nID)*10+(long)FingerNum;
		nPos = _get_pos(anpos, (long)FingerNum);
		printf("nPos %ld\r\n",nPos);
		if (nPos != gRegMax)	break;
	}
	if(FingerNum == 10)
	{
		ErrorProc(HLP_ERR_ID);
		return HLP_ERR_ID;
	}
	nRet = fp(FP_VERIFYIMAGE256, (long)nPos, (long)&dwAdapted, 0, 0, 0);

	if(nRet<0)
	{
		return 0;
	}
	else
		return anpos;
}
/*--------------------------------------------------------------------------*
@Function			:OneToNMatch - fingerprint data match
@Include      		: "finger_s10.h"
@Description			: tpath : path of fingerprint image
										fingerprint match 1:N
										waiting press on FPU£¬get fingerprint data£¬OneToN match
@Return Value		:
									Success staff_NO.*10+finger_No;
				 					Read Failure -1;
									Database empty -2;
									Comparision Failure 0 or -103;
*---------------------------------------------------------------------------*/
long _OneToNMatch(char *tpath)
{
	long nRet;
	unsigned long dwAdapted;

	if( fp_enable == 0)		return -1;
	if (_get_enroll_count() == 0)
	{
//		printf("Database empty\n");
		return -2;
	}

	nRet = fp(FP_CAPTUREFINGER, 0, 0, 0,0,0);
	if(nRet < 0)
	{
		return -1;
	}
      if(tpath)
        {
		write_bitmap(tpath, (unsigned char*)SB_FP__256IMAGE, 256, 256);
        }
	nRet = fp(FP_IDENTIFYIMAGE256, (long)&dwAdapted, 0, 0, 0, 0);

	if(nRet<0)
	{
		return 0;
	}
	else
	{
		return gMatchData[nRet].ID;
	}

}

/*--------------------------------------------------------------------------*
@Function			: SaveTempData - save fingerprint data
@Include      		: "finger_s10.h"
@Description			:
				nID£ºstaff_No£»
				nFingerNum : finger number
				dpath : path of template
@Return Value		: Success TURE, Failure FALSE
*---------------------------------------------------------------------------*/
int SaveTempData( long nID, long nFingerNum ,char *dpath)
{
	FILE * vFile;
	int vSize;
	long nRet;
	char szFileName[256];
	FPINFO *pTemplate;

	//generated fingerprint data from 3 fingerprint images
	pTemplate=&gFeature;
	nRet = fp(FP_ENROLLMERGE, (long)pTemplate, 0, 0,0,0);
	if(nRet < 0)
	{
		ErrorProc( nRet );
		return nRet;
//		i=0;
//		continue;
	}
	//load fingerprint to Fingerprint device
	nRet = hlpEnrollPrepare(nID*10+nFingerNum, nFingerNum, 0);
	if(nRet<0)
	{
		ErrorProc( nRet );
		return nRet;
		//i=0;
	}

	gValidFile[nRet] = 1;
	gFeature.ID = (DWORD)nID*10+nFingerNum;
	gFeature.FingerNum = (BYTE)nFingerNum;
	gFeature.Manager = (BYTE)(DWORD)1;
	gFeature.Valid = 1;
	memcpy(&gMatchData[nRet], &gFeature, sizeof(FPINFO));

	sprintf(szFileName, "%s%ld_%d.s10", dpath,nID, (int)nFingerNum);

       creatdir(szFileName);
	vFile = fopen( szFileName, "wb" );
	if( vFile == NULL )
		return -1;

	vSize = fwrite( &gFeature, FPDATASIZE, 1,  vFile );
	fclose( vFile );
	if( vSize != 1 )
		return -1;

	return TRUE;
}


/*--------------------------------------------------------------------------*
@Function			: InitFp - initialize the fingerprint device
@Include      		: "finger_s10.h"
@Description			: lbpath : path of fingerprint library
				initialize FP library ,fingerprint brightness setting
@Return Value		: Success TRUE, Failure FALSE
*---------------------------------------------------------------------------*/
int _InitFp(int sensor_type,char *file_name)
{
	int nRet,i;
	DWORD dwMechanical=0, dwExpose=0;
	long params[5];
	BYTE **pIndexDB, **pFingerprintDB;

//	fpdevinfo=*fpdev_info;
	params[0] = sensor_type; //reserved for sensor type
	params[1] = 0; //reserved
	params[2] = 0; //reserved
	params[3] = 0; //reserved
	params[4] = 0; //reserved
	fp_enable=0;
	fingerSum=0;
        vhfp = dlopen(file_name, RTLD_NOW );
	if( vhfp == NULL )
	{
		ErrorProc(HLP_ERR_CANNOT_LOAD_LIB);
		 return FALSE;
	}

	*(void**)(&fp) = dlsym( vhfp, "SB_fp" );	//read functions or variables from dynamic library
	if( fp == NULL )
	{
		ErrorProc(HLP_ERR_CANNOT_LOAD_LIB);
		 return FALSE;
	}
	// check version
	fp(FP_GETVERSION, 0UL, (long)&gRegMax, 0UL, 0UL, 0UL);
	gValidFile = (BYTE*)malloc(gRegMax * sizeof(BYTE));
	gMatchData = (FPINFO*)malloc(gRegMax * sizeof(FPINFO));
	pIndexDB=(BYTE**)&gValidFile;
	pFingerprintDB= (BYTE**)&gMatchData;
      for(i=0;i<1;i++)
      {
	// open
	nRet = fp(FP_OPEN, (long)params, (long)pIndexDB, (long)pFingerprintDB, 0, 0);
	if(nRet < 0)
	{
		usleep(500000);
		ErrorProc(nRet);
		continue;
	}
	engTemp	= (void*)params[0];		//engTemp, temporary buffer used by the engine.
	engTempSize	= (int)params[1];	//engTemp Size, always is greater than 640x480 bytes.
	engCmosInitResult	= (engcmos*)params[2];	//engCmosInitResult, used for debug.
	engImage256	= (void*)params[3];	//engImg256, converted fingerprint image (256x256), used for displaying.
	engFirstImage = (void*)params[4];//engFirstImage, first captured image (640x480), used for debug.

	nRet = fp(FP_CMOSINIT, (long)dwMechanical, (long)dwExpose, 0, 0, 0);
	if(nRet < 0)
	{
		ErrorProc(nRet);
		usleep(500000);
		continue;
	}
	else
	{
	 	return TRUE;
	}
    }
	return FALSE;
}

/*--------------------------------------------------------------------------*
@Function			: FpDataOneToNMatch - check FP data is exist
@Include      		: "finger_s10.h"
@Description			: FileName£ºFP data file£»
@Return Value		: >0 ,exist,ID*10+finger num;<=0  not exist;
*---------------------------------------------------------------------------*/
long _FpDataOneToNMatch(char *FileName)
{
	int vSize;
	int nRet;
	FILE *vFile;
	unsigned char	gFpdataBuff[FPDATASIZE];
	long nID=0;


	if(FileName==NULL)return -1;
	vFile = fopen( FileName, "rb" );
	if(vFile==NULL)return -1;
	vSize = fread( gFpdataBuff, FPDATASIZE, 1,  vFile );
	if(vSize != 1)
	{
		fclose( vFile );
		return -1;
	}
	fclose(vFile);
	nRet = fp(FP_IDENTIFYFPDATA, (long)&gFpdataBuff,(long)&nID, 0,0,0);
	//printf("nRet %d,%ld\n",nRet,nID);
	if(nRet >0)
	{
		//printf( "same finger %d,%d\n",gMatchData[nRet].FingerNum,gMatchData[nRet].ID);
		return gMatchData[nRet].ID;
	}
	else if(nRet<0)
	{

      	return nRet;
	}else if(nRet==0)
	{
	//printf( "same finger file %d,%d\n",gMatchData[nRet].FingerNum,gMatchData[nRet].ID);
	return gMatchData[nRet].ID;
	}


	return 0;
}
/*--------------------------------------------------------------------------*
@Function			: LoadFpData - load FP data
@Include      		: "finger_s10.h"
@Description			: nID£ºstaff_No£»
				FingerNum : finger_No.
				FileName : path of template
				load the FP data to memory
@Return Value		: Success TRUE, Failure FALSE
*---------------------------------------------------------------------------*/
int _LoadFpData(char *nID,int FingerNum,char *FileName)
{
	int vSize,nPos=0;
	FILE *vFile;
	unsigned char	gFpdataBuff[FPDATASIZE];

	if(nID==NULL || fp_enable == 0)	return FALSE;
	if(access(FileName,R_OK|F_OK) != 0) return FALSE;

	nPos = hlpEnrollPrepare(atol(nID)*10+FingerNum, FingerNum, 0);
	if(nPos<0)
	{
		ErrorProc( nPos );
		return FALSE;
		//i=0;
	}

	vFile = fopen( FileName, "rb" );
	vSize = fread( gFpdataBuff, FPDATASIZE, 1,  vFile );
	if(vSize != 1)
	{
		fclose( vFile );
		return FALSE;
	}

	gValidFile[nPos]=1;		//finger badge
	memcpy(&gMatchData[nPos],gFpdataBuff, sizeof(FPINFO));	//1404
	gMatchData[nPos].FingerNum = (BYTE)(DWORD)FingerNum;
	gMatchData[nPos].ID=(DWORD)(atol(nID)*10+FingerNum);
	fingerSum++;
	fclose( vFile );
	return TRUE;
}

/*--------------------------------------------------------------------------*
@Function			: ExposeGet - Get the brightness of FP
@Include      		: "finger_s10.h"
@Description			: nSensorType£ºFp sensor type;
@Return Value		: Success return value of brightness, Failure 0
*---------------------------------------------------------------------------*/
long ExposeGet(int nSensorType)
{
	FILE *fo;
	int vSize;
	int dat[3];
	fo = fopen( "expose.dat", "rb" );
	if( fo == NULL )
		return 0; //use std expose.
	vSize = fread( &dat[0], 4, 3, fo);
	if( vSize != 3 )
	{
		dat[0] = -1;
	}
	fclose( fo );
	if (dat[0] == nSensorType)
		return dat[1];
	return 0;

}


/*--------------------------------------------------------------------------*
@Function			: setfingerbright_s10 - set FP birghtness
@Include      		: "finger_s10.h"
@Description			: we suggest that you'd best not use this function
@Return Value		: Success 0, Failure -1
*---------------------------------------------------------------------------*/
int setfingerbright_s10(int Fingertype)
{
	FILE *fo;
	int vSize;
	int dat[3] ;
	unsigned long dwMechanical, dwExpose;;


      if(Fingertype==1)
	        dat[0]=CMOSTYPE_OV7648;
      else dat[0]=CMOSTYPE_HV7131R;

      fp( FP_CMOSCHECKADJUST,(long)&dwMechanical, (long)&dwExpose, 0 ,0 ,0);
	dat[1] = dwMechanical;
	dat[2] = dwExpose;
	if((fo = fopen("expose.dat", "wb")) == NULL) {
		return FALSE;
	}
	vSize = fwrite( &dat[0], 4, 3, fo);
	if( vSize != 3 )
	{
		return FALSE;
	}
	fclose( fo );
	return TRUE;
}


/*--------------------------------------------------------------------------*
@Function			: DeleteFpOne - delete fingerprint data
@Include      		: "finger_s10.h"
@Description			: nID -staff No.
				: FingerNUM - finger No.
				delete fingerprint template data of finger with number FingerNUm of one with number nID
@Return Value		: Success TRUE,Failure FALSE;
@Create time			: 2009-06-15 08:23
*---------------------------------------------------------------------------*/
int _DeleteFpOne(char *nID, int FingerNum)
{
	int nPos;

	if(nID==NULL|| FingerNum>=10)	return FALSE;
	nPos = _get_pos(atol(nID)*10+FingerNum, (long)FingerNum);
	if (nPos == gRegMax)
  	{
		ErrorProc(HLP_ERR_ID);
		return FALSE;
  	}
	fingerSum--;
	if(fingerSum<0)	fingerSum=0;
	gValidFile[nPos] = 0;
	return TRUE;
}


/*--------------------------------------------------------------------------*
@Function			: DeleteFpAll - delete all FP data
@Include      		: "finger_s10.h"
@Description			:
				delete all template data from memory
@Return Value		: Success TRUE,Failure FALSE
@Create time			: 2009-06-15 08:23
*---------------------------------------------------------------------------*/
int _DeleteFpAll(void)
{
	int i;
	for (i=0; i<gRegMax; i++)
	{
		gValidFile[i] = 0;
	}
	fingerSum=0;
	return TRUE;
}
/*--------------------------------------------------------------------------*
@Function			: UninitFp - unlodad FP device
@Include      		: "finger_s10.h"
@Description			: release resource
@Return Value		: Success TRUE,Failure FALSE
@Create time			: 2009-06-15 08:23
*---------------------------------------------------------------------------*/
int _UninitFp(void)
{
	if(fp_enable == 0)	return FALSE;
	fp_enable = 0;
	DeleteFpAll();
	fp(FP_CLOSE, 0,0,0, 0,0);
	dlclose( vhfp );
	return TRUE;
}


/*--------------------------------------------------------------------------*
@Function			: GetFpDataOne - get FP template
@Include      		: "finger_s10.h"
@Description			: nID Staff_No.
				FIngerNum : finger_No.
@Return Value		: Success return FP template data,Failure NULL
@Create time			: 2009-06-15 08:23
*---------------------------------------------------------------------------*/
unsigned char *GetFpDataOne(char *nID,int FingerNum)
{
	long anpos;
	int nPos;
	static unsigned char	gFpdataBuff[FPDATASIZE];

	if( fp_enable == 0||nID == NULL)	return NULL;

	anpos = (atol(nID)*10)+FingerNum;
	nPos = _get_pos(anpos, (long)FingerNum);
	if (nPos == gRegMax)
  	{
		ErrorProc(HLP_ERR_ID);
		return FALSE;
  	}
	memcpy(&gFpdataBuff[0],&gMatchData[nPos], sizeof(FPINFO));	//1404
  	return &gFpdataBuff[0];
}


/*--------------------------------------------------------------------------*
@Function			: setFpData - load FP template
@Include      		: "finger_s10.h"
@Description			: nID staff_No.
				FingerNum	finger_No.
				str template data
				load FP template to device
@Return Value		: Success TRUE,Failure FALSE
@Create time			: 2009-06-15 08:23
*---------------------------------------------------------------------------*/
int setFpData(char *nID,int FingerNum,unsigned char *str)
{
	long anpos;
	int nRet=0;

	if( fp_enable == 0||str==NULL)	return FALSE;
	anpos = atol(nID)*10+FingerNum;
      nRet = hlpEnrollPrepare(anpos, (long)FingerNum, 0);//load fingerprint to Fingerprint device
      if(nRet<0)
        {
           ErrorProc( nRet );
           return FALSE;
        }
	gValidFile[nRet]=1;
	memcpy(&gMatchData[nRet],str, sizeof(FPINFO));	//1404
	gMatchData[nRet].FingerNum = (BYTE)(DWORD)FingerNum;
	gMatchData[nRet].ID=anpos;
	fingerSum++;
	return TRUE;

}

