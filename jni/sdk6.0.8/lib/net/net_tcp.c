#include "net_tcp.h" 
#include "../public/public.h"
#include "../setdate/set_date.h"
#include "../crypt/crypt.h"
#include "../finger/finger.h"
#include "../gpio/gpio.h"
#include "../gprs/gprs.h"
#include "../sound/sound.h"
#include "../finger/finger.h"
#include "../camera/camera.h"
#include "../public/initcom.h"
#include "../com/com.h"

/*
#include "../main_ht.h"
#include "../note/wdda.h"		//personnel file
#include "../note/diskopt.h"
#include "../note/finger_opt.h"
#include "../note/wdmj.h"
#include "../note/power.h"
#include "../note/mj.h"
#include "../note/init_direction.h"
*/

int sendflag=0;
int netfd=0;
int listenfd=-1; 
int downwddaflag=0;
Sendfileproperty sendfileproperty;
int tcp_socket=-1,udp_socket=-1 /*, client_tcp */;
static unsigned short int local_udp_port=3352, remote_udp_port=3351;
time_t recv_time,send_time;
static  time_t starttime;
int sendlen=0;  
char *tmpfilename=NULL,ip_addr[18];
struct sockaddr_in saddr;
struct package fileflag;            //file attribute
NETINFO netinfo;
CALBACK *calback;

/*--------------------------------------------------------------------------*
@Function			: InitNet - Initialize network
@Include      		: "net_tcp.h"
@Description			: localport：port number
				overtime : time out
				localport:Local monitoring port
				nettype:Linking type 0-Passive reception，3-gprs-Link，2-Active receiving
				serverport:server port
				serverio:server ip
@Return Value		: 
				Success 	tcp file descriptor
				Failure	 	-1
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int InitNet(NETINFO *net_info,CALBACK *cal_back)
{
	netinfo=*net_info;
	if(init_net()==-1)	return FALSE;	
	calback=cal_back;
	return TRUE;
}

/*--------------------------------------------------------------------------*
@Function			: init_net- Initialize network
@Include      		: "net_tcp.h"
@Description			: None			
@Return Value		: 
				Success 	tcp file descriptor
				Failure	 	-1
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int init_net()
{
	int fl;
	struct sockaddr_in  udpaddr;
	unsigned short int tcp_port=3350; 

	if(listenfd>=0)	return 0;   		//if have established,return immediately

	tcp_port=atoi(netinfo.netport);
	local_udp_port=atoi(netinfo.netport)+2;
	remote_udp_port=atoi(netinfo.netport)+1;

 	listenfd=tcp_make_socket(tcp_port);	//bind 3350 port,return tcp file descriptor
	if(listenfd ==-1)	return listenfd;
	if(netinfo.linetype==0){
		if(listen(listenfd,LISTENQ)<0)	 	//listen port 
		{
			perror("connect");   
			return -1;
		}
	}
	//set as non-block
	if((fl = fcntl(listenfd, F_GETFL)) == -1)   //get current setting
	{
	    perror("fcntl get");
	    if(close(listenfd) == -1)	perror("close");
	    return -1;
	}
	if(fcntl(listenfd, F_SETFL, fl | O_NONBLOCK) == -1)//set socket to 'fl | O_NONBLOCK' mode
  	{
         perror("fcntl set");
         if(close(listenfd) == -1 )	perror("close");
        return listenfd;
        }

	//UDP init
	if(udp_socket>0)	return listenfd;
	if((udp_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("Can't create udp socket.\n");
		if(close(listenfd) == -1)		perror("close");
		return 0;
	}
	memset(&udpaddr, 0, sizeof(udpaddr));
	udpaddr.sin_family = PF_INET;
	udpaddr.sin_addr.s_addr = INADDR_ANY;
	udpaddr.sin_port = htons((uint16_t)local_udp_port);
	if(bind(udp_socket, (struct sockaddr *)&udpaddr,(socklen_t) sizeof(udpaddr)) == -1) 
	{
		if(close(listenfd) == -1 || close(udp_socket) == -1)
			perror("close");
		return 0;
	}

  return listenfd;
}

/*--------------------------------------------------------------------------*
@Function			:UninitNet - Disconnect network connection
@Include      		: "net_tcp.h"
@Description			:Release the resource occupied
@Return Value		:
				Success 	tcp fd 
				Failure 	-1
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
void UninitNet()
{
	if(listenfd>0)
	{
	   	close(listenfd);
	   	listenfd=-1;
	}

	if(udp_socket>0)  
	{
		close(udp_socket);
		udp_socket=-1;
	}
	return ;
}
//SIGPIPE signal with default action of killing process will generate, when in the process of uploading a compressed file a network interrupt come out.
//so set a handle to intercept this signal here.(system will be called when compress a file,accompanied by a new porocess,
//and signals will be shielded,for more details ,refer to 'system' manual.
//you can ignore this handle if compression-transfer is not used.
void pipe_handel(int signo)
{
	//This handle do nothing but intercept SIGPIPE signal
      printf("signal %d\n",signo);
}

/*--------------------------------------------------------------------------*
@Function			:Net - file transfer
@Include      		: "net_tcp.h"
@Description			: None
@Return Value		:
				Success 	1 
				Failure 	0
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int Net()
{
	int flag=0,endflag=0;

	user_data_len=NET_DATALEN;
	flag=net_net();
	if(flag==-1)
	{
		if(sendfileproperty.filesendflag!=0)
		{  
			if(sendfileproperty.sendfile!=NULL)
			{
				fclose(sendfileproperty.sendfile);
				sendfileproperty.sendfile=NULL;
			}  
		 }
		netinfo.netenable=0;		// if there are network connections
		memset(&sendfileproperty,0,sizeof(sendfileproperty));
		remove(tmpfilepath);
		sendflag=0;
		if(calback->endback)
			calback->endback(downwddaflag);
		downwddaflag=0;
		if(calback->write_ts)
		calback->write_ts("ok1","start",NULL);
		return endflag;
/*		menu_key("ft");
		if(downwddaflag==1)   //if download archives file,re-load this file and fingers
		{
			UninitFp();	
			if(InitFp("./lib/fp.so",1) == FALSE) // initialize fingerprint device
			{
				fpdevinfo.fpflag=0;
			}
			else
				fpdevinfo.fpflag=1;
			freerecord(headrecord);
			if((headrecord=init_wdda())==NULL)//read configeration of archives file
			{
				printf("init_wdda err!\n");
			}
			downwddaflag=0;
		}
		writets("ok1","start",NULL);
		powerwait();
		powermin();
		min(); 
*/
		return -1;
	}


    return 0;
}

static time_t netfd_lasttime=0;
/*--------------------------------------------------------------------------*
@Function					:	AcceptLinezd - waiting for connect
@Include      		: "net_tcp.h"
@Description			:	listen port，wait for network connect
@Return Value			: Success 0 .Failure -1
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int AcceptLinezd()
{
	struct sockaddr_in cliaddr;
	static struct package data; 
	int lizezdsucceed=0; 
	socklen_t len;
	int status=0,num=0;
	static int recvtotal;
	unsigned char *ptr=NULL;
      struct sockaddr_in addr;
      char buf[32];


	if(netinfo.netenable==1)	return 2;	//just in communicating

	len=sizeof(cliaddr);
	//modify by aduo 2013.7.16
	//<!--
	//printf("netinfo.linetype = %d\r\n",netinfo.linetype);
	if(netinfo.linetype==3)		//gprs continue
	{
 		if(GprsAccept((unsigned char *)&data,sizeof(data))>0&&
			strncmp(data.command,"linezd",6)==0)
	 	{
 			new_linezd(&data,-1,&lizezdsucceed);
 		}
 		if(lizezdsucceed==1)	return 1;
	 		else return -1;
	 }
	//-->

  if(listenfd>0||netinfo.linetype==2)
   {	
	if(netfd<=0)
	{
		if(netinfo.linetype==0)
		{
			if(listenfd<=0)	return -1;
			netfd=accept(listenfd,(struct sockaddr *)&cliaddr,&len);//waiting for connection
			memset(ip_addr,0,sizeof(ip_addr));
              	strcpy(ip_addr,inet_ntoa(cliaddr.sin_addr));
		}
		else
		{
            		if(listenfd<=0)	return -1;
              	addr.sin_family = AF_INET;
              	addr.sin_addr.s_addr = inet_addr(netinfo.dailiip);
              	addr.sin_port = htons(netinfo.dailisocket);
              	netfd=connect(listenfd,(struct sockaddr*)&addr, sizeof(addr));
              	if(netfd<0)		return -1;
              	netfd=listenfd;
			netinfo.netenable=1;
              	memset(buf,0,sizeof(buf));
              	sprintf(buf,"mac=%s",read_mac_config());
              	send(netfd, buf, strlen(buf), MSG_CONFIRM);
              	reset_recv_time();
              	}
	}//if(netfd<=0)

	if(netfd>0)
	{
		if(netfd_lasttime==0)	
			netfd_lasttime=time(NULL);
		else if(netinfo.linetype==0&&abs((int)difftime(time(NULL),netfd_lasttime))>atoi(netinfo.netovertime))
		{
			close(netfd);
			netfd_lasttime=0;
			netfd=-1;
			recvtotal=0;
			memset(&data,0,sizeof(data));
			return -1;
		}

		if(netinfo.linetype==2)
		{
			fd_set send_fd;
			struct timeval net_timer;
			FD_ZERO(&send_fd);
			FD_SET(netfd, &send_fd);
			net_timer.tv_sec=0;
			net_timer.tv_usec=0;
  			if((status=select(netfd + 1,&send_fd,NULL,NULL, &net_timer))>0)
  			{
  				ioctl(netfd, FIONREAD, &status);
  				if(status==0)
  				{
  					close(netfd);
  					netfd=-1;
  					listenfd=-1;
  					recvtotal=0;
                            	memset(&data,0,sizeof(data));
                            	init_net();
                            	return -1;
                            	}
  			}
		}//if(netinfo.linetype==2)
		errno=0;
		time(&starttime);		//record start time
		ioctl(netfd, FIONREAD, &status);
		if(status>=0)
		{
			
			ptr=(unsigned char*)&data;
			ptr+=recvtotal;
			if(status>=((int)sizeof(data)-recvtotal))
				num=(int)sizeof(data)-recvtotal;
			else num=status;
			if((status=read(netfd,ptr,num))!=num)
			{
				close(netfd);
                            netfd_lasttime=0;
                            netfd=-1;
                            recvtotal=0;
                            memset(&data,0,sizeof(data));
                            if(netinfo.linetype==2) listenfd=-1;
				return -1;
			}
			recvtotal+=status;
			if(recvtotal>=6)
			{
printf("%s\n",data.command);
				if((strncmp(data.command,"linezd",6)!=0))
				{
					close(netfd);
                                   netfd_lasttime=0;
                                   netfd=-1;
                                   recvtotal=0;
                                   memset(&data,0,sizeof(data));
                                   if(netinfo.linetype==2) 
					    {
                                       	listenfd=-1;
                                       	init_net();
	                                    }
					return -1;
				}
				if(recvtotal>=(int)sizeof(data))
				{
					new_linezd(&data,netfd,&lizezdsucceed);
					if(lizezdsucceed==1)
					{
						netfd_lasttime=0;
						recvtotal=0;
						netinfo.netenable=1;
						return 1;
					}
					else 
					{		
						close(netfd);
						netfd=-1;
						if(netinfo.linetype==2)
						{
							 listenfd=-1;
							 init_net();
						 }
						netfd_lasttime=0;
						recvtotal=0;
						memset(&data,0,sizeof(data));
					}
				}//if(recvtotal>=(int)sizeof(data))
			}//if(recvtotal>=6)
		}//if(status>=0)
		return -1;
	}//	if(netfd>0)
	return -1;
    }//  if(listenfd>0||netinfo.linetype==2)
    else return -1;

return -1; 
}


/*--------------------------------------------------------------------------*
@Function			:net_net - file transfer
@Include      		:"net_tcp.h"
@Description			:Executes corresponding operation according to the instructions received 
@Return Value		: Success 0 .Failure -1
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int net_net()
{                    
	struct package data1; 
	struct sigaction action;     
	int recvflag=0,overtime; 
	struct timeval tv1,tv2;

	action.sa_handler=pipe_handel;
	sigemptyset(&action.sa_mask);
	sigaction(SIGPIPE,&action,NULL);//set signal handler*/                
	memset(&data1,0,sizeof(struct package));

	gettimeofday(&tv1,NULL);
	overtime=(tv1.tv_sec % 10000) *1000+tv1.tv_usec/1000;
	while((recvflag=safe_recv(netfd,&data1))>0 ||sendflag)//receive data
	{
		recvflag=Functions_Handle(data1);
		if(recvflag==-2)	break;
		memset(&data1,0,1024);
		gettimeofday(&tv2,NULL);
		if (abs( (tv2.tv_sec % 10000) *1000+tv2.tv_usec/1000 - overtime) > 100)
			break;
	 }
