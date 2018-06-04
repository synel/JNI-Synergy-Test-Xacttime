#ifndef SENSOR_INFO_H
#define SENSOR_INFO_H

#define CMOS_SETIIC		21
#define CMOS_GETIIC		22
#define CMOS_CAPTURE		24
#define CMOS_CAPTUREFLAG	31
#define CMOS_READBUFF		32
#define CMOS_RPREADBUFF		33
#define CMOS_STOP_CAPTURE	34
#define CMOS_SET_CAPTURE	41
#define CMOS_SETATIIC		61
#define CMOS_GETATIIC		62

#define CMOS_ONOFF_GPIO		GPIO_TO_PIN(3, 13)
//#define CMOS_LIGHT_CAPTURE	42
//#define WRITE_0			22
//#define WRITE_1		24

typedef struct _sensor_ctrl
{
        int		result;
        int		len;
        int		reclen;
        unsigned char	data[256];
}SENSOR_CTRL;

struct regval_list {
        unsigned char reg_num[1];
        unsigned char value[1];
};
/*
 * The default register settings
 *
 */
#define FP_WIDTH			640
#define FP_HEIGHT			480
#define FP_MAX_BUFFER_SIZE	(FP_WIDTH * FP_HEIGHT)

#define DEF_EXPOSE			370


int initFpDEv();

int closeFpDev();

/*--------加密芯片相关接口---------*/




/*
 * 读配置区
 * data：输出数据，len：要读的数据长度
 * addr：在配置区的位置
 */
int readConfigZone(unsigned char *data, unsigned char addr,
        unsigned char len);


/*
 *读取卡制造商代码
 */
int readCardManufacturerCode(unsigned char *data, unsigned char recLen);

int get_sensor_info(void);


#endif // SENSOR_INFO_H
