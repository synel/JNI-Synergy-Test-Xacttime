#include <sys/wait.h>
#include "diskspace.h"
#include "../public/public.h"

	int SAFE1=90;                 //safety factor,according to the usage of space t, remaind user to clear disk
	int SAFE2=95;                 //when over this value ,terminal will not take photos
	int SAFE3=99;                 //when over this value,attendance is prohibited
	int OVERTIME_REBOOT=300;            //set the time how long terminal will reboot when fail to mount U-disk 
	char *mountadd;
	struct diskspace disklist[DISKCOUNT];
	struct systemstatus sysstatus; //add by aduo 2013.6.22
//add by aduo 2013.6.22
//<!--
void writets(char *command,char *command2,char *msg){

}
//-->

//when system start,first check settings,if need to mount disk,then check whether /dev/scsi/host0/bus0/target0/lun0/part4 is exist£¬if it is ,mount is¡£if not ,mount /dev/scsi/host0/bus0/target0/lun0/part1 instead
int mountdisk_frame()
{
	char *diskpath1="/dev/scsi/host0/bus0/target0/lun0/part1";//for U-disk
	char *diskpath2="/dev/scsi/host0/bus0/target0/lun0/part4";//for mp3-disk
#if defined _x86
	char *diskpath3 = "/dev/hda2";
	char *diskpath4="/dev/mmc/part1";                         //SD Card
#elif defined _2410
	char *diskpath3="/dev/mtdblock/2";                        //local FLASH
	char *diskpath4="/dev/mmc/part1";                         //SD Card
#else
	char *diskpath3="/dev/mtdblock3";                        //local FLASH
	char *diskpath4="/dev/mmcblk0p1";                         //SD Card
#endif
	char *diskpath5="/dev/scsi/host0/bus0/target0/lun0/disc";

	char *dirpath="./note/sd";      //system U-disk,used for store photos
	char *framepath="./note/sd/frame";
	char *photopath="./note/sd/photo";
	char *musicpath="./note/sd/music";
	char tmp[126];
	char command[128];

 //local disk ,mounted automatically
	strcpy(disklist[0].disktype,"local_flash");
	disklist[0].mountflag=1;         
	strcpy(disklist[0].diskpath,diskpath3);
	if((strcmp(disklist[1].diskflag,"1")==0)&&(disklist[1].mountflag!=1))//when system U-disk is needed,and it has not been mounted
	{
		safe_rm("note/sd/frame/*");
		safe_rm("note/sd/photo/*"); 
		safe_rm("note/sd/music/*");
		strcpy(disklist[1].disktype,"move_flash");
		memset(command,0,128);
		if(access(dirpath,F_OK)!=0)
		{
			mkdir(dirpath,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH);//creat "./note/sd" directory
		}
		if(access(diskpath2,F_OK)!=0) //first mount  it to part4 ,if failure, try to mount it to part1
		{
			if(mountdisk(diskpath1,dirpath)!=0) 
			{
				disklist[1].mountflag=-1;
				if((sysstatus.overtime>OVERTIME_REBOOT||sysstatus.overtime==0)&&OVERTIME_REBOOT>0)
				{
					sysstatus.overtime=OVERTIME_REBOOT;        //set overtime reboot
					sysstatus.usbsafe |=DISKFAIL;                      //attendance disable
				}
				return -1;        
			}
			else
			{
				strcpy(disklist[1].diskpath,diskpath1);
				disklist[1].mountflag=1;
			}
		}
		else 
		{
			if(mountdisk(diskpath2,dirpath)!=0) 
			{
				if(mountdisk(diskpath5,dirpath)!=0) 
				{
					disklist[1].mountflag=-1;
					if((sysstatus.overtime>OVERTIME_REBOOT||sysstatus.overtime==0)&&OVERTIME_REBOOT>0)
					{
						sysstatus.overtime=OVERTIME_REBOOT;       
						sysstatus.usbsafe |=DISKFAIL;              
					}
					return -1;   
				}
				else
				{
					strcpy(disklist[1].diskpath,diskpath5);
					disklist[1].mountflag=1;
				}                           
			}
			else
			{
				strcpy(disklist[1].diskpath,diskpath2);
				disklist[1].mountflag=1;
			}
		}
	}
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%s/a",framepath);
	if(access(framepath,F_OK)!=0)
		creatdir(tmp);
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%s/a",photopath);
	if(access(photopath,F_OK)!=0)
		creatdir(tmp);
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%s/a",musicpath);
	if(access(musicpath,F_OK)!=0)
		creatdir(tmp);
	//mount SD card
	if((strcmp(disklist[1].diskflag,"2")==0)&&(disklist[1].mountflag!=1))
	{
		safe_rm("note/sd/frame/*");
		safe_rm("note/sd/photo/*");
		safe_rm("note/sd/music/*");
		if(access(dirpath,F_OK)!=0)	mkdir(dirpath,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH);   
		if(mountdisk(diskpath4,dirpath)==0)
		{
			if(access(framepath,F_OK)!=0)
				mkdir(framepath,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH);
			if(access(photopath,F_OK)!=0)
				mkdir(photopath,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH);
			if(access(musicpath,F_OK)!=0)
				mkdir(musicpath,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH);
			strcpy(disklist[1].diskpath,diskpath4);
			disklist[1].mountflag=1;
		}
		else
		{
			disklist[1].mountflag=-1;
			if(((sysstatus.overtime>OVERTIME_REBOOT)||(sysstatus.overtime==0))&&(OVERTIME_REBOOT>0))
			{
				sysstatus.overtime=OVERTIME_REBOOT;        
				sysstatus.usbsafe |=DISKFAIL;                       
			}
			return -1;
		}
	}
	return 0;
}