//printf("recvflag=%d\n",recvflag);
	if((netinfo.linetype==0&&recvflag==-2)||recvflag==-1)
	{
		close(netfd);
		netfd_lasttime=0;
		netfd=-1;
		sendlen=0;
		sendflag=0;
		netinfo.netenable=0;
		if(netinfo.linetype==2)	listenfd=-1;
		return -1;
	}

return 0;
}
//different processing methods are adopted according to the different command in packet
int Functions_Handle(struct package data1)
{
	char command[32];
	int lizezdsucceed=1; 
	int recvflag=0; 

	reset_recv_time();//
	memset(command,0,sizeof(command));
	sscanf(data1.command,"%s ",command);
	if(calback->write_ts)
	calback->write_ts("ok1","stop",command);
//	writets("ok1","stop",command);
	printf("\ncommand=%s\n",command);
	if(strcmp(command,"dwname")==0){		//download filename
		new_dwname(&data1,netfd);
	}
	else if(strcmp(command,"dwload")==0) {		//downlaod filename
		new_dwload(netfd,&data1); 
	}
	else if(strcmp(command,"upname")==0){		
		if(new_upname(&data1,netfd))
		{
			sendlen=0;
			sendflag=1;
			new_upload(netfd);
		}
	}
	else if(strcmp(command,"delete")==0){		//delete file
		new_deletefile(&data1,netfd);
	}
	else if(strcmp(command,"retime")==0){		//calibrate
		//printf("sync clock\r\n");
		new_retime(&data1,netfd);
	}
	else if(strcmp(command,"player")==0){		//play audio remotely 
		new_player(&data1,netfd);
	}
	else if(strcmp(command,"infofp")==0) 		//check fingerprint data
		new_checkfp(&data1,netfd);
	else if(strcmp(command,"gpiozd")==0) 		//gpio delay operation
		new_gpio(&data1,netfd);
	else if(strcmp(command,"linezd")==0){		//connect terminal and get it's status
		new_linezd(&data1,netfd,&lizezdsucceed);
	}
	else if(strcmp(command,"linedk")==0){		//disconnect
		recvflag=-2;
	}
	else if(strcmp(command,"infozd")==0) 
		printf("%s,%s\n",command,data1.data);
	else if(strcmp(command,"cortzd")==0) 
		printf("control command\r\n");//			new_cortzd(&data1);
	else if(sendflag==1) 
		new_upload(netfd);
	else  
		printf("not found %s,%s\n",command,data1.data);

	return recvflag;
}


/*--------------------------------------------------------------------------*
@Function			:new_dwname - download file name
@Include      		: "net_tcp.h"
@Description			:
				tmpdata : packet received
				netfd : handle of net-connect
				tmpdata：packet received containing the handing command of the filename download
				erminal will create a file according to the filename download and inform it's status
@Return Value		:void
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
void new_dwname(struct package *tmpdata,int netfd)
{
	FILE* file=NULL;
	char buf[user_data_len],*ptr;
	long filesize;
	int err=0;

	strcpy(buf,tmpdata->data);
	cut(buf);
	memset(tmpdata->data,0,user_data_len);
	decrypt(buf,tmpdata->data,strlen(buf),user_data_len);
	if(sendfileproperty.filesendflag!=0)   //If the file is being downloaded, close it and delete section already downloaded
 	{
		if((sendfileproperty.sendfile)!=NULL)
		{
			fclose(sendfileproperty.sendfile);
  			sendfileproperty.sendfile=NULL;
  			sendfileproperty.filesendflag=0;
		}
		remove(tmpfilepath);
 	 }
 
	memset(buf,0,sizeof(buf));
	strcpy(buf,tmpdata->data);
	ptr=strtok(buf,"\n");
	if(ptr)	ptr=strtok(NULL,"\n");
	filesize=atol(ptr);
	if(new_pathtopath(tmpdata->data)!=0)    //The path name format convert (windows->linux)
		downpathtopath(tmpdata->data);		  //convert
	creatdir(tmpdata->data);                //if the path not exit ,create it
	file=fopen(tmpfilepath,"w");            //create temporary file
	if(filesize>40000000)	err=1;		 //byte
	if(file==NULL||err)      //if fail to open file,send error info to server
	{
		 memset(&fileflag,0,sizeof(fileflag));
      		 if(file!=NULL)
        	 {
      		      fclose(file);
            		file=NULL;
          	 }
	       memset(&sendfileproperty,0,sizeof(sendfileproperty));
		 sprintf(fileflag.command,"dwname");
		 encrypt("err",fileflag.data,3,user_data_len);
		 fileflag.datalen=strlen(fileflag.data);
		 safe_send(netfd,&fileflag);
		 return ;
	}
	memset(&sendfileproperty,0,sizeof(sendfileproperty));
	bcopy(tmpdata->data,&sendfileproperty.filename,sizeof(sendfileproperty.filename));//filename
	sendfileproperty.filesendflag=1;                                  //flags of up/download
	sendfileproperty.sendfile=file;                  //file descripter opened
	sscanf(tmpdata->command,"dwname as #%d",&sendfileproperty.fileno);  //get file No.
	if(filesize==0)		//when the size of download file is 0
	{
		if(file!=NULL)
		{
			fclose(file);
			sendfileproperty.sendfile=NULL;
			file=NULL;
		}
		err=mv_file(tmpfilepath,sendfileproperty.filename);
		memset(tmpdata->data,0,user_data_len);
		memset(buf,0,sizeof(buf));
		if(err==0)
			encrypt("0/0",tmpdata->data,strlen("0/0"),user_data_len);
		else 
			encrypt("err",tmpdata->data,strlen("err"),user_data_len);
		tmpdata->datalen=strlen(tmpdata->data);
		memset(tmpdata->command,0,30);
		sprintf(tmpdata->command,"dwload #%d",sendfileproperty.fileno);
		safe_send(netfd,tmpdata);
	}
}
/*--------------------------------------------------------------------------*
@Function			: new_dwname - download data
@Include      		: "net_tcp.h"
@Description			:
				tmpbuff : packet received 
				netfd : handle of net connection
				tmpbuff : packet received containing the handing command of the filename download
@Return Value		: Success 0 .Failure -1
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int new_dwload(int netfd,struct package *tmpbuff)
{
	char command[32],tmp2[32];//command1[32],
	int curcount,writecont,recvlen=0;
	short int senderr=0;
	static short int synccount=0;

	//file is not open or transfer-flag is 0
	if (sendfileproperty.sendfile==NULL||sendfileproperty.filesendflag==0)
	{ 	      	
		return -1;
	}
	synccount++;
	memset(command,0,sizeof(command));
	sscanf(tmpbuff->command,"%s %s %d/%d",
			command,tmp2,&curcount,&sendfileproperty.totalcount);//intercept data 
	recvlen=tmpbuff->datalen;   //length of packet received
	if(strcmp(command,"dwload")!=0||tmpbuff->datalen>user_data_len)//if command error
		senderr=1;
	else 
		sendfileproperty.curcount++;
	if(sendfileproperty.curcount==1) //if it is the first packet you received ,allocate space for it
	{
		if(ftruncate(fileno(sendfileproperty.sendfile),
						sendfileproperty.totalcount*user_data_len) == -1)
			senderr=1; 
		if(sendfileproperty.totalcount > 40000)
			senderr=1; 
	 }
	if(senderr)                                 //content of header error
	{
		fclose(sendfileproperty.sendfile);
	  	sendfileproperty.sendfile=NULL;
	  	remove(tmpfilepath);
	  	sendfileproperty.totalcount=0;       //number of packet received
  		sendfileproperty.filesendflag=0;	
	  	return -1;
	}
	/*put the data in the reasonable position.appendding data to file's tail is not used
	as packet will send later maybe first received
	*/
	if(fseek(sendfileproperty.sendfile,(long)((curcount-1)*user_data_len),SEEK_SET)==-1)
	 	senderr=1;
	  //write data to file
	writecont=fwrite(tmpbuff->data,sizeof(char),
						tmpbuff->datalen,sendfileproperty.sendfile);
	if(writecont != tmpbuff->datalen)	senderr=1;
	if(senderr)
	{
		memset(tmpbuff,0,sizeof(*tmpbuff));            
		encrypt("err",tmpbuff->data,3,user_data_len);
		tmpbuff->datalen=strlen(tmpbuff->data);
		sprintf(tmpbuff->command,"dwload %s",tmp2);
		safe_send(netfd,tmpbuff);   
	}
	  //when number of packet received equal to that you should receive,one transmission completed.
	if(curcount==sendfileproperty.totalcount)                        
	{
		fflush(sendfileproperty.sendfile);
		sendfileproperty.filesendflag=0;
		if(sendfileproperty.curcount!=curcount) //size of the last packet maybe less 'than user_data_len',intercept space according to actual size.
		{
			fclose(sendfileproperty.sendfile);         //close fd
			remove(tmpfilepath);                       //delete temporary files
		       //Fill message will be send to server
			memset(tmpbuff,0,sizeof(*tmpbuff));           
			sprintf(tmpbuff->data,"%d/%d",
							(sendfileproperty.curcount-1)*
							user_data_len+recvlen,					
							(sendfileproperty.totalcount-1)*
							user_data_len+recvlen);
		}
		else
		{
			if(ftruncate(fileno(sendfileproperty.sendfile),
					(sendfileproperty.totalcount-1)*user_data_len+recvlen) == 0)
			{
				fclose(sendfileproperty.sendfile);
				sendfileproperty.sendfile=NULL;
				senderr=mv_file(tmpfilepath,sendfileproperty.filename);//
				if(synccount>100)
				{
					synccount=0;
					sync();
				}
				if(senderr==0)    
				{
					//Fill message to send to the server.
					memset(tmpbuff,0,sizeof(*tmpbuff)); 
					sprintf(tmpbuff->data,"%d/%d",
						(sendfileproperty.curcount-1)*user_data_len+recvlen,
						(sendfileproperty.totalcount-1)*user_data_len+recvlen);
				}
	      		   	else 
	      		   		sprintf(tmpbuff->data,"0/%d",
						(sendfileproperty.totalcount-1)*user_data_len+recvlen);
			}
		      	 else 
		      		sprintf(tmpbuff->data,"0/%d",
						(sendfileproperty.totalcount-1)*user_data_len+recvlen);
	     }//if(sendfileproperty.curcount!=curcount)
	    memset(command,0,sizeof(command));
	    strcpy(command,tmpbuff->data);
	    memset(tmpbuff->data,0,user_data_len);
	    encrypt(command,tmpbuff->data,strlen(command),user_data_len);
	    tmpbuff->datalen=strlen(tmpbuff->data); 
	    sprintf(tmpbuff->command,"dwload %s",tmp2);
	    safe_send(netfd,tmpbuff);                   //send success/failure message to server
	    if(strstr(sendfileproperty.filename,"sys_command.sh"))
	     {
	        sys_command();
  	    }
   }//	if(curcount==sendfileproperty.totalcount)                        

  if(sendfileproperty.totalcount!=0)
  {
	memset(command,0,32);
	sprintf(command,"%d\n",sendfileproperty.curcount*100/sendfileproperty.totalcount);
	if(calback->write_ts)
	calback->write_ts("ok1","num1",command);
//	writets("ok1","num1",command);
	if(calback->netback)
		calback->netback(sendfileproperty.filename);
//	net_back();
  }
  return 0;
}

