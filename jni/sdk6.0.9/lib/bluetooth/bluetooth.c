/*
 * bluetooth.c
 *
 *  Created on: 2014-6-14
 *      Author: aduo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int g_serial_port = 3;
int g_baud_rate = 9600;

int connect_flag = 0; //1 = opened  0 = closed
int startup_flag = 0; //1 = succeed 0 = failed

#define IN_BUFF_SIZE 4*1024

//循环存储
static unsigned in_buff[IN_BUFF_SIZE];
static int in_buff_read = 0;
static int in_buff_written = 0;

/*
 * 设置波特率
 * Para1：0～8
 * 0=9600;
 * 1=19200;
 * 2=38400;
 * 3=57600;
 * 4=115200;
 * 5=4800;
 * 6=2400;
 * 7=1200;
 * 8=230400;
 * Default：0（9600）
 */
static int set_module_baud(int n){
	unsigned char ask[16], answer[16],buf[16],reply[32];
	int len = 0;
	int total = 0;
	struct timeval oldtimer, newtimer;

	memset(ask, 0, sizeof(ask));
	memset(answer, 0, sizeof(answer));
	switch(n){
	case 1200:
		strcpy(ask,"AT+BAUD7");
		strcpy(answer, "OK+Set:7");
		break;
	case 2400:
		strcpy(ask,"AT+BAUD6");
		strcpy(answer, "OK+Set:6");
		break;
	case 4800:
		strcpy(ask,"AT+BAUD5");
		strcpy(answer, "OK+Set:5");
		break;
	case 9600:
		strcpy(ask,"AT+BAUD0");
		strcpy(answer, "OK+Set:0");
		break;
	case 19200:
		strcpy(ask,"AT+BAUD1");
		strcpy(answer, "OK+Set:1");
		break;
	case 38400:
		strcpy(ask,"AT+BAUD2");
		strcpy(answer, "OK+Set:2");
		break;
	case 57600:
		strcpy(ask,"AT+BAUD3");
		strcpy(answer, "OK+Set:3");
		break;
	case 115200:
		strcpy(ask,"AT+BAUD4");
		strcpy(answer, "OK+Set:4");
		break;
	case 230400:
		strcpy(ask,"AT+BAUD8");
		strcpy(answer, "OK+Set:8");
		break;
	default:
		strcpy(ask,"AT+BAUD0");
		strcpy(answer, "OK+Set:0");
		break;
	}


	printf("request %s\n",ask);

	len = write_port(g_serial_port,ask,strlen(ask));

	memset(reply, 0, sizeof(reply));
	gettimeofday(&oldtimer, NULL);
	gettimeofday(&newtimer, NULL);
	while (1) {
		gettimeofday(&newtimer, NULL);
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000 + (newtimer.tv_usec
					- oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}

		memset(buf, 0, sizeof(buf));
		len = read_port(g_serial_port, buf, sizeof(buf));
		if (len <= 0) {
			continue;
		}

		if (total > sizeof(reply)){
			break;
		}

		memmove(reply + total, buf, len);
		total += len;
		reply[total] = '\0';


		if (total >= strlen(answer)) {
			break;
		}
	}

	printf("reply %s\n",reply);

	if (strcmp(reply, answer) == 0) {
		return 1;
	} else {
		return 0;
	}
}

static int is_startup(){
	unsigned char ask[16], answer[16], buf[16], reply[32];
	int len = 0;
	int total = 0;

	struct timeval oldtimer, newtimer;

	memset(ask, 0, sizeof(ask));
	memset(answer, 0, sizeof(answer));
	strcpy(ask, "AT");
	strcpy(answer,"OK");

	printf("request %s\n", ask);

	len = write_port(g_serial_port, ask, strlen(ask));

	memset(reply, 0, sizeof(reply));
	gettimeofday(&oldtimer, NULL);
	gettimeofday(&newtimer, NULL);
	while (1) {
		gettimeofday(&newtimer, NULL);
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000 + (newtimer.tv_usec
					- oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}

		memset(buf, 0, sizeof(buf));
		len = read_port(g_serial_port, buf, sizeof(buf));
		if (len <= 0) {
			continue;
		}

		if (total > sizeof(reply)) {
			break;
		}

		memmove(reply + total, buf, len);
		total += len;
		reply[total] = '\0';

		if (total >= 2) {
			printf("reply %d,%s\n", strlen(reply),reply);
			if (total >= 2 && strncmp(reply, answer, strlen(answer)) == 0) {
				memmove(reply, reply + 2, total - 2);
				reply[total - 2] = '\0';
				total -= 2;

				startup_flag = 1;

				break;
			} else if (total >= 7 && strncmp(reply, "OK+LOST", strlen("OK+LOST")) == 0) {
				printf("connect closed \n");

				startup_flag = 1;
				connect_flag = 0;

				break;
			}
		}

	}

//	printf("reply %s\n", reply);

	if (startup_flag == 1){
		return 1;
	} else {
		return 0;
	}
}

/*
 * 获取设备名称
 */

static int get_module_name(char *s) {
	unsigned char ask[16], answer[16], buf[16], reply[32];
	int len = 0;
	int total = 0;
	char module_name[16];

	struct timeval oldtimer, newtimer;

	memset(ask, 0, sizeof(ask));
	strcpy(ask, "AT+NAME?");

	printf("request %s\n", ask);

	len = write_port(g_serial_port, ask, strlen(ask));

	memset(reply, 0, sizeof(reply));
	gettimeofday(&oldtimer, NULL);
	gettimeofday(&newtimer, NULL);
	while (1) {
		gettimeofday(&newtimer, NULL);
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000 + (newtimer.tv_usec
					- oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}

		memset(buf, 0, sizeof(buf));
		len = read_port(g_serial_port, buf, sizeof(buf));
		if (len <= 0) {
			continue;
		}

		if (total > sizeof(reply)) {
			break;
		}

		memmove(reply + total, buf, len);
		total += len;
		reply[total] = '\0';

	}

	printf("reply %s\n", reply);

	if (strstr(reply, "OK+NAME:")) {
		memset(module_name, 0, sizeof(module_name));
		if (sscanf(reply, "%*[^:]:%s", module_name) == 1) {
			strcpy(s, module_name);
			return 1;
		} else {
			return 0;
		}
	} else {
		return 0;
	}
}

/*
 * 设置设备名称
 */

int set_module_name(char *s){
	unsigned char ask[16], answer[16],buf[16],reply[32];
	int len = 0;
	int total = 0;
	struct timeval oldtimer, newtimer;

	if ((!s) || (strlen(s) > 11)){
		return 0;
	}

	memset(ask, 0, sizeof(ask));
	memset(answer, 0, sizeof(answer));
	sprintf(ask,"AT+NAME%s",s);
	sprintf(answer,"OK+Set:%s",s);

	printf("request %s\n",ask);

	len = write_port(g_serial_port,ask,strlen(ask));

	memset(reply, 0, sizeof(reply));
	gettimeofday(&oldtimer, NULL);
	gettimeofday(&newtimer, NULL);
	while (1) {
		gettimeofday(&newtimer, NULL);
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000 + (newtimer.tv_usec
					- oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}

		memset(buf, 0, sizeof(buf));
		len = read_port(g_serial_port, buf, sizeof(buf));
		if (len <= 0) {
			continue;
		}

		if (total > sizeof(reply)){
			break;
		}

		memmove(reply + total, buf, len);
		total += len;
		reply[total] = '\0';


		if (total >= strlen(answer)) {
			break;
		}
	}

	printf("reply %s\n",reply);

	if (strcmp(reply, answer) == 0) {
		return 1;
	} else {
		return 0;
	}
}
/*
 * 查询本机 MAC 地址
 */

static int get_module_addr(char *s){
	unsigned char ask[16], answer[16], buf[16], reply[32];
	int len = 0;
	int total = 0;
	char addr[16];

	struct timeval oldtimer, newtimer;

	memset(ask, 0, sizeof(ask));
	strcpy(ask, "AT+ADDR?");

	printf("request %s\n", ask);

	len = write_port(g_serial_port, ask, strlen(ask));

	memset(reply, 0, sizeof(reply));
	gettimeofday(&oldtimer, NULL);
	gettimeofday(&newtimer, NULL);
	while (1) {
		gettimeofday(&newtimer, NULL);
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000 + (newtimer.tv_usec
					- oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}

		memset(buf, 0, sizeof(buf));
		len = read_port(g_serial_port, buf, sizeof(buf));
		if (len <= 0) {
			continue;
		}

		if (total > sizeof(reply)) {
			break;
		}

		memmove(reply + total, buf, len);
		total += len;
		reply[total] = '\0';

	}

	printf("reply %s\n", reply);

	if (strstr(reply, "OK+ADDR:")) {
		memset(addr, 0, sizeof(addr));
		if (sscanf(reply, "%*[^:]:%s", addr) == 1) {
			strcpy(s, addr);
			return 1;
		} else {
			return 0;
		}
	} else {
		return 0;
	}
}

