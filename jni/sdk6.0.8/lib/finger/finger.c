#include "../public/public.h"
#include "../sound/sound.h"
#include "finger.h"

char *fingerfilesuffix="s10";
int fp_enable=0;		//initialization success
int fp_sensor_type = 0;
FPDEVINFO fpdevinfo;

int fingerSum=0;             //the number of current fingerprints

BITMAPINFOHEADER *m_lpBMIH=NULL;
WORD bfType;


/*--------------------------------------------------------------------------*
@Function			:write_bitmap - save fingerprint image
@Include 			: "finger_s10.h"
@Description			:imgout£ºpath of image£»
				buffer : data of image;
				nWidth : Image width;
				nHeight : Image height
@Return Value		: void
*---------------------------------------------------------------------------*/
void write_bitmap(char *imgout, unsigned char *buffer, int nWidth, int nHeight)
{
	BITMAPFILEHEADER bfh;
	RGBQUAD* m_lpvColorTable;
	FILE *fo;
	int i;
	int m_nColorTableEntries=256;
	int m_nBitCount=8;
	 char *ptr;

        BYTE  gImgTmp1[nHeight][nWidth];
	if((fo = fopen(imgout, "wb")) == NULL) {
		//printf("cannot open %s\n", imgout);
		return;
		}

		m_nColorTableEntries=256;
		m_nBitCount=8;

    m_lpBMIH = (BITMAPINFOHEADER*) malloc(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * m_nColorTableEntries);

    m_lpBMIH->biSize = sizeof(BITMAPINFOHEADER);

    m_lpBMIH->biWidth  = nWidth;
    m_lpBMIH->biHeight = nHeight;
    m_lpBMIH->biPlanes = 1;
    m_lpBMIH->biBitCount = m_nBitCount;
    m_lpBMIH->biCompression = 0;
    m_lpBMIH->biSizeImage = 0;
    m_lpBMIH->biXPelsPerMeter = 0;
    m_lpBMIH->biYPelsPerMeter = 0;
    m_lpBMIH->biClrUsed      = m_nColorTableEntries;
    m_lpBMIH->biClrImportant = m_nColorTableEntries;

    m_lpvColorTable = (RGBQUAD*)((unsigned char *)m_lpBMIH + sizeof(BITMAPINFOHEADER)); // points inside m_lpBMIH.
    for (i = 0; i < m_nColorTableEntries; i++)
    {
        BYTE k = m_nColorTableEntries==256?(BYTE)i:(BYTE)i<<4;
            m_lpvColorTable[i].rgbRed      = k;  // Gray Scale !
            m_lpvColorTable[i].rgbGreen    = k;
            m_lpvColorTable[i].rgbBlue     = k;
            m_lpvColorTable[i].rgbReserved = 0;
    }

	// Fill in the Bitmap File Header
	memset(&bfh, 0, sizeof(BITMAPFILEHEADER));
	bfh.bfType = ( (WORD) ('M' << 8) | 'B');
	// Calculate the size of the bitmap including the palette
	bfh.bfSize = 14 + sizeof(BITMAPINFOHEADER) +
			m_lpBMIH->biClrUsed * sizeof(RGBQUAD) +
			((((m_lpBMIH->biWidth * m_lpBMIH->biBitCount + 31)/32) * 4) * m_lpBMIH->biHeight);
      // printf("%ld\n",bfh.bfSize);	
	bfh.bfReserved1 = 0;
	bfh.bfReserved2 = 0;

	// Offset to actual bits is after palette

	bfh.bfOffBits = 14 + sizeof(BITMAPINFOHEADER)
		+ m_lpBMIH->biClrUsed * sizeof(RGBQUAD);

	 //Write the results
	 //Notice: according to bmp file format,BITMAPFILEHEADER's size should be 14 bytes,but here it'size 
	 //is 16 bytes because of byte align.that is to say ,there are to pad bytes
	 //Though you add '-fpack-struct' when compiler it,it is still 16 bytes
	 //so other way is used to prevent writing pad bytes to file.

	ptr=(char*)&bfh;
	fwrite(ptr, 2, 1, fo);
	ptr+=4;
	fwrite(ptr, sizeof(BITMAPFILEHEADER)-4, 1, fo);
	fwrite(m_lpBMIH, sizeof(BITMAPINFOHEADER)+m_lpBMIH->biClrUsed*sizeof(RGBQUAD), 1, fo);
  	 for(i=nHeight-1;i>=0;i--)
    {
        memcpy(&gImgTmp1[i][0],buffer,nWidth);
        buffer += nWidth;
    }
	fwrite(&gImgTmp1[0][0], bfh.bfSize-14-m_lpBMIH->biClrUsed*sizeof(RGBQUAD), 1, fo);
	//fwrite(buffer, bfh.bfSize-sizeof(BITMAPFILEHEADER)-m_lpBMIH->biClrUsed*sizeof(RGBQUAD), 1, fo);
	fclose(fo);
	free(m_lpBMIH);
} 

