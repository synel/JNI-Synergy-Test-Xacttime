/*------------------------------------------------------------------------
 * description£º  
 * filename - auxi.h 
 *
 * functions list£º 
 *        force_open     - create file forcibly..
 *        file_exist     - judge if the file has exist
 *        file_rename    - rename the file
 *        file_delete    - delete the file
 *        file_length    - count file's length
 *        root_dir       - 
 *        efprintf       - print error info
 *        sysreboot      - reboot
 *-----------------------------------------------------------------------*/

/*
 * header files supporting   £¨auxiliary£©
 */

#ifndef __AUXI_H
#define __AUXI_H
 
/* followings are header files used */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/reboot.h>


FILE *force_open(char *path, const char *mode);
int file_exist(char *file_name);
int file_rename(char *old_file_name, char *new_file_name);
int file_delete(char *file_name);
long int file_length(char *file_name);
char *root_dir(char *dir);
//int efprintf(FILE *stream, const char *fmt, ...);
void sysreboot(void);


#endif /* auxi.h */