void net_back()
{
/*	char *ptr,buf[126];
	int ret=0;

	ptr=strstr(usbdisk_path,"/");
	ptr+=1;
	if(strstr(sendfileproperty.filename,ptr))
	{
		if(chmod(sendfileproperty.filename,S_IRUSR|S_IWUSR|S_IXUSR)==0)
		{
			ret=rename(sendfileproperty.filename,"kq43arm");
			if(ret==0){
				sync();
				exit(0);
			}
		}	
	}
	if(strstr(sendfileproperty.filename,"wdda.wts")||strstr(sendfileproperty.filename,fingerfilesuffix))
	{
		downwddaflag=1;
		remove("note/archives/update_wdda.wts");
	}

	if(strstr(sendfileproperty.filename,"wdmj.wts")||strstr(sendfileproperty.filename,"wdgz.wts"))
	{
		freemjgz(); 
		init_wdmj(); 
	}
	if(strstr(sendfileproperty.filename,"update.wts"))
	{
		updatefile();
		update_wdda(&headrecord);
	}
	if(strstr(sendfileproperty.filename,"changeip.wts"))
	{
		setip();
	}
	if(strstr(sendfileproperty.filename,"sys_command.sh"))
	{
		sys_command();
	}
	if(strstr(sendfileproperty.filename,"menu_change.ini"))
	{
		menu_chang();
	}
	if(strstr(sendfileproperty.filename,"direction.ini"))
	{
//		appenddirection(sendfileproperty.filename);
		init_direction();
		sprintf(buf,"cp %s backup/%d/wes/direction.ini",sendfileproperty.filename,atoi(deviceinfo.language));
		system(buf);
	}	
*/
}

