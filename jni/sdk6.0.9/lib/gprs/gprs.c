#include <stdlib.h>
#include <termios.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "gprs.h"
#include "../public/public.h"

#include "em310.h"
#include "m35.h"

int gprs_fd;

static time_t starttime;

int gprs_bianhao[MAX_CHANNEL];
int gprs_tongdao_count = -1; //
struct timeval oldtimer;

/*--------------------------------------------------------------------------*
 @Function            	my_read - read data from com
 @Include      	gprs.h
 @Description
 fd : serial file description
 buf : data com received
 len : length of data
 read len bytes to buf.if len=0£¬read() will not action and return.
 return value is the actual number of byte; when there is no data or have reached end ,return 0.
 @Return Value		Success return the actual number of byte read;  Failure return -1.
 @Create time		2009-06-15 08:23
 *---------------------------------------------------------------------------*/
int my_read(int fd, unsigned char *buf, int len) {
	struct timeval over_timer;
	fd_set read_fd;
	unsigned char *ptr;
	int num = 0;

	if (fd <= 0)
		return -1;
	FD_ZERO(&read_fd);
	FD_SET(fd, &read_fd);
	over_timer.tv_sec = 0;
	over_timer.tv_usec = 15000;

	while (len != num) {
		over_timer.tv_sec = 0;
		over_timer.tv_usec = 100000;
		if (select(fd + 1, &read_fd, NULL, NULL, &over_timer) <= 0) {
			return num;
		}
		ptr = buf + num;
		num += read(fd, ptr, len - num);
	}
	return num;
}

/*--------------------------------------------------------------------------*
 @Function            	my_write - write data to com
 @Include      	gprs.h
 @Description
 fd : serial file description
 buf : data will be send to com
 len : length of data
 write len bytes of buf to com of fd
 @Return Value		Success return the actual number of byte read;  Failure return -1.
 @Create time		2009-06-15 08:23
 *---------------------------------------------------------------------------*/
int my_write(int fd, unsigned char *buf, int len) {
	struct timeval over_timer;
	fd_set write_fd;
	unsigned char *ptr;
	int num = 0, count = 0;

	if (fd <= 0)
		return -1;
	FD_ZERO(&write_fd);
	FD_SET(fd, &write_fd);
	over_timer.tv_sec = 0;
	over_timer.tv_usec = 1250000;

	while (len != num) {
		over_timer.tv_sec = 0;
		over_timer.tv_usec = 150000;
		if (select(fd + 1, NULL, &write_fd, NULL, &over_timer) <= 0) {
			return num;
		}
		ptr = buf + num;
		count = write(fd, ptr, len - num);
		if (count <= 0)
			return num;
		num += count;
	}
	return num;
}

/*--------------------------------------------------------------------------*
 @Function            	_search_gprs_btl - set gprs baudrate
 @Include      	gprs.h
 @Description
 first set baudrate of local com,then set gprs baudrate,last make both of them unanimous
 @Return Value		Success return fd of com ; Failure return -1.
 @Create time		2009-06-15 08:23
 *---------------------------------------------------------------------------*/