/*
 * 设置主从模式
 * 1:  主设备
 * 0:  从设备
 */
static int set_master_mode(int n) {
	unsigned char ask[16], answer[16], buf[16], reply[32];
	int len = 0;
	int total = 0;
	struct timeval oldtimer, newtimer;

	memset(ask, 0, sizeof(ask));
	memset(answer, 0, sizeof(answer));
	switch (n) {
	case 0:
		strcpy(ask, "AT+ROLE0");
		strcpy(answer, "OK+Set:0");
		break;
	case 1:
		strcpy(ask, "AT+ROLE1");
		strcpy(answer, "OK+Set:1");
		break;
	default:
		strcpy(ask, "AT+ROLE0");
		strcpy(answer, "OK+Set:0");
		break;
	}

	printf("request %s\n",ask);

	len = write_port(g_serial_port, ask, strlen(ask));

	memset(reply, 0, sizeof(reply));
	gettimeofday(&oldtimer, NULL);
	gettimeofday(&newtimer, NULL);
	while (1) {
		gettimeofday(&newtimer, NULL);
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000 + (newtimer.tv_usec
					- oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}
		memset(buf, 0, sizeof(buf));
		len = read_port(g_serial_port, buf, sizeof(buf));
		if (len <= 0) {
			continue;
		}

		if (total > sizeof(reply)){
			break;
		}

		memmove(reply + total, buf, len);
		total += len;
		reply[total] = '\0';


		if (total >= strlen(answer)) {
			break;
		}
	}

	printf("reply %s\n",reply);

	if (strcmp(reply, answer) == 0) {
		return 1;
	} else {
		return 0;
	}
}

/*
 * 设置主模式下搜索时是否仅搜索 HM 模块
 * 0:  搜索所有 BLE 从设备
 * 1:  仅搜索 HM 模块
 */

static int set_scan_range(int n) {
	unsigned char ask[16], answer[16], buf[16], reply[32];
	int len = 0;
	int total = 0;
	struct timeval oldtimer, newtimer;

	memset(ask, 0, sizeof(ask));
	memset(answer, 0, sizeof(answer));
	switch (n) {
	case 0:
		strcpy(ask, "AT+FILT0");
		strcpy(answer, "OK+Set:0");
		break;
	case 1:
		strcpy(ask, "AT+FILT1");
		strcpy(answer, "OK+Set:1");
		break;
	default:
		strcpy(ask, "AT+FILT1");
		strcpy(answer, "OK+Set:1");
		break;
	}

	printf("request %s\n", ask);

	len = write_port(g_serial_port, ask, strlen(ask));

	memset(reply, 0, sizeof(reply));
	gettimeofday(&oldtimer, NULL);
	gettimeofday(&newtimer, NULL);
	while (1) {
		gettimeofday(&newtimer, NULL);
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000 + (newtimer.tv_usec
					- oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}
		memset(buf, 0, sizeof(buf));
		len = read_port(g_serial_port, buf, sizeof(buf));
		if (len <= 0) {
			continue;
		}

		if (total > sizeof(reply)) {
			break;
		}

		memmove(reply + total, buf, len);
		total += len;
		reply[total] = '\0';


		if (total >= strlen(answer)) {
			break;
		}
	}

	printf("reply %s\n", reply);

	if (strcmp(reply, answer) == 0) {
		return 1;
	} else {
		return 0;
	}
}

/*
 * 设置广播方式
 * 0:  任意设备搜索连接
 * 1:  允许 上 次 成 功设 备 连 接(在上电的 1.28 秒之内)
 * 2:  允许广播和搜索
 * 3:  只广播
 * 注：从模式下可用指令
 */
static int set_broadcast_mode(int n){
	unsigned char ask[16], answer[16], buf[16], reply[32];
	int len = 0;
	int total = 0;
	struct timeval oldtimer, newtimer;

	memset(ask, 0, sizeof(ask));
	memset(answer, 0, sizeof(answer));
	switch (n) {
	case 0:
		strcpy(ask, "AT+ADTY0");
		strcpy(answer, "OK+Set:0");
		break;
	case 1:
		strcpy(ask, "AT+ADTY1");
		strcpy(answer, "OK+Set:1");
		break;
	case 2:
		strcpy(ask, "AT+ADTY1");
		strcpy(answer, "OK+Set:1");
		break;
	case 3:
		strcpy(ask, "AT+ADTY1");
		strcpy(answer, "OK+Set:1");
		break;
	default:
		strcpy(ask, "AT+ADTY0");
		strcpy(answer, "OK+Set:0");
		break;
	}

	printf("request %s\n", ask);

	len = write_port(g_serial_port, ask, strlen(ask));

	memset(reply, 0, sizeof(reply));
	gettimeofday(&oldtimer, NULL);
	gettimeofday(&newtimer, NULL);
	while (1) {
		gettimeofday(&newtimer, NULL);
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000 + (newtimer.tv_usec
					- oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}
		memset(buf, 0, sizeof(buf));
		len = read_port(g_serial_port, buf, sizeof(buf));
		if (len <= 0) {
			continue;
		}

		if (total > sizeof(reply)) {
			break;
		}

		memmove(reply + total, buf, len);
		total += len;
		reply[total] = '\0';


		if (total >= strlen(answer)) {
			break;
		}
	}

	printf("reply %s\n", reply);

	if (strcmp(reply, answer) == 0) {
		return 1;
	} else {
		return 0;
	}
}

/*
 * 设置广播时间间隔
 * Para: 0 ~ 15
 * 0: 100ms
 * 1: 152.5 ms
 * 2: 211.25 ms
 * 3: 318.75 ms
 * 4: 417.5 ms
 * 5: 546.25 ms
 * 6: 760 ms
 * 7: 852.5 ms
 * 8: 1022.5 ms
 * 9: 1285 ms
 * A: 2000ms
 * B: 3000ms
 * C: 4000ms
 * D: 5000ms
 * E: 6000ms
 * F: 7000ms
 * HMSoft Default:0
 */

static int set_broadcast_interval(int n){
	unsigned char ask[16], answer[16], buf[16], reply[32];
	int len = 0;
	int total = 0;
	struct timeval oldtimer, newtimer;

	memset(ask, 0, sizeof(ask));
	memset(answer, 0, sizeof(answer));
	switch (n) {
	case 0:
		strcpy(ask, "AT+ADVI0");
		strcpy(answer, "OK+Set:0");
		break;
	case 1:
		strcpy(ask, "AT+ADVI1");
		strcpy(answer, "OK+Set:1");
		break;
	case 2:
		strcpy(ask, "AT+ADVI2");
		strcpy(answer, "OK+Set:2");
		break;
	case 3:
		strcpy(ask, "AT+ADVI3");
		strcpy(answer, "OK+Set:3");
		break;
	case 4:
		strcpy(ask, "AT+ADVI4");
		strcpy(answer, "OK+Set:4");
		break;
	case 5:
		strcpy(ask, "AT+ADVI5");
		strcpy(answer, "OK+Set:5");
		break;
	case 6:
		strcpy(ask, "AT+ADVI6");
		strcpy(answer, "OK+Set:6");
		break;
	case 7:
		strcpy(ask, "AT+ADVI7");
		strcpy(answer, "OK+Set:7");
		break;
	case 8:
		strcpy(ask, "AT+ADVI8");
		strcpy(answer, "OK+Set:8");
		break;
	case 9:
		strcpy(ask, "AT+ADVI9");
		strcpy(answer, "OK+Set:9");
		break;
	case 10:
		strcpy(ask, "AT+ADVIA");
		strcpy(answer, "OK+Set:A");
		break;
	case 11:
		strcpy(ask, "AT+ADVIB");
		strcpy(answer, "OK+Set:B");
		break;
	case 12:
		strcpy(ask, "AT+ADVIC");
		strcpy(answer, "OK+Set:C");
		break;
	case 13:
		strcpy(ask, "AT+ADVID");
		strcpy(answer, "OK+Set:D");
		break;
	case 14:
		strcpy(ask, "AT+ADVIE");
		strcpy(answer, "OK+Set:E");
		break;
	case 15:
		strcpy(ask, "AT+ADVIF");
		strcpy(answer, "OK+Set:F");
		break;
	default:
		strcpy(ask, "AT+ADVI0");
		strcpy(answer, "OK+Set:0");
		break;
	}

	printf("request %s\n", ask);

	len = write_port(g_serial_port, ask, strlen(ask));

	memset(reply, 0, sizeof(reply));
	gettimeofday(&oldtimer, NULL);
	gettimeofday(&newtimer, NULL);
	while (1) {
		gettimeofday(&newtimer, NULL);
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000 + (newtimer.tv_usec
					- oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}
		memset(buf, 0, sizeof(buf));
		len = read_port(g_serial_port, buf, sizeof(buf));
		if (len <= 0) {
			continue;
		}

		if (total > sizeof(reply)) {
			break;
		}

		memmove(reply + total, buf, len);
		total += len;
		reply[total] = '\0';


		if (total >= strlen(answer)) {
			break;
		}
	}

	printf("reply %s\n", reply);

	if (strcmp(reply, answer) == 0) {
		return 1;
	} else {
		return 0;
	}
}

