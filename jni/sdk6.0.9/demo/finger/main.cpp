//when include dynamic library,parameter -ldl should be added to compiler
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include "../libfunc.h"

//#define _2410
#define _2416

#define	CMOSTYPE_OV7648	0
#define	CMOSTYPE_HV7131R 1
#define CMOSTYPE_EB6048	2
#define CMOSTYPE_OV7670	3
#define CMOSTYPE_CEN931	0

#define CMOSTYPE_OP5 0x33

//load finger Template
int load_finger(char *fpath)
{
	DIR *db;
	char filename[64],*buf,tmp1[64],tmp2[64];
	struct dirent *p;
	int len;
	long nID;

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
			strncpy(tmp1,buf,len); 
			buf+=len;
			 buf++;
			len=strcspn(buf,".");
			memset(tmp2,0,sizeof(tmp2));
			strncpy(tmp2,buf,len); 
			sprintf(filename,"%s%s",fpath,p->d_name);
			//printf("filename %s\n",filename); 
			 nID=FpDataOneToNMatch(filename);
			 if(nID>0)
			 printf("finger is exist:ID=%ld,fingerNum=%ld\n",nID/10,nID%10);
			LoadFpData(tmp1,atoi(tmp2),filename);
		 }
      	}
    	memset(filename,0,64);
 	 }
   closedir(db);
   return 0;
}

//Enroll fingerprint data and save it to corresponding position
int	SaveTempData( long nID, unsigned char * DataBuff ,char *dpath)
{
	FILE * vFile;
	int vSize;
	char szFileName[256];	

	sprintf(szFileName, "%s%ld_%ld.%s",dpath, nID/10,nID%10,"s10");
	vFile = fopen( szFileName, "wb" );
	if( vFile <=0 )
		return 0;	
	
	vSize = fwrite( DataBuff, 1404, 1,  vFile );
	fclose( vFile );

	if( vSize != 1 )
		return 0;/**/
	return 1;
}

