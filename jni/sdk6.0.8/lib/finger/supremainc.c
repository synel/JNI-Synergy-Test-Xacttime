/*
 * supremainc.c
 *
 *  Created on: 2014-5-5
 *      Author: aduo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "supremainc.h"
#include "../serial/serial.h"
#include "../public/public.h"

int g_enroll_mode = 0x41;
int g_send_scan_success = 0x31;
int g_image_format = 0x31;
int g_available_finger = 0;

char g_template[384];
int g_template_size = 384;

char g_bin_buffer[BUFFER_LENGTH + 4*7];
int total_bin_size = 0;

int port = 3;

//函数功能 1到4字节的HEX数值数据转换为无符号整型
//参数：   in：输入首址
//         size: 输入的字符数，若超过4，则取4
//         flag: 输入的主机字节序，0：小端模式 其它：大端模式 (输出固定为小端模式)
//返回值   转换结果
static unsigned int HexToUInt(void *in, unsigned int size, int flag)
{
    unsigned int ret = 0;
    unsigned char * str = (unsigned char *)&ret;
    unsigned char * indata = (unsigned char *)in;

    if (size > 4)
    {
        size = 4;
    }

    if (flag)
    {
        switch (size)
        {
        case 4:
            *(str + 0) = *(indata + 3);
            *(str + 1) = *(indata + 2);
            *(str + 2) = *(indata + 1);
            *(str + 3) = *(indata + 0);

            break;
        case 3:
            *(str + 0) = *(indata + 2);
            *(str + 1) = *(indata + 1);
            *(str + 2) = *(indata + 0);

            break;
        case 2:
            *(str + 0) = *(indata + 1);
            *(str + 1) = *(indata + 0);

            break;
        case 1:
            *(str + 0) = *(indata + 0);

            break;
        default:
            break;
        }
    }
    else
    {
        switch (size)
        {
        case 4:
            *(str + 3) = *(indata + 3);
            *(str + 2) = *(indata + 2);
            *(str + 1) = *(indata + 1);
            *(str + 0) = *(indata + 0);
            break;
        case 3:
            *(str + 2) = *(indata + 2);
            *(str + 1) = *(indata + 1);
            *(str + 0) = *(indata + 0);
            break;
        case 2:
            *(str + 1) = *(indata + 1);
            *(str + 0) = *(indata + 0);
            break;
        case 1:
            *(str + 0) = *(indata + 0);
            break;
        default:
            break;
        }
    }

    return ret;

}

//函数功能：将小端模式的无符号整数转换为任何字节序的HEX格式
//参数：   in：输入无符号整数，小端模式（逆序）
//         out:输出首址
//         size:输入的字节数，1、2、3、4
//         flag:输出主机字节序，0：小端模式 其它：大端模式 (输入固定为小端模式)
//返回值   0：转换成功，-1：输出位数不足
static int UIntToHex(unsigned int in, void * out, unsigned int size, int flag)
{
    int ret = 0;
    unsigned char * str = (unsigned char *)out;
    unsigned char * indata = (unsigned char *)&in;

    if (size > 4)
    {
        size = 4;
    }

    if (flag)
    {
        switch (size)
        {
        case 4:
            *(str + 3) = *(indata + 0);
            *(str + 2) = *(indata + 1);
            *(str + 1) = *(indata + 2);
            *(str + 0) = *(indata + 3);
            break;
        case 3:
            *(str + 2) = *(indata + 0);
            *(str + 1) = *(indata + 1);
            *(str + 0) = *(indata + 2);
            break;
        case 2:
            *(str + 1) = *(indata + 0);
            *(str + 0) = *(indata + 1);
            break;
        case 1:
            *(str + 0) = *(indata + 0);

            break;
        default:
            break;
        }
    }
    else
    {
        switch (size)
        {
        case 4:
            *(str + 3) = *(indata + 3);
            *(str + 2) = *(indata + 2);
            *(str + 1) = *(indata + 1);
            *(str + 0) = *(indata + 0);
            break;
        case 3:
            *(str + 2) = *(indata + 2);
            *(str + 1) = *(indata + 1);
            *(str + 0) = *(indata + 0);
            break;
        case 2:
            *(str + 1) = *(indata + 1);
            *(str + 0) = *(indata + 0);
            break;
        case 1:
            *(str + 0) = *(indata + 0);
            break;
        default:
            break;
        }
    }

    switch (size)
    {
    case 1:
        if (in > 0x000000FF)
        {
            ret = -1;
        }
        break;
    case 2:
        if (in > 0x0000FFFF)
        {
            ret = -1;
        }
        break;
    case 3:
        if (in > 0x00FFFFFF)
        {
            ret = -1;
        }
        break;
    default:
        break;
    }

    return ret;
}

static unsigned char checkSum(char *data, int len) {
	unsigned int i = 0, sum = 0;

	if ((!data) || len < 0) {
		return -1;
	}

	while (i < len) {
		sum += data[i];
		i++;
	}
	sum %= 256;

	return sum;
}

static int pack_packet232(packet232 *in,unsigned char *out){
   unsigned char *ptr = out;
   int i = 0;

   ptr[START_CODE] = 0x40;
   ptr[COMMAND] = in->command;
   UIntToHex(in->param,ptr+PARAM,4,0);
   UIntToHex(in->size,ptr+SIZE,4,0);
   ptr[FLAG] = in->flag;
   ptr[CHECK_SUM] = checkSum((char *)ptr,CHECK_SUM);
   ptr[END_CODE] = 0x0A;

//   printf(">> ");
//   for(i = 0;i < 13;i++){
//	   printf("%02X ",ptr[i]);
//   }
//   printf("\r\n");

   return 1;
}

//成功　1
//失败　0
static int unpack_packet232(unsigned char *in,packet232 *out){
   packet232 *ptr = out;
   int i = 0;

   if (!in){
	   return 0;
   }
//
//   printf("<< ");
//   for(i = 0;i < 13;i++){
//	   printf("%02X ",in[i]);
//   }
//   printf("\r\n");


   ptr->startcode = in[START_CODE];
   //printf("start code 0x%02X\r\n",ptr->startcode);
   ptr->command = in[COMMAND];
   //printf("command 0x%02X\r\n",ptr->command);
   ptr->param = HexToUInt(in+PARAM,4,0);
   //printf("param 0x%02X\r\n",ptr->param);
   ptr->size = HexToUInt(in+SIZE,4,0);
   //printf("size 0x%02X\r\n",ptr->size);
   ptr->flag = in[FLAG];
   //printf("flag 0x%02X\r\n",ptr->flag);
   ptr->checksum = in[CHECK_SUM];
   //printf("checksum 0x%02X\r\n",ptr->checksum);
   ptr->endcode = in[END_CODE];
   //printf("end code 0x%02X\r\n",ptr->endcode);


   return 1;
}

/*
 * 获取模块的波特率
 * return 1 = true 0 = false
 */
int get_module_baud(){
	packet232 ask, ans;
	unsigned char packet[13];
	int len = 0;
	int port = 3;

	ask.command = 0x03;
	ask.param = 0x00;
	ask.size = 0x00;
	ask.flag = 0x71;

	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port,packet,sizeof(packet));
//	printf("number of bytes write %d\r\n",len);

	while (1) {
		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		if (len <= 0) {
			continue;
		}
//		printf("number of bytes read %d\r\n", len);

		unpack_packet232(packet, &ans);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			return 0;
		}

		if (ans.flag == SUCCESS) {
//			printf(
//					"The value of the requested parameter ID has been successfully read\r\n");
//
//			printf("param id 0x%02X\r\n", ans.param);
//			printf("param value 0x%02X\r\n", ans.size);
			return 1;
		} else if (ans.flag == NOT_FOUND) {
//			printf("There is no requested parameter ID found\r\n");
			return 0;
		}
	}
	return 1;
}


int set_module_baud(int baud){
	packet232 ask, ans;
	unsigned char packet[13];
	int len = 0;
	int port = 3;

	ask.command = 0x01;
	ask.param = 0x00;
	switch(baud){
	case 9600:
		ask.size = 0x31; //param value
		break;
	case 19200:
		ask.size = 0x32; //param value
		break;
	case 38400:
		ask.size = 0x33; //param value
		break;
	case 57600:
		ask.size = 0x34; //param value
		break;
	case 115200:
		ask.size = 0x35; //param value
		break;
	default:
		ask.size = 0x35; //param value
		break;
	}

	ask.flag = 0x71; //param id

	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port,packet,sizeof(packet));

	while (1) {
		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		if (len <= 0){
			continue;
		}
		unpack_packet232(packet, &ans);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			return 0;
		}

		if (ans.flag == SUCCESS) {
//				printf(
//						"Parameter setup is successfully completed.\r\n");

				return 1;
		} else if (ans.flag == NOT_FOUND) {
//				printf("There is no requested parameter ID found\r\n");
				return 0;
		} else if (ans.flag == BUSY) {
//			printf("Module is processing another command.\r\n");
			return 0;
		}
	}

	return 1;
}

/*
 *
 */

static int get_module_template_size(int *n){
	packet232 ask, ans;
	unsigned char packet[13];
	int len = 0;
	int port = 3;

	ask.command = 0x03;
	ask.param = 0x00;
	ask.size = 0x00;
	ask.flag = 0x64;

	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port,packet,sizeof(packet));
//	printf("number of bytes write %d\r\n",len);

	while (1) {
		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		if (len <= 0){
			continue;
		}
//		printf("number of bytes read %d\r\n", len);

		unpack_packet232(packet, &ans);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			return 0;
		}

		if (ans.flag == SUCCESS) {
//			printf(
//					"The value of the requested parameter ID has been successfully read\r\n");
//
//			printf("param id 0x%02X\r\n", ans.param);
//			printf("param value 0x%02X,%d\r\n", ans.size, ans.size);

			*n = ans.size;
			return 1;
		} else if (ans.flag == NOT_FOUND) {
//			printf("There is no requested parameter ID found\r\n");
			return 0;
		}
	}

	return 1;
}

static int get_module_enroll_mode(int *n){
	packet232 ask, ans;
	unsigned char packet[13];
	int len = 0;
	int port = 3;

	ask.command = 0x03;
	ask.param = 0x00;
	ask.size = 0x00;
	ask.flag = 0x65;

	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port,packet,sizeof(packet));
//	printf("number of bytes write %d\r\n",len);

	while (1) {
		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		if (len <= 0){
			continue;
		}
//		printf("number of bytes read %d\r\n", len);

		unpack_packet232(packet, &ans);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			return 0;
		}

		if (ans.flag == SUCCESS) {
//			printf(
//					"The value of the requested parameter ID has been successfully read\r\n");
//
//			printf("param id 0x%02X\r\n", ans.param);
//			printf("param value 0x%02X,%d\r\n", ans.size, ans.size);
			*n = ans.size;
			return 1;
		} else if (ans.flag == NOT_FOUND) {
//			printf("There is no requested parameter ID found\r\n");
			return 0;
		}
	}
	return 1;
}


static int set_module_enroll_mode(){
	packet232 ask, ans;
	unsigned char packet[13];
	int len = 0;
	int port = 3;

	ask.command = 0x01;
	ask.param = 0x00;
	ask.size = 0x41; //param value
	ask.flag = 0x65; //param id

	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port,packet,sizeof(packet));
	while (1) {
		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		if (len <= 0){
			continue;
		}
		unpack_packet232(packet, &ans);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			return 0;
		}

		if (ans.flag == SUCCESS) {
//				printf(
//						"Parameter setup is successfully completed.\r\n");

				return 1;
		} else if (ans.flag == NOT_FOUND) {
//				printf("There is no requested parameter ID found\r\n");
				return 0;
		} else if (ans.flag == BUSY) {
//			printf("Module is processing another command.\r\n");
			return 0;
		}

	}

	return 1;
}

