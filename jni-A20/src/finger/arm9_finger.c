/**
 * @chinese
 * @file   arm9_finger.c
 * @author 刘训
 * @date   Wed Jul 13 09:37:43 2011
 *
 * @brief 指纹操作模块
 * @endchinese
 *
 * @english
 * @file   arm9_finger.c
 * @author Liu Xun
 * @date   Wed Jul 13 09:37:43 2011
 *
 * @brief finger operation module
 * @endenglish
 */
#include "file.h"
#include "config.h"
#include "public.h"
#include "usr_sem.h"
#include "serial.h"
//#include "libserial.h"
//#include "sound.h"
#include "arm9_finger.h"
#include "debug.h"
#include "watchdog.h"
#include "readcard.h"
#include "supremainc.h"
#ifdef  _ARM_A23
#include "../android/wedsa23.h"
#include "../gpio/gpio_android.h"
#else
#include "../gpio/gpio.h"
#endif
#define APP_CAP_TIMEOUT 10000
#define hlp_printf plog
#define O_BINARY 0
#define SETTINGS_FILE	"./settings.dat"

#define FINGER_MD5 32
int fp_enable=0;		/**< 指纹是否初始化成功 */
char *finger_path = NULL;
char s_finger_path[128];
void *vhfp;
int sec_num;//the number of  fingers in card
enum
{
	HLP_ERR_CANNOT_LOAD_LIB = -101,
	HLP_ERR_CANNOT_LOAD_DB = -102,
	HLP_ERR_ID = -103,
	HLP_ERR_OVER = -104,
};

int gSensorType;
int gRegMax = 0;
FPINFO* gMatchData;
BYTE* gValidFile=NULL;
FPINFO gFeature;

FPINFO IMAGETEMPLATE;
int imagetemplateflag=0;
int imagetemplateenable=0;

int gIDFingers[256];
DWORD* gIDs = NULL;

typedef	int (*FingerBack)(char* path,char * num);

/*处理通讯内容的回掉函数*/
int SetFingerCallBack(FingerBack call_back);
int fingercallback(char *path,char *quality);
FingerBack finger_call_back=NULL;

void get_finger_path(char *finger_name)
{
	char *ptr;
	int num=0,flag=0;
	ptr=finger_name+strlen(finger_name);
	while(ptr>finger_name)
	{
		if(*ptr=='/')
		{
			if(flag==0)
				num++;
			flag=1;
		}
		else
			flag=0;
		if(num==2)
			*ptr=0;
		ptr--;
	}

}



/************************************************************************/
/* ID management                                                        */
/************************************************************************/
/**
 *
 * 获得指纹仪中加载的指纹数量
 *
 * @return 加载的指纹模板数量
 */
int _get_enroll_count()
{
	int i,nCount = 0;

	if(gValidFile==NULL)
		return 0;
	for (i=0; i<gRegMax; i++)
	{
		if(gValidFile[i] != 0)
			nCount++;
	}

	return nCount;
}

/**
 * 在内存空间中删除所有的指纹模板
 *
 */
void _delete_all()
{
	int i;

	if(gValidFile==NULL)
		return;
	for (i=0; i<gRegMax; i++)
	{
		gValidFile[i] = 0;
		memset(&gMatchData[i],0,sizeof(FPINFO));
	}
}
/**
 * 在内存空间中删除指定位置的指纹模板
 *
 * @param pos 指纹模板的位置
 */
void _delete(int pos)
{
	if(gValidFile==NULL)
		return ;
	gValidFile[pos] = 0;
	memset(&gMatchData[pos],0,sizeof(FPINFO));
}
/**
 *
 * 在内存空间中找到第一个为加载指纹模板的位置
 *
 * @return 第一个空闲位置
 */
int _get_empty_pos()
{
	int i;

	if(gValidFile==NULL)
		return 0;
	for (i=0; i<gRegMax; i++)
	{
		if (gValidFile[i] == 0)
			break;
	}

	return i;
}
/**
 *
 * 在内存空间中查找指定人员、指定手指号的指纹模板的位置
 * @param nID 人员序号
 * @param nFingerNum 手指号
 *
 * @return 模板所在的位置
 */
int _get_pos(long nID, long nFingerNum)
{
	int i=-1;

	if (nFingerNum < 0 || nFingerNum > 255 || (gValidFile==NULL)) return gRegMax;

	for (i=0; i<gRegMax; i++)
	{
		if (gValidFile[i] != 0 && gMatchData[i].ID == (DWORD)nID && gMatchData[i].FingerNum == nFingerNum)
			break;
	}
	//    printf("%s,%d,%d,%d,%d,%d,%d\n",__func__,i,gValidFile[i],gMatchData[i].ID,nID,gMatchData[i].FingerNum,nFingerNum);

	return i;
}
/**
 *
 * 在内存空间中查找指定人员的第一个指纹模板的位置
 * @param nID 人员序号
 *
 * @return 模板所在的位置
 */
int _get_posID(long nID)
{
	int i,j = 0;

	if(gValidFile==NULL)
		return 0;
	memset(gIDFingers, -1, sizeof(gIDFingers));

	for (i=0; i<gRegMax; i++)
	{
		if (gValidFile[i] != 0 && gMatchData[i].ID == (DWORD)nID)
			gIDFingers[j++] = i;
	}

	return j;
}
/**
 *
 * 检查某人员某手指的指纹模板是否加载到内存空间
 * @param nID 人员序号
 * @param nFingerNum 手指号
 *
 * @return 0 - 存在  -103 - 不存在
 */
long hlpCheckFingerNum(long nID, long nFingerNum)
{
	int nPos = _get_pos(nID, nFingerNum);
	if (nPos == gRegMax)
		return HLP_ERR_ID;

	return 0;
}

/**
 *
 * 获得登记指纹的内存位置
 * @param nID 人员序号
 * @param nFingerNum 手指号
 * @param Manager 保留，取值：0~255
 *
 * @return 空闲的位置， -103 - 已经录入 -104 - 空间满
 */
//////////////////////////////////////////////////////////////////////////
long hlpEnrollPrepare(long nID, long nFingerNum, long Manager)
{
	int nPos;
	//printf("%s 1\n",__func__);
	if (nFingerNum < 0 || nFingerNum > 255) return HLP_ERR_ID;
	// printf("%s 2\n",__func__);
	if (Manager < 0 || Manager > 255) return HLP_ERR_ID;
	//printf("%s 3\n",__func__);
	nPos = _get_pos(nID, nFingerNum);
	//printf("%s 4,%d,%d\n",__func__,nPos,gRegMax);
	if (nPos != gRegMax)
		return HLP_ERR_ID;

	nPos = _get_empty_pos();
	//printf("%s 5,%d,%d\n",__func__,nPos,gRegMax);
	if (nPos == gRegMax)
		return HLP_ERR_OVER;

	return nPos;
}
/************************************************************************/
/* settings                                                             */
/************************************************************************/
/**
 * 读设置文件中信息
 *
 * @param nSensorType 指纹仪型号
 * @param pdwMechanical  ？？
 * @param pdwExpose  ？？
 */
void _settings_get(int nSensorType, DWORD* pdwMechanical, DWORD* pdwExpose)
{
	int h;
	int dat[3];

	*pdwMechanical = 0;
	*pdwExpose = 0;

	h = open(SETTINGS_FILE, O_BINARY | O_RDONLY);
	if (h<0)
		return;
	lseek(h, 0, SEEK_SET);
	if(read(h, dat, 12) != 12 ||
			dat[0] != nSensorType)
	{
		close(h);
		return;
	}

	*pdwMechanical = dat[1];
	*pdwExpose = dat[2];

	close(h);
}

