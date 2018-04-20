#ifndef PUBLIC__H
#define PUBLIC__H
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/time.h>
#include "../version.h"

extern char tmpfilepath[128];
extern int user_data_len;

int mv_file(char *,char *);
int cp_file(char *,char *);
int appen_file(char *from ,char *to);

int testdir(char *path);

extern FILE* creatdirfile(char *path);
extern int safe_cp(char * oldpath,char *newpath);
extern int safe_rm(char * oldpath);
extern int SortSearch(char *filename,int rlength,int kpos,int klength,char *key);
extern int BinarySearch(char *filename,int rlength,int kpos,int klength,char *key);

int gsmString2Bytes(const char* pSrc, unsigned char* pDst, int nSrcLength);
int gsmBytes2String(unsigned char* pSrc, char* pDst, int nSrcLength);
void read_menu2();
int safe_cp_photo(char * oldpath,char *newpath);
//int safe_cp_finger(char * oldpath,char *newpath);
extern char kstr[500][45];
char * getfile(char *path);
int creatdir(char *path);
void cut(char *tmp);
int dir_count_file(char *path);
int dir_countfile_all(char *photopath);

int countfile_suffix(char *path,char *suffix);

void printftime(char *fun);
int digittest(const char *ptr);
int check_ip(char *ptr);
extern int clock_tz;
void syncclock();
void write_mac_config(char *mac);
int read_terminalno();
int readbh(char *param);
void readini(FILE *file,char *group,char *member,char *values);
char *read_mac_config();
void read_menu2();
void getcurtime(char *buff) ;
char * getendname(const char *path);
char * getbeginname(const char *path);
int setdaylingth(time_t curdate);
int field_handle(char *buf,char str[12][128]);
void write_mac_config(char *mac);
int rightsub(char *dst,const char *src,int postion,int len,char mask);
int leftsub( char *dst,const char *src,int postion,int len,char mask);
int FormatDateTime(const char * format,char *result);
#endif