int _search_gprs_btl() {
	speed_t gprs_bautl[] = { B300, B600, B1200, B2400, B4800, B9600, B19200,
			B38400, B57600, B115200 };
	const speed_t bautl[] = { B9600, B38400, B57600, B115200 };
	int fd, i = 0, num = 0;
	struct termios tio;
	fd_set read_fd;
	char buf[1024], *ptr = NULL;
	unsigned char ch = 0;
	struct timeval over_timer;
	char devpath[56];

	memset(devpath, 0, sizeof(devpath));
#if defined _x86
	sprintf(devpath,"/dev/ttyS%d",gprsset.port);
#elif defined _2410
	sprintf(devpath,"/dev/ttyS%d",gprsset.port);
#else
	sprintf(devpath, "/dev/ttySAC%d", gprsset.port);
#endif
	fd = open(devpath, O_RDWR | O_NOCTTY);
	tcgetattr(fd, &tio);
	cfmakeraw(&tio);
	cfsetispeed(&tio, B9600);
	cfsetospeed(&tio, B9600);
	tcsetattr(fd, TCSANOW, &tio);
	ptr = buf;
	while (1) {
		my_write(fd, (unsigned char *) "\r\n", strlen("\r\n"));
		FD_ZERO(&read_fd);
		FD_SET(fd, &read_fd);
		over_timer.tv_sec = 0;
		over_timer.tv_usec = 100000 * ((12 - i) / 2); //delay time change along with the change of baudrate
		if ((select(fd + 1, &read_fd, NULL, NULL, &over_timer)) <= 0) {
			if (i >= (sizeof(gprs_bautl) / sizeof(speed_t))) {
				return fd;
			}
			close(fd);
			fd = open(devpath, O_RDWR | O_NOCTTY);
			tcgetattr(fd, &tio);
			cfmakeraw(&tio);
			cfsetispeed(&tio, gprs_bautl[i]);
			cfsetospeed(&tio, gprs_bautl[i]);
			tcsetattr(fd, TCSANOW, &tio);
			//printf("@@@@@@@ %d\n",gprs_bautl_r[i]);
			i++;
			continue;
		}
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "at+ipr=%d\r\n", gprsset.rate); //modify baudrate
		num = my_write(fd, (unsigned char *) buf, strlen(buf));
		memset(buf, 0, sizeof(buf));
		if (my_read(fd, (unsigned char *) buf, sizeof(buf)) > 0)
			break;
	}
	close(fd);
	fd = open(devpath, O_RDWR | O_NOCTTY);
	tcgetattr(fd, &tio);
	cfmakeraw(&tio);
	if (gprsset.rate == 9600)
		i = 0;
	else if (gprsset.rate == 38400)
		i = 1;
	else if (gprsset.rate == 57600)
		i = 2;
	else if (gprsset.rate == 115200)
		i = 3;
	else
		i = 0;

	cfsetispeed(&tio, bautl[i]);
	cfsetospeed(&tio, bautl[i]);
	tcsetattr(fd, TCSANOW, &tio);
	while (my_read(fd, &ch, 1) > 0)
		;
	my_write(fd, (unsigned char *) "ate 0\r\n", strlen("ate 0\r\n")); //shutdown echo
	my_write(fd, (unsigned char *) "atq 0\r\n", strlen("atq 0\r\n"));
	while (my_read(fd, &ch, 1) > 0)
		;

	usleep(2000000);
	return fd;
}

/*--------------------------------------------------------------------------*
 @Function            	search_gprs_btl - set gprs baudrate
 @Include      	gprs.h
 @Description
 gprs baudrate is set two times in order to make sure it is initialized successfully
 @Return Value		Success return fd of com; Failure return -1.
 @Create time		2009-06-15 08:23
 *---------------------------------------------------------------------------*/
int search_gprs_btl() {
	int fd;
	unsigned char ch;

	fd = _search_gprs_btl();
	if (fd > 0)
		close(fd);
	else
		return -1;
	fd = _search_gprs_btl();
	while (my_read(fd, &ch, 1) > 0)
		;

	usleep(100000);
	while (my_read(fd, &ch, 1) > 0)
		printf("-%c", ch);
	return fd;
}

/*--------------------------------------------------------------------------*
 @Function            	open_link - open one or more TCP
 @Include      	gprs.h
 @Description
 tongdao : link No.
 set type of server£¬ip and port
 at%ipopenx= link No. opened,"link type","IP need to connect"
 @Return Value		Success return the number of bytes written; Failure return -1.
 @Create time		2009-06-15 08:23
 *---------------------------------------------------------------------------*/
int open_link(int tongdao) {
	int return_code;

	switch (gprsset.module_model) {
	case em310:
		return_code = em310_open_tcp_connect(tongdao);
		break;
	case m35:
		return_code = m35_open_tcp_connect(tongdao);
		break;
	}

	return return_code;
}