/**
 * 存储录入指纹模板
 *
 * @param nID 人员ID
 * @param nFingerNum 人员手指号
 *
 * @return 成功-0,失败-(-1)
 */
int _save_template(int nID, int nFingerNum)
{
	FILE * vFile;
	int vSize;
	char szFileName[256];

	memset(szFileName,0,sizeof(szFileName));
	hlp_printf("nID and nFingerNum = %d, %d\n", nID, nFingerNum);
	sprintf(szFileName, "%s/%d/%d_%d.s10", finger_path,nID/1000,nID, (int)nFingerNum);
	hlp_printf("save zw template to path = %s\n", szFileName);
	creat_directory(szFileName);
	vFile = fopen( szFileName, "wb" );
	if( vFile == NULL )
		return -1;

	vSize = fwrite( &gFeature, FPDATASIZE, 1,  vFile );
	fclose( vFile );
	if( vSize != 1 )
		return -1;
	return 0;
}


int _save_template_adapted(int pos)
{
	FILE * vFile=NULL;
	int vSize;
	char szFileName[256];
	long nID=gMatchData[pos].ID;
	int nFingerNum=gMatchData[pos].FingerNum;;
	char Str_md5[36],finger_md5[36];

	memset(szFileName,0,sizeof(szFileName));
	sprintf(szFileName, "%s/%d/%d_%d.s10", s_finger_path,nID/1000,nID, (int)nFingerNum);
	if(MD5_file(szFileName,FPDATASIZE,Str_md5)!=0)
		return -1;
	memset(szFileName,0,sizeof(szFileName));
	sprintf(szFileName, "%s/%d/%d_%d.s10.ad", s_finger_path,nID/1000,nID, (int)nFingerNum);
	creat_directory(szFileName);
	vFile = fopen( szFileName, "wb" );
	if( vFile == NULL )
		return -1;

	vSize = fwrite( &gMatchData[pos], FPDATASIZE, 1,  vFile );

	//2018.4.10 guo  add src finger md5 to adapted file
	fwrite( Str_md5, 32, 1,  vFile );

	fclose( vFile );
	if( vSize != 1 )
		return -1;
	return 0;
}

/**
 * 录入指纹结束,存储指纹模板。
 *
 * @param nID 人员序号
 * @param nFingerNum 手指号
 * @param Manager 保留-管理员
 *
 * @return
 */
long hlpEnrollEnd(long nID, long nFingerNum, long Manager)
{
	int nRet, nPos;

	nRet = SB_FP_ENROLLMERGE(&gFeature);

	if (nRet < 0)
		return nRet;

	nPos = hlpEnrollPrepare(nID, nFingerNum, Manager);
	//    printf("%s,3  %d\n",__func__,nPos);
	if (nPos < 0)
		return nPos;

	gFeature.ID = (DWORD)nID;
	gFeature.FingerNum = (BYTE)(DWORD)nFingerNum;
	gFeature.Manager = (BYTE)(DWORD)Manager;
	gFeature.Valid = 1;

	memcpy(&gMatchData[nPos], &gFeature, sizeof(FPINFO));
	_save_template(nID,nFingerNum);

	gValidFile[nPos] = 1;
	//	_save_valid();

	return 0;
}

//////////////////////////////////////////////////////////////////////////
struct timeval tv1,tv2;
/**
 *
 *
 */
void _tick_stat()
{
	gettimeofday(&tv1, NULL);
}
/**
 *
 *
 *
 * @return
 */
int _tick_end()
{
	gettimeofday(&tv2, NULL);
	return (tv2.tv_sec-tv1.tv_sec)*1000000+(tv2.tv_usec-tv1.tv_usec);
}
/**
 *
 *
 * @param szPrompt
 * @param nTime
 */
void _tick_print(char* szPrompt, int nTime)
{
	hlp_printf("%s %d.%03ds\n",szPrompt,nTime/1000000,(nTime/1000)%1000);
}
/**
 *
 *
 * @param nMilliSecondTimeout
 *
 * @return
 */
BOOL _capture_finger(int nMilliSecondTimeout)
{
	int nRet=-1;
	struct timeval tv3,tv4;

	gettimeofday(&tv3, NULL);
	while(1)
	{
		if(finger_probe())
		{
			nRet = SB_FP_CAPTUREFINGER();

		}
		if(nRet == 0)
		{
			break;
		}

		gettimeofday(&tv4, NULL);

		if((tv4.tv_sec-tv3.tv_sec)*1000+(tv4.tv_usec-tv3.tv_usec)/1000 > nMilliSecondTimeout)
		{
			hlp_printf("Timeout\n");
			return FALSE;
		}
		usleep(10000);
	}

	return TRUE;
}
/**
 *
 *
 * @param nMilliSecondTimeout
 *
 * @return
 */
BOOL valid_capture_finger(int nMilliSecondTimeout)
{
	int nRet=-1,uart3received=0;
	struct timeval tv3,tv4;

	gettimeofday(&tv3, NULL);
	while(1)
	{
		usr_sem_wait();  //挂起
		//        _tick_stat();
		if(check_uart_received(COM3) == 0)
		{
			//指纹仪带有传感器检测是否有手指按下
			if(finger_probe())
			{
				nRet = SB_FP_CAPTUREFINGER();
			}
			if(nRet == 0)
			{
				// finger_power_off();
				break;
			}

		}
		else
		{
			usr_sem_post();
			return -1;
		}

		gettimeofday(&tv4, NULL);

		if(labs((labs(tv4.tv_sec-tv3.tv_sec)*1000+(tv4.tv_usec-tv3.tv_usec)/1000)) > nMilliSecondTimeout)
		{
			usr_sem_post();
			// finger_power_off();
			return FALSE;
		}
		else
			usr_sem_post();
		usleep(10000);
	}
	uart3received = check_uart_received(COM3);
	usr_sem_post(); //释放
	if(uart3received)
	{
		return -1;
	}
	return TRUE;
}
// BOOL valid_capture_finger(int nMilliSecondTimeout)
// {
//     int nRet;

//        nRet = SB_FP_CAPTUREFINGER();
//        if(nRet == 0)
//        {
//                 return TRUE;
//        }
//        return FALSE;
// }

/**
 *
 *
 * @param pnID
 * @param pnFingerNum
 *
 * @return
 */
long hlpIdentify(long* pnID, long* pnFingerNum)
{
	int nRet;
	BOOL bAdapted=FALSE;

	*pnID = 0;
	*pnFingerNum = 0;

	nRet = SB_FP_IDENTIFYIMAGE256(&bAdapted);
	if (nRet < 0)
		return nRet;

	//printf("%s %d\n",__func__,bAdapted);
	if(bAdapted)
	{
		_save_template_adapted(nRet);
	}
	*pnID = gMatchData[nRet].ID;
	*pnFingerNum = gMatchData[nRet].FingerNum;

	return 0;
}
/************************************************************************/
/* bitmap                                                               */
/************************************************************************/
typedef struct tagBITMAPFILEHEADER
{
	WORD bfType;
	DWORD bfSize;
	WORD bfReserved1;
	WORD bfReserved2;
	DWORD bfOffBits;
} BITMAPFILEHEADER;//12

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

