#ifndef __FILE_H
#define __FILE_H
/**
 * @chinese
 * @file   file.h
 * @author 胡俊远
 * @date   Thu Oct 13 10:00:00 2011
 *
 * @brief  文件相关处理模块
 * @endchinese
 *
 * @english
 * @file   file.h
 * @author Hu Junyuan
 * @date   Thu Oct 13 10:00:00 2011
 *
 * @brief  file handling module
 * @endenglish
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/time.h>
#include<unistd.h>
#include<signal.h>

 long get_file_size(char *filename);
int mv_file(char *src ,char *dest);

int cp_file(char *from ,char *to);
int creat_directory(char *path);
int transform_code(char *path);
int count_file_number(char *photopath);
int check_directory(char *path);
int safe_cp_file(char * oldpath,char *newpath,int type);
int safe_rm_file(char * oldpath,int type);
FILE* get_directory_list(char *path);
int delete_record_file(char *src ,int number);
int dial_record_file(char *src ,char *dst,int number);
int count_file_line(char * filename);
int pox_system(const char *cmd_line);

int path_to_directory(char *path,char *directory);

//用户定制程序。
int safe_cp_file_jiami(char * oldpath,char *newpath,int type);

int fileEncryptionAndDecryption(char *szSource,char *szDestination);

unsigned long count_directory_size(char *path,char * util);
#endif
