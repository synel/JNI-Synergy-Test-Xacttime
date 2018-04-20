#ifndef _LIBFUNC_H
#define _LIBFUNC_H 
#include <time.h>

#define RTC_DISABLE		0
#define RTC_ENABLE		1

#define TRUE		1
#define FALSE	0
#define ERR_FORMAT 2

#define LEN 64

#define IN
#define OUT
#define INOUT
#define OPTIONAL

extern "C" {

/*========================================================================
                    Encryption/Decryption
==========================================================================*/
int _encrypt( char *src,char *dest, int src_len, int dest_len);
int _decrypt( char *src,char *dest, int src_len, int dest_len);

/*========================================================================
                    KeyBoard Equipment
==========================================================================*/
 typedef struct _key_INFO
 {
	int enable;		//0 disabled		1 enable
	int mode;		//0 numeric 1 alpha 2 alpha-numeric //modify by aduo 2013.7.12
	int timeout;		//key time out
	int keydisplay;	//0 immediately	1 until ent pressed
 }KEYINFO;

 int InitKeyBoard(KEYINFO *key_info);
 int ThreadKeyBoard(void);
 char *ReadKey(int key);
 int SetKeyMode(IN int mode);
 int GetKeyMode();

/*========================================================================
                    Camera Equipment
==========================================================================*/
 int OpenCamera(void);
 int GetCameraImage(IN const char *FileName);           
 int CameraClose(void);

/*========================================================================
                    Sound Equipment
==========================================================================*/
typedef struct _sound_INFO
{
	int sound_kq;
	int sound_type;
	int sound_key;
	int sound_menu;   //
	int sound_ok;           //sound_ok 	0-Success.VOC£¬1-serial number£¬2-name£¬3-serial number + success.VOC£¬4-name + Success.VOC
}SOUNDINFO;

int InitVolume(SOUNDINFO *sound_info);
int SetVolume(int percent);
void LinkSound(char *command);
void Sound(char * command);
void MenuKey(char * command);
void MenuSound(char * command);
int Start_BJ_Music(char *str);
int Stop_BJ_Music();

/*========================================================================
                    Wiegand Equipment
==========================================================================*/
 struct mem_inout{
  unsigned int d[3];
 };
 int InitWiganRead(void);
 int CloseWiganRead(void);


/*========================================================================
                    GPIO(I/O) Equipment
==========================================================================*/
 int OpenGpioBoard(IN char *path);
 int GpioOpt(IN int opt);
 int Gpio_Sub(const char *buf);
 int LedFlashing(void);
 int CloseGpioBoard(void);
 int GetIostate(void);

/*========================================================================
                    serviel Equipment
==========================================================================*/
 typedef struct _com_INFO
 {
	int type;		//type 0 uart 1 spi  add by aduo 2013.7.4
	int enable;		
	int baudrate;	
	int overtime;	
	int workmode;	
 }COMINFO;
 int OpenCom(IN COMINFO *com_info);
 int ReadCom1(void);//read rs232
 void UnCom(int comnum);
 int Set_Machine_Mode(IN int mode);
 //modify by aduo 2013.7.12
 //<!--
 //int read_mac_com(char * mac);
 //int write_mac_com(char *mac);
 //-->

/*========================================================================
                    Card Equipment
==========================================================================*/

struct card_INFO
{
	char cardtype;			//card-reader type(now unuse)
	int track;				//numbers of magnetic tracks 
	int card_right,card_10,card_fs,card_right10;  //how many bits of card number will be cut from right;whether to convert card number to decimal system
};

struct R_card_INFO
{
	int enable;
	char keymode;			//key type
	char Akey[24],Bkey[24];		//A-key£¬B-key
	int beg_sectors,end_sectors,beg_block,end_block;
};


struct W_card_INFO
{
	int enable;
	char keymode;	
	char Akey[24],Bkey[24];		//A-key£¬B-key
	int beg_sectors,end_sectors,beg_block,end_block;
};

 int InitCard(struct card_INFO *card_info);
 int InitCard_Read(struct R_card_INFO *R_cardinfo);
 int InitCard_Write(struct W_card_INFO *W_cardinfo);
 int ReadCard(char *cardnum);
char *wg26_0();
char *wg26_1();
char *wg26_2();
char *wg26_3();
char *wg27_0();
char *wg32_0();
char *wg32_1();
char *wg33_0();
char *wg33_1();
char *wg34_0();
char *wg34_1();
char *wg34_2();
char *wg36_0();
char *wg36_1();
char *wg36_2();
char *wg37_0();
char *wg37_1();
char *wg37_2();
char *wg37_3();
char *wg37_4();
char *wg37_5();
char *wg37_6();
char *wg37_7();
char *wg40_0();
char *wg40_1();
char *wg40_2();
char *wg86_0();
 int card_linearhandle(char *cardsno,char*param);
 int card_synelhandle(char *cardsno,char*param);
 int mifs_request();
 int MF1Read(char *cardsno,char *param);
 int MF1Write(char *cardsno, char*value);
int mifs_read(char *cardsno,char Sector, char Block, unsigned char *Data, char mode,char *key);
int mifs_write(char *cardsno,char Sector, char Block, unsigned char *Data, char mode,char *key);
int gsmString2Bytes(const char* pSrc, unsigned char* pDst, int nSrcLength);


/*========================================================================
                    Fingerprint Equipment
==========================================================================*/
 typedef struct _fp_INFO
 { 
	int sensortype;	//finger type
	int fpmath;		//finger data loading ways,1:1 loading,1;N loading.  0-1:N,1-1:1
	int (*performBeforScan)(int ScanNum);
	int (*performAfterScan)(int ScanNum);
	char sysfinger[64];
	char fingerport[64],fingertype[64],fingerbtl[64],fingerbright[64];
 }FPDEVINFO;
 int InitFp(FPDEVINFO *fpdev_info);
 int  _get_enroll_count();
 int  LoadFpData(char *nID,int FingerNum,char *FileName);
 int  Enroll( char *nID ,int FingerNum,char *tpath,char *dpath );
 long OneToNMatch(char *tpath);
 long OneToOneMatch(char *nID,char *tpath);
 int  DeleteFpOne(char *nID, int FingerNum);
 int  DeleteFpOneAll(char *nID);
 int  DeleteFpAll(void);
 int DeleteFPFlash(char *nID, int FingerNum,char *dpath);
 int DeleteFpOneAllFALSE(char *nID,char *dpath);
 unsigned char *GetFpDataOne(char *nID,int FingerNum);
 int setFpData(char *nID,int FingerNum,unsigned char *str);
 int LoadFingerTemplate(char *nID,char *fpath);
 FILE * GetFpList();
 int UninitFp(void);
 long FpDataOneToNMatch(char *FileName);


/*========================================================================
                    pc-Fingerprint Equipment
==========================================================================*/
 int InitFp_pc(char *lbpath, int nSensorType);
 long OneToNMatch_pc(long *nPos,char *tpath);
 int Enroll_pc(long nID,char *tpath) ;
 int LoadFpData_pc(long anpos,char *FileName);

/*========================================================================
                    Date/Time Set
==========================================================================*/
 int SysClock(IN int clock_tz);
 int MenuSetDate(IN const char *str,IN int opt);
 int MenuSetTime(IN const char *str,IN int opt);
 int GetDate(OUT char *str,IN int opt);

/*========================================================================
                    Net Set
==========================================================================*/
 typedef struct _cal_BACK
 {
	int (*netback)( const char* fliename );
	int (*endback)( int wddaflag );
	int (*verify_manger)(char *id,char *password,char *card);
	void (*write_ts)(char *command,const char *command2,char *msg);
 }CALBACK;

 typedef struct _net_INFO
 {
	//Link-overtime£¬Port£¬Server ip,Verfication-overtime
	char netovertime[LEN],netport[LEN],netserverip[LEN],timeout_server[LEN];
	char dailiip[LEN];    
	int dailisocket;
	int line;            //Net-link mode:0->No verify;1->verify Password;2->verfiy both
	char linepassword[LEN];          //Net-link password
	int linetype;	//0-tcp,1-485,2-modeom,3-gprs
	int vali_up;           //vali_net: report of verification successful
	int netenable;			//record if there are network connections
 }NETINFO;
 int InitNet(IN NETINFO *net_info,IN CALBACK *cal_back);
 int Net();
 int AcceptLinezd();
 void UninitNet();
 int GetDomain(IN char *buf,OUT char *ipaddr);

/*========================================================================
                    485/232 common option
==========================================================================*/
 //int OpenCom(int port,int baudrate,int overtime,int workmode);
 int AcceptLinecom();
 int ComSend();  

 
/*========================================================================
                    Sd&u-disk Set
==========================================================================*/
 FILE* creatdirfile(char *path);
 int safe_cp(char * oldpath,char *newpath);
 int safe_rm(char * oldpath);
 int UsbContion();
 int SortSearch(char *filename,int rlength,int kpos,int klength,char *key);
 int BinarySearch(char *filename,int rlength,int kpos,int klength,char *key);

//out
 int EnableOutPut();
 int CloseOut();
 int SetPulse(int msec);

//power
 int StandbyMode();
 int PowerSupply();
 int LowPowerWarning(int value);

/*========================================================================
                    Print Set
==========================================================================*/
 int GetFileInfo(char *filename);
 int PrintFile(void);
 int PrintData(char *buf);
 int UnPrintCom(void);

/*========================================================================
                    GPRS Set	--Unfinished
==========================================================================*/
 int InitGprs(int port,int rate,int overtime,int protocol,char *deputyip,int deputyport);
 int GprsAccept(unsigned char *data,int len);
 void GprsJianche();
 void CloseLink();
 int GprsRecv(unsigned char *data,int len);
 int GprsSend(unsigned char *data,unsigned int len);

/*
 int InitGprsPort(int port,int rate,int protocol);
 int SelectLineMode(char *mode);
 int SendData(char *value);
 int ReceiveData(char *Value);
 int CloseGprsPort();
*/


 int get_keyboardVer(char *ver);

/*=======================================================================
 *function : power
 *author : aduo
 *date : 2013.7.11
 ========================================================================*/
int get_battery_power_status();
}

#endif 