typedef struct tagFP_BITMAP
{
	BITMAPFILEHEADER bmfHdr;
	BITMAPINFO bmInfo;
	RGBQUAD bmiColors[255];
} FP_BITMAP;


BITMAPINFOHEADER *m_lpBMIH=NULL;
WORD bfType;


/*--------------------------------------------------------------------------*
@Function			:write_bitmap - save fingerprint image
@Include 			: "finger_s10.h"
@Description			:imgout：path of image；
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

/**
 *
 *
 * @param strImageFileName
 * @param pImage
 * @param nHeight
 * @param nWidth
 *
 * @return
 */
BOOL hlpSaveImageToFile(char* strImageFileName, void* pImage, int nHeight, int nWidth)
{
	BITMAPFILEHEADER bfh;
	RGBQUAD* m_lpvColorTable;
	FILE *fo;
	int i;
	int m_nColorTableEntries=256;
	int m_nBitCount=8;
	char *ptr;

	BYTE  gImgTmp1[nHeight][nWidth];
	if((fo = fopen(strImageFileName, "wb")) == NULL) {
		return FALSE;
	}

	m_nColorTableEntries=256;
	m_nBitCount=8;

	m_lpBMIH = (BITMAPINFOHEADER*) malloc(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * m_nColorTableEntries);
	if(m_lpBMIH == NULL)
	{
		return FALSE;
	}
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

	m_lpvColorTable = (RGBQUAD*)((unsigned char *)m_lpBMIH + sizeof(BITMAPINFOHEADER));
	for (i = 0; i < m_nColorTableEntries; i++)
	{
		BYTE k = m_nColorTableEntries==256?(BYTE)i:(BYTE)i<<4;
		m_lpvColorTable[i].rgbRed      = k;
		m_lpvColorTable[i].rgbGreen    = k;
		m_lpvColorTable[i].rgbBlue     = k;
		m_lpvColorTable[i].rgbReserved = 0;
	}

	// Fill in the Bitmap File Header
	memset(&bfh, 0, sizeof(BITMAPFILEHEADER));
	bfh.bfType = ( (WORD) ('M' << 8) | 'B');
	bfh.bfSize = 14 + sizeof(BITMAPINFOHEADER) +
			m_lpBMIH->biClrUsed * sizeof(RGBQUAD) +
			((((m_lpBMIH->biWidth * m_lpBMIH->biBitCount + 31)/32) * 4) * m_lpBMIH->biHeight);
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
		memcpy(&gImgTmp1[i][0],pImage,nWidth);
		pImage += nWidth;
	}
	fwrite(&gImgTmp1[0][0], bfh.bfSize-14-m_lpBMIH->biClrUsed*sizeof(RGBQUAD), 1, fo);
	fclose(fo);
	if(m_lpBMIH != NULL)
		free(m_lpBMIH);
	m_lpBMIH = NULL;
	return TRUE;
}
/**
 *
 *
 * @param nError
 */
void hlpErrorPrint(int nError)
{
	char* str = "hlp: unknown error\r\n";

	if (nError >= DEV_ERR && nError <= FP_ERR_PARAM)
	{
		SB_FP__PRINTLASTERROR();
		return;
	}

	switch (nError)
	{
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
		break;
	}

	hlp_printf(str);
}


/************************************************************************/
/* Interfaces                                                           */
/************************************************************************/

/**
 * @chinese
 * 初始化指纹
 * @param nSensorType 指纹设备型号
 * @param fp_path 指纹设备路径
 *
 * @return 成功-TRUE,失败-错误代码
 * @endchinese
 *
 * @english
 * initialize finger device
 * @param nSensorType finger deive's type
           Supported master sensors are as follows.
           CMOS	       LED color	Number in API
           OV7640	Green	        0
           HV7131R	Red	            1
           EB6048	Red	            2
           OV7670	Red	            3


           OP5  	Green	        4	// Synel 232 sensor
 * @param fp_path finger device's path
 *
 * @return success-TRUE,fail-error code
 * @endenglish
 *
 *
 */

int InitFp(int nSensorType,char *fp_path)
{
	WORD wVer;
	DWORD dwReleaseDate, dwLibCap;
	int nRet;
	DWORD dwMechanical, dwExpose;

#ifdef DEVELOPER
	if (check_device_info() <= 0){
		return FALSE;
	}
#endif
	SetFingerCallBack(&fingercallback);
	printf("%s,%d,%s\n",__func__, nSensorType,fp_path);
	if(fp_path == NULL)
		return HLP_ERR_CANNOT_LOAD_LIB;

	fp_enable = 0;
	if(nSensorType == 4)
	{
		gSensorType = CMOSTYPE_OP5;

	}
	else
	{
		gSensorType = nSensorType;
	}

#if 1
	if(gSensorType == CMOSTYPE_OP5)
	{
		nRet = sfm_InitFp(1, 115200);
		if(nRet == TRUE)
		{
			fp_enable = 1;
			return TRUE;
		}

		return FALSE;
	}
#endif
	if(gSensorType==9){
		if(PROBEMODE)
			finger_power_off();
	}
	// load fp.so
	hlp_printf("%d: fp_path is %s\n",__LINE__,fp_path);
	vhfp = dlopen( fp_path, RTLD_NOW | RTLD_GLOBAL );
	if (!vhfp) { fprintf(stderr, "dlopen failed: %s\n", dlerror());
	exit(EXIT_FAILURE);
	};
	//if( vhfp == NULL )
	//{
	//	hlp_printf(" %d: error cannot load lib!!!!!!\n",__LINE__);
	//	return HLP_ERR_CANNOT_LOAD_LIB;
	//}

#ifdef _ARM_A23
	fnSB_fp = (pfnENGINE)dlsym( vhfp, "BB_fp" );
#else
	fnSB_fp = (pfnENGINE)dlsym( vhfp, "SB_fp" );
#endif
	if( fnSB_fp == NULL )
	{
		hlp_printf(" %d: error cannot load lib!!!!!!\n",__LINE__);
		if(vhfp!=NULL)
			dlclose(vhfp);
		return HLP_ERR_CANNOT_LOAD_LIB;
	}

	// check version
	wVer = (WORD)SB_FP_GETVERSION(&dwReleaseDate, &dwLibCap);

	gRegMax = dwLibCap;
	if(gIDs != NULL)
	{
		free(gIDs);
		gIDs = NULL;
	}
	gIDs = (DWORD*)malloc(gRegMax * sizeof(DWORD));
	if(gIDs == NULL)
	{
		return HLP_ERR_CANNOT_LOAD_LIB;
	}

	printf("hlp: ver = %04X, release = %08X, regmax = %d\n", wVer, (unsigned int)dwReleaseDate, gRegMax);

	// open
	nRet = SB_FP_OPEN(gSensorType, (BYTE**)&gValidFile, (BYTE**)&gMatchData);
	printf("zw ret = %d\n", nRet);
	if(nRet < 0)
		return nRet;

	// sensor init
	_settings_get(gSensorType, &dwMechanical, &dwExpose);


	nRet = SB_FP_CMOSINIT(dwMechanical, dwExpose);
	printf("zw ret1 = %d\n", nRet);
	if(nRet < 0)
		return nRet;

	fp_enable = 1;
	return TRUE;
}


/**
 * @chinese
 *装载指定文件夹下的所有指纹模板
 *
 * @param fpath 装载路径
 *
 * @return Success TURE, Failure FALSE
 * @endchinese
 *
 * @english
 * load all finger templates from specified directory
 *
 * @param fpath loading path of the specified directory
 *
 * @return Success TURE, Failure FALSE
 * @endenglish
 *
 */