/*--------------------------------------------------------------------------*
@Function			: new_dwname - upload
@Include      		: "net_tcp.h"
@Description			:
				tmpbuff : packet received
				netfd : handle of net connection
				tmpbuff : the command in this packet is "upload data to PC",
				terminal get it's status and pack it to PC
@Return Value		: Success  file stream
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
FILE* new_upname(struct package *tmpbuff,int netfd) 
{
	FILE * file;
	char year[5],mon[3],day[3],date[9],buf[user_data_len];
	struct stat filestat;

	memset(year,0,5);
	memset(mon,0,3);
	memset(day,0,3);
	memset(date,0,9);
	memset(buf,0,user_data_len);
	strcpy(buf,tmpbuff->data);
	cut(buf);
	memset(tmpbuff->data,0,user_data_len);
	decrypt(buf,tmpbuff->data,strlen(buf),user_data_len);
	if(sendfileproperty.filesendflag==1)       //If the file is being downloaded, close it and delete section already downloaded
	{
		if((sendfileproperty.sendfile)!=NULL)
		{
			fclose(sendfileproperty.sendfile);
			sendfileproperty.sendfile=NULL;
			sendfileproperty.filesendflag=0;        
		}
		remove(tmpfilepath);
	}

	if(new_pathtopath(tmpbuff->data)!=0)   //path name format convert (windows->linux)
	{
		uppathtopath(tmpbuff->data);
		if(strstr(tmpbuff->data,"wdjl.wds"))
			change(tmpbuff->data);
	}      

	if(testdir(tmpbuff->data))
	{
		file=creatdirfile(tmpbuff->data);
	}
	else
		file=fopen(tmpbuff->data,"r");
	memset(&sendfileproperty,0,sizeof(sendfileproperty));
	bcopy(tmpbuff->data,&sendfileproperty.filename,sizeof(sendfileproperty.filename));//filename
	sendfileproperty.filesendflag=2;                  //up/download flag
	sendfileproperty.sendfile=file;                  //file descripter
	sscanf(tmpbuff->command,"upname as #%d",&sendfileproperty.fileno);  //read file NO.
	if(sendfileproperty.sendfile!=NULL)
		 fstat(fileno(sendfileproperty.sendfile),&filestat);     //get file status
	if(filestat.st_size==0||sendfileproperty.sendfile==NULL)       //if the file is empty
	{
		memset(tmpbuff,0,sizeof(*tmpbuff)); 
		if(sendfileproperty.sendfile==NULL)
			sprintf(tmpbuff->command,"upload #%d 0/1",sendfileproperty.fileno);
		else 
		{
			 sprintf(tmpbuff->command,"upload #%d 1/1",sendfileproperty.fileno);
			 fclose(sendfileproperty.sendfile);
			 sendfileproperty.sendfile=NULL;
		 }
		tmpbuff->datalen=0;
		safe_send(netfd,tmpbuff);
		memset(&sendfileproperty,0,sizeof(sendfileproperty));
	}
	else 
		if(filestat.st_size%user_data_len==0)
			sendfileproperty.totalcount=filestat.st_size/user_data_len; //the total number of packets that should be received
	      else 	sendfileproperty.totalcount=filestat.st_size/user_data_len+1; 
	 return sendfileproperty.sendfile;
}


/*--------------------------------------------------------------------------*
@Function			:	new_upload - upload data
@Include      		: "net_tcp.h"
@Description			:netfd :	handle
				the proccessing command is 'upload file data'
@Return Value		: Success file stream will be upload
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int new_upload(int netfd)
{
	char command[32];//,command1[32];
	static struct package data;

	//no command will be read until file transfer end
	//open the file error or it is not exit,an error message "0/1" will be writen
	if(sendfileproperty.sendfile==NULL||sendfileproperty.filesendflag!=2)            
	{ 
		memset(&data,0,sizeof(data));
		//0/1 inform the server that the former step operating errors
		sprintf(data.command,"upload #%d 0/1",sendfileproperty.fileno);    
		data.datalen=0;
		safe_send(netfd,&data);   
		return -1;
	}
	//read user_data_len bytes to fill data structure
	if(sendlen!=0 && sendlen!=Maxline)
	{
		if(safe_send(netfd,&data)<=0)
		{
			fclose(sendfileproperty.sendfile);
			sendfileproperty.sendfile=NULL;
			sendflag=0;
			return -1;
		}	
	}
	else
	{
		 memset(&data,0,sizeof(data));
		  while((data.datalen=fread(data.data,sizeof(char),
							user_data_len,sendfileproperty.sendfile)))  
		  {
			sendfileproperty.curcount++;             //add 1
			sprintf(data.command,"upload #%d %d/%d",
						sendfileproperty.fileno,sendfileproperty.curcount,
						sendfileproperty.totalcount);  
			if(safe_send(netfd,&data)<=0)
			{
		    		fclose(sendfileproperty.sendfile);
    				sendfileproperty.sendfile=NULL;
				sendflag=0;
    				return -1;
			}    //data send error,return
//			memset(command,0,30);
//			sprintf(command,"%d\n",sendfileproperty.curcount*100/
//								sendfileproperty.totalcount);
			memset(command,0,30);
			sprintf(command,"%d\n",sendfileproperty.curcount*100/
								sendfileproperty.totalcount);
			if(calback->write_ts)
			calback->write_ts("ok1","num1",command);
//			writets("ok1","num1",command);
			memset(&data,0,sizeof(data));
			break;
		}//whiel
	} //if(sendlen==0&&sendlen!=Maxline)

	if(sendfileproperty.curcount==sendfileproperty.totalcount)
	{
		fclose(sendfileproperty.sendfile);          //close file
		sendfileproperty.sendfile=NULL;
		sendfileproperty.filesendflag=0;            //set filesendflag as '0'
		sendflag=0;
//printf("tmp=%s,%s\n",tmpfilename,sendfileproperty.filename);
//		if(tmpfilename!=NULL&&strstr(tmpfilename,"wdjl.wds"))	backupwdjl();
	}
	return 0;
}

/*--------------------------------------------------------------------------*
@Function			:new_pathtopath - path convert
@Include      		: "net_tcp.h"
@Description			:
				path : file's path
				will change '\' in windows to '/' in Unix
				'\\' signitfy the current directory 
@Return Value		: Success1 Failure0.
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int new_pathtopath(char * path)
{
	int i=0,flag=0;
	char buff[user_data_len],*p,*p1;

	memset(buff,0,user_data_len);
	p=path;
	p1=buff;
	if(memcmp(p,"\\\\",2)==0)     //convert current path directly
	{
	  	p+=2;
	  	i+=2;
	}
	if(strncmp(p,"dwdata\\",7)==0)  //if the path start with dwdata/,it stands for current path
	{
		  flag=1;
		  p+=7;
	}
	if(strncmp(p,"updata\\",7)==0)  //if the path start with updata/,it stands for current path
	{
		 flag=1;
		 p+=7;
	}
	while(*p)
	{
	     if(*p=='\\')	*p1='/';                    //decollator convert
	     else if(*p=='\r'||*p=='\n')break;
	     else *p1=*p;
	     p1++;
	     p++;

	     if(i>user_data_len)	return -1;
	     i++;
	}
	memset(path,0,user_data_len);
	strncpy(path,buff,user_data_len);

	return flag;
}


/*//此函数为验证管理员权限
int netselectwdda(char *id,char *password,char *card)
{
	int  i=0, bhnum=0,bhflag=0,mmnum=0,mmflag=0,khnum=0,khflag=0,glnum=0,glflag=0;
	struct personrecord *t1;

	t1=selectwdda(headrecord,"bh",id);             //根据连接id检索档案,
     
	for(i=0;i<CLONUMB;i++)//查找此编号为于第几列
	{
		if(strcmp(wddatitle[i][0],"bh")==0)
		{
			bhnum=i;
			bhflag=1;
		}                     //获得密码列位置
		if(strcmp(wddatitle[i][0],"mm")==0)
		{
			mmflag=1;
			mmnum=i;
		}
		if(strcmp(wddatitle[i][0],"gl")==0)
		{
			glnum=i;
			glflag=1;
		}
		if(strcmp(wddatitle[i][0],"kh")==0)
		{
			khnum=i;
			khflag=1;
		}
	}
	if(t1==NULL||t1->qy!=1)		return -1;                           //如果禁用则验证失败
	if(bhflag!=1||glflag!=1)return -1;
	if(t1->record[glnum]==NULL||strcmp(t1->record[glnum],"1")!=0)return -1;//有管理权限
          
	if(card&&strlen(card))
	{
		if(khflag==1&&t1->record[khnum]!=NULL)
		if(strcmp(t1->record[khnum],card)==0) //如果密码正确且有管理权限
                    return 0;
	}
               
	if(password&&strlen(password))
	{
		if(mmflag!=1)	return -1;
		if(t1->record[mmnum]==NULL)	return -1;
		if(strcmp(t1->record[mmnum],password)==0) //如果密码正确且有管理权限
			return 0;
		else return -1;
	}
	return -1;
}
*/
/*--------------------------------------------------------------------------*
@Function			:link_valid - connection validate
@Include      		: "com.h"
@Description			:
				data : data packet
				0 - no validation;1 - Validate connection code;
				2 - Validate connection code and administor'password;									
@Return Value		: Success 0,Failure -1.
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int link_valid(char *tmpbuff,int datalen)
{
	char netline[user_data_len],netcard[user_data_len],netpassword[user_data_len],netid[user_data_len]; 
	char info[256],*ptr,*info_ptr,*out_ptr,*beg,*end;

	int return_code;

	memset(info,0,sizeof(info));
	decrypt(tmpbuff,info,strlen(tmpbuff),sizeof(info)); 
	memset(netline,0,user_data_len);
	memset(netcard,0,user_data_len);
	memset(netpassword,0,user_data_len);
	memset(netid,0,user_data_len);
	info_ptr=info;
	while((ptr=strtok_r(info_ptr,"\n",&out_ptr))!=NULL)		//split it line by line
	{
		beg=end=NULL;
		beg=strtok(ptr,"=");//break up it by using "=" as delimiter
		if(beg)
			end=strtok(NULL,"=");        
		if(beg&&strcmp(beg,"line")==0&&end)		strcpy(netline,end);	//link mode
		else if(beg&&strcmp(beg,"card")==0&&end)	strcpy(netcard,end);
		else if(beg&&strcmp(beg,"id")==0&&end)		strcpy(netid,end);
		else if(beg&&strcmp(beg,"password")==0&&end)	strcpy(netpassword,end);
		info_ptr=NULL;
	}
	cut(netline);
	cut(netcard);
	cut(netid);
	cut(netpassword);

	if(netinfo.line==0)		return 0;					//do not need verification
	else if(netinfo.line==1) 
	{
		if(strcmp(netline,netinfo.linepassword)==0)                        //verify link-code
			return_code = 0;
		else return_code = -1;
	}
	else if(netinfo.line==2&&strcmp(netline,netinfo.linepassword)==0)           
	{
		//if(calback->verify_manger) //modify by aduo 2013.7.22
		if(calback->verify_manger(netid,netpassword,netcard)==0)	return_code = 0; //verify serial No,passwd,Privilege.
		else return_code = -1;
	} 
	else return_code = -1;

	return return_code;
}

/*--------------------------------------------------------------------------*
@Function			:new_linezd - connection validate
@Include      		: "net_tcp.h"
@Description			:
				lizezdsucceed : connection mode
				netfd : handle
				tmpbuff : packet
				0 - no validation;1 - Validate connection code;
				2 - Validate connection code and administor'password;
@Return Value		: Success1 Failure0.
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int new_linezd(struct package *tmpbuff,int netfd,int *lizezdsucceed)
{
	char tmpbuf[1024],*p;

	if(link_valid(tmpbuff->data,tmpbuff->datalen)==0)
		*lizezdsucceed=1;
	memset(tmpbuff,0,1024);
	if(*lizezdsucceed==1)
	{
		 p=creat_pzinfo();
		 strcpy(tmpbuff->data,p);
	}
	else
	{
	 	memset(tmpbuf,0,sizeof(tmpbuf));
	 	sprintf(tmpbuf,"linezd=0");
	 	encrypt(tmpbuf,tmpbuff->data,strlen(tmpbuf),user_data_len);
	}
	sprintf(tmpbuff->command,"linest");
	tmpbuff->datalen=strlen(tmpbuff->data);
	safe_send(netfd,tmpbuff);
	return 0;
}

/*--------------------------------------------------------------------------*
@Function			: new_deletefile - delete file
@Include      		: "net_tcp.h"
@Description			: netfd :	handle
				tmpbuff : packet
				Remote deleting files
@Return Value		: Success0 Failure-1.
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int new_deletefile(struct package *tmpbuff,int netfd)
{
	char tmpfile[user_data_len],*p,*p2,command[60];
	char buff[user_data_len];
	int len;

	p=tmpbuff->data;
	p2=tmpfile;
	memset(buff,0,32);
	memset(p2,0,user_data_len);
	memset(command,0,60);
	while(*p)
	{
		len=strcspn(p,"\n");
		strncpy(p2,p,len);
		p+=len;
		p++;
		cut(p2);
		decrypt(p2,buff,strlen(p2),user_data_len);
		if(new_pathtopath(buff)!=0)	uppathtopath(buff);
		if(strstr(buff,"wdjl.wds"))	backupwdjl();
		safe_rm(buff);
		memset(p2,0,user_data_len);
		memset(buff,0,user_data_len);
	}
	memset(tmpbuff->data,0,user_data_len);
	tmpbuff->datalen=0;
	safe_send(netfd,tmpbuff);
	return 0;
}

/*--------------------------------------------------------------------------*
@Function			:backupwdjl - record backup
@Include      		: "net_tcp.h"
@Description			:backup records according to time
@Return Value		: void
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
void backupwdjl()    
{
	char date[24];
	char buf[128];
	char *path="note/backup";
	time_t the_time;
	struct tm * tm_time;

	time(&the_time);
	tm_time=localtime(&the_time);
	memset(date,0,sizeof(date));
	memset(buf,0,sizeof(buf));
	sprintf(date,"%04d%02d%02d%02d%02d%02d",tm_time->tm_year+1900,tm_time->tm_mon+1,tm_time->tm_mday,tm_time->tm_hour,tm_time->tm_min,tm_time->tm_sec);

	if(access(path,F_OK)!=0)
		mkdir(path,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH);
	sprintf(buf,"%s/%s.wds",path,date);
	rename("note/record/new.wds",buf);
printf("buf=%s\n",buf);
	remove("note/record/new.wds");
//	kqjlnum=0;
}

//文件合并 *str 时间段aa.txt,20090901,20091010
int meragefile(char *path)
{
	FILE *tfp,*ffp;
	DIR *db;
	struct dirent *p;
	unsigned int CMAXLEN = 1024;
	char buf[CMAXLEN],buf1[126];
	char *framepath="note/backup/";
	char tmp1[126],tmp2[126],filename[126];
	int flag=0;

	cut(path);
	memset(buf,0,sizeof(buf));	
	strcpy(buf,path);

	memset(path,0,sizeof(path));
	strcpy(path,strtok(buf,","));

	memset(tmp1,0,sizeof(tmp1));		//begin time
	strcpy(tmp1,strtok(NULL,","));
	memset(tmp2,0,sizeof(tmp2));		//end time
	strcpy(tmp2,strtok(NULL,","));
	if((tfp=fopen(path,"a+"))==NULL)
	{
	   fclose(tfp);
	   return -1;
	 }

	db=opendir(framepath);
	if(db!=NULL&&strlen(tmp1)==8&&strlen(tmp2)==8)	
	{
		memset(filename,0,64);
		while ((p=readdir(db)))
		{  
			if((strcmp(p->d_name,".")==0)||(strcmp(p->d_name,"..")==0)||(strcmp(p->d_name,"~~.wds")==0))
				continue;
			else
			{  
				memset(filename,0,sizeof(filename));
				strncpy(filename,p->d_name,8); 
				if(atoi(filename)< atoi(tmp1) || atoi(filename)>atoi(tmp2))		
					continue;
				memset(buf1,0,sizeof(buf1));
				sprintf(buf1,"%s%s",framepath,p->d_name);
				if((ffp=fopen(buf1,"r"))==NULL)	continue;
				while(fgets(buf,sizeof(buf),ffp)) {
					flag=1;
					cut(buf);
					fprintf(tfp,"%s\r\n",buf);
				}
				fclose(ffp);
				remove(buf1);
			}
			memset(filename,0,64);
		}
	}
	fclose(tfp);
	if(flag==0)	remove(path);

	  return 0;
	
}

/*--------------------------------------------------------------------------*
@Function			:retime_all - Alter terminal's time
@Include      		: "net_tcp.h"
@Description			:	None
@Return Value		: Success 1 Failure 0.
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int retime_all(int year,int mon,int day,int hour,int min,int sec)
{
	struct timeval tv;                                //time structure
	struct timezone tz;                               //time zone
	struct tm p;   
	char cmdbuf[128];
	int i,j;

	memset(&p,0,sizeof(p));
	p.tm_year=year-1900;//tm_year -- save years from 1900 to now
	p.tm_mon=mon-1;//tm_mon (0-11)
	p.tm_mday=day;
	p.tm_hour=hour;
	p.tm_min=min;
	p.tm_sec=sec;
	gettimeofday (&tv , &tz);      //Get the time zone information
	tv.tv_sec=mktime(&p);                                    
	i=settimeofday(&tv,&tz);//set system time
	
	sprintf(cmdbuf, "setdate -w %04d.%02d.%02d-%02d:%02d:%02d", 
					year, mon,day, hour, min, sec);
	if((j=system(cmdbuf))!=0)		printf("setdate fail!\n");/* set hardware clock */