int get_module_sensor_type(){
	packet232 ask, ans;
	unsigned char packet[13];
	int len = 0;
	int port = 3;

	ask.command = 0x03;
	ask.param = 0x00;
	ask.size = 0x00;
	ask.flag = 0x68;

	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port,packet,sizeof(packet));
	//printf("number of bytes write %d\r\n",len);

	while (1) {
		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		if (len <= 0) {
			continue;
		}
		//printf("number of bytes read %d\r\n",len);

		unpack_packet232(packet, &ans);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			return 0;
		}

		if (ans.flag == SUCCESS) {
//			printf(
//					"The value of the requested parameter ID has been successfully read\r\n");
//
//			printf("param id 0x%02X\r\n", ans.param);
//			printf("param value 0x%02X,%d\r\n", ans.size, ans.size);
			return 1;
		} else if (ans.flag == NOT_FOUND) {
//			printf("There is no requested parameter ID found\r\n");
			return 0;
		}
	}
	return 1;
}


static int get_module_image_format(int *n){
	packet232 ask, ans;
	unsigned char packet[13];
	int len = 0;
	int port = 3;

	ask.command = 0x03;
	ask.param = 0x00;
	ask.size = 0x00;
	ask.flag = 0x6C;

	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port,packet,sizeof(packet));
	//printf("number of bytes write %d\r\n",len);

	while (1) {
		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		if (len <= 0) {
			continue;
		}
		//printf("number of bytes read %d\r\n",len);

		unpack_packet232(packet, &ans);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			return 0;
		}

		if (ans.flag == SUCCESS) {
//			printf(
//					"The value of the requested parameter ID has been successfully read\r\n");
//
//			printf("param id 0x%02X\r\n", ans.param);
//			printf("param value 0x%02X,%d\r\n", ans.size, ans.size);
			*n = ans.size;
			return 1;
		} else if (ans.flag == NOT_FOUND) {
//			printf("There is no requested parameter ID found\r\n");
			return 0;
		}
	}
	return 1;
}

//param 0,1,2

static int set_module_image_format(int n){
	packet232 ask, ans;
	unsigned char packet[13];
	int len = 0;
	int port = 3;

	ask.command = 0x01;
	ask.param = 0x00;
	switch(n){
	case Gray:
		ask.size = 0x30;
		break;
	case Binary:
		ask.size = 0x31;
		break;
	case FourBitGray:
		ask.size = 0x32;
		break;
	default:
		ask.size = 0x31;
		break;
	}

	ask.flag = 0x6C;

	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port,packet,sizeof(packet));
	//printf("number of bytes write %d\r\n",len);

	while (1) {
		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		if (len <= 0) {
			continue;
		}
		//printf("number of bytes read %d\r\n",len);

		unpack_packet232(packet, &ans);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			return 0;
		}

		if (ans.flag == SUCCESS) {
//			printf(
//						"Parameter setup is successfully completed.\r\n");
			return 1;
		} else if (ans.flag == NOT_FOUND) {
//			printf("There is no requested parameter ID found\r\n");
			return 0;
		} else if (ans.flag == BUSY) {
//			printf("Module is processing another command.\r\n");
			return 0;
		}
	}
	return 1;
}

int get_module_enrolled_finger(int *n){
	packet232 ask, ans;
	unsigned char packet[13];
	int len = 0;
	int port = 3;

	ask.command = 0x03;
	ask.param = 0x00;
	ask.size = 0x00;
	ask.flag = 0x73;

	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port,packet,sizeof(packet));
	//printf("number of bytes write %d\r\n",len);

	while (1) {
		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		if (len <= 0) {
			continue;
		}
		//printf("number of bytes read %d\r\n",len);

		unpack_packet232(packet, &ans);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			return 0;
		}

		if (ans.flag == SUCCESS) {
//			printf(
//					"The value of the requested parameter ID has been successfully read\r\n");

//			printf("param id 0x%02X\r\n", ans.param);
//			printf("param value 0x%02X,%d\r\n", ans.size, ans.size);
			*n = ans.size / 2;
			return 1;
		} else if (ans.flag == NOT_FOUND) {
//			printf("There is no requested parameter ID found\r\n");
			return 0;
		}
	}

	return 1;
}


static int get_module_available_finger(int *n){
	packet232 ask, ans;
	unsigned char packet[13];
	int len = 0;
	int port = 3 ;

	ask.command = 0x03;
	ask.param = 0x00;
	ask.size = 0x00;
	ask.flag = 0x74;

	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port,packet,sizeof(packet));
//	printf("number of bytes write %d\r\n",len);

	while (1) {
		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		if (len <= 0){
			continue;
		}
//		printf("number of bytes read %d\r\n", len);

		unpack_packet232(packet, &ans);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			return 0;
		}

		if (ans.flag == SUCCESS) {
//			printf(
//					"The value of the requested parameter ID has been successfully read\r\n");

//			printf("param id 0x%02X\r\n", ans.param);
//			printf("param value 0x%02X,%d\r\n", ans.size, ans.size);
			*n = ans.size / 2;
			return 1;
		} else if (ans.flag == NOT_FOUND) {
//			printf("There is no requested parameter ID found\r\n");
			return 0;
		}
	}

	return 1;
}


static int get_module_send_scan_success(int *n){
	packet232 ask, ans;
	unsigned char packet[13];
	int len = 0;
	int port = 3;

	ask.command = 0x03;
	ask.param = 0x00;
	ask.size = 0x00;
	ask.flag = 0x75;

	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port,packet,sizeof(packet));
//	printf("number of bytes write %d\r\n",len);

	while (1) {
		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		if (len <= 0) {
			continue;
		}
//		printf("number of bytes read %d\r\n", len);

		unpack_packet232(packet, &ans);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			return 0;
		}

		if (ans.flag == SUCCESS) {
//			printf(
//					"The value of the requested parameter ID has been successfully read\r\n");
//
//			printf("param id 0x%02X\r\n", ans.param);
//			printf("param value 0x%02X,%d\r\n", ans.size, ans.size);
			*n = ans.size;
			return 1;
		} else if (ans.flag == NOT_FOUND) {
//			printf("There is no requested parameter ID found\r\n");
			return 0;
		}
	}

	return 1;
}


int get_module_image_quality(){
	packet232 ask, ans;
	unsigned char packet[13];
	int len = 0;
	int port = 3;

	ask.command = 0x03;
	ask.param = 0x00;
	ask.size = 0x00;
	ask.flag = 0x81;

	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port,packet,sizeof(packet));
//	printf("number of bytes write %d\r\n",len);

	while (1) {
		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		if (len <= 0){
			continue;
		}

//		printf("number of bytes read %d\r\n", len);

		unpack_packet232(packet, &ans);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			return 0;
		}

		if (ans.flag == SUCCESS) {
//			printf(
//					"The value of the requested parameter ID has been successfully read\r\n");
//
//			printf("param id 0x%02X\r\n", ans.param);
//			printf("param value 0x%02X,%d\r\n", ans.size, ans.size);
			return 1;
		} else if (ans.flag == NOT_FOUND) {
//			printf("There is no requested parameter ID found\r\n");
			return 0;
		}
	}

	return 1;
}

int reset_module() {
	packet232 ask, ans;
	unsigned char packet[13];
	int len = 0;
	int port = 3;

	ask.command = 0xD0;
	ask.param = 0x00;
	ask.size = 0x00;
	ask.flag = 0x00;

	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port, packet, sizeof(packet));
//	printf("number of bytes write %d\r\n", len);

	while (1) {
		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));

		if (len <= 0){
			continue;
		}

//		printf("number of bytes read %d\r\n", len);

		unpack_packet232(packet, &ans);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			return 0;
		}

		if (ans.flag == SUCCESS) {
//			printf(
//					"The value of the requested parameter ID has been successfully read\r\n");
//
//			printf("param id 0x%02X\r\n", ans.param);
//			printf("param value 0x%02X,%d\r\n", ans.size, ans.size);
			return 1;
		} else if (ans.flag == NOT_FOUND) {
//			printf("There is no requested parameter ID found\r\n");
			return 0;
		}
	}

	return 1;
}

int power_off_module() {
	packet232 ask, ans;
	unsigned char packet[13];
	int len = 0;
	int port = 3;

	ask.command = 0xD2;
	ask.param = 0x00;
	ask.size = 0x00;
	ask.flag = 0x00;

	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port, packet, sizeof(packet));
//	printf("number of bytes write %d\r\n", len);

	while (1) {

		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		if (len <= 0){
			continue;
		}
//		printf("number of bytes read %d\r\n", len);

		unpack_packet232(packet, &ans);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			return 0;
		}

		if (ans.flag == SUCCESS) {
//			printf(
//					"The value of the requested parameter ID has been successfully read\r\n");
//
//			printf("param id 0x%02X\r\n", ans.param);
//			printf("param value 0x%02X,%d\r\n", ans.size, ans.size);
			return 1;
		} else if (ans.flag == NOT_FOUND) {
//			printf("There is no requested parameter ID found\r\n");
			return 0;
		}
	}

	return 1;
}


static int check_user_id(int user_id){
	packet232 ask, ans;
	unsigned char packet[13];
	int len = 0;
	int port = 3;

	ask.command = 0x19;
	ask.param = user_id;
	ask.size = 0x00;
	ask.flag = 0x00;


	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port, packet, sizeof(packet));
	//printf("number of bytes write %d\r\n", len);

	while (1) {
		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		//printf("number of bytes read %d\r\n", len);

		if (len <= 0){
			continue;
		}

		unpack_packet232(packet, &ans);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			return 0;
		}

		if (ans.flag == EXIST_ID) {
//			printf("The user ID exists in the module\r\n");
			return 1;
		} else if (ans.flag == NOT_FOUND) {
//			printf("The user ID is not found in the moduler\r\n");
			return 0;
		}
	}

	return 1;

}

int _read_template(int user_id) {
	packet232 ask, ans;
	unsigned char packet[13];
	unsigned char data[64];
	int len = 0, data_len1 = 0, data_len2 = 0, data_len = 0;
	int i = 0;
	int port = 3;
	char *ptr = NULL;

	//first
	ask.command = 0x14;
	ask.param = user_id;
	ask.size = 0x00;
	ask.flag = 0x00;

	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port, packet, sizeof(packet));

	while (1) {
		memset(packet, 0, sizeof(packet));

		len = read_port(port, packet, sizeof(packet));
		if (len <= 0) {
			continue;
		}
		//printf("number of bytes write %d\r\n", len);
		unpack_packet232(packet, &ans);
		if (ans.command != ask.command) {
//			printf("command error\r\n");
			return 0;
		}

		if (ans.flag == SUCCESS) {
			data_len1 = ans.size + 1;
			g_template_size = ans.size;
//			printf("Template size 0x%02X,%d\r\n", ans.size, ans.size);
//
//			printf("Fingerprint template reading is successfully completed.\r\n");
			break;
		}else if (ans.flag == NOT_FOUND) {
//			printf("There is no fingerprint template or matching user ID found.\r\n");
			return 0;
		}else if (ans.flag == CONTINUE){
//			printf("When there are more users to be enrolled.\r\n");
			continue;
		}
	}

	//then
	memset(g_template,0,sizeof(g_template));
	ptr = g_template;
	while (1) {
		memset(data, 0, sizeof(data));

		data_len =
				data_len1 - data_len2 > sizeof(data) ?
						sizeof(data) : data_len1 - data_len2;
		len = read_port(port, data, data_len);

		if (len <= 0) {
			continue;
		}
		//printf("number of bytes read %d\r\n", len);
//
//		printf("<< ");
//		for (i = 0; i < len; i++) {
//			if (i % 16 == 0) {
//				printf("\r\n");
//			}
//			printf("%02X ", data[i]);
//		}
//		printf("\r\n");

		memcpy(ptr, data, len);
		ptr += len;

		data_len2 += len;
//		printf("data_len 2 %d\r\n", data_len2);
		if ((data_len2 >= data_len1) || (data[len - 1] == 0x0A)) {
			break;
		}
	}

	if (data_len2 >= data_len1) {
		return 1;
	} else {
		return 0;
	}
}