/*
 * 设置模块工作类型
 * 0:  上电立即工作
 * 1:  上电后暂不工作，等待AT+START/AT+CON/AT+CONN等指令
 */
static int set_work_type(int n) {
	unsigned char ask[16], answer[16], buf[16], reply[32];
	int len = 0;
	int total = 0;
	struct timeval oldtimer, newtimer;

	memset(ask, 0, sizeof(ask));
	memset(answer, 0, sizeof(answer));
	switch (n) {
	case 0:
		strcpy(ask, "AT+IMME0");
		strcpy(answer, "OK+Set:0");
		break;
	case 1:
		strcpy(ask, "AT+IMME1");
		strcpy(answer, "OK+Set:1");
		break;
	default:
		strcpy(ask, "AT+IMME0");
		strcpy(answer, "OK+Set:0");
		break;
	}

	printf("request %s\n",ask);

	len = write_port(g_serial_port, ask, strlen(ask));

	memset(reply, 0, sizeof(reply));
	gettimeofday(&oldtimer, NULL);
	gettimeofday(&newtimer, NULL);
	while (1) {
		gettimeofday(&newtimer, NULL);
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000 + (newtimer.tv_usec
					- oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}
		memset(buf, 0, sizeof(buf));
		len = read_port(g_serial_port, buf, sizeof(buf));
		if (len <= 0) {
			continue;
		}

		if (total > sizeof(reply)) {
			break;
		}

		memmove(reply + total, buf, len);
		total += len;
		reply[total] = '\0';


		if (total >= strlen(answer)) {
			break;
		}

	}

	printf("reply %s\n", reply);

	if (strcmp(reply, answer) == 0) {
		return 1;
	} else {
		return 0;
	}

	return 1;
}

/*
 * 设置模块工作模式
 * 0: 透传模式
 * 1: PIO 采集+远控+透传
 * 2：透传+远控模式
 */

static int set_work_mode(int n) {
	unsigned char ask[16], answer[16], buf[16], reply[32];
	int len = 0;
	int total = 0;
	struct timeval oldtimer, newtimer;

	memset(ask, 0, sizeof(ask));
	memset(answer, 0, sizeof(answer));
	switch (n) {
	case 0:
		strcpy(ask, "AT+MODE0");
		strcpy(answer, "OK+Set:0");
		break;
	case 1:
		strcpy(ask, "AT+MODE1");
		strcpy(answer, "OK+Set:1");
		break;
	case 2:
		strcpy(ask, "AT+MODE2");
		strcpy(answer, "OK+Set:2");
		break;
	default:
		strcpy(ask, "AT+MODE0");
		strcpy(answer, "OK+Set:0");
		break;
	}

	printf("request %s\n",ask);

	len = write_port(g_serial_port, ask, strlen(ask));

	memset(reply, 0, sizeof(reply));
	gettimeofday(&oldtimer, NULL);
	gettimeofday(&newtimer, NULL);
	while (1) {
		gettimeofday(&newtimer, NULL);
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000 + (newtimer.tv_usec
					- oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}
		memset(buf, 0, sizeof(buf));
		len = read_port(g_serial_port, buf, sizeof(buf));
		if (len <= 0) {
			continue;
		}

		if (total > sizeof(reply)) {
			break;
		}

		memmove(reply + total, buf, len);
		total += len;
		reply[total] = '\0';


		if (total >= strlen(answer)) {
			break;
		}

	}

	printf("reply %s\n", reply);

	if (strcmp(reply, answer) == 0) {
		return 1;
	} else {
		return 0;
	}
}

/*
 *设置连接安全
 *0　连接不需要密码
 *1　简单配对
 *2　需要密码配对
 *3　配对并绑定
 */

static int set_connect_security(int n){
	unsigned char ask[16], answer[16],buf[16],reply[32];
	int len = 0;
	int total = 0;
	struct timeval oldtimer, newtimer;

	memset(ask, 0, sizeof(ask));
	memset(answer, 0, sizeof(answer));
	switch(n){
	case 0:
		strcpy(ask,"AT+TYPE0");
		strcpy(answer, "OK+Set:0");
		break;
	case 1:
		strcpy(ask,"AT+TYPE1");
		strcpy(answer, "OK+Set:1");
		break;
	case 2:
		strcpy(ask,"AT+TYPE2");
		strcpy(answer, "OK+Set:2");
		break;
	case 3:
		strcpy(ask,"AT+TYPE3");
		strcpy(answer, "OK+Set:3");
		break;
	default:
		strcpy(ask,"AT+TYPE0");
		strcpy(answer, "OK+Set:0");
		break;
	}

	printf("request %s\n",ask);

	len = write_port(g_serial_port,ask,strlen(ask));

	memset(reply, 0, sizeof(reply));
	gettimeofday(&oldtimer, NULL);
	gettimeofday(&newtimer, NULL);
	while (1) {
		gettimeofday(&newtimer, NULL);
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000 + (newtimer.tv_usec
					- oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}
		memset(buf, 0, sizeof(buf));
		len = read_port(g_serial_port, buf, sizeof(buf));
		if (len <= 0) {
			continue;
		}

		if (total > sizeof(reply)){
			break;
		}

		memmove(reply + total, buf, len);
		total += len;
		reply[total] = '\0';


		if (total >= strlen(answer)) {
			break;
		}
	}

	printf("reply %s\n",reply);

	if (strcmp(reply, answer) == 0) {
		return 1;
	} else {
		return 0;
	}
}

/*
 * 设置模块成功连接后是否保存连接地址
 * Para: 0 ~ 1
 * 0:  保存
 * 1:  不保存
 * Default: 0
 */
static int set_connection_saved(int n){
	unsigned char ask[16], answer[16],buf[16],reply[32];
	int len = 0;
	int total = 0;
	struct timeval oldtimer, newtimer;

	memset(ask, 0, sizeof(ask));
	memset(answer, 0, sizeof(answer));
	switch(n){
	case 0:
		strcpy(ask,"AT+SAVE0");
		strcpy(answer, "OK+Set:0");
		break;
	case 1:
		strcpy(ask,"AT+SAVE1");
		strcpy(answer, "OK+Set:1");
		break;
	default:
		strcpy(ask,"AT+SAVE0");
		strcpy(answer, "OK+Set:0");
		break;
	}

	printf("request %s\n",ask);

	len = write_port(g_serial_port,ask,strlen(ask));

	memset(reply, 0, sizeof(reply));
	gettimeofday(&oldtimer, NULL);
	gettimeofday(&newtimer, NULL);
	while (1) {
		gettimeofday(&newtimer, NULL);
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000 + (newtimer.tv_usec
					- oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}
		memset(buf, 0, sizeof(buf));
		len = read_port(g_serial_port, buf, sizeof(buf));
		if (len <= 0) {
			continue;
		}

		if (total > sizeof(reply)){
			break;
		}

		memmove(reply + total, buf, len);
		total += len;
		reply[total] = '\0';


		if (total >= strlen(answer)) {
			break;
		}
	}

	printf("reply %s\n",reply);

	if (strcmp(reply, answer) == 0) {
		return 1;
	} else {
		return 0;
	}
}

/*
 * 设置扫描是否显示名称
 * 0 不显示
 * 1　显示
 */

static int set_show_name(int n){
	unsigned char ask[16], answer[16],buf[16],reply[32];
	int len = 0;
	int total = 0;
	struct timeval oldtimer, newtimer;

	memset(ask, 0, sizeof(ask));
	memset(answer, 0, sizeof(answer));
	switch(n){
	case 0:
		strcpy(ask,"AT+SHOW0");
		strcpy(answer,"OK+Set:0");
		break;
	case 1:
		strcpy(ask,"AT+SHOW1");
		strcpy(answer,"OK+Set:1");
		break;
	default:
		strcpy(ask,"AT+SHOW0");
		strcpy(answer,"OK+Set:0");
		break;
	}

	printf("request %s\n",ask);

	len = write_port(g_serial_port,ask,strlen(ask));

	memset(reply,0,sizeof(reply));
	gettimeofday(&oldtimer, NULL);
	gettimeofday(&newtimer, NULL);
	while (1) {
		gettimeofday(&newtimer, NULL);
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000 + (newtimer.tv_usec
					- oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}
		memset(buf, 0, sizeof(buf));
		len = read_port(g_serial_port, buf, sizeof(buf));
		if (len <= 0) {
			continue;
		}

		if (total > sizeof(reply)){
			break;
		}

		memmove(reply + total,buf,len);
		total += len;
		reply[total] = '\0';


		if (total >= strlen(answer)){
			break;
		}

	}

	printf("reply %s\n",reply);

	if (strcmp(reply,answer) == 0){
	  return 1;
	}else{
		return 0;
	}
}

