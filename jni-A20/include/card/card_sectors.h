/**
 * @file   card_sectors.h
 * @author 刘训
 * @date   Thu Jul 14 16:47:39 2011
 *
 * @brief
 *
 *
 */
#ifndef __CARD_SECTORS_H
#define __CARD_SECTORS_H
#include "_precomp.h"
#include "cardinfo.h"
#include "../general/stringchar.h"

//状态信息
//状态信息
typedef struct {
        int comfd;		// 串口操作句柄 20160114
        int isSpi;		// 是否是spi 1-是，0-232
        unsigned char manageSector; // 管理信息扇区号
        unsigned char manageBlock;  //管理信息开始块号（扇区内，连续两个块，为0或1）
        char mifareAppID[13];
        unsigned char readerType;   //卡读写器类型,0表示未初始化
        unsigned short int authDelayMsec; //认证有效延续时间（毫秒数）
        unsigned char cardType;     //当前卡片型号
        unsigned int card1;         //当前卡号高4字节（初始化读写器类型需初始化当前卡号）
        unsigned int card2;         //当前卡号低4字节（高4字节及低4字节均为0，表示未寻卡）

        unsigned char originalKey[24]; //当前卡片原始密钥,兼容CPU卡
        unsigned char writeManageSectorKey[24]; //当前卡片写管理扇区密钥
        unsigned char internalAuthKeybuf[24]; //内部认证密钥
        unsigned char dealKey[24];  //当前卡片交易密钥
        unsigned char reqKey[24];   //当前查询密钥

        signed char currFlagPoint;         //当前标志块指针（标准版）
        signed char authSectorNo;          //当前认证扇区号（绝对扇区号，负数表示未认证或失效）
        signed char authSectorKeyType;     //对应当前认证扇区认证密钥类型（A或B密钥）
        unsigned int accessMsec;    //成功访问时间（开机经过的毫秒数）
        unsigned int yooReqMsec;    //发送寻卡指令时间（开机经过的毫秒数）


        //CPU卡片状态 20121015
        unsigned int currDF;       //当前卡片目录 主目录3F00，WEDS应用目录AF01   未知：-1
        unsigned short int currBF; //当前二进制文件短文件标识符 -1：未知
        unsigned char antiStat;    //安全状态

        //CPU/PSAM卡片类型 20121015
        unsigned char cpuCodeID;  //CPU卡片的处理代码类型，不同卡片可对应同一个cpuCodeID。0无效
        unsigned char ATS[32];         //ATS或ATR
        unsigned int cosID[256];       //cos类型及其版本号列表，与处理代码对应（cpuCodeID）
        unsigned char codeID[256];     //与cosID  共同构成表格
        unsigned char InData[8];     //cpu 分散因子

        unsigned char devKey[16];

//// 20160105 add

//20130118 CPU 整合
        unsigned char scatteringFactor;      //分散因子类别，0：标准，缺省值， 1：物理卡号 ，后续可扩展
        unsigned char encryptMedia;          //PC端加密介质，0：软件加密，缺省值， 1：PSAM卡加密
        unsigned char psamMode;              //PSAM使用模式，0：标准模式， 缺省值，1：专用模式（定制）
        unsigned short int psamAdfFileID;    //PSAM的ADF文件标识符，缺省值为标准
        unsigned char issuReadLimit;         //发行区读权限，0：自由，1：需要认证
        unsigned char keyType_Edition[7][2]; //加密密钥类型及版本，分别对应：内部认证0、发行区读1、数据区读2（查询密钥）、数据区写3（交易密钥）、发行区写4（发行密钥）、扩展区读5、扩展区写6

        //20121217 CPU卡
        unsigned short int parentDFfileID;
        unsigned char internalAuthKeyType;        // = 0;        //内部认证密钥的类型 0：加密密钥 1：解密密钥 20130607 add

        unsigned short int cpuAdfFileID;     //CPU卡的ADF文件标识符，缺省值标准
        unsigned char issuSectorFileID;      //发行区短文件标识符
        unsigned char dataSectorFileID;      //数据区短文件标识符
        unsigned char extSectorFileID;       //扩展数据区短文件标识符

        unsigned short int issuSectorFileOffset;  //发行区在文件中的偏移量
        unsigned short int dataSectorFileOffset;  //数据区在文件中的偏移量
        unsigned short int extSectorFileOffset;   //扩展数据区在文件中的偏移量

        unsigned char internalAuthKeyID;     //内部认证DES密钥标识号

        unsigned char issuSectorKeyEdition_KeyID[2][3];  //发行区读、写密钥的密钥版本、密钥标识（或索引）、后续权限
        unsigned char dataSectorKeyEdition_KeyID[2][3];  //数据区读、写密钥的密钥版本、密钥标识（或索引）、后续权限
        unsigned char extSectorKeyEdition_KeyID[2][3];   //扩展数据区读、写密钥的密钥版本、密钥标识（或索引）、后续权限

        unsigned char issuSectorAcceLimit[2][2];         //发行区读、写权限范围(包含端点)
        unsigned char dataSectorAcceLimit[2][2];         //数据区读、写权限范围(包含端点)
        unsigned char extSectorAcceLimit[2][2];          //扩展数据区读、写权限范围(包含端点)
// end

//	int retCode;

}
 TstatInfo, PTstatInfo;