int _read_image() {
	packet232 ask, ans;
	unsigned char packet[13];
	unsigned char data[64];
	int len = 0, data_len1 = 0, data_len2 = 0, data_len = 0;
	int i = 0;
	int port = 3;
	char *ptr = NULL;

	//first
	ask.command = 0x20;
	ask.param = 0x00;
	ask.size = 0x00;
	ask.flag = 0x00;

	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port, packet, sizeof(packet));

	while (1) {
		memset(packet, 0, sizeof(packet));

		len = read_port(port, packet, sizeof(packet));
		if (len <= 0) {
			continue;
		}
		//printf("number of bytes write %d\r\n", len);
		unpack_packet232(packet, &ans);
		if (ans.command != ask.command) {
//			printf("command error\r\n");
			return 0;
		}

		if (ans.flag == SUCCESS) {
			data_len1 = ans.size + 1;
			total_bin_size = ans.size;
//
//			printf("total_bin_size = %d\r\n",total_bin_size);
//			printf("Data size 0x%02X,%d\r\n", ans.size, ans.size);
//
//			printf("Fingerprint image reading is successfully completed\r\n");
			break;
		}
	}

	//then
	memset(g_bin_buffer,0,sizeof(g_bin_buffer));
	ptr = g_bin_buffer;
	while (1) {
		memset(data, 0, sizeof(data));

		data_len =
				data_len1 - data_len2 > sizeof(data) ?
						sizeof(data) : data_len1 - data_len2;
		len = read_port(port, data, data_len);

		if (len <= 0) {
			continue;
		}
		//printf("number of bytes read %d\r\n", len);

//		printf("<< ");
//		for (i = 0; i < len; i++) {
//			if (i % 16 == 0) {
//				printf("\r\n");
//			}
//			printf("%02X ", data[i]);
//		}
//		printf("\r\n");

		memcpy(ptr, data, len);
		ptr += len;

		data_len2 += len;
//		printf("data_len 2 %d\r\n", data_len2);
		if ((data_len2 >= data_len1) || (data[len - 1] == 0x0A)) {
			break;
		}
	}

	if (data_len2 >= data_len1) {
		return 1;
	} else {
		return 0;
	}
}

//one fingerprint one request one replay
//g_enroll_mode = 0x30
//g_send_scan_success = 0x30
static int _enroll_by_scan1(int user_id) {
	packet232 ask, ans;
	unsigned char packet[13];
//	char uid[16];
	int len;
	int ret = 0;

//	int i = 0;
	int j = 0;
//	int sum = 0;
	int port = 3;

	printf("enroll pls press finger \n");

	ret = check_user_id(user_id);
	if (ret == 1) {
//		printf("user ID exists\r\n");

		ask.command = 0x05;
		ask.param = user_id;
		ask.size = 0x00;
		ask.flag = 0x71;
	} else {
//		printf("user ID is not found\r\n");
		ask.command = 0x05;
		ask.param = user_id;
		ask.size = 0x00;
		ask.flag = 0x00;
	}
	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port, packet, sizeof(packet));
	//printf("number of bytes write %d\r\n", len);

	while (1) {

		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		if (len <= 0) {
			continue;
		}
		//printf("number of bytes read %d\r\n", len);
		unpack_packet232(packet, &ans);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			break;
		}

		if (ans.flag == SUCCESS) {
//			printf("param user id 0x%02X,%d\r\n", ans.param, ans.param);
//			printf("param Image Quality 0x%02X,%d\r\n", ans.size, ans.size);
			j = 2;
			break;
		} else if (ans.flag == SCAN_SUCCESS) {
//			printf("Fingerprint scanning is successfully  completed\r\n");
			continue;
		} else if (ans.flag == SCAN_FAIL) {
//			printf("Sensor or fingerprint input has failed\r\n");
			continue;
		} else if (ans.flag == TRY_AGAIN) {
//			printf("An error occurred during enrollment process\r\n");
			continue;
		} else if (ans.flag == TIME_OUT) {
//			printf("Timeout for fingerprint input\r\n");
			break;
		} else if (ans.flag == MEM_FULL) {
//			printf("Flash memory is full\r\n");
			break;
		} else if (ans.flag == FINGER_LIMIT) {
//			printf(
//					"The number of fingerprints enrolled in same ID exceeds its limit (10)\r\n");
			break;
		} else if (ans.flag == EXIST_ID) {
//			printf(
//					"The requested user ID exists. ( In case CHECK_ID flag is used )\r\n");
			break;
		} else if (ans.flag == EXIST_FINGER) {
//			printf(
//					"The same finger is already enrolled. (In case CHECK_FINGER or \
//				CHECK_FINGER_AUTO_ID is used.)\
//				The same finger is already enrolled as non-duress.(In case \
//				ADD_DURESS option is given.)\r\n");
			break;
		} else if (ans.flag == INVALID_ID) {
//			printf(
//					"The requested user ID is invalid. Note that ‘0x0000’  cannot be used\r\n");
			break;
		}
	}

	if (j > 1) {
//		printf("Fingerprint enrollment is successfully completed\r\n");
		return 1;
	} else {
		return 0;
	}
}


//one fingerprint one request two replay
//g_enroll_mode = 0x30
//g_send_scan_success = 0x31
static int _enroll_by_scan2(int user_id) {
	packet232 ask, ans;
	unsigned char packet[13];
//	char uid[16];
	int len;
	int ret = 0;

	int j = 0;
	int port = 3;

	printf("enroll pls press finger \n");

	ret = check_user_id(user_id);
	if (ret == 1) {
//		printf("user ID exists\r\n");

		ask.command = 0x05;
		ask.param = user_id;
		ask.size = 0x00;
		ask.flag = 0x71;
	} else {
//		printf("user ID is not found\r\n");
		ask.command = 0x05;
		ask.param = user_id;
		ask.size = 0x00;
		ask.flag = 0x00;
	}
	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port, packet, sizeof(packet));
	//printf("number of bytes write %d\r\n", len);

	while (1) {

		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		if (len <= 0) {
			continue;
		}

		unpack_packet232(packet, &ans);
		//printf("number of bytes read %d\r\n", len);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			break;
		}

		if (ans.flag == SUCCESS) {
//			printf("param user id 0x%02X,%d\r\n", ans.param, ans.param);
//			printf("param Image Quality 0x%02X,%d\r\n", ans.size, ans.size);
			j = 2;
			break;
		} else if (ans.flag == SCAN_SUCCESS) {
//			printf("Fingerprint scanning is successfully  completed\r\n");
			continue;
		} else if (ans.flag == SCAN_FAIL) {
//			printf("Sensor or fingerprint input has failed\r\n");
			continue;
		} else if (ans.flag == TRY_AGAIN) {
//			printf("An error occurred during enrollment process\r\n");
			continue;
		} else if (ans.flag == TIME_OUT) {
//			printf("Timeout for fingerprint input\r\n");
			break;
		} else if (ans.flag == MEM_FULL) {
//			printf("Flash memory is full\r\n");
			break;
		} else if (ans.flag == FINGER_LIMIT) {
//			printf(
//					"The number of fingerprints enrolled in same ID exceeds its limit (10)\r\n");
			break;
		} else if (ans.flag == EXIST_ID) {
//			printf(
//					"The requested user ID exists. ( In case CHECK_ID flag is used )\r\n");
			break;
		} else if (ans.flag == EXIST_FINGER) {
//			printf(
//					"The same finger is already enrolled. (In case CHECK_FINGER or \
//				CHECK_FINGER_AUTO_ID is used.)\
//				The same finger is already enrolled as non-duress.(In case \
//				ADD_DURESS option is given.)\r\n");
			break;
		} else if (ans.flag == INVALID_ID) {
//			printf(
//					"The requested user ID is invalid. Note that ‘0x0000’  cannot be used\r\n");
			break;
		}
	}

	if (j > 1) {
//		printf("Fingerprint enrollment is successfully completed\r\n");
		return 1;
	} else {
		return 0;
	}
}


//one fingerprint one request two replay
//g_enroll_mode = 0x31
//g_send_scan_success = 0x30
static int _enroll_by_scan3(int user_id) {
	packet232 ask, ans;
	unsigned char packet[13];
//	char uid[16];
	int len;
	int ret = 0;

	int j = 0;
	int port = 3;

	printf("enroll pls press finger \n");

	ret = check_user_id(user_id);
	if (ret == 1) {
//		printf("user ID exists\r\n");

		ask.command = 0x05;
		ask.param = user_id;
		ask.size = 0x00;
		ask.flag = 0x71;
	} else {
//		printf("user ID is not found\r\n");
		ask.command = 0x05;
		ask.param = user_id;
		ask.size = 0x00;
		ask.flag = 0x00;
	}
	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port, packet, sizeof(packet));
	//printf("number of bytes write %d\r\n", len);

	while (1) {

		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		if (len <= 0) {
			continue;
		}

		unpack_packet232(packet, &ans);
		//printf("number of bytes read %d\r\n", len);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			break;
		}

		if (ans.flag == SUCCESS) {
//			printf("param user id 0x%02X,%d\r\n", ans.param, ans.param);
//			printf("param Image Quality 0x%02X,%d\r\n", ans.size, ans.size);
			j++;
			if (j < 2) {
				printf("enroll pls press finger again \n");
			}
		} else if (ans.flag == SCAN_SUCCESS) {
//			printf("Fingerprint scanning is successfully  completed\r\n");
			continue;
		} else if (ans.flag == SCAN_FAIL) {
//			printf("Sensor or fingerprint input has failed\r\n");
			continue;
		} else if (ans.flag == TRY_AGAIN) {
//			printf("An error occurred during enrollment process\r\n");
			continue;
		} else if (ans.flag == TIME_OUT) {
//			printf("Timeout for fingerprint input\r\n");
			break;
		} else if (ans.flag == MEM_FULL) {
//			printf("Flash memory is full\r\n");
			break;
		} else if (ans.flag == FINGER_LIMIT) {
//			printf(
//					"The number of fingerprints enrolled in same ID exceeds its limit (10)\r\n");
			break;
		} else if (ans.flag == EXIST_ID) {
//			printf(
//					"The requested user ID exists. ( In case CHECK_ID flag is used )\r\n");
			break;
		} else if (ans.flag == EXIST_FINGER) {
//			printf(
//					"The same finger is already enrolled. (In case CHECK_FINGER or \
//				CHECK_FINGER_AUTO_ID is used.)\
//				The same finger is already enrolled as non-duress.(In case \
//				ADD_DURESS option is given.)\r\n");
			break;
		} else if (ans.flag == INVALID_ID) {
//			printf(
//					"The requested user ID is invalid. Note that ‘0x0000’  cannot be used\r\n");
			break;
		}
	}

	if (j > 1) {
//		printf("Fingerprint enrollment is successfully completed\r\n");
		return 1;
	} else {
		return 0;
	}
}


