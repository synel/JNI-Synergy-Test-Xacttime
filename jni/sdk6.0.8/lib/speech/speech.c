/*
 * speech.c
 *
 *  Created on: 2014-8-26
 *      Author: aduo
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "speech.h"

int g_serial_port = 0;

int speech_init(int port,int baud_rate){
	int ret = 0;

	g_serial_port = port;

	ret = open_port(port, baud_rate, 8, "1", 'N');
	if (ret != 1) {
		return 0;
	}
	return ret;
}

//带文本编码设置的文本播放命令
//encoding 0~3
//0 GB2312
//1 GBK
//2 BIG5
//3 UNICODE
int speech_play(int encoding,char *text){
	char ask[5000],answer[16];
	short data_len = 0;
	int i = 0,len = 0,ret = -1;
	struct timeval oldtimer, newtimer;

	data_len = strlen(text);

	if (data_len > 4094) {
		return -1;
	}

	data_len += 2; //+cmd+encoding

	memset(ask,0,sizeof(ask));

	ask[0] = 0xFD; //head

	ask[1] = data_len  >> 16; //len high byte
	ask[2] = data_len  & 0x00FF; //len low byte

	ask[3] = 0x01; //cmd

	switch(encoding){
	case 0:
		ask[4] = 0x00; //encoding
		break;
	case 1:
		ask[4] = 0x01; //encoding
		break;
	case 2:
		ask[4] = 0x02; //encoding
		break;
	case 3:
		ask[4] = 0x03; //encoding
		break;
	default:
		ask[4] = 0x00; //encoding
		break;
	}

    for (i = 0;i < (data_len - 2); i++){
    	ask[i + 5] = text[i];
    }

	len = write_port(g_serial_port, ask, data_len + 3); //+header+len

	gettimeofday(&oldtimer, NULL );
	while (1) {
		gettimeofday(&newtimer, NULL );
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000
				+ (newtimer.tv_usec - oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}

		memset(answer, 0, sizeof(answer));
		len = read_port(g_serial_port, answer, sizeof(answer));

		if (len <= 0) {
			continue;
		}

		if (answer[0] == 0x41) { //cmd ok
			ret = 1;
			break;
		} else if (answer[0] == 0x45) { //cmd error
			ret = -2;
			break;
		} else if (answer[0] == 0x4E) { //busy
			ret = -3;
			break;
		} else if (answer[0] == 0x4F) { //free
			ret = 1;
			break;
		}
	}

	return ret;
}

//停止当前合成
int speech_stop(){
	char ask[16],answer[16];
	short data_len = 0;
	int i = 0,len = 0,ret = -1;
	struct timeval oldtimer, newtimer;

	data_len = 1;

	memset(ask,0,sizeof(ask));

	ask[0] = 0xFD; //head

	ask[1] = data_len  >> 16; //len high byte
	ask[2] = data_len  & 0x00FF; //len low byte

	ask[3] = 0x02; //cmd

	len = write_port(g_serial_port, ask, data_len + 3); //+header+len

	gettimeofday(&oldtimer, NULL );
	while (1) {
		gettimeofday(&newtimer, NULL );
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000
				+ (newtimer.tv_usec - oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}

		memset(answer, 0, sizeof(answer));
		len = read_port(g_serial_port, answer, sizeof(answer));
		if (len <= 0) {
			continue;
		}

		if (answer[0] == 0x41) { //cmd ok
			ret = 1;
			break;
		} else if (answer[0] == 0x45) { //cmd error
			ret = -2;
			break;
		} else if (answer[0] == 0x4E) { //busy
			ret = -3;
			break;
		} else if (answer[0] == 0x4F) { //free
			ret = 1;
			break;
		}
	}

	return ret;
}

//暂停当前合成
int speech_pause(){
	char ask[16],answer[16];
	short data_len = 0;
	int i = 0,len = 0,ret = -1;
	struct timeval oldtimer, newtimer;

	data_len = 1;

	memset(ask,0,sizeof(ask));

	ask[0] = 0xFD; //head

	ask[1] = data_len  >> 16; //len high byte
	ask[2] = data_len  & 0x00FF; //len low byte

	ask[3] = 0x03; //cmd

	len = write_port(g_serial_port, ask, data_len + 3); //+header+len


	gettimeofday(&oldtimer, NULL );
	while (1) {
		gettimeofday(&newtimer, NULL );
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000
				+ (newtimer.tv_usec - oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}

		memset(answer, 0, sizeof(answer));
		len = read_port(g_serial_port, answer, sizeof(answer));
		if (len <= 0) {
			continue;
		}

		if (answer[0] == 0x41) { //cmd ok
			ret = 1;
			break;
		} else if (answer[0] == 0x45) { //cmd error
			ret = -2;
			break;
		} else if (answer[0] == 0x4E) { //busy
			ret = -3;
			break;
		} else if (answer[0] == 0x4F) { //free
			ret = 1;
			break;
		}
	}

	return ret;
}

//恢复暂停的合成
int speech_resume(){
	char ask[16],answer[16];
	short data_len = 0;
	int i = 0,len = 0,ret = -1;
	struct timeval oldtimer, newtimer;

	data_len = 1;

	memset(ask,0,sizeof(ask));

	ask[0] = 0xFD; //head

	ask[1] = data_len  >> 16; //len high byte
	ask[2] = data_len  & 0x00FF; //len low byte

	ask[3] = 0x04; //cmd

	len = write_port(g_serial_port, ask, data_len + 3); //+header+len


	gettimeofday(&oldtimer, NULL );
	while (1) {
		gettimeofday(&newtimer, NULL );
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000
				+ (newtimer.tv_usec - oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}

		memset(answer, 0, sizeof(answer));
		len = read_port(g_serial_port, answer, sizeof(answer));
		if (len <= 0) {
			continue;
		}

		if (answer[0] == 0x41) { //cmd ok
			ret = 1;
			break;
		} else if (answer[0] == 0x45) { //cmd error
			ret = -2;
			break;
		} else if (answer[0] == 0x4E) { //busy
			ret = -3;
			break;
		} else if (answer[0] == 0x4F) { //free
			ret = 1;
			break;
		}
	}

	return 1;
}

//-3 busy
//1 free
int get_work_state() {
	char ask[16], answer[32];
	short data_len = 0;
	int i = 0, len = 0,ret = -1;
	struct timeval oldtimer, newtimer;

	data_len = 1;

	memset(ask, 0, sizeof(ask));

	ask[0] = 0xFD; //head

	ask[1] = data_len >> 16; //len high byte
	ask[2] = data_len & 0x00FF; //len low byte

	ask[3] = 0x21; //cmd

	len = write_port(g_serial_port, ask, data_len + 3); //+header+len

	gettimeofday(&oldtimer, NULL );
	while (1) {
		gettimeofday(&newtimer, NULL );
		if ((newtimer.tv_sec - oldtimer.tv_sec) * 1000
				+ (newtimer.tv_usec - oldtimer.tv_usec) / 1000 > 500) { //ms
			break;
		}

		memset(answer, 0, sizeof(answer));
		len = read_port(g_serial_port, answer, sizeof(answer));
		if (len <= 0) {
			continue;
		}

		if (answer[0] == 0x41) { //cmd ok
			ret = 1;
			break;
		} else if (answer[0] == 0x45) { //cmd error
			ret = -2;
			break;
		} else if (answer[0] == 0x4E) { //busy
			ret = -3;
			break;
		} else if (answer[0] == 0x4F) { //free
			ret = 1;
			break;
		}
	}

	return ret;
}


int speech_uninit(){
	close_port(g_serial_port);
	return 1;
}


int main(int argc,char *argv[]) {
	char a[] = "欢迎使用威尔智能识别终端";
	int ret = 0;

	if (argc > 1){
		strcpy(a,argv[1]);
	}

	ret = speech_init(3, 9600);
	if (ret == 1) {
		printf("speech init succeed\n");
	} else {
		exit(0);
	}

	ret = speech_play(0, a);
	if (ret > 0) {
		printf("speech play succeed\n");
	}

	ret = speech_uninit();
	if (ret == 1) {
		printf("speech uninit succeed\n");
	}
}