TstatInfo statInfo; //状态信息


extern char phySectorNo[72];
extern signed char phyBlockNo[72][4];

int cardHexToUInt(int len,unsigned char *cardNo);
int setCardType(int cardtype,int readertype);
int ReadFile( int fd, unsigned char * addr, int len, long *dwCount, char *s );
int WriteFile( int fd, unsigned char *addr, int len, long *dwCount, char *s );
int sendAndRecvCom( unsigned char *SendBuf, unsigned char *RecvBuf, int RetryCount, int timeout );
int block_write( unsigned int cardNo, int sectorno, signed char blockno, unsigned char *data, int keymode, unsigned char *key, int checkflag );
int block_read( unsigned int cardNo, int sectorno, signed char blockno, unsigned char *data, int keymode, unsigned char *key );
#if 0
int writeDataBlock( int keymode, unsigned char *key, int firstSector, int firstBlock, int len, unsigned char *data ,int uart_port);
int readDataBlock( int keyMode, unsigned char *key, int firstSector, int firstBlock, int len, unsigned char *data, int uart_port);
#endif
int read_dx_cpu_card_app(int fd,char *value);
void set_read_cpu_type(int type);
int read_serialno_sjy(int uart_port,char *value);
int queryPhoneCard( unsigned int comfd, unsigned short delaytime, unsigned char *simCardNo );
int releaseCardLink( unsigned int comfd, unsigned short delaytime );
int queryPhoneCard( unsigned int comfd, unsigned short delaytime, unsigned char *simCardNo );
int read_nj_citizen_card(int uart_port,char *value);
int read_hz_citizen_card(int uart_port,char *value);

int wedsApduChannel( unsigned char *buf, unsigned char buflen, unsigned char *answerBuf, int uart_port );


int read_cpu_data(int uart_port,int mode,char *aid,int efid,int offset,int len,unsigned char *buf);
int read_cpu_data_xiangtan(int uart_port,char *no);


int read_dx_cpu_card_test(int uart_port,char *value);
int read_cpu_data_yhkh(int uart_port,char *no);


int read_cpu_data_zhongguangruibo(int uart_port,char *no);
int read_cpu_data_beijingleisen_appleapy(int uart_port,char *no);

//读逻辑卡号，公司自有消费CPU卡
//
int read_cpu_data_weds(int uart_port,char *no);




#if 1		// lfg 20160128


#define TIME20000101 (946684800L); //949276800;


int sdk_get_comfd(int com);
int ReadFile( int fd, unsigned char * addr, int len, long *dwCount, char *s );
int WriteFile( int fd, unsigned char *addr, int len, long *dwCount, char *s );

int card_data_r( int m_hcom, unsigned char *RecvBuf );
int card_data_sr( int m_hcom, unsigned char *SendBuf, unsigned char *RecvBuf, int RetryCount, int timeout );

int block_write( unsigned int cardNo, int sectorno, signed char blockno, unsigned char *data, int keymode, unsigned char *key, int checkflag );
int block_read( unsigned int cardNo, int sectorno, signed char blockno, unsigned char *data, int keymode, unsigned char *key );

int writeDataBlock2( int keymode, unsigned char *key, int firstSector, int firstBlock,
                                                int len, unsigned char *data );
int readDataBlock2( int keyMode, unsigned char *key, int firstSector, int firstBlock,
                                                int len, unsigned char *data );

