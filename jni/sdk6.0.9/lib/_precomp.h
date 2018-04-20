#ifndef __PRECOMP_H__
#define __PRECOMP_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>

#include <dirent.h>
#include <sys/types.h>

#include <linux/soundcard.h>

#include <sys/time.h>


#include <termios.h>
#include <sys/ioctl.h>

#include <time.h>
#include <sys/time.h>
#include <dlfcn.h>

#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>

#define RTC_DISABLE		0
#define RTC_ENABLE		1


#define LEN 64
#define TRUE		1
#define FALSE	0
#define ERR_FORMAT 2

#define IN
#define OUT
#define INOUT
#define OPTIONAL


#define APPLICATIONTYEP "SY-2009.09.14"
#define TERMINALTYPE "SY-NERGY"
extern int fingerSum,EnrollState;

#endif /*__PRECOMP_H__*/