//one fingerprint one request four replay
//g_enroll_mode = 0x31
//g_send_scan_success = 0x31
static int _enroll_by_scan4(int user_id) {
	packet232 ask, ans;
	unsigned char packet[13];
//	char uid[16];
	int len;
	int ret = 0;

	int j = 0;
	int port = 3;

	printf("enroll pls press finger \n");

	ret = check_user_id(user_id);
	if (ret == 1) {
//		printf("user ID exists\r\n");

		ask.command = 0x05;
		ask.param = user_id;
		ask.size = 0x00;
		ask.flag = 0x71;
	} else {
//		printf("user ID is not found\r\n");
		ask.command = 0x05;
		ask.param = user_id;
		ask.size = 0x00;
		ask.flag = 0x00;
	}
	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port, packet, sizeof(packet));
	//printf("number of bytes write %d\r\n", len);

	while (1) {

		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		if (len <= 0) {
			continue;
		}

		unpack_packet232(packet, &ans);
		//printf("number of bytes read %d\r\n", len);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			break;
		}

		if (ans.flag == SUCCESS) {
//			printf("param user id 0x%02X,%d\r\n", ans.param, ans.param);
//			printf("param Image Quality 0x%02X,%d\r\n", ans.size, ans.size);
			j++;
			if (j < 2) {
				printf("enroll pls press finger again \n");
			}
		} else if (ans.flag == SCAN_SUCCESS) {
//			printf("Fingerprint scanning is successfully  completed\r\n");
			continue;
		} else if (ans.flag == SCAN_FAIL) {
//			printf("Sensor or fingerprint input has failed\r\n");
			continue;
		} else if (ans.flag == TRY_AGAIN) {
//			printf("An error occurred during enrollment process\r\n");
			continue;
		} else if (ans.flag == TIME_OUT) {
//			printf("Timeout for fingerprint input\r\n");
			break;
		} else if (ans.flag == MEM_FULL) {
//			printf("Flash memory is full\r\n");
			break;
		} else if (ans.flag == FINGER_LIMIT) {
//			printf(
//					"The number of fingerprints enrolled in same ID exceeds its limit (10)\r\n");
			break;
		} else if (ans.flag == EXIST_ID) {
//			printf(
//					"The requested user ID exists. ( In case CHECK_ID flag is used )\r\n");
			break;
		} else if (ans.flag == EXIST_FINGER) {
//			printf(
//					"The same finger is already enrolled. (In case CHECK_FINGER or \
//				CHECK_FINGER_AUTO_ID is used.)\
//				The same finger is already enrolled as non-duress.(In case \
//				ADD_DURESS option is given.)\r\n");
			break;
		} else if (ans.flag == INVALID_ID) {
//			printf(
//					"The requested user ID is invalid. Note that ‘0x0000’  cannot be used\r\n");
			break;
		}
	}

	if (j > 1) {
//		printf("Fingerprint enrollment is successfully completed\r\n");
		return 1;
	} else {
		return 0;
	}
}

//one fingerprint two request two replay
//g_enroll_mode = 0x32
//g_send_scan_success = 0x30
static int _enroll_by_scan5(int user_id) {
	packet232 ask, ans;
	unsigned char packet[13];
//	char uid[16];
	int len;
	int ret = 0;

	int j = 0;
	int port = 3;

	printf("enroll pls press finger \n");

	ret = check_user_id(user_id);
	if (ret == 1) {
//		printf("user ID exists\r\n");

		ask.command = 0x05;
		ask.param = user_id;
		ask.size = 0x00;
		ask.flag = 0x71;
	} else {
//		printf("user ID is not found\r\n");
		ask.command = 0x05;
		ask.param = user_id;
		ask.size = 0x00;
		ask.flag = 0x00;
	}
	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port, packet, sizeof(packet));
	//printf("number of bytes write %d\r\n", len);

	while (1) {

		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		if (len <= 0) {
			continue;
		}

		unpack_packet232(packet, &ans);
		//printf("number of bytes read %d\r\n", len);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			break;
		}

		if (ans.flag == SUCCESS) {
//			printf("param user id 0x%02X,%d\r\n", ans.param, ans.param);
//			printf("param Image Quality 0x%02X,%d\r\n", ans.size, ans.size);
			j = 1;
			break;
		} else if (ans.flag == SCAN_SUCCESS) {
//			printf("Fingerprint scanning is successfully  completed\r\n");
			continue;
		} else if (ans.flag == SCAN_FAIL) {
//			printf("Sensor or fingerprint input has failed\r\n");
			continue;
		} else if (ans.flag == TRY_AGAIN) {
//			printf("An error occurred during enrollment process\r\n");
			continue;
		} else if (ans.flag == TIME_OUT) {
//			printf("Timeout for fingerprint input\r\n");
			break;
		} else if (ans.flag == MEM_FULL) {
//			printf("Flash memory is full\r\n");
			break;
		} else if (ans.flag == FINGER_LIMIT) {
//			printf(
//					"The number of fingerprints enrolled in same ID exceeds its limit (10)\r\n");
			break;
		} else if (ans.flag == EXIST_ID) {
//			printf(
//					"The requested user ID exists. ( In case CHECK_ID flag is used )\r\n");
			break;
		} else if (ans.flag == EXIST_FINGER) {
//			printf(
//					"The same finger is already enrolled. (In case CHECK_FINGER or \
//				CHECK_FINGER_AUTO_ID is used.)\
//				The same finger is already enrolled as non-duress.(In case \
//				ADD_DURESS option is given.)\r\n");
			break;
		} else if (ans.flag == INVALID_ID) {
//			printf(
//					"The requested user ID is invalid. Note that ‘0x0000’  cannot be used\r\n");
			break;
		}
	}

	if (j == 0){
		return 0;
	}

	ask.command = 0x05;
	ask.param = user_id;
	ask.size = 0x00;
	ask.flag = 0x74;
	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port, packet, sizeof(packet));
	//printf("number of bytes write %d\r\n", len);

	while (1) {

		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		if (len <= 0) {
			continue;
		}

		unpack_packet232(packet, &ans);
		//printf("number of bytes read %d\r\n", len);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			break;
		}

		if (ans.flag == SUCCESS) {
//			printf("param user id 0x%02X,%d\r\n", ans.param, ans.param);
//			printf("param Image Quality 0x%02X,%d\r\n", ans.size, ans.size);
			j = 2;
			break;
		} else if (ans.flag == SCAN_SUCCESS) {
//			printf("Fingerprint scanning is successfully  completed\r\n");
			continue;
		} else if (ans.flag == SCAN_FAIL) {
//			printf("Sensor or fingerprint input has failed\r\n");
			continue;
		} else if (ans.flag == TRY_AGAIN) {
//			printf("An error occurred during enrollment process\r\n");
			continue;
		} else if (ans.flag == TIME_OUT) {
//			printf("Timeout for fingerprint input\r\n");
			break;
		} else if (ans.flag == MEM_FULL) {
//			printf("Flash memory is full\r\n");
			break;
		} else if (ans.flag == FINGER_LIMIT) {
//			printf(
//					"The number of fingerprints enrolled in same ID exceeds its limit (10)\r\n");
			break;
		} else if (ans.flag == EXIST_ID) {
//			printf(
//					"The requested user ID exists. ( In case CHECK_ID flag is used )\r\n");
			break;
		} else if (ans.flag == EXIST_FINGER) {
//			printf(
//					"The same finger is already enrolled. (In case CHECK_FINGER or \
//					CHECK_FINGER_AUTO_ID is used.)\
//					The same finger is already enrolled as non-duress.(In case \
//					ADD_DURESS option is given.)\r\n");
			break;
		} else if (ans.flag == INVALID_ID) {
//			printf(
//					"The requested user ID is invalid. Note that ‘0x0000’  cannot be used\r\n");
			break;
		}
	}

	if (j > 1) {
//		printf("Fingerprint enrollment is successfully completed\r\n");
		return 1;
	} else {
		return 0;
	}
}


//one fingerprint two request four replay
//g_enroll_mode = 0x32
//g_send_scan_success = 0x31
static int _enroll_by_scan6(int user_id) {
	packet232 ask, ans;
	unsigned char packet[13];
//	char uid[16];
	int len;
	int ret = 0;

	int j = 0;
	int port = 3;

	printf("enroll pls press finger \n");

	ret = check_user_id(user_id);
	if (ret == 1) {
//		printf("user ID exists\r\n");

		ask.command = 0x05;
		ask.param = user_id;
		ask.size = 0x00;
		ask.flag = 0x71;
	} else {
//		printf("user ID is not found\r\n");
		ask.command = 0x05;
		ask.param = user_id;
		ask.size = 0x00;
		ask.flag = 0x00;
	}
	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port, packet, sizeof(packet));
	//printf("number of bytes write %d\r\n", len);

	while (1) {

		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		if (len <= 0) {
			continue;
		}

		unpack_packet232(packet, &ans);
		//printf("number of bytes read %d\r\n", len);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			break;
		}

		if (ans.flag == SUCCESS) {
//			printf("param user id 0x%02X,%d\r\n", ans.param, ans.param);
//			printf("param Image Quality 0x%02X,%d\r\n", ans.size, ans.size);
			j = 1;
			break;
		} else if (ans.flag == SCAN_SUCCESS) {
//			printf("Fingerprint scanning is successfully  completed\r\n");
			continue;
		} else if (ans.flag == SCAN_FAIL) {
//			printf("Sensor or fingerprint input has failed\r\n");
			continue;
		} else if (ans.flag == TRY_AGAIN) {
//			printf("An error occurred during enrollment process\r\n");
			continue;
		} else if (ans.flag == TIME_OUT) {
//			printf("Timeout for fingerprint input\r\n");
			break;
		} else if (ans.flag == MEM_FULL) {
//			printf("Flash memory is full\r\n");
			break;
		} else if (ans.flag == FINGER_LIMIT) {
//			printf(
//					"The number of fingerprints enrolled in same ID exceeds its limit (10)\r\n");
			break;
		} else if (ans.flag == EXIST_ID) {
//			printf(
//					"The requested user ID exists. ( In case CHECK_ID flag is used )\r\n");
			break;
		} else if (ans.flag == EXIST_FINGER) {
//			printf(
//					"The same finger is already enrolled. (In case CHECK_FINGER or \
//				CHECK_FINGER_AUTO_ID is used.)\
//				The same finger is already enrolled as non-duress.(In case \
//				ADD_DURESS option is given.)\r\n");
			break;
		} else if (ans.flag == INVALID_ID) {
//			printf(
//					"The requested user ID is invalid. Note that ‘0x0000’  cannot be used\r\n");
			break;
		}
	}

	if (j == 0){
		return 0;
	}

	ask.command = 0x05;
	ask.param = user_id;
	ask.size = 0x00;
	ask.flag = 0x74;
	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port, packet, sizeof(packet));
	//printf("number of bytes write %d\r\n", len);

	while (1) {

		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		if (len <= 0) {
			continue;
		}

		unpack_packet232(packet, &ans);
		//printf("number of bytes read %d\r\n", len);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			break;
		}

		if (ans.flag == SUCCESS) {
//			printf("param user id 0x%02X,%d\r\n", ans.param, ans.param);
//			printf("param Image Quality 0x%02X,%d\r\n", ans.size, ans.size);
			j = 2;
			break;
		} else if (ans.flag == SCAN_SUCCESS) {
//			printf("Fingerprint scanning is successfully  completed\r\n");
			continue;
		} else if (ans.flag == SCAN_FAIL) {
//			printf("Sensor or fingerprint input has failed\r\n");
			continue;
		} else if (ans.flag == TRY_AGAIN) {
//			printf("An error occurred during enrollment process\r\n");
			continue;
		} else if (ans.flag == TIME_OUT) {
//			printf("Timeout for fingerprint input\r\n");
			break;
		} else if (ans.flag == MEM_FULL) {
//			printf("Flash memory is full\r\n");
			break;
		} else if (ans.flag == FINGER_LIMIT) {
//			printf(
//					"The number of fingerprints enrolled in same ID exceeds its limit (10)\r\n");
			break;
		} else if (ans.flag == EXIST_ID) {
//			printf(
//					"The requested user ID exists. ( In case CHECK_ID flag is used )\r\n");
			break;
		} else if (ans.flag == EXIST_FINGER) {
//			printf(
//					"The same finger is already enrolled. (In case CHECK_FINGER or \
//					CHECK_FINGER_AUTO_ID is used.)\
//					The same finger is already enrolled as non-duress.(In case \
//					ADD_DURESS option is given.)\r\n");
			break;
		} else if (ans.flag == INVALID_ID) {
//			printf(
//					"The requested user ID is invalid. Note that ‘0x0000’  cannot be used\r\n");
			break;
		}
	}

	if (j > 1) {
//		printf("Fingerprint enrollment is successfully completed\r\n");
		return 1;
	} else {
		return 0;
	}
}