/*--------------------------------------------------------------------------*
 @Function            	CloseLink - close all links
 @Include      	gprs.h
 @Description
 close tcp/ip connection
 at%ipclose= link No. 3 links at most
 @Return Value		void.
 @Create time		2009-06-15 08:23
 *---------------------------------------------------------------------------*/
void CloseLink() {
	switch (gprsset.module_model) {
	case em310:
		em310_close_all_tcp_connect();
		break;
	case m35:
		m35_close_all_tcp_connect();
		break;
	}
}

/*--------------------------------------------------------------------------*
 @Function            	gprs_apn - read configuration file of gateway
 @Include      	gprs.h
 @Description
 save the data read to memory first
 @Return Value		void.
 @Create time		2009-06-15 08:23
 *---------------------------------------------------------------------------*/
void gprs_apn() {
  switch(gprsset.module_model){
  case em310:
	  em310_get_apn();
	  break;
  case m35:
	  //m35_get_apn();
	  break;
  }
}

/*--------------------------------------------------------------------------*
 @Function            	reg_apn - configure gateway
 @Include      	gprs.h
 @Description
 at%imode=1,2,0	set data mode
 the first para=1 means: model convert the I/O data and user also should convert I/O data
 the second para=2 means: use multi-link at command; =1 means: use single-link at command
 the third para=0 means: use buffer;=1 means do not use buffer
 at%cgdcount=1,ip,cmnet	register mobile CMNET gateway
 at%etcpip="user","gprs"	register account, pw£¬allocate ip,when there is no account or pw ,you
 can keep them empty
 @Return Value		void.
 @Create time		2009-06-15 08:23
 *---------------------------------------------------------------------------*/
void reg_apn() {
	switch (gprsset.module_model) {
	case em310:
		em310_reg_apn();
		break;
	case m35:
		m35_reg_apn();
		break;
	}
}

/*--------------------------------------------------------------------------*
 @Function            	InitGprs - Initialize gprs
 @Include      	gprs.h
 @Description		port : port No.
 rate : baudrate
 Numeric area: 9600, 38400, 57600, 115200
 overtime: timeing out a data connection
 protocol: protocol used
 deputyip : proxy server ip
 deputyport :proxy server port
 @Return Value		Success return fd opened; Failure return -1
 @Create time		2009-06-15 08:23
 *---------------------------------------------------------------------------*/
int InitGprs(int port, int rate, int overtime, int protocol, char *deputyip,
		int deputyport) {
	unsigned char ch;

	gprsset.module_model = 1; //0 = em310,1 = m35

	gprsset.port = port;
	gprsset.rate = rate;
	gprsset.overtime = overtime;
	gprsset.deputyport = deputyport;
	strcpy(gprsset.deputyip, deputyip);
	printf("protocol = %d\n", protocol);

	if (gprs_fd <= 0)
		gprs_fd = search_gprs_btl();
	if (gprs_fd <= 0)
		return -1;

	my_write(gprs_fd, (unsigned char *) "ate 0\r\n", strlen("ate 0\r\n"));
	my_write(gprs_fd, (unsigned char *) "atq 0\r\n", strlen("atq 0\r\n"));
	//gprs_apn();
	while (my_read(gprs_fd, &ch, 1) > 0)
		;
	printf("basic config finished\r\n");
	gprs_apn();//get apn
	//add by auo 2013.7.11
	//<!--
	gprs_tsim(); //check sim
	gprs_rssi(); //get rssi
	//-->
	reg_apn(); //set_ap_name,set_session_mode,open_pdp_context
	printf("apn config finished\r\n");
//	netinfo.net_485=3;
	return gprs_fd;
}
/*--------------------------------------------------------------------------*
 @Function            	gprs_rssi - check signal
 @Include      	gprs.h
 @Description
 check signal intensity of local£¬MSX 31,MIN 0.it is recommended to send this command loop
 we proposed that gprs transmission activate when signal intensity is more than 15
 @Return Value		void
 @Create time		2009-06-15 08:23
 *---------------------------------------------------------------------------*/
