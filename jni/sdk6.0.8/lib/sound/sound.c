//#include <qsound.h>
#include <signal.h>
#include <ctype.h>
#include <setjmp.h>
#include "../_precomp.h"
#include "sound.h"
#include "../public/public.h"
#include "../camera/camera.h"
#include "../finger/finger.h"


#if defined _2416
#include <sys/wait.h>
#endif

/*
#include "../main.h"
#include "../form1.h"
#include "main_ht.h"
#include "../public/public.h"
#include "../note/init_direction.h"
#include "../camera/camera.h"
#include "../note/finger_opt.h"
*/
int splay_overtime_count=0;
sigjmp_buf menu_key_loop; //env-variable structure
sigjmp_buf menu_sound_loop; //env-variable structure


int bjmusic=0;
//extern int iTimerInterval;
//extern int itsTimer;
//extern int successflag;
SOUNDINFO soundinfo;

/*--------------------------------------------------------------------------*
@Function          :InitVolume - Initialize audio device
@Include      	:"sound.h"
@Description		:void
@Return value	: Failure FALSE; Success TRUE	
*---------------------------------------------------------------------------*/
int InitVolume(SOUNDINFO *sound_info)
{
	memset(&soundinfo,0,sizeof(soundinfo));
	soundinfo=*sound_info;
	return TRUE;
}

/*--------------------------------------------------------------------------*
@Function			: setvolume - Volume control
@Include 		     	: "sound.h"
@Description			: percent：volume （0-99）；
@Return Value		: Success TRUE, Failure FALSE.
*---------------------------------------------------------------------------*/
int SetVolume(int percent)
{
#if defined _x86
#elif defined _2410
	int fd = 0;
	int vol,maxyl=80;
	if ((percent<0)||(percent>100)) 
	{
		perror("percent error");
		return FALSE;
	}

	if ((fd = open("/dev/mixer", O_RDWR)) == -1) 
	{
		perror("open mixer error");
		return FALSE;
	}
	vol=percent*maxyl/100;
	vol |= vol << 8;
	ioctl(fd, MIXER_WRITE(0), &vol);
	close(fd);
#else
    char	buf[128];
    int		status = 0;
    int 	volume = 0;
	if ((percent<0)||(percent>100))
	{
		perror("percent error");
		return FALSE;
	}

    memset( buf, 0, sizeof( buf ) );
    switch( percent )
    {
    case 0:
        sprintf( buf, "amixer sset 'Headphone' off 0 >/dev/null" );
        break;
    default:
        volume = (percent / 5)+11;		// 0~20
        sprintf( buf, "amixer sset 'Headphone' on %d >/dev/null", volume );
        break;
    }

    status = system(buf);
#endif
	return TRUE;
}

//play audio file background
void Sound(char * command)
{
	char  playcommand[256];
	if(soundinfo.sound_kq !=1||command==NULL||strlen(playcommand)>=200)		return;
	Stop_BJ_Music();
	memset(playcommand,0,sizeof(playcommand));
/*	strcpy(playcommand,"splay /etc/music/");
	strcat(playcommand,command);
	strcat(playcommand,".mp3 &");
*/	
#if defined _x86
#elif defined _2410
	sprintf(playcommand,"splay %s &",command);
#else
    sprintf(playcommand,"madplay %s &",command);
#endif
	system(playcommand);
}

void overtime_menu_key(int signo) 
{
	printf("overtime_menu_key %d\n",signo);
	siglongjmp(menu_key_loop,1);//jump  位置
}

//menu prompt voice
void MenuKey(char *command)
{
	char  playcommand[256];
	struct sigaction action;

	cut(command);
	if(soundinfo.sound_key!=1||command==NULL||strlen(playcommand)>200) 	return ;
	Stop_BJ_Music();
	memset(playcommand,0,sizeof(playcommand));
/*	strcpy(playcommand,"splay /etc/music/");
	strcat(playcommand,command);
	strcat(playcommand,".mp3 ");
*/
#if defined _x86
#elif defined _2410
	sprintf(playcommand,"splay %s",command);
#else
	sprintf(playcommand,"madplay %s",command);
#endif
	action.sa_handler=overtime_menu_key;
	sigemptyset(&action.sa_mask);
	sigaction(SIGALRM,&action,NULL);
	alarm(15);
	if(sigsetjmp(menu_key_loop,1)==0)//记录位置
		system(playcommand);
	else {
		soundinfo.sound_key=0;
		soundinfo.sound_kq=0;
		soundinfo.sound_menu=0;	
		splay_overtime_count=1;
	}
	alarm(0);
}