//two fingerprint one request two replay
//g_enroll_mode = 0x41
//g_send_scan_success = 0x30
static int _enroll_by_scan7(int user_id) {
	packet232 ask, ans;
	unsigned char packet[13];
//	char uid[16];
	int len;
	int ret = 0;

	int j = 0;
	int port = 3;

	printf("enroll pls press finger \n");

	ret = check_user_id(user_id);
	if (ret == 1) {
//		printf("user ID exists\r\n");

		ask.command = 0x05;
		ask.param = user_id;
		ask.size = 0x00;
		ask.flag = 0x71;
	} else {
//		printf("user ID is not found\r\n");
		ask.command = 0x05;
		ask.param = user_id;
		ask.size = 0x00;
		ask.flag = 0x00;
	}
	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port, packet, sizeof(packet));
	//printf("number of bytes write %d\r\n", len);

	while (1) {

		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		if (len <= 0) {
			continue;
		}

		unpack_packet232(packet, &ans);
		//printf("number of bytes read %d\r\n", len);

		if (ans.command != ask.command) {
			printf("command error\r\n");
			break;
		}

		if (ans.flag == SUCCESS) {
//			printf("param user id 0x%02X,%d\r\n", ans.param, ans.param);
//			printf("param Image Quality 0x%02X,%d\r\n", ans.size, ans.size);
			j++;
			if (j < 2) {
				printf("enroll pls press finger again \n");
			}
		} else if (ans.flag == SCAN_SUCCESS) {
//			printf("Fingerprint scanning is successfully  completed\r\n");
			continue;
		} else if (ans.flag == SCAN_FAIL) {
//			printf("Sensor or fingerprint input has failed\r\n");
			continue;
		} else if (ans.flag == TRY_AGAIN) {
//			printf("An error occurred during enrollment process\r\n");
			continue;
		} else if (ans.flag == TIME_OUT) {
//			printf("Timeout for fingerprint input\r\n");
			break;
		} else if (ans.flag == MEM_FULL) {
//			printf("Flash memory is full\r\n");
			break;
		} else if (ans.flag == FINGER_LIMIT) {
//			printf(
//					"The number of fingerprints enrolled in same ID exceeds its limit (10)\r\n");
			break;
		} else if (ans.flag == EXIST_ID) {
//			printf(
//					"The requested user ID exists. ( In case CHECK_ID flag is used )\r\n");
			break;
		} else if (ans.flag == EXIST_FINGER) {
//			printf(
//					"The same finger is already enrolled. (In case CHECK_FINGER or \
//				CHECK_FINGER_AUTO_ID is used.)\
//				The same finger is already enrolled as non-duress.(In case \
//				ADD_DURESS option is given.)\r\n");
			break;
		} else if (ans.flag == INVALID_ID) {
//			printf(
//					"The requested user ID is invalid. Note that ‘0x0000’  cannot be used\r\n");
			break;
		}
	}

	if (j > 1) {
//		printf("Fingerprint enrollment is successfully completed\r\n");
		return 1;
	} else {
		return 0;
	}
}


//two fingerprint one request four replay
//g_enroll_mode = 0x41
//g_send_scan_success = 0x31
static int _enroll_by_scan8(int user_id) {
	packet232 ask, ans;
	unsigned char packet[13];
//	char uid[16];
	int len;
	int ret = 0;

	int j = 0;
	int port = 3;

	printf("enroll pls press finger \n");

	ret = check_user_id(user_id);
	if (ret == 1) {
//		printf("user ID exists\r\n");

		ask.command = 0x05;
		ask.param = user_id;
		ask.size = 0x00;
		ask.flag = 0x71;
	} else {
//		printf("user ID is not found\r\n");
		ask.command = 0x05;
		ask.param = user_id;
		ask.size = 0x00;
		ask.flag = 0x00;
	}
	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port, packet, sizeof(packet));
	//printf("number of bytes write %d\r\n", len);

	while (1) {

		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		if (len <= 0) {
			continue;
		}

		unpack_packet232(packet, &ans);
		//printf("number of bytes read %d\r\n", len);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			break;
		}

		if (ans.flag == SUCCESS) {
//			printf("param user id 0x%02X,%d\r\n", ans.param, ans.param);
//			printf("param Image Quality 0x%02X,%d\r\n", ans.size, ans.size);
			j++;
			if (j < 2) {
				printf("enroll pls press finger again \n");
			}else{
				break;
			}
		} else if (ans.flag == SCAN_SUCCESS) {
//			printf("Fingerprint scanning is successfully  completed\r\n");
			continue;
		} else if (ans.flag == SCAN_FAIL) {
//			printf("Sensor or fingerprint input has failed\r\n");
			continue;
		} else if (ans.flag == TRY_AGAIN) {
//			printf("An error occurred during enrollment process\r\n");
			continue;
		} else if (ans.flag == TIME_OUT) {
//			printf("Timeout for fingerprint input\r\n");
			break;
		} else if (ans.flag == MEM_FULL) {
//			printf("Flash memory is full\r\n");
			break;
		} else if (ans.flag == FINGER_LIMIT) {
//			printf(
//					"The number of fingerprints enrolled in same ID exceeds its limit (10)\r\n");
			break;
		} else if (ans.flag == EXIST_ID) {
//			printf(
//					"The requested user ID exists. ( In case CHECK_ID flag is used )\r\n");
			break;
		} else if (ans.flag == EXIST_FINGER) {
//			printf(
//					"The same finger is already enrolled. (In case CHECK_FINGER or \
//				CHECK_FINGER_AUTO_ID is used.)\
//				The same finger is already enrolled as non-duress.(In case \
//				ADD_DURESS option is given.)\r\n");
			break;
		} else if (ans.flag == INVALID_ID) {
//			printf(
//					"The requested user ID is invalid. Note that ‘0x0000’  cannot be used\r\n");
			break;
		}
	}

	if (j > 1) {
//		printf("Fingerprint enrollment is successfully completed\r\n");
		return 1;
	} else {
		return 0;
	}
}


//two fingerprint two request two replay
//g_enroll_mode = 0x32
//g_send_scan_success = 0x30
static int _enroll_by_scan9(int user_id) {
	packet232 ask, ans;
	unsigned char packet[13];
//	char uid[16];
	int len;
	int ret = 0;

	int j = 0;
	int port = 3;

	printf("enroll pls press finger \n");

	ret = check_user_id(user_id);
	if (ret == 1) {
//		printf("user ID exists\r\n");

		ask.command = 0x05;
		ask.param = user_id;
		ask.size = 0x00;
		ask.flag = 0x71;
	} else {
//		printf("user ID is not found\r\n");
		ask.command = 0x05;
		ask.param = user_id;
		ask.size = 0x00;
		ask.flag = 0x00;
	}
	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port, packet, sizeof(packet));
	//printf("number of bytes write %d\r\n", len);

	while (1) {

		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		if (len <= 0) {
			continue;
		}

		unpack_packet232(packet, &ans);
		//printf("number of bytes read %d\r\n", len);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			break;
		}

		if (ans.flag == SUCCESS) {
//			printf("param user id 0x%02X,%d\r\n", ans.param, ans.param);
//			printf("param Image Quality 0x%02X,%d\r\n", ans.size, ans.size);
			j = 1;
			break;
		} else if (ans.flag == SCAN_SUCCESS) {
//			printf("Fingerprint scanning is successfully  completed\r\n");
			continue;
		} else if (ans.flag == SCAN_FAIL) {
//			printf("Sensor or fingerprint input has failed\r\n");
			continue;
		} else if (ans.flag == TRY_AGAIN) {
//			printf("An error occurred during enrollment process\r\n");
			continue;
		} else if (ans.flag == TIME_OUT) {
//			printf("Timeout for fingerprint input\r\n");
			break;
		} else if (ans.flag == MEM_FULL) {
//			printf("Flash memory is full\r\n");
			break;
		} else if (ans.flag == FINGER_LIMIT) {
//			printf(
//					"The number of fingerprints enrolled in same ID exceeds its limit (10)\r\n");
			break;
		} else if (ans.flag == EXIST_ID) {
//			printf(
//					"The requested user ID exists. ( In case CHECK_ID flag is used )\r\n");
			break;
		} else if (ans.flag == EXIST_FINGER) {
//			printf(
//					"The same finger is already enrolled. (In case CHECK_FINGER or \
//				CHECK_FINGER_AUTO_ID is used.)\
//				The same finger is already enrolled as non-duress.(In case \
//				ADD_DURESS option is given.)\r\n");
			break;
		} else if (ans.flag == INVALID_ID) {
//			printf(
//					"The requested user ID is invalid. Note that ‘0x0000’  cannot be used\r\n");
			break;
		}
	}

	if (j == 0){
		return 0;
	}

	ask.command = 0x05;
	ask.param = user_id;
	ask.size = 0x00;
	ask.flag = 0x74;
	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port, packet, sizeof(packet));
	//printf("number of bytes write %d\r\n", len);

	while (1) {

		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		if (len <= 0) {
			continue;
		}

		unpack_packet232(packet, &ans);
		//printf("number of bytes read %d\r\n", len);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			break;
		}

		if (ans.flag == SUCCESS) {
//			printf("param user id 0x%02X,%d\r\n", ans.param, ans.param);
//			printf("param Image Quality 0x%02X,%d\r\n", ans.size, ans.size);
			j = 2;
			break;
		} else if (ans.flag == SCAN_SUCCESS) {
//			printf("Fingerprint scanning is successfully  completed\r\n");
			continue;
		} else if (ans.flag == SCAN_FAIL) {
//			printf("Sensor or fingerprint input has failed\r\n");
			continue;
		} else if (ans.flag == TRY_AGAIN) {
//			printf("An error occurred during enrollment process\r\n");
			continue;
		} else if (ans.flag == TIME_OUT) {
//			printf("Timeout for fingerprint input\r\n");
			break;
		} else if (ans.flag == MEM_FULL) {
//			printf("Flash memory is full\r\n");
			break;
		} else if (ans.flag == FINGER_LIMIT) {
//			printf(
//					"The number of fingerprints enrolled in same ID exceeds its limit (10)\r\n");
			break;
		} else if (ans.flag == EXIST_ID) {
//			printf(
//					"The requested user ID exists. ( In case CHECK_ID flag is used )\r\n");
			break;
		} else if (ans.flag == EXIST_FINGER) {
//			printf(
//					"The same finger is already enrolled. (In case CHECK_FINGER or \
//					CHECK_FINGER_AUTO_ID is used.)\
//					The same finger is already enrolled as non-duress.(In case \
//					ADD_DURESS option is given.)\r\n");
			break;
		} else if (ans.flag == INVALID_ID) {
//			printf(
//					"The requested user ID is invalid. Note that ‘0x0000’  cannot be used\r\n");
			break;
		}
	}

	if (j > 1) {
//		printf("Fingerprint enrollment is successfully completed\r\n");
		return 1;
	} else {
		return 0;
	}
}