int LoadAllFinger(char *fpath)
{
	DIR *db, *sdb;
	char subdir[64];
	char filename[64],*buf,tmp1[64],tmp2[64];
	struct dirent *p,*sp;
	int len;
	if(fp_enable == 0)
		return FALSE;
	hlp_printf("tmp locate: %s\n",fpath);
	db=opendir(fpath);
	if(db==NULL)		return FALSE;
	memset(filename,0,sizeof(filename));
	while ((p=readdir(db)))
	{
		//send_watchdog_info(FINGER);
		hlp_printf("read dir: %s\n",p->d_name);
		if((strcmp(p->d_name,".")==0)||(strcmp(p->d_name,"..")==0))
			continue;
		else
		{
			//TODO: need to modify this to compare all filenames in that dir...
			sprintf(subdir,"%s/%s", fpath,p->d_name);
			hlp_printf("subdir is: %s\n",subdir);
			sdb = opendir(subdir);
			if(sdb == NULL) {
				continue;
			}
			while (sp = readdir(sdb)) {
				hlp_printf("now reading dir: %s\n",sp->d_name);
				if((strcmp(sp->d_name,".")==0)||(strcmp(sp->d_name,"..")==0))
					continue;
				else {
					if(strstr(sp->d_name,".s10")){
						buf=sp->d_name;
						len=strcspn(buf,"_");
						memset(tmp1,0,sizeof(tmp1));
						strncpy(tmp1,buf,len);
						buf+=len;
						buf++;
						len=strcspn(buf,".");
						memset(tmp2,0,sizeof(tmp2));
						strncpy(tmp2,buf,len);
						sprintf(filename,"%s/%s",subdir,sp->d_name);
						hlp_printf("now loading %s ...\n", filename);
						LoadFpData(tmp1,atoi(tmp2),filename);
					}
				}
			}
		}
		memset(filename,0,sizeof(filename));
	}
	closedir(db);

	return TRUE;
}

/**
 * @chinese
 * 装载指定文件夹下一个人员所有指纹模板
 *
 * @param nID 人员序号
 * @param fpath 装载指纹模板路径
 * @return 成功-1,失败-错误代码
 * @endchinese
 *
 * @english
 * load all finger of one dir
 *
 * @param nID ID
 * @param fpath finger path
 * @return success-TRUE,fail-FALSE
 * @endenglish
 *
 *
 */
int LoadFingerTemplate(char *nID,char *finger_argv,char *fpath)
{
	char *finger,filename[256];
	int num = 0;
	//保存指纹数据加载目录
	strcpy(s_finger_path,fpath);
	////send_watchdog_info(FINGER);
	if(fp_enable == 0 || nID == NULL || finger_argv == NULL)
	{
		return FALSE;
	}

	if(strncmp(finger_argv,"0000000000",10)==0)
		return TRUE;

	memset(filename,0,sizeof(filename));
	finger = finger_argv;
	while(*finger && num<10)
	{
		if(*finger=='0')
		{
			finger++;
			num++;
			continue;
		}
		memset(filename,0,sizeof(filename));
		sprintf(filename,"%s/%ld/%s_%d.s10",fpath,atol(nID)/1000,nID,(int)num);
		LoadFpData(nID,num,filename);
		finger++;
		num++;
	}

	return TRUE;
}

/**
 * @chinese
 * 根据档案装载所有手指的指纹模板
 *
 * @param id_index 人员序号
 * @param fpath 装载路径
 * @param FileName 装载路径
 * @return 成功-1,失败-错误代码
 * @endchinese
 *
 * @english
 * load all finger of wdda
 *
 * @param nID ID
 * @param nFingerNum finger num
 * @param FileName finger path
 * @return success-TRUE,fail-FALSE
 * @endenglish
 *
 *
 */
int load_all_finger_by_wdda(char *filename, char *fpath, int id_index, int finger_index)
{
	FILE *wddafile,*vFile;
	char buf[1024];
	char p[128]={0};
	char nID[64]={0};
	char finger_argv[24]={0},finger_path[128];
	int i=0,len=0,tmp=0;
	int num = 0;
	int vSize;
	int load_finger_pos = 0;
	unsigned char	gFpdataBuff[FPDATASIZE];

	if(fp_enable == 0)
		return FALSE;
	if(id_index<0 || finger_index<0)
	{
		return FALSE;
	}
	if((wddafile=fopen(filename,"rt"))==NULL)
	{
		return FALSE;
	}
	load_finger_pos = 0;
	hlpDeleteAll();

	while(fgets(buf,sizeof(buf),wddafile)!=NULL)
	{
		len=0;
		//解析人员序号列 指纹列
		for(i=0;i<=(id_index>finger_index?id_index:finger_index);i++)
		{
			memset(p,0,sizeof(p));
			tmp=strcspn(buf+len, ",");

			memcpy(p,buf+len,tmp);
			len=tmp+len+1;

			if(i==id_index)
			{
				strcpy(nID,p);
			}
			else if(i==finger_index)
			{
				strcpy(finger_argv,p);
			}
		}
		/**装载指纹**/
		if(load_finger_pos>gRegMax) //装载到指纹最大容量
			break;

		if(strlen(finger_argv) != 10 || strncmp(finger_argv,"0000000000",10)==0)
		{
			continue;
		}
		num=0;
		do
		{
			if(finger_argv[num]=='0')
			{
				continue;
			}
			memset(finger_path,0,sizeof(finger_path));
			sprintf(finger_path,"%s/%ld/%s_%d.s10",fpath,atol(nID)/1000,nID,(int)num);
			vFile = fopen( finger_path, "rb" );
			if(vFile == NULL)
			{
				continue;
			}

			vSize = fread( gFpdataBuff, FPDATASIZE, 1,  vFile );
			if(vSize != 1)
			{
				fclose( vFile );
				continue;
			}

			gValidFile[load_finger_pos]=1;
			memcpy(&gMatchData[load_finger_pos],gFpdataBuff, sizeof(FPINFO));
			gMatchData[load_finger_pos].FingerNum = (BYTE)(DWORD)num;
			gMatchData[load_finger_pos].ID=(DWORD)(atol(nID));
			fclose(vFile);
			load_finger_pos++;
			//装载到指纹最大容量
			if(load_finger_pos>gRegMax)
				break;
		}while(++num<10);
	}
	fclose(wddafile);
	return TRUE;
}

/**
 * @chinese
 * 装载一个人一个手指的指纹模板
 *
 * @param nID 人员序号
 * @param FingerNum 指纹对应手指号
 * @param FileName 装载路径
 * @return 成功-1,失败-错误代码
 * @endchinese
 *
 * @english
 * load one finger of one
 *
 * @param nID ID
 * @param nFingerNum finger num
 * @param FileName finger path
 * @return success-TRUE,fail-FALSE
 * @endenglish
 *
 *
 */