void overtime_menu_sound(int signo) 
{
	printf("overtime_menu_sound %d\n",signo);
	siglongjmp(menu_sound_loop,1);
}

//operation prompt voice
void MenuSound(char *command)
{
	char  playcommand[256];
	struct sigaction action;

	if(soundinfo.sound_menu!=1||command==NULL||strlen(playcommand)>200)	return ;

	Stop_BJ_Music();
	memset(playcommand,0,sizeof(playcommand));
/*	strcpy(playcommand,"splay /etc/music/");
	strcat(playcommand,command);
	strcat(playcommand,".mp3 ");
*/
#if defined _x86
#elif defined _2410
	sprintf(playcommand,"splay %s",command);
#else
	sprintf(playcommand,"madplay %s",command);
#endif
	action.sa_handler=overtime_menu_sound;
	sigemptyset(&action.sa_mask);
	sigaction(SIGALRM,&action,NULL);
	alarm(15);
	if(sigsetjmp(menu_sound_loop,1)==0)
		system(playcommand);
	else {
		soundinfo.sound_key=0;
		soundinfo.sound_kq=0;
		soundinfo.sound_menu=0;	
		splay_overtime_count=1;
	}
	alarm(0);
}

void LinkSound(char *command)
{
	char  playcommand[256];
	
	cut(command);
	if(command==NULL||strlen(command)>200)	return;

	Stop_BJ_Music();
	memset(playcommand,0,sizeof(command));
#if defined _x86
#elif defined _2410
	sprintf(playcommand,"splay %s &",command);
#else
	sprintf( playcommand, "madplay 2>/dev/null /tmp/music/%s.mp3 &" ,command);
#endif
/*	strcpy(playcommand,"splay ./resfile/music/");
	strcat(playcommand,command);
	strcat(playcommand,".mp3 &");
*/
	system(playcommand);
}

void sound_ok_music(char *bh,char *ka)           //考勤成功后根据策略播放声音。
{
	char music[512],playcommand[512],bhcommand[512],namecommand[64],okcommand[64];
	char *p,tmpbuf[64];
	int i=0;   //编号不能超过7为，超过则截断。
#if defined _x86
#elif defined _2410
	char *path="/etc/music/";
#else
    char		*path  = "/tmp/music/";
#endif
	static char bhbuf[512],flag=0;

	if(soundinfo.sound_kq != 1)		return;
	memset(okcommand,0,64);
	memset(music,0,512);
	memset(namecommand,0,64);
	memset(bhcommand,0,512);
	memset(playcommand,0,512);
	Stop_BJ_Music();
//	sprintf(okcommand," %s%s.mp3",path,q_direction[curdirection].sound);   //ok

	if(bh&&(fp_enable==1&&camera_enable==0)&&(!flag))
	{
		memset(bhbuf,0,sizeof(bhbuf));
		if(bh)		strcpy(bhbuf,bh);
		flag=1;
		return ;
	}
	if(flag==1)
	{	
		flag=0;
		p=bhbuf;
	}
	else
		p=bh;

	if(p==NULL)		return;
	else
		while(*p)                      
		{
			memset(tmpbuf,0,64);
			if(!isdigit(*p))
			{
				p++;
				continue;
			}
			sprintf(tmpbuf," %sn%c.mp3",path,*p);
			strcat(bhcommand,tmpbuf);                       //报编号
			i++;   
			p++;
			if(i>7)	break;                   //编号最多报7个声音
		}
#if defined _x86
#elif defined _2410
	strcpy(playcommand,"splay ");
	if(ka)		strcat(playcommand," /etc/music/ka.mp3");  //播发卡声音
#else
	strcpy( playcommand, "madplay 2>/dev/null" );
    if( ka )
    {
        strcat( playcommand, " /tmp/music/ka.mp3" );    //播发卡声音
    }
#endif
//	if(!door_opend) 
//		strcat(playcommand," /etc/music/ec.mp3");
//	else 
		if(soundinfo.sound_ok==0)
			strcat(playcommand,okcommand);
		else if(soundinfo.sound_ok==1)
			strcat(playcommand,bhcommand);
		else if(soundinfo.sound_ok==2)
		{
			strcat(playcommand,bhcommand);
			strcat(playcommand,okcommand);  
		}   

	if(!(fp_enable==1&&camera_enable==0))
			strcat(playcommand," &");
	system(playcommand);
}
//play background music ./music/* ./music/q.mp3

