#ifndef __SOUND_H
#define __SOUND_H
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <assert.h>
#include <fcntl.h>
#include <linux/soundcard.h>
#include <sys/ioctl.h>
#include "../version.h"

extern int bjmusic;
extern int splay_overtime_count;

typedef struct _sound_INFO
{
	int sound_kq;
	int sound_type;
	int sound_key;
	int sound_menu;   //
	int sound_ok;           //sound_ok 	0-Success.VOC£¬1-serial number£¬2-name£¬3-serial number + success.VOC£¬4-name + Success.VOC
}SOUNDINFO;

int InitVolume(SOUNDINFO *sound_info);
int SetVolume(int percent);
void LinkSound(char *command);
void Sound(char * command);
void MenuKey(char * command);
void MenuSound(char * command);
int Start_BJ_Music(char *str);
int Stop_BJ_Music();

//extern "C" {
void sound_ok_music(char *bh,char *ka);
//}

#endif
