/*
 * gas.c
 *
 *  Created on: 2014-7-1
 *      Author: aduo
 */


/*
 * 酒精测试仪 FAR-Q8
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int g_serial_port = 0;

#define IN_BUFF_SIZE 4*1024

//循环存储
static unsigned in_buff[IN_BUFF_SIZE];
static int in_buff_read = 0;
static int in_buff_written = 0;


// 可打印字符串转换为字节数据
// 如："C8329BFD0E01" --> {0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01}
// 输入: pSrc - 源字符串指针
//nSrcLength - 源字符串长度
// 输出: pDst - 目标数据指针
// 返回: 目标数据长度
int gsmString2Bytes(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
	int i=0;
	for ( i = 0; i < nSrcLength; i += 2)
	{
		// 输出高4位
		if ((*pSrc >= '0') && (*pSrc <= '9'))
		{
			*pDst = (*pSrc - '0') << 4;
		}
		else
		{
			*pDst = (*pSrc - 'A' + 10) << 4;
		}
		pSrc++;
		// 输出低4位
		if ((*pSrc>='0') && (*pSrc<='9'))
		{
			*pDst |= *pSrc - '0';
		}
		else
		{
			*pDst |= *pSrc - 'A' + 10;
		}
		pSrc++;
		pDst++;
	}

	return (nSrcLength / 2);
}


unsigned char calc_sum(char * buf, int len) {
	int i;
	unsigned char sum = 0;

	for (i = 0; i < len; i++) {
		sum += buf[i];
	}
	return sum;
}


int recv_data() {
	int len = 0;
	int num = 0;
	int i = 0;
	unsigned char ch = 0;
	unsigned char buf[512];
	unsigned char *Addr = NULL;
	int dataLen = 0;

	memset(buf, 0, sizeof(buf));
	while (1) {
		len = read_port(g_serial_port, &ch, 1);
		if (len <= 0) {
			break;
		}

		if (ch != 0xAA) {
			break;
		}

		buf[0] = ch;
//		printf("start code %02X \n",ch);

		len = read_port(g_serial_port, &ch, 1);
		if (len <= 0) {
			break;
		}

		buf[1] = ch;
		num = ch;

//		printf("len  %02X,%d \n",ch,num);

		len = read_port(g_serial_port, &ch, 1);
		if (len <= 0) {
			break;
		}

		if (ch != 0x60 && ch != 0x6F) {
			break;
		}

		buf[2] = ch;
//		printf("cmd code %02X \n",ch);

		len = read_port(g_serial_port, buf + 3, num - 3);
		if (len <= 0) {
			break;
		}

//		for (i = 0; i < len; i++) {
//			printf("%02X ", buf[3 + i]);
//		}
//		printf("\n");

		len = read_port(g_serial_port, &ch, 1);
		if (len <= 0) {
			break;
		}

		buf[num] = ch;

//		printf("addr code %02X \n",ch);

		len = read_port(g_serial_port, &ch, 1);
		if (len <= 0) {
			break;
		}

		if (ch != calc_sum(buf, num + 1)) {
			break;
		}

//		printf("check code %02X \n",ch);

		dataLen = 4;
		Addr = buf;
		Addr += 9;

		if (in_buff_written + dataLen > IN_BUFF_SIZE) { //折回
			memmove(in_buff + in_buff_written, Addr,
			IN_BUFF_SIZE - in_buff_written);
			dataLen -= (IN_BUFF_SIZE - in_buff_written);
			Addr += (IN_BUFF_SIZE - in_buff_written);
			in_buff_written = 0;
		}

		memmove(in_buff + in_buff_written, Addr, dataLen);
		in_buff_written += dataLen;
		in_buff_written %= IN_BUFF_SIZE;

//		printf("in_buff_written %d\n",in_buff_written);
		break;
	}

	return in_buff_written;
}

/*
 *  return asc
 */
int read_data(char *data, int len) {
	int num = 0;
	unsigned char *Addr = NULL;
	int dataLen = 0;

	num = recv_data();

	if (in_buff_read < in_buff_written) {
		len = len > (in_buff_written - in_buff_read) ?
				(in_buff_written - in_buff_read) : len;
	} else if (in_buff_read > in_buff_written) {
		len = len > (IN_BUFF_SIZE + in_buff_written - in_buff_read) ?
				(IN_BUFF_SIZE + in_buff_written - in_buff_read) : len;
	} else {
		len = 0;
	}

	if (len > 0) {
		dataLen = len;
		Addr = data;

		if (in_buff_read + dataLen > IN_BUFF_SIZE) { //折回
			memmove(Addr, in_buff + in_buff_read, IN_BUFF_SIZE - in_buff_read);
			dataLen -= (IN_BUFF_SIZE - in_buff_read);
			Addr += (IN_BUFF_SIZE - in_buff_read);
			in_buff_read = 0;
		}

		memmove(Addr, in_buff + in_buff_read, dataLen);
		in_buff_read += dataLen;
		in_buff_read %= IN_BUFF_SIZE;

//		printf("in_buff_read %d\n", in_buff_read);
		return len;
	} else {
		return 0;
	}
}


int start_gas_test(int uart_port) {
	unsigned char ask[16];
	int len = 0;

	g_serial_port = uart_port;

	memset(ask, 0, sizeof(ask));

	ask[0] = 0xAB;
	ask[1] = 0x03;
	ask[2] = 0x99;
	ask[3] = 0x00;
	ask[4] = calc_sum(ask, 4);

	len = write_port(g_serial_port, ask, 5);

	if (len > 0) {
		return 1;
	} else {
		return 0;
	}
}

/*
 * mg/100ml
 * 1 = success 0 = fail
 *
 */
int read_gas_test(int uart_port,char *value) {
	int i = 0;
	int len = 0;
//	公式中换算系数	k:我国采用2200，美国采用2000，欧	洲大部分国家和我国台湾采用2100。
	int k = 2200;

	unsigned char data[11];

	g_serial_port = uart_port;

	memset(data, 0, sizeof(data));
	len = read_data(data, sizeof(data));

	if (len > 0){
		switch (data[0]) {
		case 0x00:
			printf("The content of gas : %d mg/100ml\n", data[2] * 256 + data[1]);
			sprintf(value,"%d",data[2] * 256 + data[1]);
			break;
		case 0x01:
			printf("The content of gas :  %f mg/l\n", (data[2] * 256 + data[1]) / 10000.0);
			sprintf(value,"%f",(data[2] * 256 + data[1]) / 10000.0 * (k / 10));
			break;
		}

		switch (data[3]) {
		case 0x00:
			printf("status : normal \n");
			break;
		case 0x01:
			printf("status : abnormal 1\n");
			break;
		case 0x02:
			printf("status : abnormal 2\n");
			break;
		}

		return 1;
	}

	return 0;
}

int stop_gas_test(int uart_port){
	unsigned char ask[16];
	int len = 0;

	g_serial_port = uart_port;

	memset(ask,0,sizeof(ask));

	ask[0] = 0xAB;
	ask[1] = 0x03;
	ask[2] = 0xEE;
	ask[3] = 0x00;
	ask[4] = calc_sum(ask,4);

	len = write_port(g_serial_port,ask,5);

	if (len > 0) {
		return 1;
	} else {
		return 0;
	}
}