void gprs_rssi() {
	unsigned char buf[32];

	memset(buf, 0, sizeof(buf));
	strcpy((char *) buf, "at+csq\r\n");
	usleep(500000);
	my_write(gprs_fd, buf, strlen((char *) buf));
	usleep(500000);
}

/*--------------------------------------------------------------------------*
 @Function            	gprs_tsim - check SIM card
 @Include      	gprs.h
 @Description
 check whether SIM has exited
 1 - not exit;0 - eixt
 @Return Value		void
 @Create time		2009-06-15 08:23
 *---------------------------------------------------------------------------*/
void gprs_tsim() {
	switch (gprsset.module_model) {
	case em310:
		em310_check_sim();
		break;
	case m35:
		m35_check_sim();
		break;
	}
}

void open_pdp_context(){
	switch (gprsset.module_model) {
	case em310:
		em310_open_pdp_context();
		break;
	case m35:
		m35_open_pdp_context();
		break;
	}
}

/*--------------------------------------------------------------------------*
 @Function            	GprsJianche - gprs connection check
 @Include      	gprs.h
 @Description
 data is send every every 5 minutes in order to keep connection
 @Return Value		void
 @Create time		2009-06-15 08:23
 *---------------------------------------------------------------------------*/
void GprsJianche() {
	int i = 0;
	struct timeval newtimer;
	//unsigned char buf[128];

	gettimeofday(&newtimer, NULL );
	//data is send every 5 minutes in order to keep connection
	if (abs((int) difftime(time(NULL ), recv_time)) > 300
			&& abs((int) difftime(time(NULL ), send_time)) > 300) {
		if (gprs_bianhao[0] == 1) { //tcp connect opened
			gprs_write_head();
		}
		//gprs_rssi();//add by aduo 2013.7.11
	}

	if (abs(newtimer.tv_sec - oldtimer.tv_sec) < 40){
		//printf("wait command timeout\r\n");
		return;
	}else{
		printf("command timeout\r\n");
	}

	gettimeofday(&oldtimer, NULL );

	//add by aduo 2013.7.11
	//<!--
/*
	if (gprsinfo.tsim == 0){ //no sim
		printf("no sim inserted\r\n");
		//gprs_tsim();
		return;
	}

	if (gprsinfo.rssi <= 0 || gprsinfo.rssi >= 99){ //no signal
		printf("no signal\r\n");
		//gprs_rssi();
		return;
	}else{
		printf("rssi %d\r\n",gprsinfo.rssi);
	}
*/
	//-->

	printf("gprs_tongdao_count = %d\r\n",gprs_tongdao_count);
	if (gprs_tongdao_count == -1) {
		open_pdp_context();
		gprs_tongdao_count = 0;
		return;
	}

	if (gprs_tongdao_count <= 0)
		return;

	if (gprs_tongdao_count >= MAX_CHANNEL)
		gprs_tongdao_count = MIN_CHANNEL;

	printf("open tcp connect\r\n");
	for (i = gprs_tongdao_count; i <= MAX_CHANNEL; i++)
		if (gprs_bianhao[i - 1] == 0) {
			open_link(i);
			gprs_tongdao_count = i;
			return;
		}

	for (i = MIN_CHANNEL; i <= gprs_tongdao_count; i++)
		if (gprs_bianhao[i - 1] == 0) {
			open_link(i);
			gprs_tongdao_count = i;
			return;
		}
}

/*--------------------------------------------------------------------------*
 @Function            	ascii_2_hex - convert ASCII to HEX
 @Include      	gprs.h
 @Description
 suction para
 O_data: data will be converted
 N_data: data after converting
 len : length of data need to be converted
 other£ºlength of data been converted
 Notice£ºdata in O_data[] array will be modified in convertion
 @Return Value		-1: Failure
 @Create time		2009-06-15 08:23
 *--------------------------------------------------------------------------*/