int sdkReadIcData( int comfd, unsigned char readerType, int keymode, unsigned char *key,
                                                        int firstSector, int firstBlock, 	int len, unsigned char *data );

int sdkWriteIcData( int comfd, unsigned char readerType, int keymode, unsigned char *key,
                                                        int firstSector, int firstBlock, 	int len, unsigned char *data );
int sdkModiSectorKey( int comfd, unsigned char readerType, int oldKeyMode, unsigned char *oldKey,
                                                int sectorNo, unsigned char *AKey, unsigned char *BKey, unsigned char *ctrlWord);


int card_passive( int reader, char *phycard );
int card_active( int reader, char *phycard );
int card_get_cpu( char *phycard, unsigned int *cardlen );
int modiSectorKey( int oldKeyMode, unsigned char *oldKey, int sectorNo,
                                                unsigned char *AKey, unsigned char *BKey, unsigned char *ctrlWord );


int wedsApduChannel2( unsigned char *buf, unsigned char buflen, unsigned char *answerBuf,
                                                        unsigned char maxAnswerlen, int timeout );

int selectFile( unsigned short fileID, unsigned char *buf, unsigned char len );
int selectFile2( unsigned char P1, unsigned char P2, unsigned char Lc,
                                        unsigned char Le, unsigned char *ioData, unsigned int oLen );
int readBinary( unsigned short fileID, unsigned short offset, unsigned char Le, unsigned char *buf );
int updateBinary( unsigned short fileID, unsigned short offset, unsigned char *buf, unsigned char len );
int verifyPIN( unsigned char *password, unsigned char len, int *retrys );
int getResponse( unsigned char *buf, unsigned char Le );
int cpuErrCode( unsigned char *sw );

// 在线消费业务层相关 20160129



typedef struct __OUT_PARAM
{
        int ret;	// 返回码
        int balance;	// 卡片余额
        int optAmt;		// 交易金额
        unsigned char crec[16*3*9];  // 最多支持9个扇区存储明细
        unsigned int recblknum;	// 离线记录(包括明细)块数	一个扇区块视为一条记录
        char phycard[32+1];	// 物理卡号
        unsigned int cardlen;	// 卡号长度，4字节、7字节或8字节

        unsigned int recnum;	// 未上传记录数
        unsigned int cardLsh;	// 卡交易计数
        unsigned int cardStat;	// 卡状态
        unsigned int cardSnr;		// 逻辑卡号

        unsigned int begd,endd; // 卡片有效起止日期
        unsigned short dayAmt;	// 日累计金额
        unsigned short mealAmt;	// 时段累计金额
        unsigned char mealTimes;// 时段累计次
        unsigned char dayTimes;	// 日累计次

}
_OUT_PARAM;


typedef struct __IN_PARAM
{
        time_t curTimes;
        unsigned int cmd;	// 操作命令。在线消费、在线查询、离线消费、离线查询、读取离线记录等
        unsigned int devId;  // 终端编号ID
        unsigned int com;	// 串口号
        unsigned int pwd;	// 验证密码
        unsigned int lsh;	// 设备流水（下一个可用的流水号）
        unsigned char isPsam;	// 是否启用psam模块 1-启用，0-否
        char mifareAppID[12+1];	// AID 在终端上通过菜单配置 示例: Mifare13 App
        int optAmt;		// 操作金额 : 如扣款金额，同步金额等
        unsigned char reader;	// 读头类型
        unsigned char cardlx;	 // 当前寻卡类型
        unsigned char phycard[16+1];	// 物理卡号
        unsigned int logcard; // 逻辑卡号
        unsigned int cardlen;	// 卡号长度，4字节、7字节或8字节
        char path[64];	// 文件路径
        unsigned int cpdata[56][2];	// 最大支持56项菜品，【0】菜品编号，【1】菜品数量
        unsigned int cpnum;		// 菜品项数,标记cpdata有效项数

// 账户时段消费属性
        unsigned char prdxh;		// 时段序号，0~15，如为0，表示当前为"非时段"
        unsigned char nonprd;	// 非时段是否允许消费标志，1-允许
        char prdbh[20];		// 时段编号
        unsigned int dayLmtAmt, dayLmtTimes, prdLmtAmt, prdLmtTimes;
        unsigned int offsetTime;	//时段偏移量

// 卡同步信息
        int sync_balance;	// 同步余额
        unsigned int sync_dealDate;	// 同步交易日期 MMDD
        unsigned int sync_dayAmt, sync_dayTimes, sync_prdAmt, sync_prdTimes;// 同步卡片累计

}
_IN_PARAM;