//two fingerprint two request four replay
//g_enroll_mode = 0x42
//g_send_scan_success = 0x31
static int _enroll_by_scan10(int user_id) {
	packet232 ask, ans;
	unsigned char packet[13];
//	char uid[16];
	int len;
	int ret = 0;

	int j = 0;
	int port = 3;

	printf("enroll pls press finger \n");

	ret = check_user_id(user_id);
	if (ret == 1) {
//		printf("user ID exists\r\n");

		ask.command = 0x05;
		ask.param = user_id;
		ask.size = 0x00;
		ask.flag = 0x71;
	} else {
//		printf("user ID is not found\r\n");
		ask.command = 0x05;
		ask.param = user_id;
		ask.size = 0x00;
		ask.flag = 0x00;
	}
	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port, packet, sizeof(packet));
	//printf("number of bytes write %d\r\n", len);

	while (1) {

		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		if (len <= 0) {
			continue;
		}

		unpack_packet232(packet, &ans);
		//printf("number of bytes read %d\r\n", len);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			break;
		}

		if (ans.flag == SUCCESS) {
//			printf("param user id 0x%02X,%d\r\n", ans.param, ans.param);
//			printf("param Image Quality 0x%02X,%d\r\n", ans.size, ans.size);
			j = 1;
			break;
		} else if (ans.flag == SCAN_SUCCESS) {
//			printf("Fingerprint scanning is successfully  completed\r\n");
			continue;
		} else if (ans.flag == SCAN_FAIL) {
//			printf("Sensor or fingerprint input has failed\r\n");
			continue;
		} else if (ans.flag == TRY_AGAIN) {
//			printf("An error occurred during enrollment process\r\n");
			continue;
		} else if (ans.flag == TIME_OUT) {
//			printf("Timeout for fingerprint input\r\n");
			break;
		} else if (ans.flag == MEM_FULL) {
//			printf("Flash memory is full\r\n");
			break;
		} else if (ans.flag == FINGER_LIMIT) {
//			printf(
//					"The number of fingerprints enrolled in same ID exceeds its limit (10)\r\n");
			break;
		} else if (ans.flag == EXIST_ID) {
//			printf(
//					"The requested user ID exists. ( In case CHECK_ID flag is used )\r\n");
			break;
		} else if (ans.flag == EXIST_FINGER) {
//			printf(
//					"The same finger is already enrolled. (In case CHECK_FINGER or \
//				CHECK_FINGER_AUTO_ID is used.)\
//				The same finger is already enrolled as non-duress.(In case \
//				ADD_DURESS option is given.)\r\n");
			break;
		} else if (ans.flag == INVALID_ID) {
//			printf(
//					"The requested user ID is invalid. Note that ‘0x0000’  cannot be used\r\n");
			break;
		}
	}

	if (j == 0){
		return 0;
	}

	ask.command = 0x05;
	ask.param = user_id;
	ask.size = 0x00;
	ask.flag = 0x74;
	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port, packet, sizeof(packet));
	//printf("number of bytes write %d\r\n", len);

	while (1) {

		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		if (len <= 0) {
			continue;
		}

		unpack_packet232(packet, &ans);
		//printf("number of bytes read %d\r\n", len);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			break;
		}

		if (ans.flag == SUCCESS) {
//			printf("param user id 0x%02X,%d\r\n", ans.param, ans.param);
//			printf("param Image Quality 0x%02X,%d\r\n", ans.size, ans.size);
			j = 2;
			break;
		} else if (ans.flag == SCAN_SUCCESS) {
//			printf("Fingerprint scanning is successfully  completed\r\n");
			continue;
		} else if (ans.flag == SCAN_FAIL) {
//			printf("Sensor or fingerprint input has failed\r\n");
			continue;
		} else if (ans.flag == TRY_AGAIN) {
//			printf("An error occurred during enrollment process\r\n");
			continue;
		} else if (ans.flag == TIME_OUT) {
//			printf("Timeout for fingerprint input\r\n");
			break;
		} else if (ans.flag == MEM_FULL) {
//			printf("Flash memory is full\r\n");
			break;
		} else if (ans.flag == FINGER_LIMIT) {
//			printf(
//					"The number of fingerprints enrolled in same ID exceeds its limit (10)\r\n");
			break;
		} else if (ans.flag == EXIST_ID) {
//			printf(
//					"The requested user ID exists. ( In case CHECK_ID flag is used )\r\n");
			break;
		} else if (ans.flag == EXIST_FINGER) {
//			printf(
//					"The same finger is already enrolled. (In case CHECK_FINGER or \
//					CHECK_FINGER_AUTO_ID is used.)\
//					The same finger is already enrolled as non-duress.(In case \
//					ADD_DURESS option is given.)\r\n");
			break;
		} else if (ans.flag == INVALID_ID) {
//			printf(
//					"The requested user ID is invalid. Note that ‘0x0000’  cannot be used\r\n");
			break;
		}
	}

	if (j > 1) {
//		printf("Fingerprint enrollment is successfully completed\r\n");
		return 1;
	} else {
		return 0;
	}
}

int _enroll_by_scan(int user_id) {
	int ret = 0;

	switch (g_enroll_mode) {
	case 0x30:
		switch (g_send_scan_success) {
		case 0x30:
			ret = _enroll_by_scan1(user_id);
			break;
		case 0x31:
			ret = _enroll_by_scan2(user_id);
			break;
		}
		break;
	case 0x31:
		switch (g_send_scan_success) {
		case 0x30:
			ret = _enroll_by_scan3(user_id);
			break;
		case 0x31:
			ret = _enroll_by_scan4(user_id);
			break;
		}
		break;
	case 0x32:
		switch (g_send_scan_success) {
		case 0x30:
			ret = _enroll_by_scan5(user_id);
			break;
		case 0x31:
			ret = _enroll_by_scan6(user_id);
			break;
		}
		break;
	case 0x41:
		switch (g_send_scan_success) {
		case 0x30:
			ret = _enroll_by_scan7(user_id);
			break;
		case 0x31:
			ret = _enroll_by_scan8(user_id);
			break;
		}
		break;
	case 0x42:
		switch (g_send_scan_success) {
		case 0x30:
			ret = _enroll_by_scan9(user_id);
			break;
		case 0x31:
			ret = _enroll_by_scan10(user_id);
			break;
		}
		break;
	}

	if (ret == 1) {
//		printf("Fingerprint enrollment is successfully completed\r\n");
		return 1;
	} else {
		return 0;
	}
}

int _enroll_by_template(int user_id, char *template,int template_size) {
	packet232 ask, ans;
	unsigned char packet[13];
	int len = 0, data_len1 = 0, data_len2 = 0, data_len = 0;
	int port = 3;
	char *ptr = NULL;

	//first
	ask.command = 0x07;
	ask.param = user_id;
	ask.size = template_size;
	ask.flag = 0x00;

	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port, packet, sizeof(packet));
	//printf("number of bytes write %d\r\n", len);
	ptr = template;
	data_len1 = template_size;

	while (1) {
		data_len =
				data_len1 - data_len2 > sizeof(packet) ?
						sizeof(packet) : data_len1 - data_len2;

		memset(packet, 0, sizeof(packet));
		memcpy(packet, ptr, data_len);
		len = write_port(port, packet, data_len);

		ptr += len;
		data_len2 += len;

		if (data_len2 >= data_len1) {
			break;
		}
	}
	memset(packet, 0, sizeof(packet));
	packet[0] = 0x0A;
	len = write_port(port, packet, 1);


	while (1) {
		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		if (len <= 0) {
			continue;
		}
		//printf("number of bytes read %d\r\n", len);
		unpack_packet232(packet, &ans);
		if (ans.command != ask.command) {
//			printf("command error\r\n");
			return 0;
		}

		if (ans.flag == SUCCESS) {
			data_len1 = ans.size + 1;
//			printf("User ID  0x%02X,%d\r\n", ans.param, ans.param);
//			printf("The number of features 0x%02X,%d\r\n", ans.size, ans.size);
//
//			printf("Successfully completed\r\n");
			break;
		} else if (ans.flag == SCAN_SUCCESS) {
//			printf("Fingerprint scanning is successfully  completed\n");
			continue;
		}else if (ans.flag == TRY_AGAIN) {
//			printf("An error occurred during enrollment process\n");
			return 0;
		}else if (ans.flag == MEM_FULL) {
//			printf("Flash memory is full.\n");
			return 0;
		}else if (ans.flag == EXIST_ID) {
//			printf("The requested user ID exists\n");
			return 0;
		}else if (ans.flag == FINGER_LIMIT) {
//			printf("The number of fingerprints enrolled in same ID exceeds its limit (10)\n");
			return 0;
		}else if (ans.flag == INVALID_ID) {
//			printf("The requested user ID is invalid. Note that ‘0x0000’  cannot be used\n");
			return 0;
		}else if (ans.flag == EXIST_FINGER) {
//			printf("The  same  finger  is  already  enrolled. \n");
			return 0;
		}
	}

	return 1;

}

int _identify_by_scan(){

	packet232 ask, ans;
	unsigned char packet[13];
	int len = 0;
	int i = 0;
	int port = 3;

	printf("enroll pls press finger \n");

	ask.command = 0x11;
	ask.param = 0x00;
	ask.size = 0x00;
	ask.flag = 0x00;

	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port, packet, sizeof(packet));
	//printf("number of bytes write %d\r\n", len);

	while (i < 2) {
		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		//printf("number of bytes read %d\r\n", len);
		if (len <= 0){
			continue;
		}

		unpack_packet232(packet, &ans);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			break;
		}

		if (ans.flag == SUCCESS) {
//			printf("param user id 0x%02X,%d\r\n", ans.param, ans.param);
//			printf("param sub id 0x%02X,%d\r\n", ans.size, ans.size);

//			printf("Identification is successfully  completed\r\n");
			return 1;
		} else if (ans.flag == SCAN_SUCCESS) {
//			printf("Fingerprint scanning is successfully  completed.\r\n");
			i++;
			continue;
		} else if (ans.flag == SCAN_FAIL) {
//			printf("Sensor or fingerprint input has failed\r\n");
			return 0;
		} else if (ans.flag == NOT_FOUND) {
//			printf("No matching template\r\n");
			return 0;
		} else if (ans.flag == TRY_AGAIN) {
//			printf("Fingerprint image is not good\r\n");
			return 0;
		} else if (ans.flag == TIME_OUT) {
//			printf("Timeout for fingerprint input\r\n");
			return 0;
		} else if (ans.flag == TIMEOUT_MATCH) {
//			printf("Timeout for matching\r\n");
			return 0;
		} else if (ans.flag == REJECTED_ID) {
//			printf(
//					"Authentication mode is AUTH_REJECT or the ID is in the blacklist\r\n");
			return 0;
		} else if (ans.flag == ENTRANCE_LIMIT) {
//			printf(
//					"Authentication fails since the entrance limit is exceeded\r\n");
			return 0;
		} else if (ans.flag == FAKE_DETECTED) {
//			printf("Scanned fingerprint is determined as fake finger\r\n");
			return 0;
		}
	}

	return 1;
}