int ascii_2_hex(unsigned char *O_data, unsigned char *N_data, int len) {
	int i, j, tmp_len;
	unsigned char tmpData;
	unsigned char *O_buf = O_data;
	unsigned char *N_buf = N_data;

	for (i = 0; i < len; i++) {
		if ((O_buf[i] >= '0') && (O_buf[i] <= '9')) {
			tmpData = O_buf[i] - '0';
		} else if ((O_buf[i] >= 'A') && (O_buf[i] <= 'F')) //A....F
				{
			tmpData = O_buf[i] - 0x37;
		} else if ((O_buf[i] >= 'a') && (O_buf[i] <= 'f')) //a....f
				{
			tmpData = O_buf[i] - 0x57;
		} else {
			return -1;
		}
		O_buf[i] = tmpData;
	}
	for (tmp_len = 0, j = 0; j < i; j += 2) {
		N_buf[tmp_len++] = (O_buf[j] << 4) | O_buf[j + 1];
	}
	return tmp_len;
}

/*--------------------------------------------------------------------------*
 @Function            	hex_2_ascii - convert HEX to ASCII
 @Include      	gprs.h
 @Description
 suction para
 data: data will be converted
 buffer: data after converting
 len : length of data need to be converted
 @Return Value		length of data been converted
 @Create time		2009-06-15 08:23
 *--------------------------------------------------------------------------*/
int hex_2_ascii(unsigned char *data, unsigned char *buffer, int len) {
	const char ascTable[17] = { "0123456789ABCDEF" };
	unsigned char *tmp_p = buffer;
	int i, pos;

	pos = 0;
	for (i = 0; i < len; i++) {
		tmp_p[pos++] = ascTable[data[i] >> 4];
		tmp_p[pos++] = ascTable[data[i] & 0x0f];
	}
	tmp_p[pos] = '\0';
	return pos;
}

/*--------------------------------------------------------------------------*
 @Function            	gprs_send - send data
 @Include      	gprs.h
 @Description
 data : data will be send
 len :length of data being send
 tongdao : Link No.
 after data has been send,wait for gprs status
 when %IPSENDX received ,it means success
 @Return Value		Success reurn the number of bytes send; Failure return -1
 @Create time		2009-06-15 08:23
 *--------------------------------------------------------------------------*/
int gprs_send(unsigned char *data, unsigned int len, int tongdao) {
	int return_code;

	switch (gprsset.module_model) {
	case em310:
		return_code = em310_gprs_send(data, len, tongdao);
		break;
	case m35:
		return_code = m35_gprs_send(data, len, tongdao);
		break;
	}

	return return_code;
}

/*--------------------------------------------------------------------------*
 @Function            	gprs_recv - receive data
 @Include      	gprs.h
 @Description
 data : save data reveived
 len : lenght of data
 tongdao : Link No.
 read data of buffer through serial port and check the type
 @Return Value		Success reurn the number of bytes received; Failure return -1
 No data read return 0
 @Create time		2009-06-15 08:23
 *--------------------------------------------------------------------------*/
int gprs_recv(unsigned char *data, int len, int tongdao) {
	int return_code;

	switch (gprsset.module_model) {
	case em310:
		return_code = em310_gprs_recv(data, len, tongdao);
		break;
	case m35:
		return_code = m35_gprs_recv(data, len, tongdao);
		break;
	}

	return return_code;
}

/*--------------------------------------------------------------------------*
 @Function            	GprsSend - send data
 @Include      	gprs.h
 @Description
 data : data will be send
 len :length of data being send
 IPSENDX: send 512 bytes each time
 @Return Value		Success reurn the number of bytes send; Failure return -1
 @Create time		2009-06-15 08:23
 *--------------------------------------------------------------------------*/
int GprsSend(unsigned char *data, unsigned int len) {
	int num = 0;
	int flag = 512;

	reset_send_time(); // timing
	num = len;
	while (num > 0) {
		time(&starttime);
		if (num <= flag) {
			if (gprs_send(data + (len - num), num, 1) > 0) {
				return len;
			} else
				return -1;
		} else {
			if (gprs_send(data + (len - num), flag, 1) > 0)
				num -= flag;
			else
				return -1;
		}
	}
	return len;
}

