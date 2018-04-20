#ifndef C_SET_DATE_H
#define C_SET_DATE_H
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>
#include "../_precomp.h"
#include "../version.h"


#define WRITESJ 0x5002
#define SETADJTIME  	4901


int SysClock(IN int clock_tz);
int MenuSetDate(IN const char *str,IN int opt);
int MenuSetTime(IN const char *str,IN int opt);
int GetDate(OUT char *str,IN int opt);

int write_rtc(struct tm *ptr);
int read_rtc(struct tm *ptr);

#endif