int _identify_by_template(char *template, int template_size,int *n) {
	packet232 ask, ans;
	unsigned char packet[13];
	int len = 0, data_len1 = 0, data_len2 = 0, data_len = 0;
	int port = 3;
	char *ptr = NULL;

	//first
	ask.command = 0x13;
	ask.param = 0x00;
	ask.size = template_size;
	ask.flag = 0x00;

	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port, packet, sizeof(packet));
	//printf("number of bytes write %d\r\n", len);
	ptr = template;
	data_len1 = template_size;

	while (1) {
		data_len =
		data_len1 - data_len2 > sizeof(packet) ?
		sizeof(packet) : data_len1 - data_len2;

		memset(packet, 0, sizeof(packet));
		memcpy(packet, ptr, data_len);
		len = write_port(port, packet, data_len);

		ptr += len;
		data_len2 += len;

		if (data_len2 >= data_len1) {
			break;
		}
	}
	memset(packet, 0, sizeof(packet));
	packet[0] = 0x0A;
	len = write_port(port, packet, 1);



	while (1) {
		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		//printf("number of bytes read %d\r\n", len);
		if (len <= 0){
			continue;
		}

		unpack_packet232(packet, &ans);

		if (ans.command != ask.command) {
			printf("command error\r\n");
			break;
		}

		if (ans.flag == SUCCESS) {
//			printf("param user id 0x%02X,%d\r\n", ans.param, ans.param);
//			printf("param sub id 0x%02X,%d\r\n", ans.size, ans.size);
//
//			printf("Identification is successfully  completed\r\n");
			*n = ans.param;
			return 1;
		} else if (ans.flag == SCAN_SUCCESS) {
//			printf("Fingerprint scanning is successfully  completed.\r\n");
			continue;
		} else if (ans.flag == SCAN_FAIL) {
//			printf("Sensor or fingerprint input has failed\r\n");
			return 0;
		} else if (ans.flag == NOT_FOUND) {
//			printf("No matching template\r\n");
			return 0;
		} else if (ans.flag == TRY_AGAIN) {
//			printf("Fingerprint image is not good\r\n");
			return 0;
		} else if (ans.flag == TIME_OUT) {
//			printf("Timeout for fingerprint input\r\n");
			return 0;
		} else if (ans.flag == TIMEOUT_MATCH) {
//			printf("Timeout for matching\r\n");
			return 0;
		} else if (ans.flag == REJECTED_ID) {
//			printf(
//					"Authentication mode is AUTH_REJECT or the ID is in the blacklist\r\n");
			return 0;
		} else if (ans.flag == ENTRANCE_LIMIT) {
//			printf(
//					"Authentication fails since the entrance limit is exceeded\r\n");
			return 0;
		} else if (ans.flag == FAKE_DETECTED) {
//			printf("Scanned fingerprint is determined as fake finger\r\n");
			return 0;
		}
	}

	return 1;
}

int _verify_by_scan(int user_id){

	packet232 ask, ans;
	unsigned char packet[13];
	int len = 0;
	int port = 3;

	ask.command = 0x08;
	ask.param = user_id;
	ask.size = 0x00;
	ask.flag = 0x00;

	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port, packet, sizeof(packet));
//	printf("number of bytes write %d\r\n", len);

	while (1) {
		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));

		if (len <= 0){
			continue;
		}
//		printf("number of bytes read %d\r\n", len);
		unpack_packet232(packet, &ans);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			return 0;
		}

		if (ans.flag == SUCCESS) {
//			printf("Identification is successfully completed\r\n");

//			printf("param user id 0x%02X,%d\r\n", ans.param, ans.param);
//			printf("param sub id 0x%02X,%d\r\n", ans.size, ans.size);
			break;
		} else if (ans.flag == SCAN_SUCCESS) {
//			printf("Fingerprint scanning is successfully  completed.\r\n");
			continue;
		} else if (ans.flag == SCAN_FAIL) {
//			printf("Sensor or fingerprint input has failed\r\n");
			return 0;
		} else if (ans.flag == NOT_FOUND) {
//			printf("No matching template\r\n");
			return 0;
		} else if (ans.flag == NOT_MATCH) {
			//printf("Fingerprint image is not good\r\n");
			return 0;
		} else if (ans.flag == TRY_AGAIN) {
//			printf("Fingerprint image is not good\r\n");
			return 0;
		} else if (ans.flag == TIME_OUT) {
//			printf("Timeout for fingerprint input\r\n");
			return 0;
		} else if (ans.flag == TIMEOUT_MATCH) {
//			printf("Timeout for matching\r\n");
			return 0;
		} else if (ans.flag == REJECTED_ID) {
//			printf(
//					"Authentication mode is AUTH_REJECT or the ID is in the blacklist\r\n");
			return 0;
		} else if (ans.flag == ENTRANCE_LIMIT) {
//			printf(
//					"Authentication fails since the entrance limit is exceeded\r\n");
			return 0;
		} else if (ans.flag == FAKE_DETECTED) {
//			printf("Scanned fingerprint is determined as fake finger\r\n");
			return 0;
		}
	}

	return 1;
}

int _delete_template(int user_id) {

	packet232 ask, ans;
	unsigned char packet[13];

	int len = 0;
//	int finger_num = 0;

	int port = 3;

	ask.command = 0x16;
	ask.param = user_id;
	ask.size = 0x00;
	ask.flag = 0x00;

	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port, packet, sizeof(packet));
	//printf("number of bytes write %d\r\n", len);

	while (1) {
		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		//printf("number of bytes read %d\r\n", len);

		if (len <= 0) {
			continue;
		}

		unpack_packet232(packet, &ans);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			return 0;
		}

		if (ans.flag == SUCCESS) {
//			printf("Templates are deleted successfully\r\n");
			break;
		} else if (ans.flag == NOT_FOUND) {
//			printf("There is no matching user ID found\r\n");
			return 0;
		}
	}

	return 1;
}

int _delete_all_templates(){

	packet232 ask, ans;
	unsigned char packet[13];
	int len = 0;

	int port = 3;

	ask.command = 0x17;
	ask.param = 0x00;
	ask.size = 0x00;
	ask.flag = 0x00;

	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port, packet, sizeof(packet));
	//printf("number of bytes write %d\r\n", len);

	while (1) {
		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		//printf("number of bytes read %d\r\n", len);

		if (len <= 0){
			continue;
		}

		unpack_packet232(packet, &ans);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			return 0;
		}

		if (ans.flag == SUCCESS) {
//			printf("Deletion of all templates is successfully completed\r\n");
			break;
		} else {
//			printf("Unknown \r\n");
			return 0;
		}
	}
	return 1;
}

int list_user_id(){
	packet232 ask, ans;
	unsigned char packet[13];
	unsigned char data[64];
	int len = 0,data_len1 = 0,data_len2 = 0,data_len = 0;
	int i = 0;
    int port = 3;

    //first
	ask.command = 0x18;
	ask.param = 0x00;
	ask.size = 0x00;
	ask.flag = 0x00;

	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port, packet, sizeof(packet));
	//printf("number of bytes write %d\r\n", len);
	while (1) {
		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		if (len <= 0){
			continue;
		}
		//printf("number of bytes read %d\r\n", len);
		unpack_packet232(packet, &ans);
		if (ans.command != ask.command) {
//			printf("command error\r\n");
			break;
		}

		if (ans.flag == SUCCESS) {
			data_len1 = ans.size + 1;
//			printf("Templates count  0x%02X,%d\r\n", ans.param, ans.param);
//			printf("Data size 0x%02X,%d\r\n", ans.size, ans.size);
//
//			printf("Successfully completed\r\n");
			break;
		} else if (ans.flag == INVALID_ID) {
//			printf("Block index is out of range\n");
			return 0;
		}
	}


	//then
	while (1) {

		memset(data, 0, sizeof(data));

		data_len =
				data_len1 - data_len2 > sizeof(data) ?
						sizeof(data) : data_len1 - data_len2;
		len = read_port(port, data, data_len);
		if (len <= 0) {
			continue;
		}

		//printf("number of bytes read %d\r\n", len);

//		printf("<< ");
//		for (i = 0; i < len; i++) {
//			if (i % 16 == 0) {
//				printf("\r\n");
//			}
//			printf("%02X ", data[i]);
//		}
//		printf("\r\n");


		data_len2 += len;
//		printf("data_len 2 %d\r\n", data_len2);
		if ((data_len2 >= data_len1) || (data[len - 1] == 0x0A)){
			break;
		}
	}

	if (data_len2 >= data_len1){
	  return 1;
	}else{
	  return 0;
	}
}

int scan_template() {
	packet232 ask, ans;
	unsigned char packet[13];
	unsigned char data[64];
	int len = 0, data_len1 = 0, data_len2 = 0, data_len = 0;
	int i = 0;
	int port = 3;

	//first
	ask.command = 0x21;
	ask.param = 0x00;
	ask.size = 0x00;
	ask.flag = 0x00;

	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port, packet, sizeof(packet));
	//printf("number of bytes write %d\r\n", len);

    while(1){
    	len = read_port(port, packet, sizeof(packet));
    	if (len <= 0){
    		continue;
    	}
    	//printf("number of bytes read %d\r\n", len);
		if (ans.command != ask.command) {
//			printf("command error\r\n");
			return 0;
		}

		if (ans.flag == SUCCESS) {
			data_len1 = ans.size + 1;
//			printf("Image quality score   0x%02X,%d\r\n", ans.param, ans.param);
//			printf("Template size  0x%02X,%d\r\n", ans.size, ans.size);
//
//			printf("Successfully completed\r\n");
			break;
		} else if (ans.flag == INVALID_ID) {
//			printf("Block index is out of range\n");
			return 0;
		}
    }

    //then
	while (1) {
		memset(data, 0, sizeof(data));

		data_len =
				data_len1 - data_len2 > sizeof(data) ?
						sizeof(data) : data_len1 - data_len2;
		len = read_port(port, data, data_len);

		if (len <= 0) {
			continue;
		}
		//printf("number of bytes read %d\r\n", len);

//		printf("<< ");
//		for (i = 0; i < len; i++) {
//			if (i % 16 == 0) {
//				printf("\r\n");
//			}
//			printf("%02X ", data[i]);
//		}
//		printf("\r\n");

		data_len2 += len;
//		printf("data_len 2 %d\r\n", data_len2);
		if ((data_len2 >= data_len1) || (data[len - 1] == 0x0A)) {
			break;
		}

	}

	if (data_len2 >= data_len1) {
		return 1;
	} else {
		return 0;
	}
}

