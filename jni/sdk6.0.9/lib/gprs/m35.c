/*
 * m35.c
 *
 *  Created on: 2013-7-10
 *      Author: aduo
 */

#include "gprs.h"

//response
static char *ipdata="+RECEIVE:";
static char *ipsendx="SEND OK";
static char *GPRS_ERROR="ERROR";
static char *gprs_error = "+CME ERROR:";
static char *GPRS_OK="OK";

static char *pdp_context_closed = "+PDP DEACT";
static char *pdp_context_disconnect_ok = "DEACT OK";

static char *ipclose="CLOSED";
static char *tcp_disconnect_ok = "CLOSE OK";
static char *gprs_csq = "+CSQ:";
static char *tcp_connect_ok = "CONNECT OK";
static char *tcp_connect_fail = "CONNECT FAIL";
static char *has_sim = "+CPIN: READY";
static char *no_sim = "+CME ERROR: 10";


//request
//apn,session_mode,open_pdp_context
static char init_ptr[][128] = {"AT+QICSGP=1,\"cmnet\"", //apn
		"AT+QIMUX=1",
		"AT+QIFGCNT=0"
		};

static unsigned char  in_buf[40960];
static int in_buf_pos=0;	//datasize in buffer

void m35_check_sim(){
	unsigned char buf[32];

	memset(buf,0,sizeof(buf));
	strcpy((char *) buf, "AT+CPIN?\r\n");
	my_write(gprs_fd,buf,strlen((char *)buf));
	sleep(1);
}

void m35_get_apn() {
	static FILE *file = NULL;
	unsigned char buf[128];

	if (!file)
		file = fopen("./apn.ini", "r");
	if (!file)
		return;
	memset(buf, 0, sizeof(buf));
	if (fgets((char*) buf, sizeof(buf), file) == NULL ) {
		fclose(file);
		file = NULL;
		return;
	}
	cut((char *) buf);
	if (strlen((char*) buf))
		strcpy(init_ptr[0], (char*) buf);
	memset((char*) buf, 0, sizeof(buf));
	if (fgets((char*) buf, sizeof(buf), file) == NULL ) {
		fclose(file);
		file = NULL;
	}
	cut((char *) buf);
	if (strlen((char*) buf))
		strcpy(init_ptr[2], (char*) buf);
}

void m35_set_ap_name(){
	unsigned char buf[128];

	memset(buf, 0, sizeof(buf));
	sprintf((char *) buf, "%s\r\n", init_ptr[0]);
	my_write(gprs_fd, buf, strlen((char *) buf));
}

void m35_set_session_mode(){
	unsigned char buf[128];

	memset(buf, 0, sizeof(buf));
	sprintf((char *) buf, "%s\r\n", init_ptr[1]);
	my_write(gprs_fd, buf, strlen((char *) buf));
}

void m35_open_pdp_context() {
	unsigned char buf[128];

	memset(buf, 0, sizeof(buf));
	sprintf((char *) buf, "%s\r\n", init_ptr[2]);
	my_write(gprs_fd, buf, strlen((char *) buf));
}

void m35_reg_apn(){
	m35_set_ap_name();
	usleep(2000000);
	m35_set_session_mode();
	usleep(2000000);
	m35_open_pdp_context();
	usleep(2000000);
}

int m35_open_tcp_connect(int tongdao){
	char buf[256];

	memset(buf,0,sizeof(buf));
	if(tongdao==1)
	    sprintf(buf, "AT+QIOPEN=%d,\"tcp\",\"%s\",%d\r\n", tongdao
	    		,gprsset.deputyip,gprsset.deputyport);
/*	else if(tongdao==2)
		sprintf(buf,"at%%ipopenx=%d,\"udp\",\"%s\",%d\r\n",tongdao,"192.168.1.255",3350+1);
	else if(tongdao==3)
		sprintf(buf,"at%%ipopenx=%d,\"udp\",\"%s\",%d\r\n",tongdao,"192.168.1.255",3350+3);
*/
	return my_write(gprs_fd,(unsigned char *)buf,strlen(buf));
}

void m35_close_tcp_connect(int tongdao)
{
	char buf[256];

	memset(buf,0,sizeof(buf));
	sprintf(buf, "AT+QICLOSE=%d\r\n", tongdao);
	my_write(gprs_fd,(unsigned char *)buf,strlen(buf));
}


