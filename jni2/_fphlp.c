#include "_precomp.h"
#include "_fp.h"
#include "_fphlp.h"

#include <dirent.h>

#define O_BINARY 0

/************************************************************************/
/* defines                                                              */
/************************************************************************/
#define hlp_printf printf

#define SETTINGS_FILE	"./settings.dat"
#define MATCH_DATA_FILE	"./enroll.dat"
#define VALID_DATA_FILE	"./valid.dat"

enum
{
	HLP_ERR_CANNOT_LOAD_LIB = -101,
	HLP_ERR_CANNOT_LOAD_DB = -102,
	HLP_ERR_ID = -103,
	HLP_ERR_OVER = -104,
};

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

	hlp_printf("%s",str);
}

/************************************************************************/
/* ID management                                                        */
/************************************************************************/
int gSensorType;
int gRegMax = 1;
FPINFO* gMatchData;
BYTE* gValidFile;
FPINFO gFeature;

int gIDFingers[256];
DWORD* gIDs = NULL;

int _get_enroll_count()
{
	int i,nCount = 0;

	for (i=0; i<gRegMax; i++)
	{
		if(gValidFile[i] != 0)
			nCount++;
	}
	return nCount;
}

void _delete_all()
{
	int i;

	for (i=0; i<gRegMax; i++)
	{
		gValidFile[i] = 0;
	}
}