//	syncclock();
	if(i<0||j<0)
		return -1;
	else return 0;
}

/*--------------------------------------------------------------------------*
@Function					:	new_retime - Network time calibration
@Include      		: "net_tcp.h"
@Description			:	process calibration command 
@Return Value			: Success 0 Failure -1
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int new_retime(struct package * tmpbuff,int netfd)
{
	int hour=0,min=0,sec=0,year=0,mon=0,day=0;
	char cmdbuf[user_data_len],curtime[32];

	memset(cmdbuf,0,user_data_len);
	decrypt(tmpbuff->data,cmdbuf,strlen(tmpbuff->data),user_data_len);
	cut(cmdbuf);
	sscanf(cmdbuf,"%4d-%2d-%2d %2d:%2d:%2d",
					&year,&mon,&day,&hour,&min,&sec);//get server's time
	retime_all(year,mon,day,hour,min,sec);
	memset(curtime,0,sizeof(curtime));
	getcurtime(curtime);
	memset(tmpbuff->data,0,user_data_len);
	encrypt(curtime,tmpbuff->data,strlen(curtime),user_data_len);
	tmpbuff->datalen=strlen(tmpbuff->data);
	safe_send(netfd,tmpbuff);
	return 0;
}

/*--------------------------------------------------------------------------*
@Function			:safe_send - send data
@Include      		: "net_tcp.h"
@Description			:
				fd : handle
				net_package : packet will be send
@Return Value		: Success 0 Failure -1
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
 int safe_send(int fd, struct package *net_package)
 {
	int  n=0;
	unsigned char *ptr = (unsigned char *)net_package;
	fd_set send_fd;
	struct timeval net_timer;

	switch(netinfo.linetype){
		case 1://RS485
			comm_put_data(serialport2,net_package);
			return 1;
		case 3://GPRS
			GprsSend((unsigned char *)net_package,sizeof(struct package));
			return 1;
		default:
		break;
	}

	FD_ZERO(&send_fd);
	FD_SET(fd,&send_fd);
	net_timer.tv_sec=0;
	net_timer.tv_usec=1000000;
	if(abs(time(NULL) - starttime)>atoi(netinfo.netovertime))
	{
		return -1;
	}

	if(select(fd+1, NULL,&send_fd, NULL, &net_timer) <= 0)	return 1;
	n=write(fd,(char *)(ptr+sendlen), Maxline-sendlen);
	if(n == -1 ) 
	{
          return -1;
      	}
	sendlen=sendlen+n;
	if(sendlen==Maxline)	sendlen=0;
	time(&starttime);
	return n;
}

/*--------------------------------------------------------------------------*
@Function			:safe_recv - receive data
@Include      		: net_tcp.h
@Description			:
				fd : handle
				net_package : packet to be received
@Return Value		: Success0 Failure-1.
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int safe_recv(int fd, struct package *net_package)
{
	int status=0,n=0;
	unsigned char *ptr=NULL;

	if(netinfo.linetype==3)
		return GprsRecv((unsigned char *)net_package,sizeof(struct package));
	if(netinfo.linetype==2)	time(&starttime);		//daili 
	if(abs(time(NULL)-starttime)>atoi(netinfo.netovertime))//check overtime
	{
		return -2;
	}

	ioctl(fd, FIONREAD, &status);
	if(status>=Maxline)
	{
		ptr=(unsigned char*)net_package;
		n=read(netfd,ptr,Maxline);
		if(n != Maxline)
		{
			return -1;
		}
		time(&starttime);		
		return n;
	}

	return 0;
}

/*--------------------------------------------------------------------------*
@Function					:	tcp_make_socket - create socket
@Include      		: "net_tcp.h"
@Description			:
										port : port number
										bind port and return tcp fd
@Return Value			: Success net handle ; Failure -1
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int tcp_make_socket(unsigned short int port)
{
	int sock ;
	struct sockaddr_in name;

	sock=socket(AF_INET,SOCK_STREAM,0);	
	if(sock<0)	return -1;
	if(netinfo.linetype==2)	return sock;
	name.sin_family=AF_INET;
	name.sin_port=htons(port);
	name.sin_addr.s_addr=htonl(INADDR_ANY);
	if(bind(sock,(struct sockaddr*)&name,sizeof(name))<0)
   	{
	    perror("bind");
	    return -1;
	}
	return sock;
}

/*--------------------------------------------------------------------------*
@Function			:downpathtopath - download path convert
@Include      		: "net_tcp.h"
@Description			:
				path : path to be loaded
@Return Value		: void
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
void downpathtopath(char *path)
{
	char tmpbuf[user_data_len];
	int xh=0,pos=0;

//printf("path=%s\n",path);
	if(path==NULL)return ;

	//printf("downpath begin: %s\n",path);
	if(strstr(path,"wdda.wts"))
	{
	   memset(path,0,user_data_len);
	   strcpy(path,"note/archives/wdda.wts");
	}
	else if(strstr(path,"wdgz.wts"))
	{
	   memset(path,0,user_data_len);
	   strcpy(path,"note/archives/wdgz.wts");
	}
	else if(strstr(path,"wdzm.wts"))
	{
	   memset(path,0,user_data_len);
	   strcpy(path,"note/archives/wdzm.wts");
	}
	else if(strstr(path,"update.wts"))
	{
	   memset(path,0,user_data_len);
	   strcpy(path,"note/archives/update.wts");
	}
	else if(strstr(path,"direction.ini"))
	{
	   memset(path,0,user_data_len);
	   strcpy(path,"resfile/wes/direction.ini");
	}
	else if(strstr(path,".pht"))
	{
	    sscanf(path,"%d.pht",&xh);
	    memset(tmpbuf,0,user_data_len);
	    strcpy(tmpbuf,path);
	    memset(path,0,user_data_len);
	    sprintf(path,"note/photo/%d/%s",xh/1000,tmpbuf);  //put it in directory named 'xh/1000'
 
	}
	else if(strstr(path,"adplay"))
	{
	    memset(tmpbuf,0,user_data_len);
	    strcpy(tmpbuf,path);
	    memset(path,0,user_data_len);
	    sprintf(path,"./%s",tmpbuf);
	}
	else if(strstr(path,"music"))
	{
	    memset(tmpbuf,0,user_data_len);
	    strcpy(tmpbuf,path);
	    memset(path,0,user_data_len);
	    sprintf(path,"./%s",tmpbuf);
  	}
	else if(strstr(path,"s10"))  //fingerprint file
	{
	    sscanf(path,"%d_%d",&xh,&pos);
	    memset(path,0,user_data_len);
	    sprintf(path,"note/finger/%d/%d_%d.s10",xh/1000,xh,pos);
	}
	return ;
}

/*--------------------------------------------------------------------------*
@Function			: uppathtopath - upload path convert
@Include      		: "net_tcp.h"
@Description			:
				path : path to be loaded
				upload the file in tertminal to PC according to path
@Return Value		: void
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
void uppathtopath(char *path)
{
   char tmpbuf[user_data_len];
   char date[12],mon[4],year[6],day[4],picturename[12];

//printf("uppath begin: %s\n",path);
   if(path==NULL)return ;
   if(strstr(path,"s10"))
   {
    memset(tmpbuf,0,user_data_len);
    strcpy(tmpbuf,path);
    memset(path,0,user_data_len);
    sprintf(path,"note/admi/finger/%s",tmpbuf); 
   }
    else if(strstr(path,"wdgl.wds"))
   {
     memset(path,0,user_data_len);
     strcpy(path,"note/admi/wdgl.wds");
   }
   else if(strstr(path,"wdjl.wds"))
   {
     memset(path,0,user_data_len);
     strcpy(path,"note/record/wdjl.wds");
   }
   else if(strstr(path,"fplist.wts"))
   {
     GetFpList();
     memset(path,0,user_data_len);
     strcpy(path,"note/record/fplist.wts");
   }
   else if(strstr(path,"jk")&&strstr(path,"jpg"))
   {
     memset(tmpbuf,0,user_data_len);
     memset(date,0,12);
     memset(year,0,6);
     memset(mon,0,4);
     memset(day,0,4);
     sscanf(path,"%4s-%2s-%2s/jk%10s",year,mon,day,picturename);
     sprintf(tmpbuf,"%s-%s-%s/jk%s",year,mon,day,picturename);
     memset(path,0,user_data_len);
     sprintf(path,"note/frame/%s",tmpbuf);
   }
   else if(strstr(path,".jpg"))
   {
    memset(tmpbuf,0,user_data_len);
    strcpy(tmpbuf,path);
    memset(path,0,user_data_len);
    sprintf(path,"note/admi/photo/%s",tmpbuf); 
   }
  else if(strstr(path,","))
  {
	meragefile(path);
  }

//printf("uppath end: %s\n",path);
}


//record the time when receive data for the last time
void reset_recv_time()
{
	time(&recv_time);
}

/*--------------------------------------------------------------------------*
@Function			:creat_pzinf - return the status of terminal
@Include      		: "net_tcp.h"
@Description			:return the status of terminal
@Return Value		: Success status has been encrypted
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
char * creat_pzinfo()
{
	static char buf[1024];
	char tmpbuf[1024];
	char time1[126],time2[126];
	time_t the_time;
	struct tm * tm_time;

	time(&the_time);
	tm_time=localtime(&the_time);

	sprintf(time1,"%04d-%02d-%02d",tm_time->tm_year+1900,tm_time->tm_mon+1,tm_time->tm_mday);
	sprintf(time2,"%02d:%02d:%02d",tm_time->tm_hour,tm_time->tm_min,tm_time->tm_sec);
	memset(tmpbuf,0,sizeof(tmpbuf));
	sprintf(tmpbuf,"linezd=1\r\nMode=%s\r\nCpu=%s\r\nKernel=%s\r\nApplication=%s\r\nTime=%s %s\r\nFpnum=%d\r\nTrd=%s\r\nMemory=%s\r\nDetails=%s\r\nzw=s10\r\nzp=1\r\n",TERMINALTYPE,getcpuinfo(),getkernelinfo(),APPLICATIONTYEP,time1,time2,getfingernum(),"101",getmeminfo(),"***");
 
	memset(buf,0,sizeof(buf));
	encrypt(tmpbuf,buf,strlen(tmpbuf),sizeof(buf));
	return buf;
}

/*--------------------------------------------------------------------------*
@Function			:getkernelinfo - get kernel version
@Include      		: "net_tcp.h"
@Description			:return the version of current OS kernel
@Return Value		: Success return version; Failure: NULL
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
char *getkernelinfo()
{
	FILE *	file;
	char buf[256];
	static char version[126];

	file=fopen("/proc/version","r");
	if(!file)	return NULL;
	memset(buf,0,sizeof(buf));
	if(fgets(buf,sizeof(buf),file)==NULL)	return NULL;
	strncpy(version,buf,31);
	fclose(file);
	return version;
}

/*--------------------------------------------------------------------------*
@Function			:getcpuinfo- get CPU model
@Include      		: "net_tcp.h"
@Description			: None
@Return Value		: Success return cpuinfo; Failure: NULL
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
char *getcpuinfo()
{
	FILE *	file;
	char buf[256],*p;
	static char cpuinfo[126];

	file=fopen("/proc/cpuinfo","r");
	if(!file)	return NULL;
	memset(buf,0,sizeof(buf));
	if(fgets(buf,sizeof(buf),file)==NULL)	return NULL;
	p=strstr(buf,":");
	p+=1;
	strncpy(cpuinfo,p,26);
	fclose(file);
	return cpuinfo;
}

/*--------------------------------------------------------------------------*
@Function			:getmeminfo- get memory size
@Include      		: "net_tcp.h"
@Description			: None
@Return Value		: Success return memoinfo; Failure: NULL
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
char *getmeminfo()
{
	FILE *	file;
	int len;
	char buf[256],*p,tmp[126];
	static char meminfo[126];

	file=fopen("/proc/meminfo","r");
	if(!file)	return NULL;
	memset(buf,0,sizeof(buf));
	if(fgets(buf,sizeof(buf),file)==NULL)	return NULL;
	if(fgets(buf,sizeof(buf),file)==NULL)	return NULL;
	p=strstr(buf,":");
	p+=3;
	memset(tmp,0,sizeof(tmp));
	strncpy(tmp,p,strcspn(p," "));
	len=atoi(tmp)/1000000;
	sprintf(meminfo,"%dM",len);
	fclose(file);
	return meminfo;
}

/*--------------------------------------------------------------------------*
@Function			:sys_command - execute script
@Include      		: "net_tcp.h"
@Description			: execute script form PC
@Return Value		: void
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
void sys_command()
{
	FILE *	file;
	char command[256];

	file=fopen("./sys_command.sh","r");
	if(!file)	return;
	memset(command,0,sizeof(command));
	while(fgets(command,sizeof(command),file))
	{
	 	cut(command);
 		system(command);
	}
	fclose(file);
	remove("./sys_command.sh");
}

/*--------------------------------------------------------------------------*
@Function			: change - change the current file to new file
@Include      		: "net_tcp.h"
@Description			: filename: the file you want to change
@Return Value		: Success 0;Failure -1
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int change(char *filename)    
{
	char *path="note/record/new.wds";

	if(access(path,F_OK)==0){			
		appen_file(filename,path);	//new file not exit
	}
	else
		rename(filename,path);	//new file not exit
	memset(filename,0,sizeof(filename));
	sprintf(filename,"%s",path);
	return 0;
}

/*--------------------------------------------------------------------------*
@Function			: new_player - play audio remotely
@Include      		: "net_tcp.h"
@Description			: tmpbuff:data packet 
				  netfd: file description
@Return Value			: Success 0;
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int new_player(struct package * tmpbuff,int netfd)
{
	char cmdbuf[user_data_len];

	memset(cmdbuf,0,user_data_len);
	strcpy(cmdbuf,tmpbuff->data);
	decrypt(cmdbuf,tmpbuff->data,strlen(cmdbuf),user_data_len);
	new_pathtopath(tmpbuff->data);    //The path name format convert (windows->linux)
	cut(tmpbuff->data);
	memset(cmdbuf,0,user_data_len);	
	sprintf(cmdbuf,"splay /%s &",tmpbuff->data);

	system(cmdbuf);
	memset(tmpbuff->data,0,user_data_len);
	encrypt(cmdbuf,tmpbuff->data,strlen(cmdbuf),user_data_len);
	tmpbuff->datalen=strlen(tmpbuff->data);
	safe_send(netfd,tmpbuff);
	return 0;
}

/*--------------------------------------------------------------------------*
@Function			: new_gpio - delay operation
@Include      		: "net_tcp.h"
@Description			: tmpbuff:data packet 
				  netfd: file description
@Return Value		: Success 0;
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int new_gpio(struct package * tmpbuff,int netfd)
{
	char cmdbuf[user_data_len];
	int flag=0;

	memset(cmdbuf,0,user_data_len);
	decrypt(tmpbuff->data,cmdbuf,strlen(tmpbuff->data),user_data_len);
	new_pathtopath(cmdbuf);    //The path name format convert (windows->linux)
	cut(cmdbuf);
      	if(GpioOpt(atoi(cmdbuf))==TRUE)	flag=1;
	memset(cmdbuf,0,sizeof(cmdbuf));
	sprintf(cmdbuf,"%d",flag);
	memset(tmpbuff->data,0,user_data_len);
	encrypt(cmdbuf,tmpbuff->data,strlen(cmdbuf),user_data_len);
	tmpbuff->datalen=strlen(tmpbuff->data);
	safe_send(netfd,tmpbuff);
	return 0;
}

int checkfile(char *framepath,char *filename)
{
	DIR *db;
	int flag=0;
	char buf[256];
	struct dirent *p;

	db=opendir(framepath);
	if(db==NULL)		return 0;
	while ((p=readdir(db)))
	{  
		if((strcmp(p->d_name,".")==0)||(strcmp(p->d_name,"..")==0))
			continue;
		else
		{  
//				sprintf(filename,"%s/%s",framepath,p->d_name); //get the second level path
			memset(buf,0,sizeof(buf));
			sprintf(buf,"%s/%s",framepath,p->d_name);
			if(testdir(buf)!=1){
				if(strncmp(p->d_name,filename,strlen(p->d_name))==0)		
				{
					flag=1;
					closedir(db);
					return flag;
				}		
			}
			else
				if(checkfile(buf,filename)==1)
				{
					flag=1;
					closedir(db);
					return flag;
				}
		}
	}
	closedir(db);
	return 0;
}

/*--------------------------------------------------------------------------*
@Function			: new_checkfp - check fingerprint templates
@Include      		: "net_tcp.h"
@Description			: tmpbuff:data packet 
				  netfd: file description
@Return Value		: Success 0;
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int new_checkfp(struct package * tmpbuff,int netfd)
{
	char cmdbuf[user_data_len];
	int flag=0;

	memset(cmdbuf,0,user_data_len);
	decrypt(tmpbuff->data,cmdbuf,strlen(tmpbuff->data),user_data_len);
	new_pathtopath(cmdbuf);    //The path name format convert (windows->linux)
	cut(cmdbuf);
	flag=checkfile("./note/finger",cmdbuf);
	if(flag==0)
		flag=checkfile("./note/admi/finger",cmdbuf);

	memset(cmdbuf,0,sizeof(cmdbuf));
	sprintf(cmdbuf,"%d",flag);
	memset(tmpbuff->data,0,user_data_len);
	encrypt(cmdbuf,tmpbuff->data,strlen(cmdbuf),user_data_len);
	tmpbuff->datalen=strlen(tmpbuff->data);
	safe_send(netfd,tmpbuff);
	return 0;
}

/*--------------------------------------------------------------------------*
@Function			: appendwdda - add a new record
@Include      		: "net_tcp.h"
@Description			: char *str: employee record you want to add
@Return Value		: mac
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int appendwdda(char *str)
{
	FILE *file=NULL;

	file=popen("note/archives/wdda.wts"," a+");
	if(!file)	return -1;
	fprintf(file,"\r\n%s",str);
	fclose(file);
	return 0;
}

/*--------------------------------------------------------------------------*
@Function			: appenddirection- Modify the direction key
@Include      		: "net_tcp.h"
@Description			: char *str: 
				allow the host to force the terminal into a given mode or function(IN,OUT,Locked...)
@Return Value		: Success 0; Failure -1.
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int appenddirection(char *filename)
{
	printf("filename=%s\n",filename);
/*	FILE *fd;
	int num=1,flag=0,len=0;
	char buf[1024],tmp[15][128],tmp1[126];	

	if((fd=fopen(filename,"r"))==NULL)	return -1;

	memset(tmp,0,sizeof(tmp));
	memset(buf,0,sizeof(buf));
	while(fgets(buf,sizeof(buf),fd))
	{  
		cut(buf);
		if(strcmp(buf,"[direction]")==0)
		{
			flag=1;
			continue;
		}// find the group position and set it
		if(flag==0)	return -1;
		memset(tmp,0,sizeof(tmp));
		len=strcspn(buf,"=");     //cut up members of group and value with '='
		if(len==0)	continue;
		memset(tmp1,0,sizeof(tmp1));
		memcpy(tmp1,buf,len);
		num=0;
		while(strlen(q_direction[num].direction))
		{
			if(strncmp(q_direction[num].direction,tmp1,len)!=0)
				continue;
			field_handle(&buf[len+1],tmp);	
			memcpy(q_direction[num].direction,tmp1,len);
			memcpy(q_direction[num].text,tmp[0],strlen(tmp[0]));
			if(strlen(tmp[1])==0)
				q_direction[num].lx=-1;
			else			
				q_direction[num].lx=atoi(tmp[1]);
			memcpy(q_direction[num].number,tmp[2],strlen(tmp[2]));
			memcpy(q_direction[num].sound,tmp[3],strlen(tmp[3]));
			num++;
		}
		memset(buf,0,sizeof(buf)); 
	}
	fclose(fd);

	if((fd=fopen("resfile/wes/direction.ini","w"))==NULL)	return -1;
	num=0;
	memset(buf,0,sizeof(buf));
	while(strlen(q_direction[num].direction))
	{
		sprintf(buf,"%s=%s,%d,%s,%s",q_direction[num].direction,q_direction[num].text,q_direction[num].lx,q_direction[num].number,q_direction[num].sound);
		fprintf(fd,"%s\n",buf);
		memset(buf,0,sizeof(buf));
	}
	fclose(fd);
	memset(buf,0,sizeof(buf));
	sprintf(buf,"cp resfile/wes/direction.ini backup/%d/wes/direction.ini",atoi(deviceinfo.language));
	system(buf);
*/
	  return -1;
}