/*
 * 设置配对密码
 */
int set_pair_password(char *s){
	unsigned char ask[16], answer[16],buf[16],reply[32];
	int len = 0;
	int total = 0;
	struct timeval oldtimer, newtimer;

	if ((!s) || strlen(s) > 6){
		return 0;
	}

	memset(ask, 0, sizeof(ask));
	memset(answer, 0, sizeof(answer));
	sprintf(ask,"AT+PASS%s",s);
	sprintf(answer,"OK+Set:%s",s);

	printf("request %s\n",ask);

	len = write_port(g_serial_port,ask,strlen(ask));

	memset(reply,0,sizeof(reply));
	gettimeofday(&oldtimer, NULL);
	gettimeofday(&newtimer, NULL);
	while (1) {
		gettimeofday(&newtimer, NULL);
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000 + (newtimer.tv_usec
					- oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}
		memset(buf, 0, sizeof(buf));
		len = read_port(g_serial_port, buf, sizeof(buf));
		if (len <= 0) {
			continue;
		}

		if (total > sizeof(reply)){
			break;
		}

		memmove(reply + total,buf,len);
		total += len;
		reply[total] = '\0';


		if (total >= strlen(answer)){
			break;
		}

	}

	printf("reply %s\n",reply);

	if (strcmp(reply,answer) == 0){
	  return 1;
	}else{
		return 0;
	}
}

/*
 * 设置是否通知上位机连接状态
 * 0:  连接后不通知上位机
 * 1:  连接后通知上位机
 */

static int set_connect_report(int n) {
	unsigned char ask[16], answer[16], buf[16], reply[32];
	int len = 0;
	int total = 0;
	struct timeval oldtimer, newtimer;

	memset(ask, 0, sizeof(ask));
	memset(answer, 0, sizeof(answer));
	switch (n) {
	case 0:
		strcpy(ask, "AT+NOTI0");
		strcpy(answer, "OK+Set:0");
		break;
	case 1:
		strcpy(ask, "AT+NOTI1");
		strcpy(answer, "OK+Set:1");
		break;
	default:
		strcpy(ask, "AT+NOTI0");
		strcpy(answer, "OK+Set:0");
		break;
	}

	printf("request %s\n",ask);

	len = write_port(g_serial_port, ask, strlen(ask));

	memset(reply, 0, sizeof(reply));
	gettimeofday(&oldtimer, NULL);
	gettimeofday(&newtimer, NULL);
	while (1) {
		gettimeofday(&newtimer, NULL);
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000 + (newtimer.tv_usec
					- oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}
		memset(buf, 0, sizeof(buf));
		len = read_port(g_serial_port, buf, sizeof(buf));
		if (len <= 0) {
			continue;
		}

		if (total > sizeof(reply)) {
			break;
		}

		memmove(reply + total, buf, len);
		total += len;
		reply[total] = '\0';


		if (total >= strlen(answer)) {
			break;
		}

	}

	printf("reply %s\n", reply);

	if (strcmp(reply, answer) == 0) {
		return 1;
	} else {
		return 0;
	}
	return 1;
}

/*
 * 配置连接超时
 * 单位:毫秒
 * 0 代表持续连接
 * n 取值[0..9999]
 */
static int set_connect_timeout(int n){
	unsigned char ask[16], answer[16],buf[16],reply[32];
	int len = 0;
	char s[16];
	int total = 0;
	struct timeval oldtimer, newtimer;

	if (n < 0 || n > 9999){
		return 0;
	}

	memset(s,0,sizeof(s));
    sprintf(s,"%06d",n);

	memset(ask, 0, sizeof(ask));
	memset(answer, 0, sizeof(answer));
	sprintf(ask,"AT+TCON%s",s);
	sprintf(answer,"OK+Set:%s",s);

	printf("request %s\n",ask);

	len = write_port(g_serial_port,ask,strlen(ask));

	memset(reply, 0, sizeof(reply));
	gettimeofday(&oldtimer, NULL);
	gettimeofday(&newtimer, NULL);
	while (1) {
		gettimeofday(&newtimer, NULL);
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000 + (newtimer.tv_usec
					- oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}
		memset(buf, 0, sizeof(buf));
		len = read_port(g_serial_port, buf, sizeof(buf));
		if (len <= 0) {
			continue;
		}

		if (total > sizeof(reply)){
			break;
		}

		memmove(reply + total, buf, len);
		total += len;
		reply[total] = '\0';


		if (total >= strlen(answer)) {
			break;
		}
	}

	printf("reply %s\n",reply);

	if (strcmp(reply, answer) == 0) {
		return 1;
	} else {
		return 0;
	}
}

/*
 * 恢复默认设置
 */
static int restore_factory_default() {
	unsigned char ask[16], answer[16], buf[16], reply[32];
	int len = 0;
	int total = 0;
	struct timeval oldtimer, newtimer;

	memset(ask, 0, sizeof(ask));
	memset(answer, 0, sizeof(answer));
	strcpy(ask, "AT+RENEW");
	strcpy(answer, "OK+RENEW");

	printf("request %s\n", ask);

	len = write_port(g_serial_port, ask, strlen(ask));

	memset(reply, 0, sizeof(reply));
	gettimeofday(&oldtimer, NULL);
	gettimeofday(&newtimer, NULL);
	while (1) {
		gettimeofday(&newtimer, NULL);
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000 + (newtimer.tv_usec
					- oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}
		memset(buf, 0, sizeof(buf));
		len = read_port(g_serial_port, buf, sizeof(buf));
		if (len <= 0) {
			continue;
		}

		if (total > sizeof(reply)) {
			break;
		}

		memmove(reply + total, buf, len);
		total += len;
		reply[total] = '\0';


		if (total >= strlen(answer)) {
			break;
		}
	}

	printf("reply %s\n", reply);

	if (strcmp(reply, answer) == 0) {
		return 1;
	} else {
		return 0;
	}
}

/*
 * 模块复位，重启
 */

static int reset_module(){
	unsigned char ask[16], answer[16],buf[16],reply[32];
	int len = 0;
	int total = 0;
	struct timeval oldtimer, newtimer;

	memset(ask, 0, sizeof(ask));
	memset(answer, 0, sizeof(answer));
	strcpy(ask,"AT+RESET");
	strcpy(answer,"OK+RESET");

	printf("request %s\n",ask);

	len = write_port(g_serial_port,ask,strlen(ask));

	memset(reply, 0, sizeof(reply));
	gettimeofday(&oldtimer, NULL);
	gettimeofday(&newtimer, NULL);
	while (1) {
		gettimeofday(&newtimer, NULL);
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000 + (newtimer.tv_usec
					- oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}
		memset(buf, 0, sizeof(buf));
		len = read_port(g_serial_port, buf, sizeof(buf));
		if (len <= 0) {
			continue;
		}

		if (total > sizeof(reply)){
			break;
		}

		memmove(reply + total, buf, len);
		total += len;
		reply[total] = '\0';


		if (total >= strlen(answer)) {
			break;
		}
	}

	printf("reply %s\n",reply);

	if (strcmp(reply, answer) == 0) {
		return 1;
	} else {
		return 0;
	}
}

/*
 * 开始工作指令
 * 注： 该指令配合 AT+IMME 设置值为 1 时有效,指令执行后， 模块延时 500ms开始工作。
 */

static int start_module(){
	unsigned char ask[16], answer[16],buf[16],reply[32];
	int len = 0;
	int total = 0;
	struct timeval oldtimer, newtimer;

	memset(ask, 0, sizeof(ask));
	memset(answer, 0, sizeof(answer));
	strcpy(ask,"AT+START");
	strcpy(answer,"OK+START");

	printf("request %s\n",ask);

	len = write_port(g_serial_port,ask,strlen(ask));

	memset(reply, 0, sizeof(reply));
	gettimeofday(&oldtimer, NULL);
	gettimeofday(&newtimer, NULL);
	while (1) {
		gettimeofday(&newtimer, NULL);
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000 + (newtimer.tv_usec
					- oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}
		memset(buf, 0, sizeof(buf));
		len = read_port(g_serial_port, buf, sizeof(buf));
		if (len <= 0) {
			continue;
		}

		if (total > sizeof(reply)){
			break;
		}

		memmove(reply + total, buf, len);
		total += len;
		reply[total] = '\0';


		if (total >= strlen(answer)) {
			break;
		}
	}

	printf("reply %s\n",reply);

	if (strcmp(reply, answer) == 0) {
		return 1;
	} else {
		return 0;
	}
}
/*
 * Para: 0 ~ 1
 * 0:  自动休眠
 * 1:  不 自 动 休 眠 ，等待A T+SLEEP 进入休眠状态
 * Default: 1
 */

