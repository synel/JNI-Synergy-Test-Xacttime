#ifndef NET_TCP_H
#define NET_TCP_H
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/time.h> 
#include <unistd.h>
#include <errno.h>
#include<signal.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <error.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include "../_precomp.h"
#include "../public/protocol.h"

#define LISTENQ 1           	             
#define Maxline 1024
//#define NET_DATALEN 992
#define LEN 64
#define GETWORD(__w,__p) do{__w=*(__p++)<<8;__w|=*(p++);}while(0)
#define GETLONG(__l,__p) do{__l=*(__p++)<<24;__l|=*(__p++)<<16;__l|=*(__p++)<<8;__l|=*(p++);}while(0)


/*========================================================================
                    CalBack Set
==========================================================================*/
typedef struct _cal_BACK
{
	int (*netback)( const char* fliename );
	int (*endback)( int wddaflag );
	int (*verify_manger)(char *id,char *password,char *card);
	void (*write_ts)(char *command,const char *command2,char *msg);
}CALBACK;
extern CALBACK *calback;
/*========================================================================
                    NetInfo Set
==========================================================================*/
typedef struct _net_INFO
{
	//Link-overtime£¬Port£¬Server ip,Verfication-overtime
	char netovertime[LEN],netport[LEN],netserverip[LEN],timeout_server[LEN];
	char dailiip[LEN];    
	int dailisocket;
	int line;            //Net-link mode:0->No verify;1->verify Password;2->verfiy both
	char linepassword[LEN];          //Net-link password
	int linetype;	//0-tcp,1-485,2-modeom,3-gprs
	int vali_up;           //vali_net: report of verification successful
	int netenable;			//record if there are network connections
}NETINFO;
//extern NETINFO netinfo;
/*
struct package{             		       
	char command[30];                            
	short datalen;                               
	char data[NET_DATALEN];                           
};
*/
typedef struct _sendfileproperty
{
	char filename[128];           //filename
	int fileno;                   //Number of file (not used in terminal,only adapt to server.refer to tcp/udp)
	int filesendflag;             //0-transmit,1-download,2-upload
	int totalcount;               //the total number of packet which should be send
	int curcount;                 //the total number of packet have send
	FILE *sendfile;               //file pointer
	int wddaflag;
}Sendfileproperty;
extern Sendfileproperty sendfileproperty;

extern int listenfd,sendflag; //
extern int tcp_socket , udp_socket /*, client_tcp */;
extern int contype;
extern int downwddaflag;
//
 int safe_recv(int fd, struct package *net_package);
//
 int safe_send(int fd, struct package *net_package);
//
void new_dwname(struct package *tmpdata,int netfd);
//
int new_dwload(int netfd,struct package *data1);
//
int link_valid(char *tmpbuff,int datalen);
int new_linezd(struct package*data1,int netfd,int *lizezdsucceed);
//
FILE* new_upname(struct package *tmpbuff,int netfd);
//
int new_upload(int netfd);
//
int new_pathtopath(char *path);
//
int new_deletefile(struct package *tmpbuff,int netfd);
//
int retime(struct package *tmpbuff,int netfd);
//
int tcp_make_socket(unsigned short int);
//
int net_net();
int Functions_Handle(struct package data1);
//
int init_net();
//extern "C"
 int InitNet(IN NETINFO *net_info,OUT CALBACK *cal_back);
 int Net();
 int AcceptLinezd();
 void UninitNet();
 int GetDomain(IN char *buf,OUT char *ipaddr);

int retime_all(int year,int mon,int day,int hour,int min,int sec);
void net_back();
void uppathtopath(char *path);
void downpathtopath(char *path);
int new_retime_all(int year,int mon,int day,int hour,int min,int sec);
int new_retime(struct package * tmpbuff,int netfd);
int filetofile();

int infozd(struct package *tmpbuff);
int config(struct package * tmpbuff,int netfd);
void reset_recv_time();

char * creat_pzinfo();
int appenddirection(char *filename);
void sys_command();
void backupwdjl() ;   //record backup
int meragefile(char *path);
extern time_t recv_time,send_time;//take a record of time when the last data is received
int change(char *filename);
int new_cortzd(struct package *data1);    
char *getkernelinfo();
char *getcpuinfo();
char *getmeminfo();
int checkfile(char *framepath,char *filename);
int new_checkfp(struct package * tmpbuff,int netfd);
int new_gpio(struct package * tmpbuff,int netfd);
int new_player(struct package * tmpbuff,int netfd);
int netselect();

//udp opt
int udp_send(struct package * netdata);
int udp_recv(struct package * netdata,int overtime);
int udp_cortzd(struct package *data1);

#endif