/*************************************************************
		Get Domain		
*************************************************************/
void printmessage(unsigned char *buf,char *ipaddr);
static unsigned char *printnamestring(unsigned char *p,unsigned char *buf);

int host_udp_send(unsigned char *buf,int len)
{
  struct sockaddr_in saddr;
  socklen_t saddrlen;
  struct in_addr addr,*p_addr;
  int i;

	if(udp_socket<=0)	return -1;
	saddrlen = sizeof(saddr);
	//set server address
	saddr.sin_family=PF_INET;
	inet_aton(netinfo.netserverip,&addr);
	p_addr=&addr;
	bcopy(p_addr,&saddr.sin_addr,sizeof(saddr.sin_addr));
	saddr.sin_port=htons(remote_udp_port);

	i=sendto(udp_socket,(char*)buf,len,0,(struct sockaddr *)&saddr,saddrlen);
	return i;
}


int host_udp_recv(unsigned char *buf,int len,int overtime)
{
	socklen_t saddrlen;
	fd_set recv_fd;
	struct timeval net_timer;
	int i;

	if(udp_socket<=0)	return -3;
	net_timer.tv_sec=overtime;
	net_timer.tv_usec=0;
	FD_ZERO(&recv_fd);
	FD_SET(udp_socket, &recv_fd);

	saddrlen=sizeof(saddr);
	if((select(udp_socket+1,&recv_fd,NULL , NULL, &net_timer)) >0)      //-1 will be return immediately if disconnect
	{  
      		errno=0;
		i= recvfrom(udp_socket,(char *)buf,len,0,(struct sockaddr *)&saddr,&saddrlen);
		memset(ip_addr,0,sizeof(ip_addr));
		strcpy(ip_addr,inet_ntoa(saddr.sin_addr));
		if(strcmp(ip_addr,netinfo.netserverip)!=0)
			return -2;
		return i;
	}
	else 
	{
		return -1;
	}
}