/*--------------------------------------------------------------------------*
@Function			: getfingernum - get numbers of FP
@Include      		: "finger_s10.h"
@Description			: used for network communication			
@Return Value		: Success return the number of fingerprints loaded
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int getfingernum()
{
	return fingerSum;
}

/*--------------------------------------------------------------------------*
@Function			: DeleteFPFlash - delete FP data
@Include      		: "finger_s10.h"
@Description			: 
				nID staff_No.
				FingerNUM - finger No.
				dpath : path of template
				delete finger template form ram
@Return Value		: Success TRUE,Failure FALSE
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int DeleteFPFlash(char *nID, int FingerNum,char *dpath)
{
	char szFileName[256];

	if(FingerNum>=10)	return FALSE;
	if(nID == NULL ||strlen(nID)>sizeof(szFileName)-10)		return FALSE;
	memset(szFileName,0,sizeof(szFileName));
	sprintf(szFileName, "%s%ld_%d.s10", dpath,atol(nID), (int)FingerNum);
	remove(szFileName);                       //delete temporary files
	return TRUE;
}

/*--------------------------------------------------------------------------*
@Function			: DeleteFpOneAllFALSE - delete FP data from flash
@Include      		: "finger_s10.h"
@Description			: 
				nID staff_No.
				dpath : path of template
				delete finger template form ram
@Return Value		: Success TRUE,Failure FALSE
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int DeleteFpOneAllFALSE(char *nID,char *dpath)
{
	int nRet,FingerNum=0;

	if(nID==NULL || fp_enable == 0 || dpath==NULL)	return FALSE;

	for(FingerNum=0;FingerNum<=9;FingerNum++)
	{
		nRet=DeleteFPFlash(nID,FingerNum,dpath);
/*		if(nRet == 0)
		{
			ErrorProc(nRet);
			return FALSE;
		}
*/
	}
	return TRUE;
}

/*--------------------------------------------------------------------------*
@Function			: DeleteFpOneAll - delete FP data
@Include      		: "finger_s10.h"
@Description			: nID staff_No.
				Only delete template of someone from fp memory			
@Return Value		: Success TRUE,Failure FALSE
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int DeleteFpOneAll(char *nID)
{
	int nRet,FingerNum=0;

	if(nID==NULL || fp_enable == 0)	return FALSE;

	for(FingerNum=0;FingerNum<=9;FingerNum++)
	{
		switch (fp_sensor_type) {
		case CMOSTYPE_CEN931:
			nRet = DeleteFpOne(nID, FingerNum);
			break;
		case CMOSTYPE_OP5:
			nRet = sfm_DeleteFpOne(nID, FingerNum);
			break;
		}
/*		if(nRet == 0)
		{
			ErrorProc(nRet);
			return FALSE;
		}
*/
		if (nRet == 1){
			continue;
		}
	}
	return TRUE;
}