int LoadFpData(char *nID,int FingerNum,char *FileName)
{
	int vSize,nPos=0;
	FILE *vFile=NULL;
	unsigned char	gFpdataBuff[FPDATASIZE];
	char ad_FileName[128];
	char Str_md5[36],Finger_md5[36];
	if(fp_enable == 0)
	{
		return FALSE;
	}
#if 1

	if(gSensorType == CMOSTYPE_OP5 )
	{
		return sfm_LoadFpData(nID,FingerNum,FileName);
	}
#endif
	if((nID==NULL) || access(FileName,R_OK|F_OK) != 0)
	{
		return FALSE;
	}
	nPos = hlpEnrollPrepare((long)(atol(nID)), (long)FingerNum, (long)0);
	if(nPos<0)
	{
		return FALSE;
	}

	sprintf(ad_FileName,"%s.ad",FileName);
	//adapted finger size is FPDATASIZE+32
	if(get_file_size(ad_FileName)==FPDATASIZE+FINGER_MD5){

		vFile = fopen( ad_FileName, "rb" );
		if(vFile)
		{
			fseek(vFile,FPDATASIZE,SEEK_SET);
			fread(Finger_md5,FINGER_MD5,1,vFile);
			if(!(MD5_file(FileName,FPDATASIZE,Str_md5)==0&&strncmp(Str_md5,Finger_md5,FINGER_MD5)==0))
			{
				fclose(vFile);
				vFile=NULL;
				remove(ad_FileName);
			}else {
				fseek(vFile,0L,SEEK_SET);
			}

		}
	}
	if(vFile==NULL)
	{
		vFile = fopen( FileName, "rb" );

	}
	if(vFile == NULL)
	{
		return FALSE;
	}

	vSize = fread( gFpdataBuff, FPDATASIZE, 1,  vFile );
	if(vSize != 1)
	{
		fclose( vFile );
		return FALSE;
	}
	gValidFile[nPos]=1;
	memcpy(&gMatchData[nPos],gFpdataBuff, sizeof(FPINFO));
	gMatchData[nPos].FingerNum = (BYTE)(DWORD)FingerNum;
	gMatchData[nPos].ID=(DWORD)(atol(nID));
	fclose( vFile );
	// printf("%s,%s,%d,%d\n",__func__,nID,FingerNum,gMatchData[nPos].ID);
	return TRUE;
}

int EnrollState=0;

/**
 * @chinese
 * @param nID 人员序号
 * @param FingerNum  指纹对应手指号
 * @param tpath 指纹图像存储路径
 * @param dpath 指纹模板存储路径
 * @param formBeforScan 回调函数,传给当前录入的次数，可以用来控制播放提示声音
 * @param formAfterScan 回调函数,传给当前录入的次数，可以用来控制显示指纹图像
 *
 * @return 成功-TRUE，失败-错误代码
 * @endchinese
 *
 *
 * @english
 * @param nID identify number of a persion
 * @param FingerNum  finger number of a person
 * @param tpath storing path of finger image
 * @param dpath storing path of finger template
 * @param formBeforScan callback funtion,pass `current enrolling's times`,in order to control playing differnt sound file.
 * @param formAfterScan callback function,pass `current enrolling's times`,in order to control showing which finger's image file.
 *
 * @return success-TRUE，fail-error code
 * @endenglish
 *
 *
 */
int Enroll( char * nID ,int FingerNum,char *tpath,char *dpath,
		performBeforScan formBeforScan,performAfterScan formAfterScan )
{
	long i;
	long nRet;
	int flag =0;
	long nPos;
	int j=0;
	int quality;
	char fingerpath[128];

	hlp_printf("Enroll Function at %d, %s, %s\n",__LINE__,tpath,dpath);
	if(fp_enable == 0)
		return FALSE;
	hlp_printf("Enroll Function at %d\n",__LINE__);

#if 1

	if(gSensorType == CMOSTYPE_OP5 )
	{
		return sfm_Enroll(nID, FingerNum, tpath, dpath);
	}
#endif	
	if(uart3_transport == UART3)
		return -101;
	finger_enroll = ENROLL;
	int fc = _get_enroll_count();
	hlp_printf("Enroll count is %d\n",fc);
	if(fc == gRegMax )
	{
		hlp_printf("Database overflow\n");
		serial_clear(COM3);
		finger_enroll = FREE;
		return -1;
	}
	nPos = _get_pos(nID, FingerNum);
	hlp_printf("nPos is %d\n",nPos);
	if (nPos != gRegMax)
		return -7;
	if((nRet = SB_FP_ENROLLSTART()) < 0)
	{
		serial_clear(COM3);
		finger_enroll = FREE;
		hlpErrorPrint(nRet);
		return -2;
	}

	hlp_printf("Enroll: I am at %d\n",__LINE__);
	for(i=1;i<=3;)
	{
		////send_watchdog_info(FINGER_ENROLL);
		formBeforScan(i);
		flag = _capture_finger(APP_CAP_TIMEOUT);
		if (flag == FALSE)
		{
			serial_clear(COM3);
			finger_enroll = FREE;
			return -3;
		}
		if(tpath){
			hlpSaveImageToFile(tpath,SB_FP__256IMAGE,256,256);
			//sprintf(fingerpath,"/weds/kq42/note/finger/%d.bmp",i);
			// hlpSaveImageToFile(fingerpath,SB_FP__LIVEIMAGE,480,640);
		}
		formAfterScan(i);
		nRet=SB_FP_ENROLLNTH256(i);
		hlp_printf("%s ret=%d,quality=%d\n",__func__,nRet,SB_FP_GET_QUALITY(SB_FP__256IMAGE));
		if(FP_ERR_QUALITY==nRet&&j<9)
		{
			j++;
			continue;
		}
		if(nRet < 0)
		{
			serial_clear(COM3);
			finger_enroll = FREE;
			hlpErrorPrint( nRet );
			return -4;
		}
		else if(nRet > 0)   //指纹以存在
		{
			serial_clear(COM3);
			finger_enroll = FREE;
			hlp_printf( "Finger Duplicated: ID = %d\n", gMatchData[nRet-1].ID );
			return -5;
		}
		else
			hlp_printf( "Enroll OK\n");
		if(i<3)
		{
			hlp_printf( "Please take off finger\n");
			while(SB_FP_ISPRESSFINGER() == 0)
				finger_probe();
		}
		i++;
	}//end for
	finger_path = dpath;
	if(hlpEnrollEnd((long)atol(nID),(long)FingerNum,0) < 0)
	{
		serial_clear(COM3);
		finger_enroll = FREE;
		return -6;
	}

	serial_clear(COM3);
	finger_enroll = FREE;
	return TRUE;
}

int fingercallback(char *path,char* quality)
{
	printf("%s,%s,%s\n",__func__,path,quality);
}

/*处理通讯内容的回掉函数*/
int SetFingerCallBack(FingerBack call_back)
{
	finger_call_back=call_back;
}

/**
 * @chinese
 * 指纹1:N比对方式
 *
 * @param tpath 指纹图像存储路径
 *
 * @return 成功-人员序号+手指号,失败-错误代码
 * @endchinese
 *
 * @english
 * finger 1:N match method
 *
 * @param tpath storing path of finger image
 *
 * @return success:person's id-number+finger number,fail:error code
 * @endenglish
 *
 */

int getfingername(char *str)
{
	struct tm *tm;
	time_t timep;

	time(&timep);
	tm=localtime(&timep);
	//memset(str,0,sizeof(str));
	sprintf(str,"%04d%02d%02d%02d%02d%02d\0",tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour, tm->tm_min, tm->tm_sec);
	return 0;
}

