
#include <unistd.h>
#include "weigand.h"
#include "../card/readcard.h"
#include "../serial/serial.h"
#include "../public/initcom.h"


int wgreadfd=0,wgoutfd=0;
 
//Initialize WG read
int InitWiganRead(void)
{
#if defined _x86
#elif defined _2410
	if((wgreadfd=open("/dev/wg_drv",O_RDWR))==-1)
	{
		perror("open wg_drv error");
   		return FALSE;
	}
#else
//    if(access("/dev/wg",F_OK) != 0)
//    {
//        perror("open wg_drv error");
//        return FALSE;
//    }
//	if((wgreadfd=open("/dev/wg",O_RDWR))==-1)
//	{
//		perror("open wg_drv error");
//   		return FALSE;
//	}

	COMINFO cominfo[4];

	cominfo[0].enable=0;
	cominfo[2].enable=0;//rs485

	cominfo[1].enable=0;//gsm

	cominfo[3].type = 1; //0 = uart 1 = spi
	cominfo[3].enable=1;
	cominfo[3].baudrate= 5;	//2400,4800,9600,19200,38400,57600,115200
	cominfo[3].workmode=1; //answer


	if(OpenCom(cominfo) == FALSE) // Initialize serial port
	{
		perror("com init error");
		return -1;
	}
#endif
	//printf("initWiganread success\n");
	return TRUE;
}

//Hight-Low bits transform
// 10000100->00100001
struct mem_inout data_covert(struct mem_inout smem,size_t len)
{
struct mem_inout tmp;
 int i;
 unsigned int j,k,c;
memset(&tmp,0,sizeof(tmp));
if(len>sizeof(tmp)*8)return tmp;

for(i=len-1;i>=0;i--)
{
j=i/(sizeof(int)*8);
k=i%(sizeof(int)*8);
c=1<<k;
if(c&smem.d[j]){
        j=(len-i-1)/(sizeof(int)*8);
        k=(len-i-1)%(sizeof(int)*8);
        c=1<<k;
        tmp.d[j]=tmp.d[j]|c;
        }
//else flag=0;
}
return tmp;
}

#define CLE_BIT(c,bit) (c &=~(1<<bit))  //把c的第bit位清0
#define SET_BIT(c,bit) (c |=(1<<bit))   //把c的第 bit位置成1
#define GET_BIT(c,bit) (c&(1<<bit))    //从c内取第bit位

int read_wiegand(char *out) {
	int err_code = 0;
	char buf[128];
	int cardtype;
	unsigned char data[256], tmp_value[256], i = 0;
	unsigned int ep_count = 0, op_count = 0;
	unsigned int even_bit = 0, odd_bit = 0, mid_bit = 0;
	int ep, op, mp;
	struct mem_inout smem;
	memset(&smem, 0, sizeof(smem));
	//	if (wgreadfd <= 0) return 0;
	//	read(wgreadfd, data, 0);
	if (ReadCom1() == FALSE) {
		return -1;
	}
	memset(buf, 0, sizeof(buf));
	cardtype = ReadCard(buf);
	switch (cardtype) {
	case 0x97:
		//		for (i = 2; i < 6; i++) {
		//					printf("%02x ",buf[i]);
		//				}
		//		printf("\n");
		switch (buf[1]) {
		case 26:
//			printf("7\n");
			memset(data, 0, sizeof(data));
			for (i = 0; i < 4; i++) {
				data[i] = buf[i + 2];
//				printf("%02x ", data[i]);
			}
//			printf("\n");

			//        strncpy((char*)data,&weds_spi_data.buf[1],4);
			memset(tmp_value, 0, sizeof(tmp_value));
			ep_count = op_count = 0;
			if (GET_BIT(data[0], 7)) //sum
				even_bit = 1;
			else
				even_bit = 0;

			for (i = 1; i < 13; i++) //left data
					{
				if (GET_BIT(data[i / 8], 7 - (i % 8))) {
					ep_count++;
					SET_BIT(tmp_value[(i - 1) / 8], 7 - ((i - 1) % 8));
				}
			}
			if (ep_count % 2)
				ep = 1;
			else
				ep = 0;
			for (i = 13; i < 25; i++) //right data
					{
				if (GET_BIT(data[i / 8], 7 - (i % 8))) {
					op_count++;
					SET_BIT(tmp_value[(i - 1) / 8], 7 - ((i - 1) % 8));
				}
			}
			if (op_count % 2)
				op = 0;
			else
				op = 1;

			if (GET_BIT(data[25 / 8], 7 - (25 % 8)))
				odd_bit = 1;
			else
				odd_bit = 0;
			if (ep == even_bit && op == odd_bit) {
				sprintf(out, "%02X%02X%02X", tmp_value[0], tmp_value[1],
						tmp_value[2]);

				err_code = 1;
			}

			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return err_code;
}

//Read WG data
int ReadWigan(int swg,struct mem_inout*wg_data)
 {
	//read(wgreadfd,(unsigned char *)&wg_data,4);
#if defined _x86
#elif defined _2410
	struct mem_inout smem;
	if (wgreadfd <= 0) return 0;
	memset(&smem,0,sizeof(smem));
	if(ioctl(wgreadfd,swg,&smem)!=0)
	{
		perror("wg error\n");
		return -1;
	}
//printf("read success 1 %X\n",smem.d[0]);
	//Read Success
	if(smem.d[2]&&0X80000000)
	smem=data_covert(smem,swg);
	else return -1;
//printf("read success 2\n");
	*wg_data=smem;
#endif
	return 0;
}

//Close WG read
int CloseWiganRead(void)
{
  if(wgreadfd <=0 )	return FALSE;
  if(close(wgreadfd) ==-1 ) return FALSE;
  wgreadfd = -1;
  return TRUE;
}

//Initialize WG out
int InitWiganOut(IN char *path)
{
	wgoutfd=open(path,O_RDWR);
	if(wgoutfd==-1)
        {
	   return FALSE;
        }

	return TRUE;
}

//WG output
int SendDataWigan(char *buf)
{
	unsigned int data=0;

	if(wgoutfd ==-1 || buf ==NULL )	return FALSE;
	data=atoi(buf);
	ioctl(wgoutfd,WGOUT26,&data);
	return TRUE;
}

//Close WG output
int  CloseWiganOut(void)
{
  if(wgoutfd != -1)
	if(close(wgoutfd) ==-1 ) return FALSE;
  wgoutfd = -1;
  return TRUE;
} 

//Enable output
int EnableOutPut()
{
	return TRUE;
}

//Close output
int CloseOut()
{
	return TRUE;
}

//Set pulse
int SetPulse(int msec)
{
	return TRUE;
}

//Standby mode
int StandbyMode()
{
	return TRUE;
}

//Power supply mode
int PowerSupply()
{
	return TRUE;
}

//Low power warning
int LowPowerWarning(int value)
{
	return TRUE;
}