typedef struct __CARD_CONFIG
{
        unsigned char isPsam;	// 是否启用psam模块 1-启用，0-否
        char mifareAppID[12+1];	// AID 在终端上通过菜单配置 示例: Mifare13 App
        unsigned int mngSect[3];		//管理扇区号,分别表示S50卡，S70卡，uim卡的管理扇区
        unsigned char devKey[16];
        unsigned char interAuthKey[16];	// 无psam卡时，cpu卡内部认证母密钥

        char enLmt;		// 设备是否禁用限额限次 1-禁用限额限次

}_CARD_CONFIG;

extern _CARD_CONFIG card_config;



// 交易记录
typedef struct __REC_DATA
{
        unsigned int lsh;			// 流水号
        unsigned int tradeType;		//交易类型
        unsigned int devId;			// 设备ID
        char phyCSN[16+1];
        unsigned int cardSNR;		// 逻辑卡号
        unsigned short cardLsh;		// 卡交易计数
        char datetime[19+1];	// 2015-12-04 11:28:20
        unsigned int  optAmt;	// 交易金额

        char userPrdNo[16+1];		//个人消费时段编号
        char devPrdNo[16+1];		//设备餐别统计编号

        char userBh[16+1];	// 人员编号
        char userName[36+1];	// 人员姓名
}
_REC_DATA;


//////////////////////////

//密钥
typedef struct
{
        unsigned char devKey[16];
        unsigned char interAuthKey[16];	// 无psam卡时，cpu卡内部认证母密钥

        unsigned char originalKey[24];
        unsigned char readManageSectorKey[24];
        unsigned char writeManageSectorKey[24];
        unsigned char dealKey[24];
        unsigned char reqKey[24];
}
TmasterKey, *PTmasterKey;
extern TmasterKey MasterKey;

//钱包块信息 20110805增加
typedef struct {
        int walletAmt; //钱包余额	4B（单位：分）  //程序中使用的是pubInfo中的钱包余额
        int savingAmt; //充值金额   3B
        unsigned char dealType;  //交易类型
        unsigned int savingDate; //充值日期
        unsigned int savingSerialNo; //充值序号	2B（充值序号） //为充值做准备
        unsigned char blockBuf[16];  //校验： 异或 + CRC8

} Twallet, *PTwallet;


typedef struct {
        int totalConsumeAmt;      //日累计消费金额（3B，HEX，单位：分）
        unsigned int consumeDate; //日累计消费日期（3B，YYMMDD）
        unsigned short int totalQty;    //日累计消费次数（2B，HEX）
        unsigned char currHourSession;     //时段号（1B，HEX）
        int hourSessionAmt;       //时段累计金额（3B，HEX，单位：分）
        unsigned short int hourSessionCopies;   //时段消费次数（2B，HEX）
        unsigned char sbsFlag;	//日补贴时段补贴标志
        unsigned char blockBuf[16];
} TsumInfo, *PTsumInfo; //日累计信息

typedef struct {
        unsigned short int subsidySerialNo; //内部补助批号（2B，HEX）
        unsigned int writeCardDate; //补助领取时间（3B，YYMMDD）
        unsigned int endDate;       //有效截至日期（3B）

        unsigned int endTime;       //有效截至时间（1B）2012-06-09 add    1~143

//	unsigned int subsidyAmt;    //本次领取金额（3B，单位：分）
        int subsidyAmt;    //本次领取金额（3B，单位：分）
//	unsigned int balance;		//补贴余额（3B，单位：分）
        int balance;       //补贴余额（3B，单位：分）----补贴余额修改为有符号数fuglee 2012-05-03
        unsigned char blockBuf[16];
} TsubsidyInfo, *PTsubsidyInfo; //补贴信息