void printmessage(unsigned char *buf,char *ipaddr)
{
    unsigned char *p;
    unsigned int ident,flags,qdcount,ancount,nscount,arcount;
    unsigned int i,j,type,classes,ttl,rdlength;
	char tmp[126];

    p = buf;
    GETWORD(ident,p);
//    printf("ident=%#x\n",ident);
    GETWORD(flags,p);
/*    printf("flags=%#x\n",flags);
    //printf("qr=%u\n",(flags>>15)&1);
    printf("qr=%u\n",flags>>15);
    printf("opcode=%u\n",(flags>>11)&15);
    printf("aa=%u\n",(flags>>10)&1);
    printf("tc=%u\n",(flags>>9)&1);
    printf("rd=%u\n",(flags>>8)&1);
    printf("ra=%u\n",(flags>>7)&1);
    printf("z=%u\n",(flags>>4)&7);
    printf("rcode=%u\n",flags&15);  
*/
    GETWORD(qdcount,p);
//    printf("qdcount=%u\n",qdcount);
    GETWORD(ancount,p);
//    printf("ancount=%u\n",ancount);
    GETWORD(nscount,p);
//    printf("nscount=%u\n",nscount);
    GETWORD(arcount,p);
//    printf("arcount=%u\n",arcount);
    for(i=0; i<qdcount; i++)
    {
//       printf("qd[%u]:\n",i);
       while(*p!=0)
       {
           p = printnamestring(p,buf);
           if(*p != 0)
              printf(".");
       }
       p++;
       printf("\n");
       GETWORD(type,p);
//       printf("type=%u\n",type);
       GETWORD(classes,p);
//       printf("classes=%u\n",classes);
    }
	 
    for(i=0; i<ancount; i++)
    {
//       printf("an[%u]:\n",i);
       p = printnamestring(p,buf);
       printf("\n");
       GETWORD(type,p);
//       printf("type=%u\n",type);
       GETWORD(classes,p);
//       printf("classes=%u\n",classes);
       GETLONG(ttl,p);
//       printf("ttl=%u\n",ttl);
       GETWORD(rdlength,p);
//       printf("rdlength=%u\n",rdlength);
       printf("rd=");
	if((int)rdlength==4)
	{
		sprintf(ipaddr,"%d.%d.%d.%d",*p++,*p++,*p++,*p++);
		break;
	}
       for(j=0; j<rdlength; j++)
         {
           printf("%2.2x(%u)",*p,*p);
           p++;
         }
       printf("\n");
    }

}

//
static unsigned char *printnamestring(unsigned char *p,unsigned char *buf)
{

    unsigned int nchars,offset;

    nchars = *(p++);
    if((nchars & 0xc0) == 0xc0)
    {
       offset = (nchars & 0x3f) << 8;
       offset |= *(p++);
       nchars = buf[offset++];
       printf("%*.*s",nchars,nchars,buf+offset);
    }
    else
    {
       printf("%*.*s",nchars,nchars,p);
       p += nchars;
    }
    return (p);
}
/*--------------------------------------------------------------------------*
@Function			:GetDomain - The DNS domain conversion
@Include      		: "net_tcp.h"
@Description			: 
				buf : analytical url
				ipaddr : return ip addr
				set the serial port
@Return Value		: Success TRUE .Failure FALSE
*---------------------------------------------------------------------------*/
int GetDomain(IN char *buf,OUT char *ipaddr)
{
    time_t ident;
    int rc;
    char *q;
    unsigned char *p;
    unsigned char *countp;
    unsigned char reqBuf[512] = {0};
    unsigned char rplBuf[512] = {0};

	cut(buf);
    if(buf == NULL)
    {
       printf("usage: dnsclient <host_name>\n");
       return FALSE;
    }

    //Domain message
    time(&ident);
    //copy
    p = reqBuf;
    //Transaction ID
    *(p++) = ident;
    *(p++) = ident>>8;
    //Header section
    //flag word = 0x1000
    *(p++) = 0x01;
    *(p++) = 0x00;
    //Questions = 0x0001
    //just one query
    *(p++) = 0x00;
    *(p++) = 0x01;
    //Answer RRs = 0x0000
    //no answers in this message
    *(p++) = 0x00;
    *(p++) = 0x00;
    //Authority RRs = 0x0000
    *(p++) = 0x00;
    *(p++) = 0x00;
    //Additional RRs = 0x0000
    *(p++) = 0x00;
    *(p++) = 0x00;
    //Query section
    countp = p;   
    *(p++) = 0;
    for(q=buf; *q!=0; q++)
    {
       if(*q != '.')
       {
           (*countp)++;
           *(p++) = *q;
       }
       else if(*countp != 0)
       {
           countp = p;
           *(p++) = 0;
       }
    }
    if(*countp != 0)
       *(p++) = 0;
    //Type=1(A):host address
    *(p++)=0;
    *(p++)=1;
    //classes=1(IN):internet
    *(p++)=0;
    *(p++)=1;
 
    printf("\nRequest:\n");
    printmessage(reqBuf,ipaddr);
    //send to DNS Serv
    if(host_udp_send(reqBuf,p-reqBuf) < 0)
    {
       perror("error sending request");
       return FALSE;
    }
 
   //recev the reply
    rc = host_udp_recv(rplBuf,sizeof(rplBuf),atoi(netinfo.timeout_server));
    if(rc < 0)
    {
       perror("error receiving request\n");
       return FALSE;
    }   
   //print out results
    printf("\nReply:\n");
	memset(ipaddr,0,sizeof(ipaddr));
    printmessage(rplBuf,ipaddr);
    //exit
    printf("Program Exit\n");
	return TRUE;
}

/*--------------------------------------------------------------------------*
//udp opt
*--------------------------------------------------------------------------*/
int udp_send(struct package * netdata)
{
  struct sockaddr_in saddr;
  socklen_t saddrlen;
  struct in_addr addr,*p_addr;
  int i;

//	if(netinfo.net_485==3){
//		return gprs_udp_send((unsigned char*)netdata,sizeof(struct package));
//	}
	if(udp_socket<=0)	return -1;
	saddrlen = sizeof(saddr);
	//set server address
	saddr.sin_family=PF_INET;
	inet_aton(netinfo.netserverip,&addr);
	p_addr=&addr;
	bcopy(p_addr,&saddr.sin_addr,sizeof(saddr.sin_addr));
	saddr.sin_port=htons(remote_udp_port);

	i=sendto(udp_socket,(char*)netdata,1024,0,(struct sockaddr *)&saddr,saddrlen);
	return i;
}


int udp_recv(struct package * netdata,int overtime)
{
	socklen_t saddrlen;
	fd_set recv_fd;
	struct timeval net_timer;
	int i;

//	if(netinfo.net_485==3){
//		return gprs_udp_recv((unsigned char*)netdata,sizeof(struct package));
//	}
	if(udp_socket<=0)	return -1;
	net_timer.tv_sec=overtime;
	net_timer.tv_usec=0;
	FD_ZERO(&recv_fd);
	FD_SET(udp_socket, &recv_fd);
	memset(netdata,0,sizeof(*netdata));
	saddrlen=sizeof(saddr);
	if((select(udp_socket+1,&recv_fd,NULL , NULL, &net_timer)) >0)      //-1 will be return immediately if disconnect
	{  
      		errno=0;
		i= recvfrom(udp_socket,(char *)netdata,1024,0,(struct sockaddr *)&saddr,&saddrlen);
		memset(ip_addr,0,sizeof(ip_addr));
		strcpy(ip_addr,inet_ntoa(saddr.sin_addr));
		if(strcmp(ip_addr,netinfo.netserverip)!=0)
			return -1;
		return 0;
	}
	else 
	{
		return -1;
	}
}


