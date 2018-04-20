#ifndef PRINT_H__
#define PRINT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include "../version.h"

enum _printer_model{s310};

extern int fspfd;        //serial port
struct printdevinfo
{
	int fd;
	int model; //add by aduo 2013.7.19
	int printflag;
};
extern struct printdevinfo printdevinfo;

#define TRUE	1
#define FALSE	0

int set_speed(char *dev, int speed);
int WriteComm( unsigned char * data, int datalength) ;
int ReadComm (unsigned char *buf,int len);

int GetFileInfo(char *filename);
int PrintFile(void);
int SendOrder(char *command);
int Readstatus();
int ReadComStatus();

//api
int OpenDev(int comport,int speed,int databits,int stopbits,int parity );
int PrintData(char *buf);
int UnPrintCom(void);
void PrintClear(void);

#endif