typedef struct {
        char employeeCode[17]; //16B,ASCII，位数不足补0 工号/学号

        unsigned int password; //3BCD,与卡片物理序号前3字节异或
        int passStat;          //1B,HEX, 密码开关 01:开启 02:关闭密码功能
        int cardEdition;       //1B,hex,卡结构版本，目前为1
        unsigned char groupID;                  //1B,HEX,权限组别
        unsigned int onceConsumeLimitation;     //3B,HEX,单次消费密码限额，单位：分
        unsigned int dailyConsumeLimitation;    //3B,HEX,日累计消费密码限额，单位：分
        unsigned short int dailyConsumeCopies;  //2B,HEX,日累计密码限次

        unsigned char blockBuf[32];
} TbaseInfo, *PTbaseInfo; //基础信息

typedef struct {
        unsigned int dealTime;     //交易时间，相对于2000-01-01 00：00：00经过的秒数（4B，HEX）
        int balanceBeforeDeal;     //交易前余额，单位：分（4B，HEX）
        int dealAmt;               //交易金额，单位：分（3B，HEX）
        unsigned char dealType;    //交易类型，1B，HEX，01：消费，02：充值，04：补贴，08：提现 16：补贴清零，32：冲正
        unsigned int terminalNo;   //终端机编号，4B，BCD
        unsigned char blockBuf[16];
} TdetailInfo, *PTdetailInfo; //明细信息



typedef struct
{
        unsigned char validFlag; //有效标志
        unsigned int catalogFlag;   //目录区标识 0x535444
        unsigned char issuSectorNo; //发行扇区号
        unsigned char pubSectorNo;  //公共扇区号
        unsigned char sumSectorNo;  //累计扇区号
        unsigned char baseSectorNo; //基础扇区号
        unsigned char subsidySectorNo;   //补贴扇区号
        unsigned char detailSectorNo[5]; //明细扇区号，0表示结束
        unsigned int copiesSectorNo; //份记录扇区号 20110803
        unsigned int linkageSectorNo; //联动扇区号  20110803 A密钥固定值，下一扇区为考勤机用写的扇区

        unsigned char pubReadFlag; //公共已读
        unsigned char walletReadFlag; //钱包已读
        unsigned char baseReadFlag; //基础已读
        unsigned char sumReadFlag; //累计已读
        unsigned char subsidyReadFlag; //补贴已读
        unsigned char personalReadFlag; //个人已读
        unsigned char copiesReadFlag;   //份记录已读 20110803

        unsigned short catalogArea;    //目录绝对块号
        unsigned short issuArea[2];    //发行区绝对块号
        unsigned short pubArea[2];     //公共区绝对块号
        unsigned short walletArea[2];  //钱包区绝对块号
        unsigned short sumArea[2];     //累计区绝对块号
        unsigned short baseArea[2];    //基础区绝对块号
        unsigned short subsidyArea[2]; //补贴区绝对块号
        unsigned short personArea;     //个人区绝对块号
        unsigned short detailArray[16];//明细绝对块号
        unsigned short copiesArray[2]; //份记录绝对块号 20110803
        unsigned short linkageArray[2];//联动绝对块号 20110803
        unsigned short linkageAuxBlockNo; //辅助绝对块号（考勤机写）20110803

// 20151203
        unsigned char recSctNum; 		// 可存储记录和明细的扇区数
        unsigned short sctArray[9];	// 最多9个记录扇区
        unsigned short recArray[27];	// 记录绝对块号 ，最多 9*3=27块

        unsigned char blockBuf[16];

}
TCatalog, *PTCatalog; //应用目录
extern TCatalog CCatalog;

typedef struct {
        unsigned short int appType;  //2B, BCD, 应用类型(卡种标识）8665:全国应用 8667:省应用
        unsigned short int areaCode; //2B, BCD, 3位电话区号+0
        unsigned int issuSerialNo;   //4B, BCD, 卡顺序号
        unsigned int cardAuthCode;   //4B, HEX, 卡认证码
        unsigned char useFlag;       //1B, HEX, 启用标志, 0:未启用, 1:启用
        unsigned short int deposit;  //2B, HEX, 押金,单位:分

        unsigned int issuDate;  //4BCD, YYYYMMDD
        unsigned int endDate;   //4BCD, YYYYMMDD
        unsigned int beginDate; //4BCD, YYYYMMDD
        char issuDateStr[12];
        char endDateStr[12];
        char beginDateStr[12];
        unsigned char statFlag; //1B, HEX, 卡状态标志, 0:未启用 1:已启用 2:已停用 3:已退卡 4:黑名单卡
        unsigned char blackTimes; //1B, HEX 黑名单次数
        unsigned char cardType; //1B, HEX 卡类 0-用户卡， 1-管理卡，2-商户卡
        char CardNo[16];		//卡循序号,BCD
        unsigned char blockBuf[32];

}
TissuInfo, *PTissuInfo; //发行信息
extern TissuInfo IssuInfo;