/*--------------------------------------------------------------------------*
@Function            	_gprs_send - send data through serial port
@Include      	gprs.h
@Description
			data : data will be send
			len : length of data being send
			tongdao : Link No.
			at%ipsendx=Link No. opened; data send;
			all the data will be send must be placed in quotes ""
			send data to opened TCP/UDP connection in multi-link mode
@Return Value		Success reurn the number of bytes send; Failure return -1
@Create time		2009-06-15 08:23
*--------------------------------------------------------------------------*/
static int _gprs_send(unsigned char *data,unsigned int len,int tongdao){

	char *ptr = "AT+QISEND=";
	struct timeval oldtimer,newtimer;
	unsigned char buf[20480], src[40960];
	int num, count = 0;
	char ans[128];

	if (tongdao < 0 || tongdao > 3){
		return - 1;
	}
	if (gprs_bianhao[tongdao - 1] != 1) {
		//printf("no tcp/udp session\r\n");
		return -1;
	}
	memset(buf, 0, sizeof(buf));
	memset(src, 0, sizeof(src));

	memcpy(buf, data, len);

	sprintf((char *) src, "%s%d,%d\r\n", ptr, tongdao, len);
	//printf("The data of send %s\r\n", src);
	num = strlen((char*) src);

	count = my_write(gprs_fd, src, num);

	//printf("writed %d\r",count);
	if (count != num) return -1;

	gettimeofday(&oldtimer,NULL);
	gettimeofday(&newtimer,NULL);
	while((newtimer.tv_sec-oldtimer.tv_sec)*1000+(newtimer.tv_usec-oldtimer.tv_usec)/1000<=500)
	{
	    gettimeofday(&newtimer, NULL);
		if (gprs_bianhao[tongdao - 1] == 0)
			return -1;
		memset(ans, 0, sizeof(ans));
		count = my_read(gprs_fd, (unsigned char *) ans, 128);
		if (count == 0)
			continue;

		if (strstr(ans, ">")) {
			//sprintf((char *) src, "%s\r\n", buf);
			memcpy(src, buf, len);
			src[len] = '\r';
			src[len + 1] = '\n';
			src[len + 2] = '\0';
			break;
		}else
			return -1;
	}

	count = my_write(gprs_fd, src, len);
	//printf("count %d,%d\n",count,num);
	if (count != len)
		return -1;
	else
		return len;
}

static int _get_data_public_buf(unsigned char *data, int len) {
	int num = 0;
	num = (len > in_buf_pos ? in_buf_pos : len);
	//printf("get_data_public_buf %d,%d,%d\n",num,len,Ipos);
	memcpy(data, in_buf, num);
	in_buf_pos -= num;
	memmove(in_buf, in_buf + num, in_buf_pos);
	//printf("%s\n",public_buf);
	return num;
}


