/*
 * YYD120.c
 *
 *  Created on: 2014-6-28
 *      Author: aduo
 */




#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * 2.4G有源卡
 */

int g_serial_port = 2;

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



// 字节数据转换为可打印字符串
// 如：{0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01} --> "C8329BFD0E01"
// 输入: pSrc - 源数据指针
//nSrcLength - 源数据长度
// 输出: pDst - 目标字符串指针
// 返回: 目标字符串长度
int gsmBytes2String(unsigned char* pSrc, char* pDst, int nSrcLength)
{
	const char tab[]="0123456789ABCDEF";	// 0x0-0xf的字符查找表
	int i=0;

	for (i = 0; i < nSrcLength; i++)
	{
		*pDst++ = tab[*pSrc >> 4];	// 输出高4位
		*pDst++ = tab[*pSrc & 0x0f];	// 输出低4位
		pSrc++;
	}
	*pDst = '\0';

	return (nSrcLength * 2);
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
	int i = 0;
	unsigned char ch = 0;
	unsigned char buf[20];
	unsigned char asc[16];
	unsigned char hex[8];
	unsigned char *Addr = NULL;
	int dataLen = 0;

	memset(buf, 0, sizeof(buf));
	memset(asc,0,sizeof(asc));
	memset(hex,0,sizeof(hex));
	while (1) {
		len = read_port(g_serial_port, &ch, 1);
		if (len <= 0) {
			break;
		}

//		printf("start code %02X \n",ch);

		if (ch != 0x02) {
			break;
		}

		len = read_port(g_serial_port, &ch, 1);
		if (len <= 0) {
			break;
		}

		buf[0] = ch;

//		printf("addr code %02X \n",ch);

		len = read_port(g_serial_port, &ch, 1);
		if (len <= 0) {
			break;
		}

		buf[1] = ch;

//		printf("addr code %02X \n",ch);

		len = read_port(g_serial_port, buf + 2, 10);
		if (len <= 0) {
			break;
		}

		len = read_port(g_serial_port, &ch, 1);
		if (len <= 0) {
			break;
		}

		buf[12] = ch;

//		printf("check code %02X \n",ch);

		len = read_port(g_serial_port, &ch, 1);
		if (len <= 0) {
			break;
		}

		buf[13] = ch;

//		printf("check code %02X \n",ch);


		memmove(asc, buf, 14);
//		printf("asc %s\r\n", asc);

		len = gsmString2Bytes(asc, hex, strlen(asc));
//		for (i = 0; i < len; i++) {
//			printf("%02X ", hex[i]);
//		}
//		printf("\n");

		if (hex[len - 1] != calc_sum(hex, len - 1)) {
			break;
		}

		len = read_port(g_serial_port, &ch, 1);
		if (len <= 0) {
			break;
		}

//		printf("stop code %02X \n",ch);

		if (ch != 0x0D) {
			break;
		}

		len = read_port(g_serial_port, &ch, 1);
		if (len <= 0) {
			break;
		}
//		printf("stop code %02X \n",ch);

		if (ch != 0x0A) {
			break;
		}

		dataLen = 10;
		Addr = buf;
		Addr += 2;

		if (in_buff_written + dataLen > IN_BUFF_SIZE) { //折回
			memmove(in_buff + in_buff_written, Addr, IN_BUFF_SIZE - in_buff_written);
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

int read_card_no(char *value) {
	int i = 0;
	int len = 0;
	unsigned char data[11];
	unsigned char asc[16];

	memset(asc,0,sizeof(asc));

	memset(data, 0, sizeof(data));
	len = read_data(data, sizeof(data));

	if (len > 0) {
		memmove(asc, data, len);
//		printf("asc %s\r\n", asc);
		memmove(value, asc, len);
	}


	return len;
}


int main() {
	int ret = 0;
	int len = 0;
	int i = 0;
	unsigned char data[16];

	ret = open_port(g_serial_port, 9600, 8, "1", 'N');

	while (1) {
		memset(data, 0, sizeof(data));

		len = read_card_no(data);

		if (len > 0) {
			printf("card no. %s\n",data);
		}
	}

	ret = close_port(g_serial_port);

	return 0;
}