/*--------------------------------------------------------------------------*
 @Function            	get_gprs_data - receive data
 @Include      	gprs.h
 @Description
 data : data received
 @Return Value		Success reurn the number of bytes received; Failure return -1
 @Create time		2009-06-15 08:23
 *--------------------------------------------------------------------------*/
int get_gprs_data(unsigned char *data) {
	int num = 0, i = 0, j = 0,/*status=0,*/count1 = 0, count2 = 0;
	static unsigned char buf[40960];
	static int pos = 0; //datasize in buffer

	if (sizeof(buf) - pos > 0) //if buffer is not full,read data.

		num = my_read(gprs_fd, buf + pos, sizeof(buf) - pos);

	pos += num;
	if (pos <= 0)
		return -1;
	num = strcspn((char*) buf, "\r\n");
	if (num == pos) {
		if (pos != (count1 = strcspn((char *) buf, "\""))
				&& (pos - count1 - 1)
						!= (count2 = strcspn((char *) buf + count1 + 1, "\""))) // "" appear
						{
			num = count1 + count2 + 2;
			memcpy(data, buf, num); //one line data read
			for (i = num, j = 0; i < pos; i++, j++) //move data of public buffer
				buf[j] = buf[i];
			pos = pos - num; //datasize in buffer
			return num;
		}
		return -1;
	}

	//if still no "\r\n" are read.then
	memcpy(data, buf, num + 2); //one line data read
	data[num] = '\0'; //cut "\r\n"

	for (i = num + 2, j = 0; i < pos; i++, j++) //move data of public buffer
		buf[j] = buf[i];
	pos = pos - num - 2; //datasize in buffer
	return num;
}

/*--------------------------------------------------------------------------*
 @Function            	gprs_write_mac - send MAC
 @Include      	gprs.h
 @Description
 send MAC to test after successfully connected
 @Return Value		viod
 @Create time		2009-06-15 08:23
 *--------------------------------------------------------------------------*/
void gprs_write_mac() {
	unsigned char buf[64];

	memset(buf, 0, sizeof(buf));
	sprintf((char *) buf, "mac=%s", "0A:08:00:08:03:13");
	GprsSend(buf, strlen((char *) buf));
}

/*--------------------------------------------------------------------------*
 @Function            	gprs_write_head - send heartbeat packet
 @Include      	gprs.h
 @Description
 Send the MAC address is to keep GPRS connection.
 for no data is send to mobile base station in a period of time,the link will be disconnected without return
 @Return Value		viod
 @Create time		2009-06-15 08:23
 *--------------------------------------------------------------------------*/
void gprs_write_head() {
	unsigned char buf[64];

	memset(buf, 0, sizeof(buf));
	sprintf((char *) buf, "MAC=%s", "0A:08:00:08:03:13");
	GprsSend(buf, strlen((char *) buf));
}

/*--------------------------------------------------------------------------*
 @Function            	GprsRecv - receive data
 @Include      	gprs.h
 @Description
 data : save data received
 len : lenght of data
 read data through serial port until 1024 bytes are read
 @Return Value		Success reurn the number of bytes received; Failure return -1
 No data read return 0
 @Create time		2009-06-15 08:23
 *--------------------------------------------------------------------------*/
//int total = 0;
//unsigned char *recvbuf[1024];
int GprsRecv(unsigned char *data, int len) {
/*	int num = 0;

	if (!gprs_bianhao[0] || abs(time(NULL ) - starttime) > gprsset.overtime) //check overtime
			{
		total = 0;
		memset(recvbuf, 0, sizeof(recvbuf));
		return -1;
	}
	if ((num = gprs_recv(recvbuf[total], len - total, 1)) <= 0)
		return 0;

	total += num;
	time(&starttime);
	if (total == len) {
		memcpy(data, recvbuf, total);
		memset(recvbuf, 0, sizeof(recvbuf));
		total = 0;
		return len;
	}
*/

//add by aduo 2013.7.17
//<!--
	static unsigned char buf[1024];
	static int total = 0;
	int num = 0;

	//if (!gprs_bianhao[0] || abs(time(NULL ) - recv_time) > gprsset.overtime){ //check overtime
	//	total = 0;
	//	memset(buf, 0, sizeof(buf));
	//	return -1;
	//}

	num = gprs_recv(buf + total, len - total, 1);
	if (num <= 0)
		return -1;

	total += num;
	//reset_recv_time();
	if (total == len) {
		memcpy(data, buf, len);
		memset(buf, 0, sizeof(buf));
		total = 0;
		return len;
	}
//-->
	return 0;
}