/*--------------------------------------------------------------------------*
@Function            	get_gprs_data - receive data
@Include      	gprs.h
@Description
			data : data received
@Return Value		Success reurn the number of bytes received; Failure return -1
@Create time		2009-06-15 08:23
*--------------------------------------------------------------------------*/
static int _gprs_recv(unsigned char *data) {
	int num = 0, i = 0, j = 0,/*status=0,*/count1 = 0, count2 = 0;

	if (sizeof(in_buf) - in_buf_pos > 0) //if buffer is not full,read data.

		num = my_read(gprs_fd, in_buf + in_buf_pos,
				sizeof(in_buf) - in_buf_pos);

	in_buf_pos += num;
	if (in_buf_pos <= 0)
		return -1;
	num = strcspn((char*) in_buf, "\r\n");
	if (num == in_buf_pos) {
		if (in_buf_pos != (count1 = strcspn((char *) in_buf, "\""))
				&& (in_buf_pos - count1 - 1)
						!= (count2 = strcspn((char *) in_buf + count1 + 1, "\""))) // "" appear
						{
			num = count1 + count2 + 2;
			memcpy(data, in_buf, num); //one line data read
			for (i = num, j = 0; i < in_buf_pos; i++, j++) //move data of public buffer
				in_buf[j] = in_buf[i];
			in_buf_pos = in_buf_pos - num; //datasize in buffer
			return num;
		}
		return -1;
	}

	//if still no "\r\n" are read.then
	memcpy(data, in_buf, num + 2); //one line data read
	data[num] = '\0'; //cut "\r\n"

	for (i = num + 2, j = 0; i < in_buf_pos; i++, j++) //move data of public buffer
		in_buf[j] = in_buf[i];
	in_buf_pos = in_buf_pos - num - 2; //datasize in buffer
	return num;
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
int m35_gprs_recv(unsigned char *data, int len, int tongdao) {
	unsigned char buf[20480], tmp[20480];
	static char buffer[MAX_CHANNEL][10240]; //Receiving buffer of 3 channels
	static int pos[MAX_CHANNEL]; //length of every channel
	static int count = 0,cur_tongdao=0;
	int num = 0, n = 0;

	if (count > 0) {
		if (in_buf_pos)
			num = _get_data_public_buf(
					(unsigned char*) buffer[cur_tongdao - 1]
							+ pos[cur_tongdao - 1], count);
		else
			num = my_read(gprs_fd,
					(unsigned char*) buffer[cur_tongdao - 1]
							+ pos[cur_tongdao - 1], count);

		pos[cur_tongdao - 1] += num;
		count -= num;
		//printf("count %d,%d,%d\n", count, num, pos[cur_tongdao-1]);
	} else {
		memset(buf, 0, sizeof(buf));
		if (_gprs_recv(buf) > 0) {
			//memset(tmp, 0, sizeof(tmp));
    		printf("buf= %.25s\n",(char*)buf);
			if (strstr((char*) buf, ipclose)) {  //tcp closed
				strcpy((char *) data, (char *) buf);
				n = atoi((char*) buf);
				if (n >= MIN_CHANNEL && n <= 3) {
					gprs_tongdao_count = n; //open tcp connect [n] again
					gprs_bianhao[n - 1] = 0;
					pos[n - 1] = 0;
					oldtimer.tv_sec = 0;
					return -1;
				}
			}

			if (strstr((char*) buf, pdp_context_closed)) {
				strcpy((char *) data, (char *) buf);

				gprs_tongdao_count = -1;  //open pdp context again
				memset(gprs_bianhao, 0, sizeof(gprs_bianhao));
				memset(pos, 0, sizeof(pos));

				oldtimer.tv_sec = 0;
				return -1;
			}

			if (strstr((char*) buf, tcp_disconnect_ok)) {
				strcpy((char *) data, (char *) buf);
				n = atoi((char*) buf);
				if (n >= MIN_CHANNEL && n <= MAX_CHANNEL) {
					gprs_tongdao_count = n; //open tcp connect [n] again
					gprs_bianhao[n - 1] = 0;
					pos[n - 1] = 0;
					oldtimer.tv_sec = 0;
					return -1;
				}
			}

			if (strstr((char*) buf, pdp_context_disconnect_ok)) {
				strcpy((char *) data, (char *) buf);
				gprs_tongdao_count = -1; //open pdp context again
				memset(gprs_bianhao, 0, sizeof(gprs_bianhao));
				memset(pos, 0, sizeof(pos));
				oldtimer.tv_sec = 0;
				return -1;
			}

			if (strncmp((char *) buf, GPRS_OK, strlen(GPRS_OK)) == 0) {
				if (gprs_tongdao_count == 0) // fail to register
						{
					gprs_tongdao_count = 1; //register the first channel
					oldtimer.tv_sec = 0;
					sleep(1);
					return -1;
				}
				if (tongdao == 0 && data)
					strcpy((char *) data, (char *) buf);
				else
					return -1;
				return strlen(GPRS_OK);
			}

			if (strncmp((char*) buf, (char *) gprs_error,
					strlen((char *) gprs_error)) == 0) //ERROR
					{
				strcpy((char *) data, (char *) buf);

				if (strlen((char*) buf) == strlen((char*) gprs_error))
					return -1;

				if (sscanf((char*) buf, "%*[^:]: %d", &n) == 1){
					switch(n){
					case 10://no sim
					case 13://sim failure
					case 15://sim wrong
						oldtimer.tv_sec = 0;
						break;
					case 30://no network service
					case 31://network timeout
					case 107://gprs services not allowed
					case 112://location area not allowed
					case 113://roaming not allowed in this location area
					case 148://unspecified GPRS error
					case 149://PDP authentication failure
						oldtimer.tv_sec = 0;
						break;
					case 2088://GPRS - no PDP context activated
						gprs_tongdao_count = -1; //open pdp context again
						break;
					case 3099://GPRS - unknown APN
						m35_get_apn();
						m35_reg_apn();
					break;
					}
					return -1;
				}

			} //if

			if (strstr((char*) buf, tcp_connect_ok)) //CONNECT OK
					{
				n = atoi((char*) buf);
				if (n >= MIN_CHANNEL && n <= MAX_CHANNEL) {
					gprs_bianhao[n - 1] = 1; //connected
					if(n == 1)
					{
						gprs_write_mac();
					}
					sleep(1);
					oldtimer.tv_sec = 0;
					return 0;
				}
			}

			if (strstr((char*) buf, tcp_connect_fail)){ //CONNECT FAIL
				n = atoi((char*) buf);
				if (n >= MIN_CHANNEL && n <= MAX_CHANNEL) {
					gprs_bianhao[n - 1] = 0; //disconnected
					pos[n - 1] = 0;
					//oldtimer.tv_sec = 0;
					m35_close_tcp_connect(n);
					return 0;
				}
			}

			if (strncmp((char *) buf, ipsendx, strlen(ipsendx)) == 0) {
				if (tongdao == 0 && data)
					strcpy((char *) data, (char *) buf);
				else
					return -1;
				return strlen(ipsendx);
			}

			if (strncmp((char *) buf, ipdata, strlen(ipdata)) == 0) //data come
					{
				reset_recv_time();

				memset(tmp, 0, sizeof(tmp));

				//printf("data analyse\r\n");

				if (sscanf((char*) buf, "%*[^:]: %d, %d", &n, &num) == 2) {
					if (n >= MIN_CHANNEL && n <= MAX_CHANNEL) {
						gprs_bianhao[n - 1] = 1;
						cur_tongdao=n;
						count = num;
						//printf("theory num = %d\r\n",num);
						num = _get_data_public_buf(
								(unsigned char*) buffer[n - 1] + pos[n - 1],
								num);
						//printf("fact num = %d\r\n",num);
						pos[n - 1] += num;
						count -= num;
						//printf("count = %d\r\n",count);
					}
				}
			}
			//check signal intensity£¬ MAX 31£¬MIN 0
			if (strncmp((char*) buf, gprs_csq, strlen(gprs_csq)) == 0) {
				sscanf((char*) buf, "%*[^:]:%d,%d", &num, &n);
				gprsinfo.rssi = num;
			}

			//check whether SIM exit
			if (strncmp((char*) buf, has_sim, strlen(has_sim)) == 0) {
				gprsinfo.tsim = 1;
			} else if (strncmp((char*) buf, no_sim, strlen(no_sim))
					== 0) {
				gprsinfo.tsim = 0;
			}
		} //if(get_gprs_data(buf)>0)
	}
	if (pos[0] > 0 && strncmp(buffer[0], "MAC=OK", 6) == 0) {
		memmove(buffer[0], buffer[0] + 6, pos[0] - 6);
		pos[0] = pos[0] - 6;
	}

	if (tongdao >= MIN_CHANNEL && tongdao <= MAX_CHANNEL) {
		num = (pos[tongdao - 1] > len ? len : pos[tongdao - 1]);
		//printf("cache num = %d\r\n",num);
		if (num > 0) {
			memcpy(data, buffer[tongdao - 1], num);
			memmove(buffer[tongdao - 1], buffer[tongdao - 1] + num,
					pos[tongdao - 1] - num);
			pos[tongdao - 1] = pos[tongdao - 1] - num;
			return num;
		}
	}
	return 0;
}


int m35_gprs_send(unsigned char *data,unsigned int len,int tongdao)
{
	struct timeval oldtimer,newtimer;
	unsigned char buf[128];
	//int num=0;

	gettimeofday(&oldtimer,NULL);
	gettimeofday(&newtimer,NULL);
	if(_gprs_send(data,len,tongdao)<=0)	return -1;
	while(abs(newtimer.tv_sec-oldtimer.tv_sec)<= gprsset.overtime)
	{
		gettimeofday(&newtimer,NULL);
		if((m35_gprs_recv(buf,0,0))==0)	continue;
		printf("gprs_send %.25s\n",(char*)buf);
		if(strncmp((char *)buf,ipsendx,strlen(ipsendx))==0)
		{
			return len;
		}
		if(strncmp((char *)buf,GPRS_ERROR,strlen(GPRS_ERROR))==0)		{return -1;}
		if(strncmp((char *)buf,ipclose,strlen(ipclose))==0)			{return -1;}
	}
	return -1;
}


void m35_close_all_tcp_connect(){
	int i=0;

	for(i=3;i>=1;i--)
	{
		m35_close_tcp_connect(i);
		usleep(200000);
	}
}

