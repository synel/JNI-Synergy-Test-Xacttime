#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "sensor_info.h"
#define	DEV_NAME 			"/dev/sensor"
#define DEV_ADDR307			0x42
#define GREEN_LED

int fpHandle = -1;


int initFpDEv()
{
        fpHandle = open(DEV_NAME, O_RDWR);
        //printf("%s %d\n",__func__,fpHandle);
        if(fpHandle<0) {
                printf("Open device %s  error", DEV_NAME);
                return -1;
        }
        return 0;
}
int closeFpDev()
{
        if (fpHandle >= 0){
                close(fpHandle);
                fpHandle = -1;
        }

        return -1;
}


/**------------------------加密芯片相关 -------------------------*/
#define CM_CMD_LEN				4
int readConfigZone(unsigned char *data, unsigned char addr,
        unsigned char len)
{
        int j=-1;
        SENSOR_CTRL param;

        if (!data)
                return -1;

        memset(&param, 0, sizeof(SENSOR_CTRL));
        param.data[0] = CM_CMD_LEN;
        param.data[1] = 0xB6;
        param.data[2] = 0x00;
        param.data[3] = addr;
        param.data[4] = len;

        if (ioctl(fpHandle, CMOS_GETATIIC, &param)){
                printf("%s: ioctl CMOS_GETATIIC error\n", __func__);
                return -1;
        }

        memcpy(data, param.data+1, len);

        return 0;
}
int readCardManufacturerCode(unsigned char *data, unsigned char recLen)
{
        return readConfigZone(data, 0x0C, (recLen>4)?4:recLen);
}


typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned long BOOL;
#define FP_ERR_SENSOR					-7
int	SB_FP_CMOSINIT1()
{
        int dwRet = FP_ERR_SENSOR;

        dwRet = initFpDEv();
        if (dwRet<0){
                return FP_ERR_SENSOR;
        }

        return 0;
}
int get_sensor_info(void)
{
        int ret = -1;
        int sensor_info=0;

        unsigned char cardcode[10];

        //printf("[Init]:SB_FP_CMOSINIT \n");
        ret = SB_FP_CMOSINIT1();
        if (ret < 0){
                printf("SB_FP_CMOSINIT fail!!\n");
        }

        ret =  readCardManufacturerCode(cardcode, 4);
        printf("cardcode = %d,%d,%d,%d\n",cardcode[0],cardcode[1],cardcode[2],cardcode[3]);

        if(cardcode[2] == 1)
        {
             sensor_info=cardcode[3];
        }

//        if ((cardcode[2] == 1)&&(cardcode[3] == 1))
//        {
//                printf("BB:This  AT88SC is for 3000 fingerprint!");
//                finger_max=3000;
//        }
//        else
//        {
//                if ((cardcode[2] == 1)&&(cardcode[3] == 2))
//                {
//                        finger_max=10000;
//                        printf("BB:This  AT88SC is for 10000 fingerprint!");
//                }
//                else
//                {
//                        if ((cardcode[2] == 1)&&(cardcode[3] == 3))
//                        {
//                                printf("BB:This  AT88SC is for 30000 fingerprint!");
//                                finger_max=30000;
//                        }
//                        else printf("I don't know !");
//                }


//        }

        closeFpDev();

        return sensor_info;
}