static int set_sleep_mode(int n){
	unsigned char ask[16], answer[16], buf[16], reply[32];
	int len = 0;
	int total = 0;
	struct timeval oldtimer, newtimer;

	memset(ask, 0, sizeof(ask));
	memset(answer, 0, sizeof(answer));
	switch (n) {
	case 0:
		strcpy(ask, "AT+PWRM0");
		strcpy(answer, "OK+Set:0");
		break;
	case 1:
		strcpy(ask, "AT+PWRM1");
		strcpy(answer, "OK+Set:1");
		break;
	default:
		strcpy(ask, "AT+PWRM1");
		strcpy(answer, "OK+Set:1");
		break;
	}

	printf("request %s\n",ask);

	len = write_port(g_serial_port, ask, strlen(ask));

	memset(reply, 0, sizeof(reply));
	gettimeofday(&oldtimer, NULL);
	gettimeofday(&newtimer, NULL);
	while (1) {
		gettimeofday(&newtimer, NULL);
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000 + (newtimer.tv_usec
					- oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}
		memset(buf, 0, sizeof(buf));
		len = read_port(g_serial_port, buf, sizeof(buf));
		if (len <= 0) {
			continue;
		}

		if (total > sizeof(reply)) {
			break;
		}

		memmove(reply + total, buf, len);
		total += len;
		reply[total] = '\0';


		if (total >= strlen(answer)) {
			break;
		}

	}

	printf("reply %s\n", reply);

	if (strcmp(reply, answer) == 0) {
		return 1;
	} else {
		return 0;
	}
	return 1;
}

/*
 * slave mode
 * 让模块进行休眠状态
 * 该指令仅在从模式下待机状态下生效
 */
static int sleep_module() {
	unsigned char ask[16], answer[16], buf[16], reply[32];
	int len = 0;
	int total = 0;
	struct timeval oldtimer, newtimer;

	memset(ask, 0, sizeof(ask));
	memset(answer, 0, sizeof(answer));
	strcpy(ask, "AT+SLEEP");
	strcpy(answer, "OK+SLEEP");

	printf("request %s\n", ask);

	len = write_port(g_serial_port, ask, strlen(ask));

	memset(reply, 0, sizeof(reply));
	gettimeofday(&oldtimer, NULL);
	gettimeofday(&newtimer, NULL);
	while (1) {
		gettimeofday(&newtimer, NULL);
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000 + (newtimer.tv_usec
					- oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}
		memset(buf, 0, sizeof(buf));
		len = read_port(g_serial_port, buf, sizeof(buf));
		if (len <= 0) {
			continue;
		}

		if (total > sizeof(reply)) {
			break;
		}

		memmove(reply + total, buf, len);
		total += len;
		reply[total] = '\0';


		if (total >= strlen(answer)) {
			break;
		}
	}

	printf("reply %s\n", reply);

	if (strcmp(reply, answer) == 0) {
		return 1;
	} else {
		return 0;
	}
}

/*
 * 查询成功连接过的远程主机地址
 */

static int get_connected_bluetooth(char *addr_list) {
	unsigned char ask[16], answer[16], buf[16], reply[32];
	int len = 0;
	int total = 0;
	char mac[16];

	struct timeval oldtimer, newtimer;

	memset(ask, 0, sizeof(ask));
	strcpy(ask, "AT+RADD?");

	printf("request %s\n", ask);

	len = write_port(g_serial_port, ask, strlen(ask));

	memset(reply, 0, sizeof(reply));
	gettimeofday(&oldtimer, NULL );
	gettimeofday(&newtimer, NULL );
	while (1) {
		gettimeofday(&newtimer, NULL );
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000
				+ (newtimer.tv_usec - oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}

		memset(buf, 0, sizeof(buf));
		len = read_port(g_serial_port, buf, sizeof(buf));
		if (len <= 0) {
			continue;
		}

		if (total > sizeof(reply)) {
			break;
		}

		memmove(reply + total, buf, len);
		total += len;
		reply[total] = '\0';

	}

	printf("reply %s\n", reply);

	if (strstr(reply, "OK+RADD:")) {
		memset(mac, 0, sizeof(mac));
		if (sscanf(reply, "%*[^:]:%s", mac) == 1) {
			if (strcmp(mac, "000000000000") == 0) {
				strcpy(addr_list, mac);
				return 0;
			} else {
				strcpy(addr_list, mac);
				return 1;
			}
		} else {
			return 0;
		}
	} else {
		return 0;
	}
}

/*
 * 擦除绑定信息
 */
int erase_bound_bluetooth(){
	unsigned char ask[16], answer[16],buf[16],reply[32];
	int len = 0;
	int total = 0;
	struct timeval oldtimer, newtimer;

	memset(ask, 0, sizeof(ask));
	memset(answer,0,sizeof(answer));

	strcpy(ask,"AT+ERASE");
	strcpy(answer,"OK+ERASE");

	printf("request %s\n",ask);

	len = write_port(g_serial_port,ask,strlen(ask));

	memset(reply, 0, sizeof(reply));
	gettimeofday(&oldtimer, NULL );
	gettimeofday(&newtimer, NULL );
	while (1) {
		gettimeofday(&newtimer, NULL );
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000
				+ (newtimer.tv_usec - oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}
		memset(buf, 0, sizeof(buf));
		len = read_port(g_serial_port, buf, sizeof(buf));
		if (len <= 0) {
			continue;
		}

		if (total > sizeof(reply)){
			break;
		}

		memmove(reply + total, buf, len);
		total += len;
		reply[total] = '\0';


		if (total >= strlen(answer)) {
			break;
		}
	}

	printf("reply %s\n",reply);

	if (strcmp(reply, answer) == 0) {
		return 1;
	} else {
		return 0;
	}
}

/*
 * 清除设备配对信息
 */
int clear_connected_bluetooth(){
	unsigned char ask[16], answer[16],buf[16],reply[32];
	int len = 0;
	int total = 0;
	struct timeval oldtimer, newtimer;

	memset(ask, 0, sizeof(ask));
	memset(answer,0,sizeof(answer));

	strcpy(ask,"AT+CLEAR");
	strcpy(answer,"OK+CLEAR");

	printf("request %s\n",ask);

	len = write_port(g_serial_port,ask,strlen(ask));

	memset(reply, 0, sizeof(reply));
	gettimeofday(&oldtimer, NULL );
	gettimeofday(&newtimer, NULL );
	while (1) {
		gettimeofday(&newtimer, NULL );
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000
				+ (newtimer.tv_usec - oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}
		memset(buf, 0, sizeof(buf));
		len = read_port(g_serial_port, buf, sizeof(buf));
		if (len <= 0) {
			continue;
		}

		if (total > sizeof(reply)){
			break;
		}

		memmove(reply + total, buf, len);
		total += len;
		reply[total] = '\0';


		if (total >= strlen(answer)) {
			break;
		}
	}

	printf("reply %s\n",reply);

	if (strcmp(reply, answer) == 0) {
		return 1;
	} else {
		return 0;
	}
}

//master mode
//搜索到的数量　0~5
//最长 11 位数字或字母，含中划线和下划线，不建议用其它字符。
int scan_bluetooth(char *addr_list) {
	unsigned char ask[16], ans[16], buf[20], reply[128];
	int len = 0;
	int total = 0;
	int pos = 0;
	unsigned char temp[16];
	unsigned char addr[85]; //(12 + 2) * 6
	int addr_len = 0;
	unsigned char buff[20];
	unsigned char name[79]; //(11 + 2) * 6
	int name_len = 0;
	int scan_flag = 0;
	struct timeval oldtimer, newtimer;

	memset(ask, 0, sizeof(ask));
	strcpy(ask, "AT+DISC?");

	printf("request %s\n", ask);
	len = write_port(g_serial_port, ask, strlen(ask));

	memset(reply, 0, sizeof(reply));
	memset(addr, 0, sizeof(addr));
	memset(name, 0, sizeof(name));
//	gettimeofday(&oldtimer, NULL );
//	gettimeofday(&newtimer, NULL );
	while (1) {
//		gettimeofday(&newtimer, NULL );
//		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000
//				+ (newtimer.tv_usec - oldtimer.tv_usec) / 1000 > 15000) { //ms
//			break;
//		}
		memset(buf, 0, sizeof(buf));
		len = read_port(g_serial_port, buf, sizeof(buf));
		if (len <= 0) {
			continue;
		}

//		printf("len %d\r\n", len);
//		printf("buf %s\r\n", buf);

//		if (total > sizeof(reply)) {
//			break;
//		}

//		printf("offset %d\r\n",total);

		memmove(reply + total, buf, len);
		total += len;
		reply[total] = '\0';

//		printf("total %d\r\n",total);

		if (total >= 8) {
			printf("reply %d,%s\n",strlen(reply),reply);
			if (strncmp(reply, "OK+DISCS", strlen("OK+DISCS")) == 0) {
//				printf("scan start \n");

				memmove(reply, reply + 8, total - 8);
				reply[total - 8] = '\0';
				total -= 8;
				continue;
			} else if (strncmp(reply, "OK+DISC:", strlen("OK+DISC:")) == 0) {

				if (total >= 20) {
//					printf("addr reply %s\n", reply);

					scan_flag ++;

					memset(temp, 0, sizeof(temp));
					sscanf(reply, "%*[^:]:%12s", temp);
//					printf("addr %s\r\n", temp);

					memmove(addr + addr_len, temp, strlen(temp));
					strcat(addr, "\r\n");
					addr_len += strlen(temp);
					addr_len += 2;

					memmove(reply, reply + 20, total - 20);
					reply[total - 20] = '\0';
					total -= 20;
				} else {
					continue;
				}
			} else if (strncmp(reply, "OK+NAME:", strlen("OK+NAME:")) == 0) {
				pos = strcspn(reply, "\r\n");
				if (pos >= 0) {
//					printf("name reply %s\n", reply);
//					printf("crlf pos %d\r\n",pos);

					memset(buff, 0, sizeof(buff));
					strncpy(buff, reply, pos);

//					printf("buff %s\r\n",buff);

					memset(temp, 0, sizeof(temp));
					sscanf(buff, "%*[^:]:%s", temp);
//					printf("name %s\r\n", temp);

					memmove(name + name_len, temp, strlen(temp));
					strcat(name, "\r\n");
					name_len += strlen(temp);
					name_len += 2;

					memmove(reply, reply + strlen(buff) + 2,
							total - strlen(buff) - 2);
					reply[total - strlen(buff) - 2] = '\0';

					total -= strlen(buff);
					total -= 2;
				} else {
					continue;
				}
			} else if (strncmp(reply, "OK+DISCE", strlen("OK+DISCE")) == 0) {
//				printf("scan end \n");
				break;
			}
		}

	}

	if (scan_flag > 0) {
		//	printf("======addr list====== \n");
//		printf("%s\n", addr);

		strcpy(addr_list, addr);

		printf("======name list====== \n");
		printf("%s\n", name);

		return 1;
	} else {
		return 0;
	}
}

