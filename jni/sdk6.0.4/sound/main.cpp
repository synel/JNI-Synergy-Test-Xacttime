#include <unistd.h>
#include <sys/stat.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../libfunc.h"

//#define _2410
#define _2416

int main(int argc, char *argv[])
{
	SOUNDINFO soundinfo;

	soundinfo.sound_kq=1;
	soundinfo.sound_kq=1;
	soundinfo.sound_type=1;
	soundinfo.sound_key=1;
	soundinfo.sound_menu=1;   //
	InitVolume(&soundinfo);
	SetVolume(50);
#if defined _2410
//	MenuKey("/etc/music/ft.mp3");
	MenuSound("/etc/music/ft.mp3");
//	Sound("/etc/music/welcome.mp3");
//	Start_BJ_Music("./music/*");
//	Stop_BJ_Music();
#elif defined _2416
	//	MenuKey("/weds/kq42/backup/music/ft.mp3");
		MenuSound("/weds/kq42/backup/music/ft.mp3");
	//	Sound("/weds/kq42/backup/music/ft.mp3");
	//	Start_BJ_Music("/weds/kq42/backup/music/ft.mp3");
	//	Stop_BJ_Music();
#endif
	return 0;
}