/*--------------------------------------------------------------------------*
 @Function            	close_one_link - close tcp connection
 @Include      	gprs.h
 @Description		refer to CloseLink()
 @Return Value		void
 @Create time		2009-06-15 08:23
 *--------------------------------------------------------------------------*/
void close_one_link(int tongdao) {
	switch (gprsset.module_model) {
	case em310:
		em310_close_tcp_connect(tongdao);
		break;
	case m35:
		m35_close_tcp_connect(tongdao);
		break;
	}
}

/*--------------------------------------------------------------------------*
 @Function            	GprsAccept - waiting for connection
 @Include      	gprs.h
 @Description		data: save data
 len : length of data
 @Return Value		Success reurn the number of bytes received; Failure return -1
 @Create time		2009-06-15 08:23
 *--------------------------------------------------------------------------*/
int GprsAccept(unsigned char *data, int len) {
	static unsigned char buf[1024];
	int num = 0, total = 0;

	num = gprs_recv(buf + total, len - total, 1);
	if (num <= 0)
		return -1;

	total += num;
	if (total == len) {
		memcpy(data, buf, len);
		total = 0;
		printf("gprs_accet=%s,%s\n", buf, data);
		if (strncmp((char *) buf, "linezd", 6) == 0) {
			time(&starttime); //record start time
			return len;
		} else if (strncmp((char *) buf, "linedk", 6) == 0)
			return -1;
		else {
			close_one_link(1);
			return -1;
		}

	}
	return -1;
}

/*--------------------------------------------------------------------------*
 @Function            	gprs_reset - reboot gprs link
 @Include      	gprs.h
 @Description

 @Return Value		Success reurn the number of bytes received; Failure return -1
 @Create time		2009-06-15 08:23
 *--------------------------------------------------------------------------*/
int gprs_reset_time;
int gprs_reset(void) {
#if defined _2410
	int w;
	int fd;
	if ((fd = open("/dev/GSM", O_RDWR)) == -1) {
		perror("open eror");
		close(fd);
		return -1;
	}

	w = 0; //output low-level
	if (ioctl(fd, 0, &w) != 0) {
		perror("ioctl erreor");
		close(fd);
		return -1;
	}
	usleep(3000000);
	w = 1; //output high-level
	if (ioctl(fd, 0, &w) != 0) {
		perror("ioctl erreor");
		close(fd);
		return -1;
	}
	if (gprs_reset_time == 0)
		gprs_reset_time = 6000;
	usleep(gprs_reset_time * 1000);

	w = 0; //output low-level
	if (ioctl(fd, 0, &w) != 0) {
		perror("ioctl erreor");
		close(fd);
		return -1;
	}
	usleep(100000);
	w = 1; //output high-level
	if (ioctl(fd, 0, &w) != 0) {
		perror("ioctl erreor");
		close(fd);
		return -1;
	}
	sleep(3);
	close(fd);
#endif
	InitGprs(gprsset.port, gprsset.rate, gprsset.overtime, 0, gprsset.deputyip,
			gprsset.deputyport);
	memset(gprs_bianhao, 0, sizeof(gprs_bianhao));
	gettimeofday(&oldtimer, NULL );
	reset_recv_time();
	return 0;
}

void reset_send_time() {
	static int num = 0;

	if (abs((int) difftime(time(NULL ), send_time)) > gprsset.overtime
			&& abs((int) difftime(send_time, recv_time)) > 300) {
		num++;
		if (num > 1) {
			num = 0;
			gprs_reset();
		}
	} else {
		num = 0;
	}

	time(&send_time);
}