/*
 * 连接最后一次连接成功的从设备
 */

static int connect_lasted_bluetooth(){
	unsigned char ask[16], answer[16],buf[16],reply[32];
	int len = 0;
	int total = 0;
	struct timeval oldtimer, newtimer;

	memset(ask, 0, sizeof(ask));
	memset(answer, 0, sizeof(answer));

	strcpy(ask,"AT+CONNL");
	strcpy(answer,"OK+CONNL");

	printf("request %s\n",ask);

	len = write_port(g_serial_port,ask,strlen(ask));

	memset(reply, 0, sizeof(reply));
	gettimeofday(&oldtimer, NULL );
	gettimeofday(&newtimer, NULL );
	while (1) {
		gettimeofday(&newtimer, NULL );
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000
				+ (newtimer.tv_usec - oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}
		memset(buf, 0, sizeof(buf));
		len = read_port(g_serial_port, buf, sizeof(buf));
		if (len <= 0) {
			continue;
		}

		if (total > sizeof(reply)) {
			break;
		}

		memmove(reply + total, buf, len);
		total += len;
		reply[total] = '\0';

		if (total >= 7) {
			printf("reply %d,%s\n", strlen(reply),reply);
			if (total >= 8 && strncmp(reply, answer, strlen(answer)) == 0) {
				printf("attempt connect \n");

				memmove(reply, reply + 8, total - 8);
				reply[total - 8] = '\0';
				total -= 8;
				continue;
			} else if (total >= 8 && strncmp(reply, "OK+CONNE", strlen("OK+CONNE")) == 0) {
				printf("connect error \n");
				break;
			} else if (total >= 8 && strncmp(reply, "OK+CONNF", strlen("OK+CONNF")) == 0) {
				printf("connect fail \n");
				break;
			} else if (total >= 8 && strncmp(reply, "OK+CONNN", strlen("OK+CONNN")) == 0) {
				printf("addr is null \n");
				break;
			} else if (strncmp(reply, "OK+CONN", strlen("OK+CONN")) == 0) {
				connect_flag = 1;
//				printf("connect success \n");
				break;
			}
		}
	}

//	printf("reply %s\n", reply);

	if (connect_flag == 1) {
		return 1;
	} else {
		return 0;
	}
}

/*
 * 连接指定蓝牙地址的从设备
 */
int connect_specified_bluetooth(char *addr){
	unsigned char ask[16], answer[16],buf[16],reply[32];
	int len = 0;
	int total = 0;
	struct timeval oldtimer, newtimer;

	memset(ask, 0, sizeof(ask));
	memset(answer, 0, sizeof(answer));

	sprintf(ask,"AT+CON%s",addr);
	strcpy(answer,"OK+CONNA");

	printf("request %s\n",ask);

	len = write_port(g_serial_port,ask,strlen(ask));

	memset(reply, 0, sizeof(reply));
	gettimeofday(&oldtimer, NULL );
	gettimeofday(&newtimer, NULL );
	while (1) {
		gettimeofday(&newtimer, NULL );
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000
				+ (newtimer.tv_usec - oldtimer.tv_usec) / 1000 > 5000) { //ms
			break;
		}
		memset(buf, 0, sizeof(buf));
		len = read_port(g_serial_port, buf, sizeof(buf));
		if (len <= 0) {
			continue;
		}

		if (total > sizeof(reply)) {
			break;
		}

		memmove(reply + total, buf, len);
		total += len;
		reply[total] = '\0';

		if (total >= 7) {
			printf("reply %d,%s\n", strlen(reply),reply);
			if (total >= 8 && strncmp(reply, answer, strlen(answer)) == 0) {
				printf("attempt connect \n");

				memmove(reply, reply + 8, total - 8);
				reply[total - 8] = '\0';
				total -= 8;
				continue;
			} else if (total >= 8 && strncmp(reply, "OK+CONNE", strlen("OK+CONNE")) == 0) {
				printf("connect error \n");
				break;
			} else if (total >= 8 && strncmp(reply, "OK+CONNF", strlen("OK+CONNF")) == 0) {
				printf("connect fail \n");
				break;
			} else if (strncmp(reply, "OK+CONN", strlen("OK+CONN")) == 0) {
				connect_flag = 1;
//				printf("connect success \n");
				continue;
			}else if (strncmp(reply, "OK+LOST", strlen("OK+LOST")) == 0) {
				connect_flag = 0;
//				printf("connect success \n");
				break;
			}
		}
	}

//	printf("reply %s\n", reply);

	if (connect_flag == 1) {
		return 1;
	} else {
		return 0;
	}

}

/*
 * 连接搜索返回的设备
 * Para1:  0~5, 具 体 取 决 于A T+DISC?搜索到的数量,下标从 0 开始
 */
int connect_scanned_bluetooth(int index){
	unsigned char ask[16], answer[16],buf[16],reply[32];
	int len = 0;
	int total = 0;
	struct timeval oldtimer, newtimer;

	if (index < 0 || index > 5){
		return 0;
	}

	memset(ask, 0, sizeof(ask));
	memset(answer, 0, sizeof(answer));

	sprintf(ask,"AT+CONN%d",index);
	sprintf(answer,"OK+CONN%d",index);

	printf("request %s\n",ask);

	len = write_port(g_serial_port,ask,strlen(ask));

	memset(reply, 0, sizeof(reply));
	gettimeofday(&oldtimer, NULL );
	gettimeofday(&newtimer, NULL );
	while (1) {
		gettimeofday(&newtimer, NULL );
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000
				+ (newtimer.tv_usec - oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}
		memset(buf, 0, sizeof(buf));
		len = read_port(g_serial_port, buf, sizeof(buf));
		if (len <= 0) {
			continue;
		}

		if (total > sizeof(reply)) {
			break;
		}

		memmove(reply + total, buf, len);
		total += len;
		reply[total] = '\0';

		if (total >= 7) {
			printf("reply %d,%s\n", strlen(reply),reply);
			if (total >= 8 && strncmp(reply, answer, strlen(answer)) == 0) {
				printf("attempt connect \n");

				memmove(reply, reply + 8, total - 8);
				reply[total - 8] = '\0';
				total -= 8;
				continue;
			} else if (total >= 8 && strncmp(reply, "OK+CONNE", strlen("OK+CONNE")) == 0) {
				printf("connect error \n");
				break;
			} else if (total >= 8 && strncmp(reply, "OK+CONNF", strlen("OK+CONNF")) == 0) {
				printf("connect fail \n");
				break;
			} else if (strncmp(reply, "OK+CONN", strlen("OK+CONN")) == 0) {
				connect_flag = 1;
//				printf("connect success \n");
				break;
			}
		}
	}

//	printf("reply %s\n", reply);

	if (connect_flag == 1) {
		return 1;
	} else {
		return 0;
	}
}