long OneToNMatch(char *tpath)
{
	long nRet,nRet2;
	DWORD ID=0, FingerNum=0;
	char tmppath[128],tmppath1[128],tmppath2[128];
	static int debug_num=0;
	static int debug_start=0;
	char curtime[32],finger_zp[128],tmpbuf[32];
	getfingername(curtime);
	//    if(debug_start==0)
	//    {
	//        if(strncmp(curtime,"20180410",8)<=0)
	//        system("touch /tmp/debug.flag");
	//    }else {
	//        if(strncmp(curtime,"20180410",8)>0){
	//            system("rm /tmp/debug.flag");
	//            debug_start=0;
	//        }
	//    }
	printftime(NULL);
	sprintf(finger_zp,"%s/weds/kq42/note/record/finger_zp.wds",WORKPATH);
	if(access("/tmp/debug.flag",F_OK)==0&&debug_start==0){
		debug_start=1;

		sprintf(tmppath1,"%s/weds/kq42/note/frame/finger_photo",WORKPATH);
		if(jupk_upload_file_start(2,finger_zp,tmppath1,"*#\\wwwroot\\frame\\finger_photo\\*$\\*0")==0)
			printf("upload_file start ok\n");
	}

	if( fp_enable == 0)
		return -1;
#if 1

	if(gSensorType == CMOSTYPE_OP5 )
	{
		nRet = sfm_OneToNMatch(tpath);
		if (nRet == -100)
		{
			hlp_printf("NG:%ld!\n",nRet);
			return FP_ERR_IDENTIFY;
		}

		return nRet;	// ID
	}
#endif
	_tick_stat();
	if (valid_capture_finger(0) != TRUE)
	{
		_tick_print("valid_capture", _tick_end());
		return FP_ERR_NOT_PRESSED;
	}

	// printf("%s,quality=%d\n",__func__,SB_FP_GET_QUALITY(SB_FP__256IMAGE));
	if(tpath)
	{
		hlpSaveImageToFile(tpath, SB_FP__256IMAGE, 256, 256);
		memset(tmppath,0,sizeof(tmppath));
		sprintf(tmppath,"%s.live.bmp",tpath);
		hlpSaveImageToFile(tmppath,SB_FP__LIVEIMAGE,480,640);
		if(finger_call_back)
		{
			//sprintf(tmpbuf,"%d",SB_FP_GET_QUALITY(SB_FP__256IMAGE));
			sprintf(tmpbuf,"%d",0);
			finger_call_back(tpath,tmpbuf);
		}
	}

	nRet = hlpIdentify((long*)&ID, (long*)&FingerNum);
	// _tick_print("hlpIdentify", _tick_end());
	if(imagetemplateenable)
	{
		nRet2=SB_FP_PROCESSIMAGE256(&IMAGETEMPLATE);//提取指纹模板，供保存用
		if(nRet2<0)
			imagetemplateflag=0;
		else {
			imagetemplateflag=1;
		}
	}

	if(access("/tmp/debug.flag",F_OK)==0)
	{
		//printf("%s,3\n",__func__);
		getfingername(curtime);
		sprintf(tmppath2,"%s/weds/kq42/note/frame/finger_photo/%s_%ld.bmp",WORKPATH,curtime,ID*10+FingerNum);
		creat_directory(tmppath2);
		cp_file(tmppath,tmppath2);
		sprintf(tmppath2,"%s_%ld.bmp",curtime,ID*10+FingerNum);
		RecFile_Write(finger_zp,tmppath2);
	}
	if (nRet < 0)
	{
		hlp_printf("NG:%ld!\n",nRet);
		return FP_ERR_IDENTIFY;
	}
	//printftime("finger success");
	return ID*10+FingerNum;
}

/**
 * @chinese
 * 指纹识别时直接提前指纹模板
 *
 * @param tpath 指纹图像存储路径
 *
 * @return 成功,失败-错误代码
 * @endchinese
 *
 * @english
 * finger 1:N match method
 *
 * @param tpath storing path of finger image
 *
 * @return success:person's id-number+finger number,fail:error code
 * @endenglish
 *
 */
int ImageToTemplate(char *path)
{
	FILE *file;
	int nRet;
	if(imagetemplateflag==0)
		return FALSE;
	file=fopen(path,"w");
	if(!file)return FALSE;
	nRet=fwrite(&IMAGETEMPLATE,sizeof(IMAGETEMPLATE),1,file);
	fclose(file);
	if(nRet!=1)
	{
		remove(path);
		return FALSE;
	}
	return TRUE;
}
/**
 * @chinese
 * 是否开启指纹识别时直接提取指纹模板功能
 *
 * @param mode ,0-关闭，1-启用
 *
 * @return 成功,失败-错误代码
 * @endchinese
 *
 * @english
 * finger image to template mode
 *
 * @param 0-close,1-open
 *
 * @return
 * @endenglish
 *
 */
void ImageTemplateMode(int flag)
{
	if(flag==0)
		imagetemplateenable=0;
	else imagetemplateenable=1;
}

/**
 * @chinese
 * 指纹1:1比对方式
 *
 * @param nID 人员序号
 * @param FingerNum 人员手指号
 * @param tpath 指纹图像存储路径
 * @return 成功-1,失败-错误代码
 * @endchinese
 *
 * @english
 * one to one match
 *
 * @param nID ID
 * @param nFingerNum finger num
 * @param tpath finger path
 * @return success-TRUE,fail-FALSE
 * @endenglish
 *
 *
 */
long OneToOneMatch(char *nID,int FingerNum,char *tpath)
{
	int nRet=0,nPos=0;
	long ID = 0 ;
	DWORD bAdapted;

	if( fp_enable == 0||nID==NULL)
		return -1;
#if 1
	if(gSensorType == CMOSTYPE_OP5 )
	{
		return sfm_OneToOneMatch(nID, tpath);
	}
#endif	
	ID = atol(nID);
	if ((nRet = hlpCheckFingerNum((long)ID, FingerNum)) < 0)
	{
		return nRet;
	}

	if (valid_capture_finger(0) != TRUE)
	{
		return FP_ERR_NOT_PRESSED;
	}

	if(tpath)
	{
		hlpSaveImageToFile(tpath, SB_FP__256IMAGE, 256, 256);
	}

	nPos = _get_pos((long)ID, FingerNum);
	hlp_printf("nPos is %d, gRegMax is %d\n",nPos,gRegMax);
	if (nPos == gRegMax)
		return HLP_ERR_ID;

	nRet = SB_FP_VERIFYIMAGE256(nPos, &bAdapted);
	hlp_printf("%d: nRet is %d\n",__LINE__,nRet);

	if(nRet<0)
	{
		return 0;
	}

	return 1;
}

char *old_id=NULL;
/**
 * @chinese
 *指纹1:1比对方式
 *
 * @param nID 人员序号
 * @param fppath 指纹模板存储路径
 * @param tpath 指纹图像存储路径
 *
 * @return 成功-1,失败-错误代码
 * @endchinese
 *
 * @english
 * finger 1:1 match method
 *
 * @param nID identify number of a person
 * @param fppath storing path of finger template
 * @param tpath storing path of finger images
 *
 * @return success:1,fail:error code
 * @endenglish
 *
 */
int hlpOneToOneMatch(char *nID,char *finger_argv,char *fppath,char *tpath)
{
	int flag =0;

	if(nID == NULL || finger_argv == NULL ||
			fppath == NULL || tpath == NULL)
	{
		return 0;
	}

	if(old_id == NULL || strcmp(old_id,nID)!=0)
	{
		old_id = nID;
		hlpDeleteAll();
		flag = LoadFingerTemplate(nID,finger_argv,fppath);
		if(flag == FALSE)
		{
			return 0;
		}
	}
	flag = OneToNMatch(tpath);
	if(flag < 0)
		return 0;
	return 1;

}