/*--------------------------------------------------------------------------*
@Function			:GetFpList - get FP list
@Include      		: "finger_s10.h"
@Description			:get FP list loaded in FP device										
@Return Value		: Success return a file-stream pointer ,Failure NULL
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
FILE * GetFpList()
{
//	int i=0;
//	char *filepath="note/record/fplist.wts";
//	FILE *fd;
//
//	if( fp_enable == 0||fingerSum == 0)	return NULL;
//	fd=fopen(filepath,"w");
//	if(fd==NULL)
//	 {
//	   return NULL;
//	 }
//
//	for(i=0;i<fingerSum;i++)
//	      fprintf(fd,"%d\r\n",gMatchData[i].ID/10);
//
//	fclose(fd);
//	fd=fopen(filepath,"r");
//	return fd;
  return NULL;
}
 


int InitFp(FPDEVINFO *fpdev_info){
	int ret = 0;

//	if (!fpdev_info){
//		return FALSE;
//	}
    switch(fpdev_info->sensortype){
    case CMOSTYPE_CEN931:
    	printf("cen931\r\n");
    	fp_sensor_type = CMOSTYPE_CEN931;
    	ret = _InitFp(fpdev_info->sensortype,"./lib/fp.so");
    	break;
    case CMOSTYPE_OP5:
    	printf("sfm\n");
    	fp_sensor_type = CMOSTYPE_OP5;
    	ret = sfm_InitFp(atoi(fpdev_info->fingerport),atoi(fpdev_info->fingerbtl));
    	break;
    }

    if (ret == TRUE){
		fp_enable = 1;
    }

    return ret;
}

//load fingerprint template
int LoadFpData(char *nID,int FingerNum,char *FileName){
	int ret = 0;
	switch(fp_sensor_type){
	case  CMOSTYPE_CEN931:
		ret = _LoadFpData(nID,FingerNum,FileName);
		break;
	case CMOSTYPE_OP5:
		ret = sfm_LoadFpData(nID,FingerNum,FileName);
		break;
	}

	return ret;
}


/*--------------------------------------------------------------------------*
@Function			: LoadFingerTemplate - load FP data
@Include      		: "finger_s10.h"
@Description			:
				nID£ºstaff_No£»
				fpath : path of template
				load the FP data to memory
@Return Value		: Success TURE, Failure FALSE
*---------------------------------------------------------------------------*/
int LoadFingerTemplate(char *nID,char *fpath)
{
	DIR *db;
	char filename[64],*buf,tmp1[64],tmp2[64];
	struct dirent *p;
	int len;

	db=opendir(fpath);
	if(db==NULL)		return -1;
	memset(filename,0,64);
	while ((p=readdir(db)))
	 {
	    if((strcmp(p->d_name,".")==0)||(strcmp(p->d_name,"..")==0))
      		 continue;
	    else
      	     {
		 if(strstr(p->d_name,".s10")){
			buf=p->d_name;
		      len=strcspn(buf,"_");
			memset(tmp1,0,sizeof(tmp1));
			strncpy(tmp1,buf,len);	//NO.
		 	if(nID != NULL)
			{
				if(strcmp(nID,tmp1)!=0)	continue;
			}
			buf+=len;
			 buf++;
			len=strcspn(buf,".");
			memset(tmp2,0,sizeof(tmp2));
			strncpy(tmp2,buf,len); 	//FingerNum
			sprintf(filename,"%s%s",fpath,p->d_name);
			LoadFpData(tmp1,atoi(tmp2),filename);
		 }
      	     }
    	    memset(filename,0,64);
 	 }
   closedir(db);
   return 0;
}


int Enroll(char * nID, int FingerNum, char *tpath, char *dpath) {
	int ret = 0;
	switch (fp_sensor_type) {
	case CMOSTYPE_CEN931:
		ret = _Enroll(nID, FingerNum, tpath, dpath);
		break;
	case CMOSTYPE_OP5:
		ret = sfm_Enroll(nID, FingerNum, tpath, dpath);
		break;
	}

	return ret;
}

//check fingerprint template exists
long FpDataOneToNMatch(char *FileName){
	int ret = 0;
	switch (fp_sensor_type) {
	case CMOSTYPE_CEN931:
		ret = _FpDataOneToNMatch(FileName);
		break;
	case CMOSTYPE_OP5:
		ret = sfm_FpDataOneToNMatch(FileName);
		break;
	}

	return ret;
}

long OneToNMatch(char *tpath) {
	int ret = 0;
	switch (fp_sensor_type) {
	case CMOSTYPE_CEN931:
		ret =  _OneToNMatch(tpath);
		break;
	case CMOSTYPE_OP5:
		ret = sfm_OneToNMatch(tpath);
		break;
	}

	return ret;
}

long OneToOneMatch(char *nID,char *tpath){
	int ret = 0;
	switch (fp_sensor_type) {
	case CMOSTYPE_CEN931:
		ret =  _OneToOneMatch(nID,tpath);
		break;
	case CMOSTYPE_OP5:
		ret = sfm_OneToOneMatch(nID,tpath);
		break;
	}

	return ret;
}

int DeleteFpOne(char *nID, int FingerNum){
	int ret = 0;
	switch (fp_sensor_type) {
	case CMOSTYPE_CEN931:
		ret = _DeleteFpOne(nID,FingerNum);
		break;
	case CMOSTYPE_OP5:
		ret = sfm_DeleteFpOne(nID,FingerNum);
		break;
	}

	return ret;
}

int DeleteFpAll(void) {
	int ret = 0;
	switch (fp_sensor_type) {
	case CMOSTYPE_CEN931:
		ret = _DeleteFpAll();
		break;
	case CMOSTYPE_OP5:
		ret = sfm_DeleteFpAll();
		break;
	}

	return ret;
}

int _get_enroll_count() {
	int ret = 0;
	switch (fp_sensor_type) {
	case CMOSTYPE_CEN931:
		ret = __get_enroll_count();
		break;
	case CMOSTYPE_OP5:
		ret = sfm_get_enroll_count();
		break;
	}

	return ret;
}

int UninitFp(void) {
	int ret = 0;
	switch (fp_sensor_type) {
	case CMOSTYPE_CEN931:
		ret = _UninitFp();
		break;
	case CMOSTYPE_OP5:
		ret = sfm_UninitFp();
		break;
	}

	return ret;
}