typedef struct
{
        unsigned char detailPoint;      //明细指针	1B（下次写明细块的指针，初始值为0）
        unsigned short int totalCopies; //2B 累计交易次数

        unsigned short int totalSavingCopies; //2B,累计充值次数
        unsigned char blackFlag;              //黑名单标志，1B，HEX，01：正常，04：黑名单
        int totalSavingAmt;            //4B，HEX,累计充值金额
        unsigned char dailyTotalPoint; //日累计指针（1B，HEX，取值为0、1，其它值非法）
        unsigned char subsidyPoint;    //补贴指针（1B，HEX，取值为0、1，其它值非法）
        unsigned char walletPoint;     //钱包指针（1B，HEX，取值为0、1，其它值非法）
        unsigned char copiesPoint;     //份记录指针 20110803
        unsigned char linkagePoint;    //联动指针   20110803

        unsigned char keyType;  //读认证密钥类型, 1:A密钥, 2:B密钥

// new online 20151126
        unsigned char validFlag;		//有效标志	1B（0x35：标志块有效，其它值：标志块无效）
        unsigned char updateNo; 	   //更新序号（1B，HEX）
        unsigned short int recNum;	// 离线记录数
        unsigned char cardStat; // 4b, 0：未激活，1：激活 2：冻结 4：黑名单卡 5：退卡 6：升级中
        unsigned char mealNo;	// 4b, 餐别，时段号
        int walletAmt; //钱包余额	3B（单位：分）
        unsigned int dealDate; //交易日期（卡片：2B，MMDD）
        unsigned short dayAmt;	// 日累计金额
        unsigned short mealAmt;	// 时段累计金额
        unsigned char mealTimes;	// 3b,时段累计次
        unsigned char dayTimes;	// 5b,日累计次
        unsigned short int cardLsh; //2B 卡交易次数
        unsigned char blockBuf[16];

}
TpubInfo, *PTpubInfo; //公共信息 + 钱包余额
extern TpubInfo pubInfo;


typedef struct
{
        unsigned char name[12];          //11B, 姓名,不足位数补0
        unsigned char pwd[3];            //3B，个人密码，BCD，位数不足补 "F" 为全零表示禁用密码

        unsigned char pwdValid;          //密码有效标志 1：有效 0：无效 2：禁用

        unsigned char blockBuf[16];
}
TpwdInfo, *PTpwdInfo;              //在线消费个人密码信息



// 消费写卡数据
typedef struct __CARD_DATA
{
        unsigned int devId;			// 设备ID
        unsigned int lsh;			// 流水号
        unsigned short cardLsh;		// 卡交易计数
        unsigned int relativeTime;	// 相当于 2000-01-01 00:00:00
        unsigned int optAmt;	// 交易金额
        unsigned int pwd;	// 交易密码
        unsigned int cpblknum;	// 菜品块数量

        unsigned char cpblk[512];
}
_CARD_DATA;


typedef struct __CP_INFO
{
//	char cpSData[512];		// asc
        unsigned char cpHData[512]; // hex
        unsigned int  cpblknum;
}_CP_INFO;
extern _CP_INFO CP_INFO;


int pit_get_comfd(unsigned int com);

void pit_online_test();

typedef char    (*PSubKey)[16][48];

//加密解密
enum
{
        ENCRYPT =0,  //加密
        DECRYPT 	 //解密

};

//DES算法的模式
enum
{
        ECB 	=	0,	//ECB模式
        CBC 			//CBC模式
};



int RunDes(char bType,char* In,char* Out,unsigned datalen,const char* Key,const unsigned char keylen);
int RunTea(char bType, char *In, char *Out, unsigned int datalen, const char *Key, const unsigned char keylen);


int test_std_card( );



#endif



int weds_cpu_init(int ispsam);
int read_cpu_data_xn(int uart_port,char *no);



int read_ztx_g20(int fd,char *value);



int leeDebugData( unsigned char *blkData, unsigned int dataLen, unsigned int lineLen, unsigned int sendRecvFlag );





#endif











