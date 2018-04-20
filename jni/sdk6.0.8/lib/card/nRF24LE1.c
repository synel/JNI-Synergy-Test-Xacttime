/*
 * nRF24LE1.c
 *
 *  Created on: 2014-6-27
 *      Author: aduo
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * 2.4G有源卡
 */

int g_serial_port = 3;

#define IN_BUFF_SIZE 4*1024

//循环存储
static unsigned in_buff[IN_BUFF_SIZE];
static int in_buff_read = 0;
static int in_buff_written = 0;


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
	unsigned char ch = 0;
	unsigned char buf[16];
	unsigned char *Addr = NULL;
	int dataLen = 0;

	memset(buf, 0, sizeof(buf));
	while (1) {
		len = read_port(g_serial_port, &ch, 1);
		if (len <= 0) {
			break;
		}

//		printf("%02X \n",ch);

		if (ch != 0xAA) {
			break;
		}

		len = read_port(g_serial_port, &ch, 1);
		if (len <= 0) {
			break;
		}

//		printf("%02X \n",ch);

		if (ch != 0xAA) {
			break;
		}

		len = read_port(g_serial_port, &ch, 1);
		if (len <= 0) {
			break;
		}
//		printf("%02X \n",ch);

		if (ch != 0xAB) {
			break;
		}

		len = read_port(g_serial_port, buf, 5);
		if (len <= 0) {
			break;
		}

		len = read_port(g_serial_port, &ch, 1);
		if (len <= 0) {
			break;
		}
		if (ch != calc_sum(buf, 5)) {
			break;
		}


		dataLen = 4;
		Addr=buf;

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
 * return hex
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
	unsigned char data[4];

	memset(data, 0, sizeof(data));
	len = read_data(data, sizeof(data));

	if (len > 0) {
		for (i = 0; i < len; i++) {
			sprintf(value + i * 2, "%02X", data[i]);
//			if ((i + 1) % 4 == 0) {
//				strcat(value, ",");
//				value++;
//			}
		}
	}

	return 2 * len;
}


int main(){
	int ret = 0;
	int len = 0;
	int i = 0;
	unsigned char data[10];

	ret = open_port(g_serial_port, 9600, 8, "1", 'N');

	while (1) {
		memset(data, 0, sizeof(data));
//		len = read_data(data, sizeof(data));
		len = read_card_no(data);

		if (len > 0) {
			printf("card no. %s\n",data);

//			for (i = 0; i < len; i++) {
//				printf("%02X ", data[i]);
//			}
//			printf("\n");
		}
	}


	ret = close_port(g_serial_port);

	return 0;
}