int scan_image(){
	packet232 ask, ans;
	unsigned char packet[13];
	unsigned char data[64];
	int len = 0, data_len1 = 0, data_len2 = 0, data_len = 0;
	int i = 0;
	int port = 3;

	//first
	ask.command = 0x15;
	ask.param = 0x00;
	ask.size = 0x00;
	ask.flag = 0x00;

	memset(packet, 0, sizeof(packet));
	pack_packet232(&ask, packet);

	len = write_port(port, packet, sizeof(packet));
	//printf("number of bytes write %d\r\n", len);

	while (1) {
		memset(packet, 0, sizeof(packet));
		len = read_port(port, packet, sizeof(packet));
		if (len <= 0) {
			continue;
		}
		//printf("number of bytes read %d\r\n", len);

		unpack_packet232(packet, &ans);

		if (ans.command != ask.command) {
//			printf("command error\r\n");
			return 0;
		}

		if (ans.flag == SUCCESS) {
			data_len1 = ans.size + 1;
//			printf("Image size   0x%02X,%d\r\n", ans.size, ans.size);
//
//			printf("Successfully completed\r\n");

			break;
		} else if (ans.flag == INVALID_ID) {
//			printf("Block index is out of range\n");
			return 0;
		}
	}

	//then
	while (1) {

		memset(data, 0, sizeof(data));
		data_len =
				data_len1 - data_len2 > sizeof(data) ?
						sizeof(data) : data_len1 - data_len2;
		len = read_port(port, data, data_len);
		if (len <= 0) {
			continue;
		}
		//printf("number of bytes read %d\r\n", len);

//		printf("<< ");
//		for (i = 0; i < len; i++) {
//			if (i % 16 == 0) {
//				printf("\r\n");
//			}
//			printf("%02X ", data[i]);
//		}
//		printf("\r\n");

		data_len2 += len;
//		printf("data_len 2 %d\r\n", data_len2);
		if ((data_len2 >= data_len1) || (data[len -1] == 0x0A)){
			break;
		}
	}

	if (data_len2 >= data_len1) {
		return 1;
	} else {
		return 0;
	}
}

int save_image_to_file(char *file_name) {

	unsigned char *img_buf,*ptr;
	int i;
	image_t receive_img;
	// g_bin_buffer(buffer with received data inside)
	// total_bin_size(total size of received data) are global variables
	memcpy(&receive_img,(image_t*)g_bin_buffer,total_bin_size);

//	printf("receive_img.width %d\r\n", receive_img.width);
//	printf("receive_img.height %d\r\n", receive_img.height);
//	printf("receive_img.compressed %d\r\n", receive_img.compressed);
//	printf("receive_img.encrypted %d\r\n", receive_img.encrypted);
//	printf("receive_img.binary %d\r\n", receive_img.binary);
//	printf("receive_img.img_len %d\r\n", receive_img.img_len);
//	printf("receive_img.template_len %d\r\n", receive_img.template_len);

	img_buf = (unsigned char *) malloc(receive_img.img_len);
	ptr=img_buf;

// Check if the image format is binary
	if (receive_img.binary == 1) {
// Data size sent in binary = (Actual image size)/8
		for (i = 0; i < receive_img.img_len / 8; i++) {
//			bit	operation to divide	1 byte	into 8 bytes
			*ptr = (receive_img.buffer[i] & 1) ? 255 : 0;
			ptr++;
			*ptr = (receive_img.buffer[i] >> 1 & 1) ? 255 : 0;
			ptr++;
			*ptr = (receive_img.buffer[i] >> 2 & 1) ? 255 : 0;
			ptr++;
			*ptr = (receive_img.buffer[i] >> 3 & 1) ? 255 : 0;
			ptr++;
			*ptr = (receive_img.buffer[i] >> 4 & 1) ? 255 : 0;
			ptr++;
			*ptr = (receive_img.buffer[i] >> 5 & 1) ? 255 : 0;
			ptr++;
			*ptr = (receive_img.buffer[i] >> 6 & 1) ? 255 : 0;
			ptr++;
			*ptr = (receive_img.buffer[i] >> 7 & 1) ? 255 : 0;
			ptr++;
		}

//
//		for (i = 0; i < receive_img.img_len; i++) {
//			if (i % 16 == 0) {
//				printf("\r\n");
//			}
//			printf("%02X ", img_buf[i]);
//		}
//		printf("\r\n");

		write_bitmap(file_name,img_buf,receive_img.width,receive_img.height);

		free(img_buf);
	} else if (receive_img.binary == 0) {

		memcpy(img_buf, receive_img.buffer, receive_img.img_len);

		write_bitmap(file_name, img_buf, receive_img.width, receive_img.height);

		free(img_buf);
	} else {

		memcpy(img_buf, receive_img.buffer, receive_img.img_len);

		write_bitmap(file_name, img_buf, receive_img.width, receive_img.height);

		free(img_buf);
	}

	return 1;
}

int save_template_to_file(char *file_name){
	FILE *file;
	int ret = 0;

	file=fopen(file_name,"w");
	if(!file)return 0;

	ret = fwrite(g_template,g_template_size,1,file);
	if (file){
	  fclose(file);
	}

	if(ret != 1)
	{
	    remove(file_name);
	    return 0;
	}
	return 1;
}

int sfm_InitFp(int port,int baud_rate) {
	int ret = 0;

	ret = open_port(port, baud_rate, 8, "1", 'N');
	if (ret != 1) {
		return FALSE;
	}

	ret = get_module_available_finger(&g_available_finger);
	if (ret == 1) {
		printf("gRegMax = %d\r\n", g_available_finger);
		printf("get_module_available_finger success\r\n");
	} else {
		printf("get_module_available_finger fail\r\n");
	}

	ret = get_module_template_size(&g_template_size);
	if (ret == 1) {
		printf("g_template_size = %d\r\n", g_template_size);
		printf("get_module_template_size success\r\n");
	} else {
		printf("get_module_template_size fail\r\n");
	}

	ret = get_module_enroll_mode(&g_enroll_mode);
	if (ret == 1) {
		printf("g_enroll_mode = %d,0x%02X\r\n", g_enroll_mode, g_enroll_mode);
		printf("get_module_enroll_mode success\r\n");
	} else {
		printf("get_module_enroll_mode fail\r\n");
	}

	ret = get_module_send_scan_success(&g_send_scan_success);
	if (ret == 1) {
		printf("g_send_scan_success = %d,0x%02X\r\n", g_send_scan_success,
				g_send_scan_success);
		printf("get_module_send_scan_success success\r\n");
	} else {
		printf("get_module_send_scan_success fail\r\n");
	}

	return ret;
}

long sfm_FpDataOneToNMatch(char *FileName){
	int ret = 0,user_id = 0;
    char template[384];
    FILE *file;


	file = fopen(FileName, "r");
	if (!file) {
		return 0;
	}

	memset(template, 0, sizeof(template));
	ret = fread(template, g_template_size, 1, file);
	if (ret != 1) {
		fclose(file);
		return 0;
	}

  ret = _identify_by_template(template,g_template_size,&user_id);
  if (ret == 1){
	  return user_id;
  }else{
	  return 0;
  }
}

int sfm_LoadFpData(char *nID, int FingerNum, char *FileName) {
	int ret = 0;
    char template[384];
	char uid[16];
    FILE *file;

	memset(uid,0,sizeof(uid));
	sprintf(uid,"%d",atoi(nID) * 10 + FingerNum);

	file = fopen(FileName, "r");
	if (!file) {
		return 0;
	}

	memset(template, 0, sizeof(template));
	ret = fread(template, 384, 1, file);
	if (ret != 1) {
		fclose(file);
		return 0;
	}

	ret = _enroll_by_template(atoi(uid), template, 384);
	if (ret == 1) {
		printf("_enroll_by_template success\r\n");
	} else {
		printf("_enroll_by_template fail\r\n");
	}
	return ret;
}

int sfm_Enroll(char * nID, int FingerNum, char *tpath, char *dpath) {
	int ret = 0;
	char uid[16];
	char file_name[128];
	if (FingerNum < 0 || FingerNum > 9) {
		printf("param error\r\n");
		return 0;
	}

	memset(uid, 0, sizeof(uid));
	sprintf(uid, "%d", atoi(nID) * 10 + FingerNum);

	ret = _enroll_by_scan(atoi(uid));
	if (ret == 1) {
		printf("Enroll success\r\n");
	} else {
		printf("Enroll fail\r\n");
	}

	if (tpath) {
		ret = _read_image();
		if (ret == 1) {
			printf("read fingerprint image success\r\n");

			ret = save_image_to_file(tpath);
			if (ret == 1) {
				printf("save image to file success\n");
			} else {
				printf("save image to file fail\n");
			}
		} else {
			printf("read fingerprint image fail\r\n");
		}
	}

	if (dpath) {
		ret = _read_template(0); //latest fingerprint template created
		if (ret == 1) {
			printf("read fingerprint template success\r\n");

			memset(file_name,0,sizeof(file_name));
			sprintf(file_name,"%s%s_%d.s10",dpath,nID,FingerNum);
			printf("template file name %s\r\n",file_name);

			creatdir(file_name);

			ret = save_template_to_file(file_name);
			if (ret == 1) {
				printf("save template to file success\n");
			} else {
				printf("save template to file fail\n");
			}

		} else {
			printf("read fingerprint template fail\r\n");
		}
	}

	return ret;
}

long sfm_OneToNMatch(char *tpath) {
	int ret = 0;
	while (1) {

		ret = _identify_by_scan();

		if (ret > 0) {
			printf("onetoN=%d\n", ret);
			break;
		}
	}

	if (tpath) {
		ret = _read_image();
		if (ret == 1) {
			printf("read fingerprint image success\r\n");

			ret = save_image_to_file(tpath);
			if (ret == 1) {
				printf("save image to file success\n");
			} else {
				printf("save image to file fail\n");
			}

		} else {
			printf("read fingerprint image fail\r\n");
		}
	}

	return ret;
}

long sfm_OneToOneMatch(char *nID,char *tpath){
	int ret = 0;
	int FingerNum=0;
	char uid[16];

	for(FingerNum=0;FingerNum<10;FingerNum++)
	{
		memset(uid,0,sizeof(uid));
		sprintf(uid,"%d",atoi(nID) * 10 + FingerNum);

		ret = check_user_id(atoi(uid));
		if (ret == 0) { //not exists
			continue;
		}else{
			break;
		}
	}

	if (FingerNum == 10){
		return 0;
	}

	while (1) {
		ret = _verify_by_scan(atoi(uid));

		if (ret > 0) {
			printf("onetoone=%d\n", ret);
			break;
		}
	}

	if (tpath) {
		ret = _read_image();
		if (ret == 1) {
			printf("read fingerprint image success\r\n");

			ret = save_image_to_file(tpath);
			if (ret == 1) {
				printf("save image to file success\n");
			} else {
				printf("save image to file fail\n");
			}

		} else {
			printf("read fingerprint image fail\r\n");
		}
	}

	return ret;
}

int sfm_DeleteFpOne(char *nID, int FingerNum) {
	int ret = 0;
	char uid[16];
	memset(uid, 0, sizeof(uid));
	sprintf(uid, "%d", atoi(nID) * 10 + FingerNum);

	ret = check_user_id(atoi(uid));
	if (ret == 0) { //not exists
		return 1;
	}
	return _delete_template(atoi(uid));
}

int sfm_DeleteFpAll(void){
	return _delete_all_templates();
}

int sfm_get_enroll_count(){
	int ret = 0,n = 0;
	ret = get_module_enrolled_finger(&n);
	if (ret == 1) {
		printf("n = %d\r\n",n);
		printf("get_module_enrolled_finger success\r\n");
	} else {
		printf("get_module_enrolled_finger fail\r\n");
	}

	return n;
}

int sfm_UninitFp(void){
	close_port(port);
	return 1;
}