int Start_BJ_Music(char *str)
{
	char command[128],*mplayer="/weds/kq42/mplayer";
	char *dir,*file;
	int count1=0,count2=0,count=0;	

	if(str==NULL||bjmusic>0)	return FALSE;

	dir=getbeginname(str);
	if(!dir_countfile_all(dir))	return FALSE;

	file=getendname(str);
	memset(command,0,sizeof(command));
	if(strcmp(file,"*")==0){
		count1=countfile_suffix(dir,".mp3");
		count2=countfile_suffix(dir,".MP3");
		if(count1>0)		count=count1;
		if(count2>0)		count+=count1;
	}
	if(count||strstr(file,".mp3")||strstr(file,".MP3"))
	{
		count=1;
#if defined _x86
#elif defined _2410
		sprintf(command,"splay %s -r >/dev/null &",str);
#else
		sprintf( command, "madplay %s -r >/dev/null &",str);
#endif

	}
	else
	{
		if((access(mplayer,F_OK)==0)&&(access(mplayer,X_OK)==0))
			sprintf(command,"%s -ac mad -framedrop -loop 0 -fs %s >/dev/null &",mplayer,str);
	}

//	printf("%s\n",command);
	if(system(command)==0)
	{
	//		form1->setFocus();
		if(count>0)		bjmusic=1;	//play mp3 file
		else bjmusic=2;			//play video file
		return TRUE;
	}

	return FALSE;
}
//stop playing background music
int Stop_BJ_Music()
{
	FILE   *stream; 
	char command[128],buff[512],*p;
	pid_t splaypid;
	int pidnum;

	if(bjmusic==0)	return FALSE;
	memset(command,0,sizeof(command));
	if(bjmusic==2)
		sprintf(command,"ps -e|grep mplayer");
	else {
#if defined _x86
#elif defined _2410
		sprintf(command,"ps -e |grep splay");
#else
		sprintf( command, "ps -e |grep madplay" );
#endif
	}
	stream=popen(command,"r");
	if(stream!=NULL)
	{
		memset(buff,0,sizeof(buff));
		while(fgets(buff,512,stream))
		{
			p=strtok(buff," ");
			if(p==NULL)		break ;
			if(strlen(p)==0)	break;
			if((pidnum=atoi(p))==0)	break;
			splaypid=(pid_t)pidnum;
			kill(splaypid,SIGKILL);
		}
		pclose(stream);
	}
	bjmusic=0;
	return TRUE;
/*
	if(bjmusic==2)
	{ 
		form1->hide();
		form1->show();
	}
	bjmusic=0;

	getdirection();
	if(curdirection>=0)
	{
		sprintf(buff,"%s %s %s",kstr[105],q_direction[curdirection].text,kstr[106]);
		writets("ok3",buff,NULL);
	}
	else 
	{
		sprintf(buff,"%s",kstr[163]);
		writets("ok3",buff,NULL);
	}
*/
}

/*
//qt播放声音
void sound(char *wavpath)
{
	char  buf[128];

	memset(buf,0,sizeof(buf));
	sprintf(buf,"/etc/music/%s.wav",wavpath);
	if(QSound::available()== true) {
		QSound::play(buf);
	}
	else
	{
		printf("error\n");
	}
}

void soundstop(char *wavpath)
{
	QSonud::stop();
}
*/