static int autobauding_synchronization() {
	int ret = 0;
	int i = 0;
	int baud_rate = 9600;

	int baud_rate_list[] = { 9600, 19200, 38400, 57600, 115200 };

	for (i = 0; i <= sizeof(baud_rate_list) / sizeof(baud_rate_list[0]) - 1;
			i++) {

		baud_rate = baud_rate_list[i];

		printf("try baud rate %d,%d\n", i, baud_rate);

		ret = open_port(g_serial_port, baud_rate, 8, "1", 'N');
		if (ret == 1) {
			printf("open serial port success \n");
		} else {
			printf("open serial port fail \n");
			return ret;
		}

		ret = is_startup();
		if (ret == 1) {
			printf("module is startup \n");
			break;
		} else {
			printf("module is not startup or baud rate is wrong \n");
		}

		ret = close_port(g_serial_port);
		if (ret == 1) {
			printf("close serial port success \n");
		} else {
			printf("close serial port fail \n");
		}
	}

	if (baud_rate != g_baud_rate) {

		switch (baud_rate) {
		case 9600:
			i = 0;
			break;
		case 19200:
			i = 1;
			break;
		case 38400:
			i = 2;
			break;
		case 57600:
			i = 3;
			break;
		case 115200:
			i = 4;
			break;
		default:
			baud_rate = 9600;
			i = 0;
			break;
		}

		ret = set_module_baud(i);
		if (ret == 1) {
			printf("set module baud success \n");
		} else {
			printf("set module baud fail \n");
		}

		ret = close_port(g_serial_port);
		if (ret == 1) {
			printf("close serial port success \n");
		} else {
			printf("close serial port fail \n");
			return ret;
		}

		ret = open_port(g_serial_port, baud_rate, 8, "1", 'N');
		if (ret == 1) {
			printf("open serial port success \n");
		} else {
			printf("open serial port fail \n");
		}

		return ret;
	}

	return ret;
}

//api
int init_bluetooth(char *model,int port,int rate){
	int ret = 0;

	g_serial_port = port;
	g_baud_rate = rate;

	ret = autobauding_synchronization();
	if (ret == 1){
		printf("autobauding synchronization success \n");
		printf("==========================\n");
	} else {
		printf("autobauding synchronization fail \n");
		return 0;
	}

	ret = set_show_name(1);
	if (ret == 1){
		printf("set show name success \r\n");
		printf("==========================\n");
	}else{
		printf("set show name fail \r\n");
		return ret;
	}

	ret = set_work_type(1);
	if (ret == 1){
		printf("set work is not immediately  success \r\n");
		printf("==========================\n");
	}else{
		printf("set work is not immediately fail \r\n");
		return ret;
	}

	ret = set_work_mode(0);
	if (ret == 1){
		printf("set work mode success \r\n");
		printf("==========================\n");
	}else{
		printf("set work mode fail \r\n");
		return ret;
	}

	ret = set_sleep_mode(1);
	if (ret == 1){
		printf("set sleep mode success \r\n");
		printf("==========================\n");
	}else{
		printf("set sleep mode fail \r\n");
		return ret;
	}

	ret = set_master_mode(1);
	if (ret == 1){
		printf("set master mode success \r\n");
		printf("==========================\n");
	}else{
		printf("set master mode fail \r\n");
		return ret;
	}

	ret = set_scan_range(0);
	if (ret == 1){
		printf("set scan range success \r\n");
		printf("==========================\n");
	}else{
		printf("set scan range fail \r\n");
		return ret;
	}

	ret = set_connect_report(1);
	if (ret == 1){
		printf("set_connect_reportsuccess \n");
		printf("==========================\n");
	}else{
		printf("set_connect_report fail \n");
		return ret;
	}

	ret = set_connect_timeout(0);
	if (ret == 1){
		printf("set_connect_timeout success \r\n");
		printf("==========================\n");
	}else{
		printf("set_connect_timeout fail \n");
		return ret;
	}

	ret = set_connect_security(2);
	if (ret == 1){
		printf("set_connect_security success \r\n");
		printf("==========================\n");
	}else{
		printf("set_connect_securityfail \r\n");
		return ret;
	}

	ret = set_connection_saved(0);
	if (ret == 1){
		printf("set connection saved success \r\n");
		printf("==========================\n");
	}else{
		printf("set connection saved fail \r\n");
		return ret;
	}

	return ret;
}

int open_bluetooth() {
	int ret = 0;
	char name[32],addr[32];

//	ret = start_module();

	memset(name, 0, sizeof(name));
	ret = get_module_name(name);

	if (ret == 1) {
		printf("bluetooth name %s\r\n", name);
		printf("==========================\n");
	}

	memset(addr, 0, sizeof(addr));
	ret = get_module_addr(addr);
	if (ret == 1) {
		printf("bluetooth addr %s\r\n", addr);
		printf("==========================\n");
	}

	return ret;
}


//slave mode
int accept_bluetooth() {
	int ret = 0, len = 0;
	char data[128];
	static int master_mode = 0,broadcast_mode = 0,broadcast_interval = 0;

	if (master_mode == 0) { //仅执行一次
		ret = set_master_mode(0);
		if (ret == 1) {
			master_mode = 1;
			printf("set slave mode success \n");
		} else {
			printf("set slave mode fail \n");
		}
	}

	if (broadcast_mode == 0) { //仅执行一次
		ret = set_broadcast_mode(0);
		if (ret == 1) {
			broadcast_mode = 1;
			printf("set broadcast mode success \n");
		} else {
			printf("set broadcast mode fail \n");
		}
	}

	if (broadcast_interval == 0) { //仅执行一次
		ret = set_broadcast_interval(0);
		if (ret == 1) {
			broadcast_interval = 1;
			printf("set broadcast interval success \n");
		} else {
			printf("set broadcast interval fail \n");
		}
	}

	return ret;
}

int connect_bluetooth() {
	int ret = 0;
	char addr_list[128];
	char *buf = NULL;
	char *delim = "\r\n";
	char *token = NULL;
	int index = 0;
	static char addr[64];
	static struct timeval oldtimer, newtimer;
	static int connection_exists = 0;

	if (connect_flag){
		return 1;
	}

	if (connection_exists == 0) {
		memset(addr, 0, sizeof(addr));
		ret = get_connected_bluetooth(addr);
		if (ret == 1) {
			connection_exists = 1; //存在成功连接地址
			gettimeofday(&oldtimer, NULL );
		} else {
			connection_exists = 2; //无成功连接地址
		}
	}

	if (connection_exists == 1) { //若存在成功连接地址
		gettimeofday(&newtimer, NULL );

		//连接超时生效,生效后若没有连接成功则进入搜索状态
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000
				+ (newtimer.tv_usec - oldtimer.tv_usec) / 1000 > 5000) { //ms
			connection_exists = 2;
			return -1;
		}

		ret = connect_specified_bluetooth(addr); //尝试建立连接
		if (ret == 1) { //接成功开始工作
			return 1;
		} else {
			return 0;
		}
	} else { //搜索设备
		memset(addr_list, 0, sizeof(addr_list));
		ret = scan_bluetooth(addr_list);
		if (ret == 1) {
			printf("scan bluetooth success \r\n");
			printf("======addr list====== \n");
			printf("%s\n", addr_list);
		} else {
			printf("scan bluetooth fail \r\n");
			return 0;
		}

		//搜到设备后尝试接
		buf = addr_list;
		while ((token = strsep(&buf, delim)) != NULL ) {
			printf("index %d\n", index);
			printf("MAC %s\n", token);

			ret = connect_scanned_bluetooth(index);
			if (ret == 1) {
//				printf("connect success \n");
				break;
			}

			index++;
		}

		if (ret == 1) {
			return 1;
		}else{
			return 0;
		}
	}
}

int disconnect_bluetooth(){
	unsigned char ask[16], answer[16],buf[16],reply[32];
	int len = 0;
	int total = 0;
	struct timeval oldtimer, newtimer;

	memset(ask, 0, sizeof(ask));
	strcpy(ask,"AT");
	strcpy(answer,"OK+LOST");

	printf("request %s\n",ask);

	len = write_port(g_serial_port,ask,strlen(ask));

	memset(reply, 0, sizeof(reply));
	gettimeofday(&oldtimer, NULL );
	gettimeofday(&newtimer, NULL );
	while (1) {
		gettimeofday(&newtimer, NULL );
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000
				+ (newtimer.tv_usec - oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}
		memset(buf, 0, sizeof(buf));
		len = read_port(g_serial_port, buf, sizeof(buf));
		if (len <= 0) {
			continue;
		}

		if (total > sizeof(reply)) {
			break;
		}

		memmove(reply + total, buf, len);
		total += len;

		if (total >= strlen(answer)) {
			break;
		}

	}

	printf("reply %s\n", reply);

	if (strcmp(reply, answer) == 0) {
		return 1;
	} else {
		return 0;
	}
}

int send_by_bluetooth(char *data,int len){
	int num = 0;
    int max_data_size = 20;
	int sended_len = 0;
	int total = 0;


	if (len <= 0){
		return 0;
	}

//	if (connect_flag != 1){
//		return 0;
//	}

	max_data_size = 20;
	total = len;
	while (total > 0) {
		if (total > max_data_size) {
			num = write_port(g_serial_port, data + (len - total), max_data_size);
		} else {
			num = write_port(g_serial_port, data, total);
		}

		if (num <= 0) {
			continue;
		}

		if (num >= 0) {
			total -= num;
			sended_len += num;
		}
	}

	return sended_len;
}