int umountdisk_frame()
{
	if(disklist[1].mountflag==1)
		if(umountdisk("./note/sd")==0)
		{
			disklist[1].mountflag=0;
			return 0;
		}
		else return -1;
	return 0;
}

int umountdisk_wedsv3()
{
	if(disklist[2].mountflag==1)
		if(umountdisk(mountadd)==0)
			disklist[2].mountflag=0;
		else return -1;
	return 0;
}

int mountdisk(char *dev,char *dir)
{
	char tmpbuf[256];
	int i;

	if(access(dev,F_OK)!=0)		return -1;
	if(access(dir,F_OK)!=0)		return -1;
	memset(tmpbuf,0,256);
	sprintf(tmpbuf,"mount %s  %s -t vfat",dev,dir);
	i=system(tmpbuf);
	return i;
}

int umountdisk(char *dir)
{
	char command[128];
	int i;

	memset(command,0,128);
	sprintf(command,"umount -f %s",dir);
	i=system(command);
	return i;
}

//mount when use U-disk for transfer
int mountdiks_wedsv3()
{
	char *diskpath1="/dev/scsi/host0/bus0/target0/lun0/part1";     
	char *diskpath2="/dev/scsi/host0/bus0/target0/lun0/part4"; 
	char *diskpath3="/dev/scsi/host0/bus0/target0/lun0/disc";
#if defined _x86
	char *diskpath4="/dev/mmc/part1";                         //SD
#elif defined _2410
	char *diskpath4="/dev/mmc/part1";                         //SD
#else
	char *diskpath4="/dev/mmcblk0p1";                         //SD Card
#endif
//	char *dirpath="./w4400v3u";                            
//	char *dirpath1="./sd";                                
	char command[128];

	if(strcmp(mountadd,"w4400v3u")==0&&disklist[2].mountflag!=1)
	{
		memset(command,0,128);
		if(access(mountadd,F_OK)!=0)
			sprintf(command,"mkdir %s",mountadd);// create directory
		system(command);
		memset(command,0,128);
		sprintf(command,"mount %s %s -t vfat",diskpath2,mountadd);
		if(access(diskpath2,F_OK)!=-1)
		{
			if(system(command)!=0)       //first mount to part2,if fail,mount to part1
			{
				memset(command,0,128);
				sprintf(command,"mount %s %s -t vfat ",diskpath3,mountadd);
				if(system(command)!=0) 
				{
					disklist[2].mountflag=-1;
					return -1;
				}
				else 
				{
					strcpy(disklist[2].diskpath,diskpath3);
					disklist[2].mountflag=1;
				}                          
			}
			else
			{
				strcpy(disklist[2].diskpath,diskpath2);
				disklist[2].mountflag=1;
			}
		}
		else
		{
			memset(command,0,128);
			sprintf(command,"mount %s %s -t vfat ",diskpath1,mountadd);
			if(system(command)!=0) 
			{
				disklist[2].mountflag=-1;
				return -1;             
			}
			else
			{
				strcpy(disklist[2].diskpath,diskpath2);
				disklist[2].mountflag=1;
			}
		}
	}

	//mount SD
	if((strcmp(disklist[1].diskflag,"2")==0)&&strcmp(mountadd,"sd")==0&&disklist[2].mountflag!=1)	
	{
		if(access(mountadd,F_OK)!=0)	mkdir(mountadd,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH);   
		if(mountdisk(diskpath4,mountadd)==0)
		{
			strcpy(disklist[2].diskpath,diskpath4);
			disklist[2].mountflag=1;
		}
		else
		{
			disklist[2].mountflag=-1;
			return -1;
		}
	}
	return 0;
}

