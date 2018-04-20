#ifndef DISKSPACE_H
#define DISKSPACE_H
#define DISKCOUNT 3
//#define SAFE   95                       //attendance will be prohibited if the utilization rate over this setting value
#include <string.h> 
#include   <stdlib.h>  
#include   <stdio.h> 
#include "../sysconsts.h"

//add by aduo 2013.6.22
//<!--
#define DISKFAIL 2
struct systemstatus
{
int jinyong;                       //�Ƿ��ڽ���״̬.1-����״̬,0-����״̬
int lianji;                        //�Ƿ�������״̬ .1-������֤,0-��������֤
int disksafe;                      //���̿ռ䰲ȫ״̬.0-����,1-����������,2-��������,3-������ʾ.
int overtime;
int usbsafe;                     //1-�豸����ʧ�ܣ���Ҫ����
};
extern struct systemstatus sysstatus;
//-->

struct diskspace
{
char disktype[32];                  //disk type: local disk,mobile disk
char diskpath[128];                 //path of disk device (eg: if mount U-disk,it will be U-disk path)
int mountflag;                      //disk mounting flag,1-have mounted,0-have not mounted,-1-mounted failure
char  diskflag[5];                  //whether it is needed to mount disk 1-need to mount diskj,0-do not need(this flag is not used when mount disk for file transfer
int total;                          //total space (k)
int used;                           //space has been used
int available;                      //space has not used
int percentage;                     //space utility
};

extern char *mountadd;		//file transfer, u disk ,sd card
int disksafe(char *tmpbuf);
int disk();
int mountdisk_frame();
int mountdiks_wedsv3();
//disklist[0]-info of local disk
//disklist[1]-save disk's info who was mounted to ./note/frame,disklist[2]-save disk's info which was mounted to ./wedsv3
extern struct diskspace disklist[DISKCOUNT];
int umountdisk_frame();
int umountdisk_wedsv3();
extern int OVERTIME_REBOOT;
extern int SAFE1;
extern int SAFE2;
extern int SAFE3;
int mountdisk(char *dev,char *dir);
int umountdisk(char *dir);
extern pid_t frame_rm_pid;
#endif