//int recv_by_bluetooth(char *data,int len){
//	int num = 0;
//	int recved_len = 0;
//	int total = 0;
//	char buf[128];
//	int max_data_size = 0;
//
//	if (len <= 0){
//		return 0;
//	}
//
////	if (connect_flag != 1){
////		return 0;
////	}
//
//	max_data_size = sizeof(buf);
//	total = len;
//	while (total > 0) {
//		if (total > max_data_size) {
//			memset(buf, 0, max_data_size);
//			num = read_port(g_serial_port, buf, max_data_size);
//		} else {
//			memset(buf, 0, max_data_size);
//			num = read_port(g_serial_port, buf, total);
//		}
//
//		if (num <= 0) {
//			continue;
//		}
//
//		if (num > 0) {
//			memmove(data + (len - total), buf, num);
//			total -= num;
//			recved_len += num;
//		}
//	}
//
//	return recved_len;
//}

static int read_by_bluetooth(){
	int dataLen = 0;
	char buf[128];
	unsigned char *Addr = NULL;

	memset(buf, 0, sizeof(buf));

	dataLen = read_port(g_serial_port, buf, sizeof(buf));

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

	return dataLen;
}

int recv_by_bluetooth(char *data, int len) {
	int num = 0;
	unsigned char *Addr = NULL;
	int dataLen = 0;

	num = read_by_bluetooth();

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

int close_bluetooth(){
	int ret = 1;

	if (connect_flag) {
		ret = disconnect_bluetooth();
	}

	return ret;
}


//////////////////////////////////////////////////////////////////////////
#define OK       0
#define NO_INPUT 1
#define TOO_LONG 2
static int getLine (char *prmpt, char *buff, size_t sz) {
    int ch, extra;

    // Get line with buffer overrun protection.
    if (prmpt != NULL) {
        printf ("%s", prmpt);
        fflush (stdout);
    }
    if (fgets (buff, sz, stdin) == NULL)
        return NO_INPUT;

    // If it was too long, there'll be no newline. In that case, we flush
    // to end of line so that excess doesn't affect the next call.
    if (buff[strlen(buff)-1] != '\n') {
        extra = 0;
        while (((ch = getchar()) != '\n') && (ch != EOF))
            extra = 1;
        return (extra == 1) ? TOO_LONG : OK;
    }

    // Otherwise remove newline and give string back to caller.
    buff[strlen(buff)-1] = '\0';
   return OK;
}



int _input_number(char* szPrompt, int dwDefault)
{
	int	vKey, vCnt = 0;
	char vStr[16] = {0}, *vStr2;
	int vRet = dwDefault;

	printf("Please input %s [default = %u] ", szPrompt, (unsigned int)dwDefault);

	while(1)
	{
		vKey = getchar();
		if ( vKey >= '0' && vKey <= '9')
		{
			vStr[vCnt] = (char)vKey;
			vCnt++;
			vStr[vCnt] = 0;

			vRet = (int)strtoul(vStr, &vStr2, 10);

			if(vCnt > 10)
				break;
		}
		else if ( vKey == '\n' )
		{
			if (vCnt == 0)
				vRet = dwDefault;
			goto RET;
		}
	}

	while(1)
	{
		vKey = getchar();
		if ( vKey == 0x0a )
			break;
	}

RET:
	return vRet;
}

int main(){
	int ret = 0,len = 0;
	int i = 0;
	int index = 0;
	char data[128];
	int nContinue = 1,vNum=0;

	int rc;
	char buff[256];

	ret = init_bluetooth("hm11",3,9600);
	if (ret == 1){
		printf("init bluetooth success \r\n");
		printf("==========================\n");
	}else{
		printf("init bluetooth fail \r\n");
		return 0;
	}

	while(nContinue){
   		printf( "\n" );
   		printf( "-------Menu-------\n" );
   		printf( "0  : Exit\n" );
   		printf( "1  : open blue tooth \n" );
   		printf( "2  : Password\n" );
   	    printf( "3  : Slave Mode\n" );
   		printf( "4  : Connect\n" );
//   		printf( "5  : Send\n" );
//   		printf( "6  : Receive \n" );
   		printf( "7  : close blue tooth \n" );
   		printf( "8  : Rename \n" );
   		printf( "------------------\n" );

		printf("\n\n");
		vNum = (int) _input_number("menu item", 100);

		switch (vNum) {
		case 0: //Exit
			nContinue = 0;
			printf("Exit OK.\n");
			break;
		case 1: //open
			ret = open_bluetooth();
			if (ret == 1){
				printf("open bluetooth success \r\n");
				printf("==========================\n");
			}else{
				printf("open bluetooth fail \r\n");
				return 0;
			}
			break;
		case 2: //pair
			memset(buff,0,sizeof(buff));
			rc = getLine("Enter pair password > ", buff, sizeof(buff));
			if (rc == NO_INPUT) {
				// Extra NL since my system doesn't output that on EOF.
				printf("\nNo input\n");
				break;
			}

			if (rc == TOO_LONG) {
				printf("Input too long [%s]\n", buff);
				break;
			}
			ret = set_pair_password(buff);
			if (ret == 1){
				printf("set pair password success \n");
			}else{
				printf("set pair password fail \n");
			}
			break;
		case 3: //accept
				ret = accept_bluetooth();
				if (ret == 1) {
					printf("The bluetooth is visible \n");

				}else{
					printf("The bluetooth is not visible \n");
				}

				while (1) {
					memset(data, 0, sizeof(data));
					len = recv_by_bluetooth(data, sizeof(data));
					if (len > 0) {
						printf("recv data %s\n", data);
						len = send_by_bluetooth(data,strlen(data)); //回显
						if (len > 0) {
							printf("send data success\n");
						}
					}
				}

			break;
		case 4: //connect
			ret = connect_bluetooth();
			if (ret == 1){
				printf("connect success \n");
			} else if (ret == -1){
				printf("connect timeout\n");
				break;
			}else {
				printf("connect fail\n");
				break;
			}

			len = send_by_bluetooth("linezd",strlen("linezd"));
			if (len > 0){
				printf("send data success\n");
			}

			//recv
			while (1) {
				memset(data, 0, sizeof(data));
				len = recv_by_bluetooth(data, sizeof(data));
				if (len > 0) {
					printf("recv data %s\n", data);

					memset(buff,0,sizeof(buff));
					rc = getLine("Enter send string > ", buff, sizeof(buff));
					if (rc == NO_INPUT) {
						// Extra NL since my system doesn't output that on EOF.
						printf("\nNo input\n");
//						return 1;
					}

					if (rc == TOO_LONG) {
						printf("Input too long [%s]\n", buff);
//						return 1;
					}

//					printf("OK [%s]\n", buff);

					if (strcmp(buff,"quit") == 0 || strcmp(buff,"exit") == 0){
						break;
					}

					len = send_by_bluetooth(buff, strlen(buff));
					if (len > 0) {
						printf("send data success\n");
					}
				}
			}

			break;
		case 5://send
			len = send_by_bluetooth("linezd",strlen("linezd"));
			if (len > 0){
				printf("send data success\n");
			}
			break;
		case 6://recv
			while (1) {
				memset(data, 0, sizeof(data));
				len = recv_by_bluetooth(data, sizeof(data));
				if (len > 0) {
					printf("recv data %s\n", data);

					rc = getLine("Enter send string> ", buff, sizeof(buff));
					if (rc == NO_INPUT) {
						// Extra NL since my system doesn't output that on EOF.
						printf("\nNo input\n");
//						return 1;
					}

					if (rc == TOO_LONG) {
						printf("Input too long [%s]\n", buff);
//						return 1;
					}

//					printf("OK [%s]\n", buff);

					if (strcmp(buff,"quit") == 0 || strcmp(buff,"exit")){
						break;
					}

					len = send_by_bluetooth(buff, strlen(buff));
					if (len > 0) {
						printf("send data success\n");
					}
				}
			}
			break;
		case 7://close bluetooth
			ret = close_bluetooth();
			if (ret == 1){
				printf("close bluetooth success \n");
			}
			break;
		case 8: //rename
			memset(buff,0,sizeof(buff));
			rc = getLine("Enter name > ", buff, sizeof(buff));
			if (rc == NO_INPUT) {
				// Extra NL since my system doesn't output that on EOF.
				printf("\nNo input\n");
				break;
			}

			if (rc == TOO_LONG) {
				printf("Input too long [%s]\n", buff);
				break;
			}
			ret = set_module_name(buff);
			if (ret == 1){
				printf("rename success \n");
			} else {
				printf("rename fail \n");
				return 0;
			}
			break;
		}
	}

	close_port(g_serial_port);
	return 0;
}
