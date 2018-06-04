/**
 * @file   dx-RF-SIM.h
 * @author 刘训
 * @date   Wed Jul 13 09:35:09 2011
 *
 * @brief
 *
 *
 */
#ifndef PRINTCOM_H__
#define PRINTCOM_H__
#include "_precomp.h"

int read_RFID_SIM_0201A(int uart_port,char *value);
int read_RFID_SIM_0100D(int uart_port,char *value);
int read_M300_card(int uart_port, char *value);
int clear_M300_readcard_data(int uart_port);
int read_xdf_card(int uart_port, char *value);
// 2.4G gmjs
int read_24G_card(int fd,char *value);
int gmjs_24G_read(int fd);
int gmjs_24G_write(int fd, unsigned char value);
int queryPhoneCard(unsigned int comfd, unsigned short delaytime, unsigned char *simCardNo);
int releaseCardLink(unsigned int comfd, unsigned short delaytime);
int checkLinkStatus(unsigned int comfd);
int getReaderStatus(unsigned int comfd);
int getPostResult(unsigned int comfd);
int  softResetReader(unsigned int comfd);
int  getVersionInfo(unsigned int comfd, unsigned char *cmcc_interface, unsigned char *third_interface, unsigned char *proinfomation);
int transferCommand(unsigned char cmdType, unsigned char cmdCode, unsigned char *cmdParam, unsigned int paramLen, unsigned int comfd);
int receiveRespond(unsigned int comfd, unsigned char *recvData);
int read_24G_card_app(int fd,char *value);

//联通2.4
int recvPackage(int uart_port,unsigned char *package);
int sendCmdData(int uart_port,unsigned char overtime);
int read_zkxl_card(int uart_port,char *value);

int read_ID2_card(int uart_port,char *value);
int read_mj_card(int uart_port,char *value);
int set_id_card_photo_addr(char *path);
int read_hirf_24g_card(int uart_port,char *value);
//正元
int zy_24G_card_app(int fd,char *value);

// 20160331 新中新
int read_24G_card_xzx(int fd,char *value);

//usb读头 青岛天高
typedef struct _TG_CARD_NUMBER
{
    int Electricity;//电池电量/按键值
    time_t sTime;  //
    time_t eTime;
    char cardNo[64];
   struct _TG_CARD_NUMBER *next;       //指向下一节点的指针
}_TgCardNo;

extern _TgCardNo *TgCardNoHead[256];

int hidCardDeviceInit(void);
int hidCardDeviceRead(void* buffer, int buffer_size);



int hidCardDeviceWrite(void* buffer, int buffer_size);

int readHighFrequency(  char *data);
//mode ://0 是定位模式，1是按键模式
int setHfCardMode(int mode);
//overtime :单位：秒，即多长时间读不到卡会返回一次卡离开状态
long int setHfCardOverTime(int overtime);

int readHighFrequency_szztx( int fd, unsigned char *data);


//通过串口读二维码扫描仪（E20）数据

int read_qrcode_E20(int fd,unsigned char *data,int len);

// tag-it iso 15693card
int read_iso_15693card(int port, unsigned char *oData);


#endif // !defined(COMM_H__)