/**
 * @chinese
 *从FLash和指纹设备中删除一个人一个指纹模板
 *
 * @param nID 人员序号
 * @param FingerNum 人员指纹号
 * @param dpath 指纹模板存放路径
 *
 * @return 成功-TRUE,失败-FALSE
 * @endchinese
 *
 * @english
 * delete a finger template of a persion from Flash adn finger device
 *
 * @param nID identify number of a person
 * @param FingerNum finger number of a person
 * @param dpath the path of storing finger templates
 *
 * @return success-TRUE,fail-FALSE
 * @endenglish
 *

 */
int hlpDeleteFlash(char* nID, int FingerNum,char *dpath)
{
	char szFileName[256];

	if(fp_enable == 0)
		return FALSE;

	if(hlpDelete(nID,FingerNum) < 0 )
		return FALSE;

	memset(szFileName,0,sizeof(szFileName));
	sprintf(szFileName, "%s/%ld/%s_%d.s10", dpath,atol(nID)/1000,nID, (int)FingerNum);
	remove(szFileName);                       //delete temporary files
	return TRUE;
}


/**
 * @chinese
 *从FLash和指纹设备中删除一个人所有指纹模板
 *
 * @param nID 人员序号
 * @param dpath 指纹模板存放路径
 *
 * @return 成功-TRUE,失败-FALSE
 * @endchinese
 *
 * @english
 * delate all finger templates of a person from Flash adn finger device
 *
 * @param nID identify number of a person
 * @param dpath the path of storing finger templates
 *
 * @return success-TRUE,fail-FALSE
 * @endenglish
 *
 */
int hlpDeleteFpIdFlash(char* nID,char *dpath)
{
	int i,nCount = 0;

	if(fp_enable == 0)
		return FALSE;

	nCount = _get_posID(atol(nID));
	if (nCount == 0)
		return HLP_ERR_ID;

	for (i=0; i<nCount; i++)
	{
		_delete(gIDFingers[i]);
		hlpDeleteFlash(nID,i,dpath);
	}

	return TRUE;
}

/**
 * @chinese
 * 从指纹设备中删除一个人一个指纹模板
 *
 * @param nID 人员序号
 * @param nFingerNum 人员指纹号
 * @return 成功-TRUE,失败-FALSE
 * @endchinese
 *
 * @english
 * delete one finger templates of one body from finger device
 *
 * @param nID ID
 * @param nFingerNum finger num
 * @return success-TRUE,fail-FALSE
 * @endenglish
 *
 *
 */
long hlpDelete(char* nID, long nFingerNum)
{
	int nPos = 0;
	long ID;

	if(nID == NULL || fp_enable == 0)
	{
		return FALSE;
	}
#if 1

	if(gSensorType == CMOSTYPE_OP5 )
	{
		return sfm_DeleteFpOne(nID, nFingerNum);
	}
#endif
	ID = atol(nID);
	nPos = _get_pos(ID, nFingerNum);
	if (nPos == gRegMax)
	{
		return HLP_ERR_ID;
	}

	_delete(nPos);
	return 0;
}

/**
 * @chinese
 * 从指纹设备中删除一个人所有指纹模板
 *
 * @param nID 人员序号
 * @return 成功-TRUE,失败-FALSE
 * @endchinese
 *
 * @english
 * delete all finger templates of one body from finger device
 *
 * @param nID ID
 * @return success-TRUE,fail-FALSE
 * @endenglish
 *
 *
 */
int hlpDeleteID(char *nID)
{
	long ID;
	int FingerNum=0;
	int i=0,nCount = 10,nPos=0;

	if(nID == NULL || fp_enable == 0)
	{
		return FALSE;
	}

	if(gSensorType == CMOSTYPE_OP5 )
	{
		for(FingerNum = 0; FingerNum <= 9; FingerNum++)
		{
			sfm_DeleteFpOne(nID, FingerNum);
		}		

		return TRUE;
	}

	ID = atol(nID);
	for (i=0; i<nCount; i++)
	{
		nPos = _get_pos(ID,(long)i);
		if(nPos == gRegMax)
		{
			continue;
		}
		_delete(nPos);
	}

	return TRUE;
}


/**
 * @chinese
 *从指纹设备中删除所有指纹模板
 *
 * @return 成功-TRUE,失败-FALSE
 * @endchinese
 *
 * @english
 * delete all finger templates from finger device
 *
 * @return success-TRUE,fail-FALSE
 * @endenglish
 *
 *
 */
int hlpDeleteAll(void)
{
	if(fp_enable == 0)
	{
		return FALSE;
	}
#if 1

	if(gSensorType == CMOSTYPE_OP5 )
	{
		return sfm_DeleteFpAll();
	}
#endif	
	_delete_all();
	return TRUE;
}


/**
 * @chinese
 *关闭指纹设备
 *
 * @return Success TRUE,Failure FALSE
 * @endchinese
 *
 * @english
 * close finger device
 *
 * @return Success:TRUE,Failure:FALSE
 * @endenglish
 *
 *
 */
int hlpClose()
{
	int retval = 0;

	hlpDeleteAll();
	fp_enable = 0;

	retval = SB_FP_CLOSE();
	if(vhfp!=NULL)
	{
		dlclose(vhfp);
	}
	if(retval !=0 )
	{
		return FALSE;
	}
	return TRUE;
}

/**
 * @chinese
 * 获得指纹设备能存储指纹模板大小
 *
 * @return 指纹模板大小
 * @endchinese
 *
 * @english
 * get gRegMax
 *
 * @return gRegMax
 * @endenglish
 *
 *
 */

extern int g_available_finger;

int hlpGetRegMax()
{
	if(gSensorType == CMOSTYPE_OP5 )
	{
		return g_available_finger;
	}

	return gRegMax;
}

/**
 * @chinese
 * 获得录入的指纹模板个数
 *
 * @return 录入指纹模板个数
 * @endchinese
 *
 * @english
 * get enroll num
 *
 * @return enroll num
 * @endenglish
 *
 *
 */
int hlpGetEnrollCount()
{
#if 1

	if(gSensorType == CMOSTYPE_OP5 )
	{
		return sfm_get_enroll_count();
	}
#endif	
	return _get_enroll_count();
}

/**
 * @chinese
 * 获得某个指纹
 *
 * @param nID 员工号
 * @param FingerNum 指纹号
 * @return
 * @endchinese
 *
 * @english
 * get one finger
 *
 * @param nID ID
 * @param FingerNum finger num
 *
 * @return
 * @endenglish
 *
 *
 */
unsigned char *GetFpDataOne(char *nID,int FingerNum)
{
	long anpos;
	int nPos;
	static unsigned char	gFpdataBuff[FPDATASIZE];

	hlp_printf("nID is %s, fingernum is %d\n",nID, FingerNum);
	if( fp_enable == 0||nID == NULL)	return NULL;

	anpos = (atol(nID));
	nPos = _get_pos(anpos, (long)FingerNum);
	hlp_printf("%d: nPos is %d\n",__LINE__,nPos);
	if (nPos == gRegMax)
	{
		return FALSE;
	}
	memset(&gFpdataBuff[0],0,sizeof(FPINFO));
	memcpy(&gFpdataBuff[0],&gMatchData[nPos], sizeof(FPINFO));	//1404
	return &gFpdataBuff[0];
}