void _delete(int pos)
{
	gValidFile[pos] = 0;
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

int _get_pos(long nID, long nFingerNum)
{
	int i;

	if (nFingerNum < 0 || nFingerNum > 255) return gRegMax;

	for (i=0; i<gRegMax; i++)
	{
		if (gValidFile[i] != 0 && gMatchData[i].ID == (DWORD)nID && gMatchData[i].FingerNum == nFingerNum)
			break;
	}

	return i;
}

int _get_posID(long nID)
{
	int i,j = 0;

	memset(gIDFingers, -1, sizeof(gIDFingers));

	for (i=0; i<gRegMax; i++)
	{
		if (gValidFile[i] != 0 && gMatchData[i].ID == (DWORD)nID)
			gIDFingers[j++] = i;
	}

	return j;
}

int _compare( const void *arg1, const void *arg2 )
{
	if (*(DWORD*)arg1 > *(DWORD*)arg2)
		return 1;

	if (*(DWORD*)arg1 < *(DWORD*)arg2)
		return -1;

	return 0;
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

DWORD _searchID()
{
	DWORD* ids = NULL;
	DWORD dwRet;
	int i,j = 0;
	ids = _gId_Iterator();

	if (0 == ids[0])
		return 0;
	j = sizeof(ids)/sizeof(DWORD);
	qsort(ids, j, sizeof(DWORD), _compare);

	dwRet = ids[0]+1;
	for (i=1; i<j; i++)
	{
		if (dwRet != ids[i])
			break;
		dwRet ++;
	}

	return dwRet;
}

DWORD _searchFN(long nID)
{
	int i,j = _get_posID(nID);
	DWORD dwRet;

	if (j==0)
		return -1;

	for (i=0; i<j; i++)
		gIDFingers[i] = (DWORD)gMatchData[gIDFingers[i]].FingerNum;

	qsort(gIDFingers, j, sizeof(DWORD), _compare);

	dwRet = gIDFingers[0]+1;
	for (i=1; i<j; i++)
	{
		if (dwRet != (DWORD)gIDFingers[i])
			break;

		dwRet ++;
	}

	return dwRet;
}

/************************************************************************/
/* settings                                                             */
/************************************************************************/
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

void _settings_set(int nSensorType, DWORD dwMechanical, DWORD dwExpose)
{
	int h;
	int dat[3] = {nSensorType, dwMechanical, dwExpose};

	h = open(SETTINGS_FILE, O_BINARY | O_RDWR | O_CREAT, S_IREAD | S_IWRITE);
	if (h<0)
		return;
	lseek(h, 0, SEEK_SET);
	write(h, dat, 12);

	close(h);
}

/************************************************************************/
/* database                                                             */
/************************************************************************/
BOOL _load_database()
{
	int i,h;
	hlp_printf("%s: called \n",__func__);
	memset(&gValidFile[0], 0, gRegMax);

	h = open(VALID_DATA_FILE, O_BINARY | O_RDWR | O_CREAT, S_IREAD | S_IWRITE);
	if (h<0)
		return FALSE;
	lseek(h, 0, SEEK_SET);
	read(h, &gValidFile[0], gRegMax);
	close(h);

	h = open(MATCH_DATA_FILE, O_BINARY | O_RDWR | O_CREAT, S_IREAD | S_IWRITE);
	if (h<0)
		return FALSE;
	lseek(h, 0, SEEK_SET);

	hlp_printf("%s: hlp: Loading FPU database.",__func__);
	for (i=0; i<gRegMax; i++)
	{
		read(h, &gMatchData[i], sizeof(FPINFO));

		if ((i+1) % 100 == 0)
		{
			hlp_printf(".");
			fflush(stdout);
		}
	}
	hlp_printf("done\n");

	close(h);
	return TRUE;
}

void _save_template(int nPos)
{
	int h;

	h = open(MATCH_DATA_FILE, O_BINARY | O_RDWR | O_CREAT, S_IREAD | S_IWRITE);
	if (h<0)
		return;
	lseek(h, nPos * sizeof(FPINFO), SEEK_SET);
	write(h, &gMatchData[nPos], sizeof(FPINFO));
	close(h);
}

void _save_valid()
{
	int h;
	hlp_printf("%s: called \n",__func__);
	h = open(VALID_DATA_FILE, O_BINARY | O_RDWR | O_CREAT, S_IREAD | S_IWRITE);
	if (h<0)
		return;
	lseek(h, 0, SEEK_SET);
	write(h, &gValidFile[0], gRegMax);
	close(h);
}

/************************************************************************/
/* Interfaces                                                           */
/************************************************************************/
long hlpOpen(char* szLibSuffix, int nSensorType) 
{
	char szLibName[32];
	void *vhfp;
	WORD wVer;
	DWORD dwReleaseDate, dwLibCap;
	int nRet;
	DWORD dwMechanical, dwExpose;

	gSensorType = nSensorType;

	// load fp.so
	sprintf(szLibName, "/usr/lib/fpu/fp_%s.so", szLibSuffix);
	hlp_printf("loading lib %s \n",szLibName);
	vhfp = dlopen( szLibName, RTLD_NOW );
	if( vhfp == NULL ){
		hlp_printf("%s: can not load lib %s\n",__func__,szLibName);
		return HLP_ERR_CANNOT_LOAD_LIB;
	}
	hlp_printf("loading lib symble %s \n",szLibName);
	fnSB_fp = (pfnENGINE)dlsym( vhfp, "SB_fp" );
	if( fnSB_fp == NULL ){
		hlp_printf("loading lib symble %s faild: no symble named 'SB_fp'\n",szLibName);
		return HLP_ERR_CANNOT_LOAD_LIB;
	}
	// check version
	wVer = (WORD)SB_FP_GETVERSION(&dwReleaseDate, &dwLibCap);

	gRegMax = dwLibCap;
	hlp_printf("%s: gRegMax Capacity is %lu\n",__func__,dwLibCap);
	if(gIDs)
	{
		free(gIDs);
		gIDs = NULL;
	}
	gIDs = (DWORD*)malloc(gRegMax * sizeof(DWORD));

	hlp_printf("hlp: ver = %04X, release = %08X, regmax = %d\n", wVer, (unsigned int)dwReleaseDate, gRegMax);

	// open
	hlp_printf("%s: open finger print reader... \n",__func__);
	nRet = SB_FP_OPEN(gSensorType, (BYTE**)&gValidFile, (BYTE**)&gMatchData);
	if(nRet < 0)
		return nRet;

	// sensor init
	_settings_get(gSensorType, &dwMechanical, &dwExpose);

	nRet = SB_FP_CMOSINIT(dwMechanical, dwExpose);
	if(nRet < 0)
		return nRet;

	// load database
	if (!_load_database())
	{
		return HLP_ERR_CANNOT_LOAD_DB;
	}

	return 0;
}

long hlpClose() 
{
	return SB_FP_CLOSE();
}

//////////////////////////////////////////////////////////////////////////
long hlpGetEnrollCount() 
{
	return (long)_get_enroll_count();
}

long hlpDelete(long nID, long nFingerNum) 
{
	int nPos = _get_pos(nID, nFingerNum);
	if (nPos == gRegMax)
		return HLP_ERR_ID;
	hlp_printf("hlpDelete: deleting position %d\n",nPos);
	_delete(nPos);
	hlp_printf("hlpDelete: saving valid position %d\n",nPos);
	_save_valid();

	return 0;
}

long hlpDeleteID(long nID) 
{
	int i,nCount = _get_posID(nID);
	if (nCount == 0)
		return HLP_ERR_ID;

	for (i=0; i<nCount; i++)
	{
		_delete(gIDFingers[i]);
	}
	_save_valid();

	return 0;
}

long hlpDeleteAll() 
{
	_delete_all();
	_save_valid();

	return 0;
}

long hlpSearchID(long* pnID) 
{
	*pnID = _searchID();

	return 0;
}

long hlpSearchFingerNum(long nID, long* pnFingerNum) 
{
	int nRet;

	*pnFingerNum = 0;

	nRet = _searchFN(nID);
	if (nRet == -1)
		return HLP_ERR_ID;

	*pnFingerNum = nRet;

	return nRet;
}

long hlpCheckID(long nID) 
{
	int nCount = _get_posID(nID);
	if (nCount == 0)
		return HLP_ERR_ID;

	return 0;
}

long hlpCheckFingerNum(long nID, long nFingerNum) 
{
	int nPos = _get_pos(nID, nFingerNum);
	if (nPos == gRegMax)
		return HLP_ERR_ID;

	return 0;
}

long hlpCheckManager(long nID, long nFingerNum) 
{
	int nPos = _get_pos(nID, nFingerNum);
	if (nPos == gRegMax)
		return HLP_ERR_ID;

	if (!gMatchData[nPos].Manager)
		return HLP_ERR_ID;

	return 0;
}

long hlpCheckManagerID(long nID, long* pnFingerNum) 
{
	int i, nCount;

	*pnFingerNum = 0;

	nCount = _get_posID(nID);
	if (nCount == 0)
		return HLP_ERR_ID;

	for (i=0; i<nCount; i++)
	{
		if (gMatchData[gIDFingers[i]].Manager)
		{
			*pnFingerNum = gMatchData[gIDFingers[i]].FingerNum;
			break;
		}
	}

	if (i==nCount)
		return HLP_ERR_ID;

	return 0;
}

//////////////////////////////////////////////////////////////////////////
long hlpCheckAdjustSensor() 
{
	int nRet;
	DWORD dwMechanical, dwExpose;

	nRet = SB_FP_CMOSCHECKADJUST(&dwMechanical, &dwExpose);

	hlp_printf("hlp: Mechanical = %08X, Expose = %08X\n", (unsigned int)dwMechanical, (unsigned int)dwExpose);

	if(nRet < 0)
		return nRet;

	_settings_set(gSensorType, dwMechanical, dwExpose);

	return 0;
}

//////////////////////////////////////////////////////////////////////////
long hlpEnrollPrepare(long nID, long nFingerNum, long Manager) 
{
	int nPos;

	if (nFingerNum < 0 || nFingerNum > 255) return HLP_ERR_ID;
	if (Manager < 0 || Manager > 255) return HLP_ERR_ID;

	nPos = _get_pos(nID, nFingerNum);
	hlp_printf("hlpEnrollPrepare: nPos = %d\n",nPos);
	if (nPos == gRegMax) {//was nPos!=gRegMax
		nPos = _get_empty_pos();
		hlp_printf("hlpEnrollPrepare: get Empty_Pos = %d\n",nPos);
		if (nPos == gRegMax)
			return HLP_ERR_OVER;
	}
	return nPos;
}

long hlpEnrollEnd(long nID, long nFingerNum, long Manager) 
{
	int nRet, nPos;

	nRet = SB_FP_ENROLLMERGE(&gFeature);
	hlp_printf("hlpEnrollEnd: nRet = %d\n",nRet);
	if (nRet < 0)
		return nRet;

	nPos = hlpEnrollPrepare(nID, nFingerNum, Manager);
	hlp_printf("hlpEnrollEnd: nPos = %d\n",nPos);
	if (nPos < 0)
		return nPos;

	gFeature.ID = (DWORD)nID;
	gFeature.FingerNum = (BYTE)(DWORD)nFingerNum;
	gFeature.Manager = (BYTE)(DWORD)Manager;
	gFeature.Valid = 1;

	memcpy(&gMatchData[nPos], &gFeature, sizeof(FPINFO));
	_save_template(nPos);

	gValidFile[nPos] = 1;
	_save_valid();

	return 0;
}

//////////////////////////////////////////////////////////////////////////
long hlpIdentify(long* pnID, long* pnFingerNum) 
{

	int nRet;
	BOOL bAdapted;

	*pnID = 0;
	*pnFingerNum = 0;

	nRet = SB_FP_IDENTIFYIMAGE256(&bAdapted);
	if (nRet < 0)
		return nRet;

	*pnID = gMatchData[nRet].ID;
	*pnFingerNum = gMatchData[nRet].FingerNum;

	//if(bAdapted) _save_template(nRet);

	return gMatchData[nRet].ID;
}

long hlpVerify(long nID, long nFingerNum) 
{
	int nRet, nPos;
	BOOL bAdapted;

	nPos = _get_pos(nID, nFingerNum);
	if (nPos == gRegMax)
		return HLP_ERR_ID;

	nRet = SB_FP_VERIFYIMAGE256(nPos, &bAdapted);
	if (nRet < 0)
		return nRet;

	if(bAdapted) _save_template(nPos);

	return 0;
}

//////////////////////////////////////////////////////////////////////////
long hlpTemplateGetFromImage(void* pTemplate) 
{
	int nRet;

	nRet = SB_FP_PROCESSIMAGE256(&gFeature);
	if (nRet < 0)
		return nRet;

	memcpy(pTemplate, &gFeature, sizeof(FPINFO));

	return 0;
}

long hlpTemplateIdentify(long* pnID, long* pnFingerNum, void* pTemplate) 
{
	int nRet;
	BOOL bAdapted;

	*pnID = 0;
	*pnFingerNum = 0;

	size_t size = 0;
	if(pTemplate == NULL) {
		return -2; //NULL
	}
	if (size == 0) {
		size = sizeof((char*)pTemplate);
		//hlp_printf("hlpTemplateIdentify: size = %u\n",size);
	}
	if (size == 0) {
		//hlp_printf("hlpTemplateIdentify: size = %u\n",size);
		return -3;
	}

	memcpy(&gFeature, pTemplate, sizeof(FPINFO));
	nRet = SB_FP_IDENTIFYFPDATA(&gFeature, &bAdapted);
	if (nRet < 0)
		return nRet;

	*pnID = gMatchData[nRet].ID;
	*pnFingerNum = gMatchData[nRet].FingerNum;
	if(bAdapted) _save_template(nRet);

	return 0;
}

long hlpTemplateVerify(long nID, long nFingerNum, void* pTemplate) 
{
	int nRet, nPos;
	BOOL bAdapted;

	size_t size = 0;
	if(pTemplate == NULL) {
		return -2; //NULL
	}
	if (size == 0) {
		size = sizeof((char*)pTemplate);
		//hlp_printf("hlpTemplateVerify: size = %u\n",size);
	}
	if (size == 0) {
		return -3;
	}

	nPos = _get_pos(nID, nFingerNum);
	if (nPos == gRegMax)
		return HLP_ERR_ID;

	memcpy(&gFeature, pTemplate, sizeof(FPINFO));
	nRet = SB_FP_VERIFYFPDATA(&gFeature, nPos, &bAdapted);
	if (nRet < 0)
		return nRet;

	if(bAdapted) _save_template(nPos);

	return 0;
}

long hlpGetQualityScore(long* pnScore, long nID, long nFingerNum) 
{
	int nRet, nPos;

	*pnScore = 0;
	nPos = _get_pos(nID, nFingerNum);
	if (nPos == gRegMax)
		return HLP_ERR_ID;

	memcpy(&gFeature, &gMatchData[nPos], sizeof(FPINFO));
	nRet = SB_FP_GET_QUALITY(&gFeature);
	if (nRet < 0)
		return nRet;
	*pnScore = nRet;
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

void FP_BITMAP_INIT(FP_BITMAP* pfp_bmp, int cx, int cy)
{
	int i;
	RGBQUAD *pals;

	pfp_bmp->bmfHdr.bfType = ((WORD) ('M' << 8) | 'B');  // "BM"
	pfp_bmp->bmfHdr.bfSize = sizeof(FP_BITMAP) + cx*cy;
	pfp_bmp->bmfHdr.bfReserved1 = 0;
	pfp_bmp->bmfHdr.bfReserved2 = 0;
	pfp_bmp->bmfHdr.bfOffBits = sizeof(FP_BITMAP);

	pfp_bmp->bmInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pfp_bmp->bmInfo.bmiHeader.biWidth = cx;
	pfp_bmp->bmInfo.bmiHeader.biHeight	= -cy;
	pfp_bmp->bmInfo.bmiHeader.biPlanes	= 1;
	pfp_bmp->bmInfo.bmiHeader.biBitCount = 8;
	pfp_bmp->bmInfo.bmiHeader.biCompression = 0;
	pfp_bmp->bmInfo.bmiHeader.biSizeImage = cx*cy;
	pfp_bmp->bmInfo.bmiHeader.biXPelsPerMeter = 0;
	pfp_bmp->bmInfo.bmiHeader.biYPelsPerMeter = 0;
	pfp_bmp->bmInfo.bmiHeader.biClrUsed = 0;
	pfp_bmp->bmInfo.bmiHeader.biClrImportant = 0;

	pals = pfp_bmp->bmInfo.bmiColors;
	for (i = 0; i < 256; i++) {
		pals[i].rgbBlue = i;
		pals[i].rgbGreen = i;
		pals[i].rgbRed = i;
		pals[i].rgbReserved = 0;
	}
}

BOOL hlpSaveImageToFile(char* strImageFileName, void* pImage, int cx, int cy) 
{
	FP_BITMAP fp_bmp;
	int h;

	FP_BITMAP_INIT(&fp_bmp, cx, cy);

	h = open(strImageFileName, O_BINARY | O_RDWR | O_CREAT, S_IREAD | S_IWRITE);
	if (h<0)
		return FALSE;
	lseek(h, 0, SEEK_SET);
	if(write(h, &fp_bmp, sizeof(FP_BITMAP)) != sizeof(FP_BITMAP) ||
			write(h, pImage, cx*cy) != cx*cy)
	{
		close(h);
		return FALSE;
	}

	close(h);
	return TRUE;
}

/************************************************************************/
/* template                                                             */
/************************************************************************/
BOOL hlpLoadTemplateFromFile(char* strTemplFileName, void* pTemplate) 
{
	int h;

	h = open(strTemplFileName, O_BINARY | O_RDONLY);
	if (h<0)
		return FALSE;
	lseek(h, 0, SEEK_SET);
	if(read(h, &gFeature, sizeof(FPINFO)) != sizeof(FPINFO))
	{
		close(h);
		return FALSE;
	}

	close(h);
	memcpy(pTemplate, &gFeature, sizeof(FPINFO));
	return TRUE;
}

int hlpLoadCompressedTemplateFromFile(char* strCompressedTemplFileName, void* pTemplate) 
{
	int h, len;

	h = open(strCompressedTemplFileName, O_BINARY | O_RDONLY);
	if (h<0)
		return 0;
	lseek(h, 0, SEEK_SET);
	if((len = read(h, &gFeature, sizeof(FPINFO))) == 0)
	{
		close(h);
		return 0;
	}

	close(h);
	memcpy(pTemplate, &gFeature, sizeof(FPINFO));
	return len;
}

BOOL hlpSaveTemplateToFile(char* strTemplFileName, void* pTemplate) 
{
	int h;

	h = open(strTemplFileName, O_BINARY | O_RDWR | O_CREAT, S_IREAD | S_IWRITE);
	if (h<0)
		return FALSE;
	lseek(h, 0, SEEK_SET);
	if(write(h, pTemplate, sizeof(FPINFO)) != sizeof(FPINFO))
	{
		close(h);
		return FALSE;
	}

	close(h);
	return TRUE;
}

////////////////// Additional public APIs for synergy 2416 JNI /////////////////////////
DWORD *  hlpIdIterator()
{
	return _gId_Iterator();
}

int hlpLoadFingersDB(char* fpath)
{
	DIR *db;
	char filename[64], *buf, tmp1[64], tmp2[64];
	struct dirent *p;
	int len;

	db = opendir(fpath);
	if (db == NULL)
		return -1;
	memset(filename, 0, 64);
	while ((p = readdir(db))) {
		if ((strcmp(p->d_name, ".") == 0) || (strcmp(p->d_name, "..") == 0))
			continue;
		else {
			if (strstr(p->d_name, ".template")) {
				buf = p->d_name;
				len = strcspn(buf, "_");
				memset(tmp1, 0, sizeof(tmp1));
				strncpy(tmp1, buf, len);
				buf += len;
				buf++;
				len = strcspn(buf, ".");
				memset(tmp2, 0, sizeof(tmp2));
				strncpy(tmp2, buf, len);
				sprintf(filename, "%s%s", fpath, p->d_name);
				hlpLoadFpDataFile(tmp1, atoi(tmp2), filename);
			}
		}
		memset(filename, 0, 64);
	}
	closedir(db);
	return 0;

}
int hlpGetFpDataOne(long nID,long FingerNum,unsigned char *str)
{
	long anpos;
	int nPos;

	anpos = nID;
	nPos = _get_pos(anpos, FingerNum);
	if (nPos == gRegMax) {
		return FALSE;
	}
	memcpy(str,&gMatchData[nPos],sizeof(FPINFO));
	return TRUE;
}
int hlpLoadFpDataFile(char *nID,long FingerNum,char *FileName)
{
	int vSize,nPos=0;
	FILE *vFile;
	unsigned char gFpdataBuff[FPDATASIZE];
	if( access(FileName, R_OK|F_OK) != 0 ) return FALSE;
	nPos = hlpEnrollPrepare(atol(nID),FingerNum, 0);
	if(nPos < 0) {
		//ErrorProc( nPos );
		return FALSE;
	}

	vFile = fopen( FileName, "rb");
	vSize = fread( gFpdataBuff, FPDATASIZE, 1, vFile);
	if (vSize != 1) {
		fclose( vFile );
		return FALSE;
	}

	gValidFile[nPos] = 1; //finger badge valid
	memcpy(&gMatchData[nPos],gFpdataBuff, sizeof(FPINFO));
	gMatchData[nPos].FingerNum = (BYTE)(DWORD)FingerNum;
	gMatchData[nPos].ID=(DWORD)(atol(nID));
	fclose( vFile );
	return TRUE;
}

int hlpSetFpDataOne(long nID, long FingerNum, unsigned char* decodedTemplate){
	int nPos = 0;
	//BOOL bAdapted;
	int nRet = 0;
	//check the size of the decodedtemplate, reject it if it does not math sizeof(FPINFO)
	size_t size = 0;
	if(decodedTemplate == NULL) {
		return -2; //NULL
	}
	if (size == 0) {
		size = sizeof((char*)decodedTemplate);
	}
	nPos = hlpEnrollPrepare(nID, FingerNum, 0);
	hlp_printf("hlpSetFpDataOne: nPos = %d, size = %u\n",nPos, size);
	if(nPos < 0) {
		//ErrorProc( nPos );
		return -1;
	}
	if (size == 0) {
			hlp_printf("hlpSetFpDataOne: size == %u\n",size);
	}
	gValidFile[nPos] = 1; //finger badge valid
	memcpy(&gMatchData[nPos],decodedTemplate, sizeof(FPINFO));
	gMatchData[nPos].FingerNum = (BYTE)(DWORD)FingerNum;
	gMatchData[nPos].ID = (DWORD)nID;
	_save_template(nPos);
	_save_valid();
	return nRet;
}