//calback function
int performBeforScan(int ScanNum)
{
	if(ScanNum==1)
		printf("enroll pls press finger:%d\n",ScanNum);
	else 		printf("enroll pls press finger agin:%d\n",ScanNum);

	return TRUE;
}
//calback function
int performAfterScan(int ScanNum)
{
	printf("fingerprint picture show:%d\n",ScanNum);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
int _input_number(char* szPrompt, int dwDefault)
{
	int	vKey, vCnt = 0;
	char vStr[16] = {0}, *vStr2;
	int vRet = dwDefault;

	printf("Please input %s [default = %u] ", szPrompt, (unsigned int)dwDefault);

	while(1)
	{
		vKey = getchar();
		if ( vKey >= '0' && vKey <= '9') 
		{
			vStr[vCnt] = (char)vKey;
			vCnt++;
			vStr[vCnt] = 0;
			
			vRet = (int)strtoul(vStr, &vStr2, 10);

			if(vCnt > 10)
				break;
		}
		else if ( vKey == '\n' ) 
		{
			if (vCnt == 0)
				vRet = dwDefault;
			goto RET;
		}
	}
	
	while(1)
	{
		vKey = getchar();
		if ( vKey == 0x0a )
			break;
	}
	
RET:
	return vRet;
}

int main(int argc, char *argv[])
{
	char nID[128];
	long nRet;
	int flag=0,nContinue = 1,vNum=0,fingernum=0;
	FPDEVINFO	fpinfo;

	fpinfo.performBeforScan=performBeforScan;	
	fpinfo.performAfterScan=performAfterScan;
#if defined _2410
	fpinfo.sensortype=2;	//finger type
#elif defined _2416
//	fpinfo.sensortype = CMOSTYPE_CEN931;	//finger type
	fpinfo.sensortype = 0x33;	//finger type
	strcpy(fpinfo.fingerport,"3");
	strcpy(fpinfo.fingerbtl,"115200");
#endif
	if(InitFp(&fpinfo) == FALSE) // initialize fingerprint device
	{
		printf("init error\n");	
		return -1;
	}
	printf( "\n" );
	printf( "-------Menu-------\n" );
	printf( "0  : Exit\n" );
    	printf( "1  : Enroll\n" );
	printf( "2  : 1:N Matching\n" );
	printf( "3  : 1:1 Matching\n" );
	printf( "4  : load fingerprint data\n" );
	printf( "5 :  Delete finger one \n" );
	printf( "6 :  Delete one All\n" );
	printf( "7 :  Delete All\n" );
	printf( "8 : GetEnrollCount\n" );
	printf( "------------------\n" );

   	while( nContinue )
	{
		memset(nID,0,sizeof(nID));
		printf( "\n\n" );
		vNum = (int) _input_number("menu item", 100);

		switch(vNum)
		{
		case 0://Exit
	      		nContinue = 0;
			printf("Exit OK.\n");
			break;
		case 1://Enroll
			printf("===== Enroll\n");
			sprintf(nID,"%d",_input_number("Input staff_No:", 10));
			fingernum=_input_number("Input Finger No:", 10);
#if defined _2410
			flag=Enroll( nID,fingernum,"/dev/shm/zw.bmp","./note/admi/finger/");
#elif defined _2416
			flag=Enroll( nID,fingernum,"/tmp/zw.bmp","./note/admi/finger/");
#endif
			if(flag != TRUE)
			{
				printf("Enroll error:%d\n",flag);
			}
			else 
				printf("Enroll ok:%d\n",flag);
			break;

		case 2://1:N Matching
			printf("===== 1:N Matching\n");
			while(1){
#if defined _2410
				nRet = OneToNMatch("/dev/shm/zw.bmp");
#elif defined _2416
				nRet = OneToNMatch("/tmp/zw.bmp");
#endif
				if(nRet > 0)
				{
					printf("onetoN=%ld\n",nRet);
					break;	
				}
			}
			break;

		case 3://1:1 Matching
			printf("===== 1:1 Matching\n");
			sprintf(nID,"%d",_input_number("Input staff_No:", 10));
			LoadFingerTemplate(nID,"./note/admi/finger/");
			while(1){
#if defined _2410
				nRet=OneToOneMatch(nID,"/dev/shm/zw.bmp");
#elif defined _2416
				nRet=OneToOneMatch(nID,"/tmp/zw.bmp");
#endif
				if(nRet>0)
				{
					printf("onetoone=%ld\n",nRet);
					break;
				}
			}
			DeleteFpOneAll(nID);
			break;
		case 4://load fingerprint data
			printf("===== load fingerprint data\n");
			load_finger("./note/admi/finger/");
			break;
		case 5://Delete
			printf("===== Delete\n");
			sprintf(nID,"%d",_input_number("Input staff_No:", 10));
			fingernum=_input_number("Input Finger No:", 10);

			DeleteFpOne(nID,fingernum);
			DeleteFPFlash(nID,fingernum,"./note/admi/finger/");
			break;
		case 6:
			printf("===== Delete one all\n");
			sprintf(nID,"%d",_input_number("Input staff_No:", 10));

			DeleteFpOneAll(nID);
			DeleteFpOneAllFALSE(nID,"./note/admi/finger/");
			break;
		case 7://DeleteAll
			printf("===== DeleteAll\n");
			DeleteFpAll();
			break;
		case 8:	
			printf("===== GetEnrollCount:%d\n",_get_enroll_count() );
			break;
		case 9:
			printf("===== get FP list loaded in FP\n");
			GetFpList();
			break;
		}
	}

//	GetFpList();							//get fingerprint list
//	str=GetFpDataOne(nID,fpnum);					//get fingerprint template
//	SaveTempData((long)(atol(nID)*10+fpnum),str,"/weds/kq42/");		//save template file

	UninitFp();
	return 0;
}