/*--------------------------------------------------------------------------*
@Function			:GetFpList - get FP list dispose
@Include      		: "finger_s10.h"
@Description			:get FP list loaded in FP device
@Return Value		: Success return a file-stream pointer ,Failure NULL
@Create time			: 2009-06-15 08:23
 *---------------------------------------------------------------------------*/
FILE * GetFpList()
{
	plog("get fp list\n");

	return 0;
}

/*--------------------------------------------------------------------*
@Function			: setFpData - load FP template
@Include      		: "finger_s10.h"
@Description			: nID staff_No.
                FingerNum	finger_No.
                str template data
                load FP template to device
@Return Value		: Success TRUE,Failure FALSE
@Create time			: 2009-06-15 08:23
 *---------------------------------------------------------------------*/
/**
 *
 * 根据指纹模板把指纹数据加载到指纹设备中
 * @param nID 人员序号
 * @param FingerNum 手指号
 * @param str 指纹模板内容
 *
 * @return 1 - 成功 0 - 失败
 */
int setFpData(char *nID,int FingerNum,unsigned char *str)
{
	long anpos;
	int nRet=0;

	if( fp_enable == 0||str==NULL)
	{
		return FALSE;
	}
	anpos = atol(nID);
	nRet = hlpEnrollPrepare(anpos, (long)FingerNum, 0);
	if(nRet<0)
	{
		return FALSE;
	}
	gValidFile[nRet]=1;
	memcpy(&gMatchData[nRet],str, sizeof(FPINFO));
	gMatchData[nRet].FingerNum = (BYTE)(DWORD)FingerNum;
	gMatchData[nRet].ID=anpos;
	return TRUE;
}
/**
 * Read finger datas in s70 card after reading the card number
 * @param:port_number---card reader connected port
 * @return:-1 error >0 the number of fingers in card
 */
int read_s70_finger(int port_number)
{
	int pnext_finger = 0, p_compress = 0, sum = 0, sum_buf = 0, i = 0, vSize = 0;
	int sec_end=0,sec_end_moc=0,sec_begin = 33, data_len=0;
	char sec_buf[10],buf[2048];
	unsigned char sec_des_buf[10];
	FILE *vFile=NULL;
	FILE *fp = NULL;
	int recv_count = 0;
	pnext_finger = 1;
	if(!fp_enable)
		return -1;
	sec_num = 0;
	while(pnext_finger)
	{
		memset(buf, 0, sizeof(buf));
		memset(sec_buf, 0, sizeof(sec_buf));
		memset(sec_des_buf, 0, sizeof(sec_des_buf));

		recv_count = mf1_read_sectors_bk(port_number,sec_begin,sec_begin,0,0,0X01,"FFFFFFFFFFFF",buf,3);
		if((buf[0]!=0XBE) || (recv_count <= 0))
		{
			return -1;
		}
		p_compress = buf[1]&0x01;
		pnext_finger = (buf[1]>>1)&0x01;

		sprintf(sec_buf,"%02X%02X",buf[3],buf[2]);
		gsmString2Bytes(sec_buf, sec_des_buf, strlen(sec_buf));
		data_len = HexToUInt(sec_des_buf, 2, 1)+4;
		if(data_len<=6)
		{
			return -1;
		}
		sec_end = data_len/16/15;
		sec_end_moc = data_len%(16*15);

		if(sec_end_moc!=0)
			sec_end += 1;
		memset(buf, 0, sizeof(buf));
		recv_count = mf1_read_sectors_bk(port_number,sec_begin,sec_begin+sec_end-1,0,14,0X01,"FFFFFFFFFFFF",buf,3);
		if (recv_count<data_len) return -1;
		sec_begin += sec_end;
		sum = buf[data_len-2]&0xff;

		for(i=0;i<data_len-6;i++)
			sum_buf += buf[4+i];
		sum_buf = sum_buf&0xff;
		if((sum_buf != sum)||(buf[0]!=0XBE)||(buf[data_len-1]!=0XEE))
		{
			return -1;
		}
		if(p_compress == 1)
		{
			fp = fopen("1.gz","w+");
			fwrite(&buf[4],1,data_len-6,fp);
			fclose(fp);
			system("gunzip -c 1.gz > 1.s10");

			memset(buf, 0, sizeof(buf));
			vFile = fopen( "1.s10", "rb" );
			if(vFile == NULL)
			{
				return -1;
			}
			vSize = fread( buf, 1404, 1, vFile );
			if(vSize != 1)
			{
				fclose( vFile );
				return -1;
			}
		}

		memset(&gMatchData[sec_num],0,sizeof(FPINFO));

		gValidFile[sec_num]=1;		//finger badge
		if(p_compress == 1)
			memcpy(&gMatchData[sec_num],buf, sizeof(FPINFO));
		memcpy(&gMatchData[sec_num],&buf[4], sizeof(FPINFO));
		sec_num++;
	}
	return sec_num;
}
/**
 * Finger match after reading finger datas in a card
 * @param:seconds--- over time (unit:second)
 * @return:1 match success
 * 		   0 match failed
 */
int finger_one_to_one_match(int seconds)
{
	int i = 0, nRet = 0;
	unsigned long bAdapted;
	int overtime=0;
	struct timeval tv1_finger,tv2_finger;
	if(seconds <= 0) seconds = 3;
	gettimeofday(&tv1_finger,NULL);
	gettimeofday(&tv2_finger,NULL);

	overtime=tv1_finger.tv_sec *1000000+tv1_finger.tv_usec;
	while(1)
	{
		if(abs( (tv2_finger.tv_sec *1000000 + tv2_finger.tv_usec) - overtime) > seconds*1000000)
		{
			return ERROR;
		}
		gettimeofday(&tv2_finger,NULL);
		if (valid_capture_finger(0) == TRUE)
		{
			for(i=0;i<sec_num;i++)
			{
				nRet = SB_FP_VERIFYIMAGE256(i, &bAdapted);
				if(nRet<0)
				{
					continue;
				}
				else
				{
					return 1;
				}
			}
			break;
		}
		//    	usleep(4000);
	}

	return 0;
}


int finger_backlight_time=500; //探测到指纹后，指纹背光打开时长，单位ms
int set_finger_backlight_time(int backlight_time)
{
	finger_backlight_time=backlight_time;
}

int finger_probe()
{
	if(PROBEMODE==0)
		return TRUE;
	if(get_import_state(FINGER_PROBE)==TRUE){
		turn_on_gpio(finger_backlight_time,FINGER_POWER);
		return TRUE;
	}
	//    else {
	//        set_gpio_off(FINGER_POWER);
	//        return FALSE;
	//    }
}

int finger_power_on()
{
	if(PROBEMODE==0)
		return TRUE;
	set_gpio_on(FINGER_POWER);
}
int finger_power_off()
{
	if(PROBEMODE==0)
		return TRUE;
	set_gpio_off(FINGER_POWER);
}

int SetFingerProbeMode(int mode)
{
	PROBEMODE=mode;
}

DWORD * _gId_Iterator()
{
	int i,j = 0;

	for (i=0; i<gRegMax; i++)
	{
		if (gValidFile[i] != 0){
			gIDs[j++] = gMatchData[i].ID;
			//hlp_printf("ID Found:%d\n", gMatchData[i].ID);
		}
	}
	return gIDs;
}

DWORD *  hlpIdIterator()
{
	return _gId_Iterator();
}