//get disk space  info
int disk()
{
	char tmp[128], *p1,*p2;     
	FILE   *stream; 
	int i=0,j=0;
	char *p3=" "; 
	char  buf[512],command[128]; 

	for(j=0;j<DISKCOUNT;j++)
	{
		memset(command,0,128);
		disklist[j].total=-1;          
		disklist[j].used=-1;                           
		disklist[j].available=-1;                     
		disklist[j].percentage=-1;      
		if(disklist[j].mountflag!=1)	continue;      
		sprintf(command,"df -k %s",disklist[j].diskpath);
		stream=popen(command,"r");  
		memset(buf,0,512);
		if(stream==NULL)	{return -1;printf("fail\n");}
		fgets(buf,512,stream);
		if(!strlen(buf))
		{
			printf("Ê§°Ü!\n");
			pclose(stream);
			continue; 
		}
		memset(buf,0,512);
		fgets(buf,512,stream);
		if(!strlen(buf))
		{
			pclose(stream);
			continue; 
		}
		p2=p1=buf;
		strcpy(tmp,strtok(buf,p3));
		if(strcmp(tmp,disklist[j].diskpath)!=0)
		{
			pclose(stream);
			continue; 
		}
		i=0;
		while((p2=strtok(NULL,p3)))
		{
			switch (i)
			{
				case 1:
					disklist[j].used=atoi(p2);
					break;
				case 2:
					disklist[j].available=atoi(p2);
					break;
				case 0:
					disklist[j].total=atoi(p2);
					break;
				case 3:
					disklist[j].percentage=atoi(p2);
					break;
				case 4:
					break;
			}
			i++;
		}
		pclose(stream);  
	}
	return 0;
}
//limits of disk usage
void frame_delete();
int disksafe(char *tmpbuf) 
{  
	int sysflag=0,i=0,j=0;
	char buf1[64];
	static int flag=0;

	disk();      
	if(tmpbuf!=NULL)
		if((disklist[0].percentage>=SAFE3)||(disklist[1].percentage>=SAFE3))
		{
			sysflag=-1;
			sysstatus.disksafe=2;
			strcpy(tmpbuf,kstr[180]);
			if(sysstatus.overtime<=0)
			{
				writets("ok6","ts3","");
				flag=1;
			}
			frame_delete();
		}
		else if((disklist[0].percentage>=SAFE2)||(disklist[1].percentage>=SAFE2))
		{
			sysflag=-1;
			sysstatus.disksafe=1;
			strcpy(tmpbuf,kstr[181]); 
			if(sysstatus.overtime<=0)
			{
				writets("ok6","ts2","");
				flag=1;
			}
			frame_delete();                 
		}
		else if((disklist[0].percentage>=SAFE1)||(disklist[1].percentage>=SAFE1))
		{
			sysflag=-1;
			sysstatus.disksafe=3;
			strcpy(tmpbuf,kstr[182]);
			if(sysstatus.overtime<=0)
			{
				writets("ok6","ts1","");
				flag=1;
			}
		}
		else 
		{
			sysstatus.disksafe=0;
			sysflag=0;
			if(flag==1)
			{
				writets("ok7",NULL,NULL);
				flag=0;
			}
		}
		i=0;
		j=0;
		if((disklist[0].total>0)&&(disklist[0].mountflag==1))
			i=disklist[0].used*100/disklist[0].total;
		if((disklist[1].total>0)&&(disklist[1].mountflag==1))
			j=disklist[1].used*100/disklist[1].total;
		if(i>j)
			sprintf(buf1,"%d",i);
		else sprintf(buf1,"%d",j);
		writets("ok1","num2",buf1);
		return sysflag;
}  

int frame_rm_flag=0;
pid_t frame_rm_pid;
struct sigaction oldaction; 

static void sig_usr(int signo)
{
int stat;
if(waitpid(frame_rm_pid,&stat,WNOHANG)==frame_rm_pid)
{
 frame_rm_flag=0;
 sigaction(SIGCHLD,&oldaction,NULL);
 }
//printf("signo=%d,%d\n",signo,frame_rm_flag);
}

void frame_delete()
{
FILE   *stream; 
char command[128],buff[32],path[128];
if(frame_rm_flag)return;
memset(command,0,sizeof(command));
sprintf(command,"ls ./note/frame/ |head -n 1");
stream=popen(command,"r");
if(stream==NULL)return;
memset(buff,0,sizeof(buff));
fgets(buff,sizeof(buff),stream);
pclose(stream);
if(strlen(buff)==0)return ;
cut(buff);
memset(path,0,sizeof(path));
sprintf(path,"./note/frame/%s",buff);
struct sigaction action; 
action.sa_handler=sig_usr;
sigemptyset(&action.sa_mask);
action.sa_flags = SA_NOCLDSTOP|SA_RESTART;
sigaction(SIGCHLD,&action,&oldaction);//set signal handler
if((frame_rm_pid=fork())==0)
 {
    safe_rm(path);
    perror(path);
    exit(0);
 }
 else if(frame_rm_pid<0) sigaction(SIGCHLD,&oldaction,NULL);
else frame_rm_flag=1;

}


  