int udp_linezd(char *tmpbuff,int datalen)//0 -pass validation, ohters value mean failure
{    
	char netline[128],netcard[128],netpassword[128],netid[128]; //obtain those value from configuration files
	char tmp1[128],tmp2[128],tmp3[128],tmp4[128],*p1,*p2;
	int i,j,len=0;
	int return_code;

	p1=tmpbuff;
	for(;;)
	{
		memset(tmp1,0,128);
		memset(tmp2,0,128);
		memset(tmp3,0,128);
		memset(tmp4,0,128);
		i=strcspn(p1,"\n");            //data is splited by '\r\n' .each line of data will be encrypted before to transfer,
		strncpy(tmp1,p1,i);                      //decrypt will do backwards operation
		cut(tmp1);
		decrypt(tmp1,tmp2,strlen(tmp1),128);                //tmp2 dest,tmp1 src

		j=strcspn(tmp2,"=");
		p2=tmp2;
		strncpy(tmp3,p2,j);
		p2+=j;
		p2++;
		strcpy(tmp4,p2);  
		p1+=i;
		p1++;
		if(strcmp(tmp3,"line")==0)	strcpy(netline,tmp4);
		else if(strcmp(tmp3,"card")==0)strcpy(netcard,tmp4);
		else if(strcmp(tmp3,"id")==0)strcpy(netid,tmp4);
		else if(strcmp(tmp3,"password")==0)strcpy(netpassword,tmp4);
		len+=i;
		len++;
		len++;
		if(len>=datalen)	break;
	}

	if(netinfo.line==0)return 0;
	else if(netinfo.line==1) 
	{
		if(strcmp(netline,netinfo.linepassword)==0)                       
			return_code = 0;
		else return_code = -1;
	}
	else if(netinfo.line==2&&strcmp(netline,netinfo.linepassword)==0)       
	{
		if(calback->verify_manger)
		{
	            if(calback->verify_manger(netid,netpassword,netcard)==0)	return_code = 0;
			else return_code = -1;
		}
	} 
	else return_code = -1;

	return return_code;
}



/*
//online opt
int netselect()
{
struct package data1; 
char tmp1[1000],tmp2[64],tmp3[64],tmp4[128],*p1,*p2,bhbuf[128],mmbuf[128],khbuf[128],zwbuf[128];
//size_t msglen;
//char buf[BUFSIZ];
int len,i,j,xs=0,xmbh=0,overtimeflag=0;
char zp[32],xh[32],ts[128],timebuff[32],mj[32];
char info[CLONUMB][2][16],sr[32];
char sy[32];
FILE *show;
//printf("new_netselect()\n");
//在wddatitle数组中保存的为人员档案的标题信息,由于udp传输来的人员档案信息与本地档案的标题头可能有不一致的
//地方,顾要对显示的数据进行正理,此处要用到此信息.在info数组中,每行记录人员档案的一个属性,第一列记录属性名称,第二列记录属性值.如 (xm,姓名)
memset(zp,0,32);
memset(xh,0,32);
memset(sy,0,sizeof(sy));
memcpy(info,wddatitle,sizeof(info));

for(j=0;j<CLONUMB;j++)
{
  memset(info[j][1],0,16);
}
memset(timebuff,0,sizeof(timebuff));
getcurtime(timebuff);                       //获取当前时间
//根据不同考勤类别给服务器发送不同数据
memset(bhbuf,0,128);
memset(mmbuf,0,128);
memset(khbuf,0,128);
memset(zwbuf,0,128);
//memset(xh,0,32);
if(strcmp(kaoqininfo.sbfangshi,"bh")==0)strcpy(bhbuf,kaoqininfo.sbbuff);
if(strcmp(kaoqininfo.sbfangshi,"kh")==0)strcpy(khbuf,kaoqininfo.sbbuff);
if(strcmp(kaoqininfo.sbfangshi,"xh")==0)strcpy(zwbuf,kaoqininfo.sbbuff);
if(strcmp(kaoqininfo.yzfangshi,"kh")==0)strcpy(khbuf,kaoqininfo.yzbuff);
if(strcmp(kaoqininfo.yzfangshi,"xh")==0)strcpy(zwbuf,kaoqininfo.yzbuff);
if(strcmp(kaoqininfo.yzfangshi,"mm")==0)strcpy(mmbuf,kaoqininfo.yzbuff);
memset(tmp1,0,sizeof(tmp1));
sprintf(tmp1,"svip=%s\r\ntime=%s\r\nqd=%s\r\nyz=%s\r\nfz=%s\r\nfx=%d\r\nbh=%s\r\nmm=%s\r\nkh=%s\r\nxh=%s\r\n",netinfo.netserverip,timebuff,"sb","yes","0",curdirection,bhbuf,mmbuf,khbuf,zwbuf);
//加密数据包内数据
i=getpicturename(NULL);  //获取照片序号.
memset(tmp2,0,sizeof(tmp2));
if(i>0&&videoflag2)
sprintf(tmp2,"jk=jk%06d\r\n",i);

strcat(tmp1,tmp2);
//printf("udp send= %s",tmp1);  
memset(sr,0,sizeof(sr)); 
encrypt(tmp1,data1.data,strlen(tmp1),user_data_len);
data1.datalen=strlen(data1.data);                 //命令长度
strcpy(data1.command,"infozd");                   //命令类型
  udp_send(&data1);                               //发送考勤数据到服务器
  memset(ts,0,sizeof(ts));
  memset(&data1,0,sizeof(data1));
  if(udp_recv(&data1,atoi(netinfo.timeout_server))==0)   //接收服务器相应
      if(udp_linezd(data1.data,data1.datalen)==0)        //服务器连接验证
        {
             if(strcmp(data1.command,"cortzd")!=0)  //如果接受的数据包类型不对，返回。
                  return -1;                       //此处不应直接返回，应进行其他处理
         }   
      else sprintf(ts,"%s,%s",kstr[218],kstr[219]);
  else { 
           sprintf(ts,"%s",kstr[220]);
           overtimeflag=1;
         } 
   len=0;
   //strncpy(ts,data1.data,32);
   memset(tmp1,0,sizeof(tmp1));
   decrypt(data1.data,tmp1,strlen(data1.data),1000);
   p1=tmp1;
  while(len<data1.datalen)//处理服务器相应数据
  {
//每次循环处理一行记录.记录格式一般为：命令=值
 // memset(tmp1,0,1000);
  memset(tmp2,0,64);
  memset(tmp3,0,64);
  memset(tmp4,0,64);
  i=strcspn(p1,"\n");
  strncpy(tmp2,p1,i);
  cut(tmp2);
  memset(bhbuf,0,128);
 //printf("%s,%s\n",tmp2,tmp1);
  p2=tmp2;
  j=strcspn(p2,"=");
  strncpy(tmp3,p2,j);//处理"="前字符,(命令）
  p2+=j;
  p2+=1;
  strcpy(tmp4,p2);   //处理"="后字符（值）
  p1+=i;
  p1+=1;
  len+=i;
  len+=1;
  if(strcmp(tmp3,"xs")==0) 
   xs++;                           //用以记录 xs字段出现的顺序
   for(j=0;j<CLONUMB;j++)
    {
      if(strcmp(tmp3,info[j][0])==0)
        {  
          memset(info[j][1],0,16);
          strcpy(info[j][1],tmp4);
         }
   
     }
// xs字段用以显示xm,bh等字段,表示的意义由顺序和档案文件头的对应顺序确定
   if(strcmp(tmp3,"xs")==0)
    {   
       for(i=0;i<CLONUMB;i++)
        {
          if(strlen(wddatitle[i][1])!=0)
             { 
               xmbh++;
               if(xmbh==xs)
                 {
                 memset(info[i][1],0,16);
                 strcpy(info[i][1],tmp4);
                 xmbh=0;
                 break ;
                 }  
             }
       
          }
    }
    
  if(strcmp(tmp3,"xh")==0)//保存序号
    {
        strcpy(xh,tmp4);
     }
  if(strcmp(tmp3,"ts")==0)//保存提示信息
     strcpy(ts,tmp4);
  if(strstr(tmp3,"mj"))
  {
  memset(mj,0,32);
  strcpy(mj,tmp4); 
  }
  if(strcmp(tmp3,"bh")==0)
  {
  memset(bhbuf,0,128);
  strcpy(bhbuf,tmp4);
  }
  if(strcmp(tmp3,"sy")==0)
  {
  memset(sy,0,sizeof(sy));
  strcpy(sy,tmp4);
  }
  if(strcmp(tmp3,"sr")==0)
  {
    memset(sr,0,sizeof(sr));
    //strcpy(sr,"Happy birthday!");
     strcpy(sr,tmp4);
    kaoqininfo.sr=atoi(tmp4);
  }
   //printf("get messages :%s\n",tmp2);

  }
   /此处应添加代码,用以处理有多个设备服务程序的情况:终端发送一次请求,收到多次应答,本部分函数读取一次应答数据后,其
    他应答数据将留在缓存内,下次读取时,可能读到不一致的数据.由于判断是否是本次请求的应答数据的指令尚不确定.待讨论确定后再添加/
  recordjl("q","",0,NULL);//写考勤记录

  show=fopen("/dev/shm/show.txt","w");	  //根据服务器数据作相应显示
  memset(menjininfo.lastxh,0,LEN);
  if(strlen(xh)==0)
   {
	  menjininfo.light=1;		//红灯
        fprintf(show,"err1\n%s\n%s",kaoqininfo.sbbuff,ts);
        fclose(show);
        if(overtimeflag==0)
          {
            if(strlen(sy))
               link_sound(sy);
            else
             menu_key("ye");
          }
        else menu_key("ec");
        videoflag2=0;   //考勤失败,不保存拍照.
	 if(udp_sendfileflag>=1)
        	view_video("e");
        menjininfo.lasttime=time(NULL);
    }
  else
   { 
	recordsuccess=1;
	strcpy(menjininfo.lastxh,xh);
	if(atoi(mj))		val_ok();          //门禁
	else 	val_err();
	menjininfo.light=1;		//绿灯
	fprintf(show,"ok1\n");
        for(j=0;j<CLONUMB;j++)
          {   
           if(strlen(wddatitle[j][1]))           //根据档案标题信息输出检索结果
              { 
                  if(strlen(info[j][1])==0)
                     fprintf(show,"\n");         //如果需要输出此列,且此列无数据则输出空行
                   else
                     fprintf(show,"%s\n",info[j][1]);
              }
            if(strcmp(info[j][0],"zp")==0)       //照片要最后输出
            strcpy(zp,info[j][1]);
             if(strcmp(info[j][0],"bh")==0)
               {
               memset(bhbuf,0,sizeof(bhbuf));
                 strcpy(bhbuf,info[j][1]);
               }
          }
	if(strlen(ts))
	fprintf(show,"%s\n",ts);
	else 
        fprintf(show,"%s %s %s %s %s\n",sr,q_direction[curdirection].text,kstr[222],kstr[186],timebuff);
        if(strcmp(zp,"1")==0)
           fprintf(show,"./note/photo/%d/%s.pht\n",atoi(xh)/1000,xh);
        else fprintf(show,".\n"); 
        
       if(cameradevinfo.cameraflag)
            fprintf(show,"%s\n","/dev/shm/view.jpg");
        else fprintf(show,".\n");
        
        if(strlen(sy))
          link_sound(sy);
      else
       if((!cameradevinfo.cameraflag)||(sysstatus.disksafe==1))
  	 sound_ok_music(bhbuf,NULL);
  	 else  sound_ok_music(bhbuf,"ka");
      fclose(show);
     	
      sync();    
   }
menjininfo.lasttime=time(NULL);
return 0;
}
*/
