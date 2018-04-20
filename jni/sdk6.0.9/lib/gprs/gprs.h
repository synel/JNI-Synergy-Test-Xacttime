#ifndef __GPRS_H
#define __GPRS_H

#include "../public/public.h"
#include "../net/net_tcp.h"
#include "../_precomp.h"
#include "../version.h"

enum _module{em310,m35};
#define MIN_CHANNEL 1
#define MAX_CHANNEL 1 //1..3

struct {
	int module_model;//add by aduo 2013.7.10
	int rate, port, overtime;
	char deputyip[56];
	int deputyport;
} gprsset;

struct {
	int rssi; //signal intensity
	int tsim; //sim card exit or not
} gprsinfo;


extern int gprs_fd;
extern int gprs_reset_time;
extern int gprs_bianhao[MAX_CHANNEL];
extern int gprs_tongdao_count;  //
extern struct timeval oldtimer;

int InitGprs(int port,int rate,int overtime,int protocol,char *deputyip,int deputyport);
int GprsAccept(unsigned char *data,int len);
void CloseLink();
int GprsRecv(unsigned char *data,int len);
int GprsSend(unsigned char *data,unsigned int len);
void GprsJianche();

int my_read(int fd,unsigned char *buf,int len);
int my_write(int fd,unsigned char *buf,int len);


int search_gprs_btl();
int _search_gprs_btl();
int open_link(int tongdao);
void gprs_apn();
void reg_apn();

int gprs_send(unsigned char *data,unsigned int len,int tongdao);
int gprs_tcp_send(unsigned char *data,unsigned int len);
int get_gprs_data(unsigned char *data);
int gprs_recv(unsigned char *data,int len,int tongdao);
int gprs_tcp_recv(unsigned char *data,int len);
int gprs_udp_send(unsigned char *data,unsigned int len);//channel 2
int gprs_video_send(unsigned char *data,unsigned int len);//channel 3
int gprs_udp_recv(unsigned char *data,int len);
int gprs_reset(void);
void gprs_config();

void gprs_write_mac();
void gprs_rssi();
int read_state(int tongdao);
void gprs_tsim();
int read_rssi();
void gprs_write_head();

void reset_send_time();
int ascii_2_hex(unsigned char  *O_data,unsigned char *N_data, int len);
int hex_2_ascii(unsigned char *data, unsigned char *buffer, int len);

#endif
