#include "com.h"
#include "../_precomp.h"
#include "../crypt/crypt.h"
#include "../public/public.h"
#include "../public/initcom.h"
#include "../sound/sound.h"
#include "../finger/finger.h"
#include "../gpio/gpio.h"
/*
#include "../main_ht.h"
#include "../note/wdda.h"
#include "../note/diskopt.h"
#include "../note/finger_opt.h"
#include "../note/wdmj.h"
#include "../note/power.h"
*/ 
int dwload_dup=0,Dwload=0;//In order to avoid the repetition and loss,the value of itemnum in the packet will check whether this packet is the same as the former one
													//itemnum's range is 1-32.client will check it 
unsigned char lastcommand=0;
long lastpos=0;
//TCom serialport2=COM3;
static unsigned char  UPLOAD_ANSWER=0X42;
static  time_t starttime;
int com2_enable=0;

/*--------------------------------------------------------------------------*
@Function			:OpenCom - Initialize serial port
@Include      		: "com.h"
@Description			: port：serial port No.
				baudrate : serial band rate
				overtime : overtime
				set the serial port
@Return Value		: Success 0 .Failure -1
*---------------------------------------------------------------------------*/
/*
int OpenCom(int port, int baudrate, int overtime,int workmode)
{
	char id;
	int bianhao;

	bianhao=readbh();
	id=bianhao%1000;

	com=(enum _TCom)port;
	comovertime=overtime;
	if(start_com_io(com,baudrate)==0)
	{
		printf("set com error\n");
		return -1;
	}
	_set_work_mode((enum _TWorkMode)workmode);
	if(_set_address(id) == 0)
	{
		printf("set address error\n");
		return -1;
	}
	return 0;
}
*/
/*--------------------------------------------------------------------------*
@Function		: AcceptLinecom - wait for network to connect
@Include      	: "com.h"
@Description		: by polling way 
@Return Value	: Success 0 .Failure -1
*---------------------------------------------------------------------------*/
int AcceptLinecom()
{
	struct package data;
	int err,netfd=-1,lizezdsucceed=0;
	int err_code = -1;
	
	//printf("accept com\r\n");
	if(com2_enable==1)	return 2;
	//printf("recv com port %d\r\n",serialport2);
	err=comm_get_data(serialport2,&data);
	//err=get_data(serialport2,&data); //modify by aduo 2013.7.13
//	err=_get_data(serialport2,&data);
	switch (err)
	{
		case SUCCESS:
			time(&starttime);		
			new_linezd(&data,netfd,&lizezdsucceed);//is connect request valid and response
			if(lizezdsucceed==1)
			{
				com2_enable=1;
				err_code = 1;
			}
//			if(linezd_com(&data)==0){
//				com2_enable=1;
//			 	return 1;
//			}
			break;
		case TIME_OUT:
		case FAILURE: 
		case NOTMYADDRESS:
		case BEGINERROR:    
		case ENDERROR:
		case SUMERROR:
		case BCCERROR:
		default://return -1;
			err_code = -1;
			break;

 	}
	//return -1;
	return err_code;
}

/*--------------------------------------------------------------------------*
@Function		:linezd_com - link verification
@Include      	:com.h
@Description		:data : data packet
@Return Value	: Success 0,Failure -1
*---------------------------------------------------------------------------*/
int linezd_com(_TData *data)
{
	_TData answer;
	char *ptr,buffer[_USERDATALEN];

	if(data->instruction != LINEZD)
	{
		answer_put_data(ANSWER_LINE_ERR);
		return -1;
    	}

	memset(buffer,0,sizeof(buffer));
	memset(&answer,0,sizeof(answer));
	answer.address=_get_address();
	answer.instruction=LINEZD;
	answer.itemnum=1;
	if(link_valid((char *)data->user_data,data->nbytes)!=0)
	{
		sprintf(buffer,"linezd=0");
      		encrypt(buffer,(char *)answer.user_data,strlen(buffer),sizeof(answer.user_data));
		answer.nbytes=strlen((char *)answer.user_data);
		_put_data(serialport2,&answer);
		return -1;
	}
	else 
	{
		ptr=creat_pzinfo();
		memcpy((char *)answer.user_data,ptr,_USERDATALEN);
		answer.nbytes=strlen((char *)answer.user_data);
		_put_data(serialport2,&answer);
		return 0;
	}
	return 0;
}

/*--------------------------------------------------------------------------*
@Function		: ComSend - file transfer
@Include      	: "com.h"
@Description		: None
@Return Value	: Success 0 .Failure -1
*---------------------------------------------------------------------------*/
int ComSend()
{
	int flag=0;
//	user_data_len=_USERDATALEN;	//length
	user_data_len=NET_DATALEN;
	flag=com_sendfile();
	if(flag==-1){
		if(sendfileproperty.filesendflag!=0)
		{  
			if(sendfileproperty.sendfile!=NULL)
			{
				fclose(sendfileproperty.sendfile);
				sendfileproperty.sendfile=NULL;
			}  
		}
		memset(&sendfileproperty,0,sizeof(sendfileproperty));
		remove(tmpfilepath);
		com2_enable=0;		//if there are network connection
		ComPort_Clear(serialport2);
		calback->endback(downwddaflag);
		downwddaflag=0;
/*
		menu_key("ft");
		if(downwddaflag==1)   //如果下载档案文件,则重新加载档案,指纹
		{
			UninitFp();	
			if(InitFp("./lib/fp.so",1) == FALSE) // initialize fingerprint device
			{
				fpdevinfo.fpflag=0;
			}
			else
				fpdevinfo.fpflag=1;
			freerecord(headrecord);
			if((headrecord=init_wdda())==NULL)//读取档案文件配置信息
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

int get_data(TCom com,struct package *data); //accept the whole data
/*--------------------------------------------------------------------------*
@Function		: com_sendfile - file transfer
@Include      	: "com.h"
@Description		:by polling way.exit when over 100ms.
@Return Value	: Success  0 .Failure -1
*---------------------------------------------------------------------------*/
int com_sendfile()
{
	struct package data;
	 int overtime; 
	struct timeval tv1,tv2;
	int recvflag=0;



	gettimeofday(&tv1,NULL);
	overtime=(tv1.tv_sec % 10000) *1000+tv1.tv_usec/1000;
	//memset(&data,0,sizeof(_TData));
	memset(&data,0,sizeof(data));//modify by aduo 2013.7.18
	//while ((recvflag=get_data(serialport2,&data))>0||sendflag)		//receive data
	while ((recvflag=comm_get_data(serialport2,&data))>0||sendflag)		//receive data modify by aduo 2013.7.17
	{
		recvflag=Functions_Handle(data);
		if(recvflag==-2)	break;
		  //memset(&data,0,sizeof(_TData));
		  memset(&data,0,sizeof(data));//modify by aduo 2013.7.22
		  gettimeofday(&tv2,NULL);
		  if (abs( (tv2.tv_sec % 10000) *1000+tv2.tv_usec/1000 - overtime) > 100)
			break;
	 }

	if(recvflag==-1)
	{
	      memset(&data,0,sizeof(data));
/*
	      data.instruction=ANSWER;
	      data.itemnum=1;
	      data.user_data[0]=ANSWER_SUCCESS;
	      data.nbytes=1;
	      _put_data(serialport2,&data);
*/
		return -1;
	}
	return 0; 
}
/*
int com_sendfile()
{
	_TData data;
	 while (_get_data(serialport2,&data)==SUCCESS)
	 {
	  // printf("com_sendfile %02X\n",data.instruction);
	   switch(data.instruction)
	   {
	   case UPNAME:com_upname(&data);break;
	   case DWNAME:com_dwname(&data);break;
	   case UPLOAD:com_upload(&data);break;
	   case UPLOAD_DUP:com_upload_dup(&data);break;
	   case DWLOAD_DUP:com_dwload_dup(&data);break;
	   case DWLOAD:com_dwload(&data);break;
	   case RETIME:com_retime(&data);break;
	   case CLOSE:{
	      memset(&data,0,sizeof(data));
	      data.address=_get_address(serialport2);
	      data.instruction=ANSWER;
	      data.itemnum=1;
	      data.user_data[0]=ANSWER_SUCCESS;
	      data.nbytes=1;
	      _put_data(serialport2,&data);
	      return 0;
	     }
	   case LINEZD:linezd_com(&data);break;
	   case DELETE:com_delete(&data);break;
	   default:answer_put_data(ANSWER_DIMMEDINSTRUCTION);break;
	   }
	   lastcommand=data.instruction;
	 }
	 return 0;
}
*/
/*--------------------------------------------------------------------------*
@Function		: get_data - receive data
@Include      	: "com.h"
@Description		:
			com : serial No.
			data : packet will be received
			if no data was received with incomovertime，exit and close serial
@Return Value	: Success 1 ; there was no data return 0; overtime return -1.
*---------------------------------------------------------------------------*/
int get_data(TCom com,struct package *data)
{
	int error;


	if(abs(time(NULL)-starttime)>comovertime)
	{
		return -1;
	}

	//printf("recv com port %d\r\n",com);
	//memset(data,0,sizeof(_TData));
	memset(data,0,sizeof(struct package)); //modify by aduo 2013.7.12
	error=comm_get_data(com,data);
	switch(error){
		case SUCCESS:
			time(&starttime);		
			return 1;
		case TIME_OUT:
		break;
		case FAILURE: 
		break;      
		case SUMERROR:
//			answer_put_data(AMSWER_VERIFY_ERR);
		break;
		case BCCERROR:
//		      answer_put_data(AMSWER_VERIFY_ERR);
	      break;
	      default: 
		break;
	}
	return 0;
}

/*--------------------------------------------------------------------------*
@Function			: get_data - acknowledge instruction
@Include      		: "com.h"
@Description			:
				value : acknowledge content
				client send acknowledge instruction to server
@Return Value		: void
*---------------------------------------------------------------------------*/
void answer_put_data(unsigned char value)
{
	_TData answer;

	memset(&answer,0,sizeof(answer));
	answer.address=_get_address();
	answer.instruction=ANSWER;
	answer.itemnum=1;
	answer.user_data[0]=value;
	answer.nbytes=1;
	_put_data(serialport2,&answer);
}

/*--------------------------------------------------------------------------*
@Function			: new_com_upname - upload file attributes
@Include      		: "com.h"
@Description			:
				data : packet received
@Return Value		: void 
*---------------------------------------------------------------------------*/
void new_com_upname(_TData *data)
{
	struct stat filestat;
	char buf[32],buffer[_USERDATALEN];
	_TData answer;

	if(sendfileproperty.filesendflag!=0)
	{
		if(sendfileproperty.sendfile)
			fclose(sendfileproperty.sendfile);
		sendfileproperty.sendfile=NULL;
		memset(&sendfileproperty,0,sizeof(sendfileproperty));
	}

	memset(buffer,0,sizeof(buffer));
	decrypt((char *)data->user_data,buffer,strlen((char *)data->user_data),sizeof(buffer));
	if(new_pathtopath(buffer)!=0)
		uppathtopath(buffer);
	//if(atoi(deviceinfo.sysfinger)&&strstr(buffer,fingerfilesuffix))
	//      finger_linuxtowindows(buffer);
printf("buf=%s\n",buffer);
	if(testdir(buffer))
	{
	    sendfileproperty.sendfile=creatdirfile(buffer);
	  }
	else
	     sendfileproperty.sendfile=fopen(buffer,"r");

	if(sendfileproperty.sendfile==NULL)
	{
		   answer_put_data(ANSWER_INSTRUCTIONNOTRUN);
		   return ;
	 }

	 sendfileproperty.filesendflag=2;	//upload
	 fstat(fileno(sendfileproperty.sendfile),&filestat); 
	 memset(&answer,0,sizeof(answer));
	 answer.address=_get_address();
	 answer.instruction=NEW_UPNAME_ANSWER;
	 answer.itemnum=1;
	 memset(buf,0,sizeof(buf));
	 sprintf(buf,"%ld",filestat.st_size);
	 memset(buffer,0,sizeof(buffer));
	 encrypt(buf,buffer,strlen(buf),sizeof(buffer));	//crypt
	 sendfileproperty.totalcount=filestat.st_size;
	 strcpy((char *)answer.user_data,buffer);
	 answer.nbytes=strlen((char *)answer.user_data);
	 _put_data(serialport2,&answer);
	 UPLOAD_ANSWER=NEW_UPLOAD_ANSWER; 
 } 

/*--------------------------------------------------------------------------*
@Function			: com_upload - upload file content
@Include      		: "com.h"
@Description			:
				data : packet received
@Return Value		void .
*---------------------------------------------------------------------------*/
void com_upload(_TData *data)
 {
	 _TData answer;
	 long pos=0;
	 char buf[64];

	 memset(&answer,0,sizeof(answer));
	 if(sendfileproperty.filesendflag!=2)
	 {
		answer_put_data(ANSWER_INSTRUCTIONNOTRUN);
		return ;
	 }
	 memset(buf,0,64);
	 decrypt((char *)data->user_data,buf,strlen((char *)data->user_data),64);
	 pos=atol(buf);          
//	 printf("com_upload 2 %ld\n",pos);
	 fseek(sendfileproperty.sendfile,pos,SEEK_SET);
	 answer.nbytes=fread(answer.user_data, 1, _USERDATALEN,sendfileproperty.sendfile);
	 lastpos=sendfileproperty.curcount;
	  sendfileproperty.curcount+=answer.nbytes;
	  answer.instruction=UPLOAD_ANSWER;
	  answer.address=_get_address();
	  answer.itemnum=1;
	  _put_data(serialport2,&answer);
	  if(sendfileproperty.totalcount!=0)
	  {
		memset(buf,0,sizeof(buf));
		sprintf(buf,"%d\n",sendfileproperty.curcount*100/sendfileproperty.totalcount);
//		writets("ok1","num1",buf);
	  }
}

/*--------------------------------------------------------------------------*
@Function			: com_upload_dup - upload file content
@Include      		: "com.h"
@Description			:
				data : packet received
				if the upload packet lost or repeat，send the lost packet again
@Return Value		: void .
*---------------------------------------------------------------------------*/
void com_upload_dup(_TData *data)
 {
	_TData answer;
	long pos=0;
	char buf[64];

	memset(&answer,0,sizeof(answer));
	if(sendfileproperty.filesendflag!=2)
	{
	   answer_put_data(ANSWER_INSTRUCTIONNOTRUN);
	   return ;
	 }
	memset(buf,0,64);
	decrypt((char *)data->user_data,buf,strlen((char *)data->user_data),64);
	pos=atol((char *)data->user_data);    
	// printf("com_upload 2 %ld\n",pos);
	fseek(sendfileproperty.sendfile,pos,SEEK_SET);
 	answer.nbytes=fread(answer.user_data, 1, _USERDATALEN,sendfileproperty.sendfile);
	lastpos=sendfileproperty.curcount;
	sendfileproperty.curcount=lastpos+answer.nbytes;
	answer.instruction=UPLOAD_ANSWER;
	answer.address=_get_address();
	answer.itemnum=1;
	_put_data(serialport2,&answer);
}

/*--------------------------------------------------------------------------*
@Function			: new_com_dwname - download filename
@Include      		: "com.h"
@Description			:
				data : packet received						
@Return Value		: void
*---------------------------------------------------------------------------*/
void new_com_dwname(_TData *data)
{
	long len=0;
	char *ptr,filename[_USERDATALEN],buffer[_USERDATALEN];

	if(sendfileproperty.filesendflag!=0) //if there is file transfering ,close file.
	{
		 if(sendfileproperty.sendfile)
			   fclose(sendfileproperty.sendfile);
		 memset(&sendfileproperty,0,sizeof(sendfileproperty));
	}
	memset(filename,0,sizeof(filename));
	memset(&sendfileproperty,0,sizeof(sendfileproperty));
	memset(buffer,0,sizeof(buffer));

	decrypt((char *)data->user_data,buffer,strlen((char *)data->user_data),sizeof(buffer));
	ptr=strtok(buffer,"\n");
	if(ptr)
		strcpy(filename,ptr);
	ptr=strtok(NULL,"\n");
	if(ptr)
		len=atol(ptr);
	if(len>30000000)
	{
		 answer_put_data(ANSWER_INSTRUCTIONNOTRUN);
		 return ;
	}
	//printf("dwname, %s,\n%s,%ld\n",data->user_data,filename,len);

	if(new_pathtopath(filename)!=0)
		downpathtopath(filename);
	creatdir(filename);                   //if not exit,create it

	 if((sendfileproperty.sendfile=fopen(tmpfilepath,"w"))!=NULL)
	 {
		if(ptr&&ftruncate(fileno(sendfileproperty.sendfile),atol(ptr))==0)
		{
		     sendfileproperty.filesendflag=1;
		     strcpy(sendfileproperty.filename,filename);
		     sendfileproperty.totalcount=atol(ptr);
		     answer_put_data(ANSWER_SUCCESS);
		     dwload_dup=0;
		     Dwload=0;
		 }
		else 
		 {
		     answer_put_data(ANSWER_INSTRUCTIONNOTRUN);
		     fclose(sendfileproperty.sendfile);
		     sendfileproperty.sendfile=NULL;
		     remove(tmpfilepath);
		     return ;
		 }
	 }
	 else 
	 {
		answer_put_data(ANSWER_INSTRUCTIONNOTRUN);
		return ;
	 }	
}

/*--------------------------------------------------------------------------*
@Function			: com_dwload - download file name
@Include      		: "com.h"
@Description			:
				data : packet received
@Return Value		: void .
*---------------------------------------------------------------------------*/
void com_dwload(_TData *data)
{
	_TData answer;
	int num,senderr=0;
	char command[32];

//printf("%02X,%02X,%02X\n",data->nbytes,data->instruction,data->itemnum);
	if(sendfileproperty.filesendflag!=1)
	 {
		answer_put_data(ANSWER_INSTRUCTIONNOTRUN);
		return ;
	 }
	if(Dwload==data->itemnum)		//when lost，rewrite it
	{
		com_dwload_dup(data);
		return ;
	}
	Dwload=data->itemnum;
	memset(&answer,0,sizeof(answer));
	lastpos=sendfileproperty.curcount;	//record the size of the packet received
	fseek(sendfileproperty.sendfile,sendfileproperty.curcount,SEEK_SET);
	num=fwrite(data->user_data, 1,data->nbytes, sendfileproperty.sendfile);
	sendfileproperty.curcount+=num;

	if(num!=data->nbytes||sendfileproperty.curcount>sendfileproperty.totalcount)
	{
	      answer_put_data(ANSWER_INSTRUCTIONNOTRUN);
	      fclose(sendfileproperty.sendfile);
	      remove(tmpfilepath);
	      memset(&sendfileproperty,0,sizeof(sendfileproperty));
	      return;
	}

	if(sendfileproperty.totalcount!=0)
	{
		memset(command,0,32);
		sprintf(command,"%d\n",sendfileproperty.curcount*100/sendfileproperty.totalcount);
//		writets("ok1","num1",command);
	}
	if(sendfileproperty.curcount==sendfileproperty.totalcount)
	{
	     fclose(sendfileproperty.sendfile);
		senderr=mv_file(tmpfilepath,sendfileproperty.filename);
	     if(senderr==-1)
	       {
	         memset(&sendfileproperty,0,sizeof(sendfileproperty));
	         answer_put_data(ANSWER_INSTRUCTIONNOTRUN);
	         return ;
	         }     
		calback->netback(sendfileproperty.filename);
	     memset(&sendfileproperty,0,sizeof(sendfileproperty));
	 }
	answer_put_data(ANSWER_SUCCESS);
}

/*--------------------------------------------------------------------------*
@Function			: com_filetofile - move file
@Include      		: "com.h"
@Description			: after the file has downloaded ,move it from memory to disk
@Return Value		: Success 0,Failure -1 .
*---------------------------------------------------------------------------*/
int com_filetofile()
{
	mv_file(tmpfilepath,sendfileproperty.filename);	
//       net_back();
/*       if(atoi(deviceinfo.sysfinger)&&strstr(sendfileproperty.filename,fingerfilesuffix))
          return convert_windowstolinux(tmpfilepath,sendfileproperty.filename);
       else 
*/
	return 0;
}

/*--------------------------------------------------------------------------*
@Function			: com_dwload_dup - download file content
@Include      		: "com.h"
@Description			:
				data : packet received
				if the download packet lost or repeat，send the lost packet again
@Return Value		: void 
*---------------------------------------------------------------------------*/
void com_dwload_dup(_TData *data)
{
	_TData answer;
	int num;

//	printf(".............com_dwload_dup\n");
	if(sendfileproperty.filesendflag!=1)
	{
		 //Success should be send when terminal has recevied the packet and response to server,but it didn't receveid
		if(lastcommand==DWLOAD)
		{
		    lastcommand=DWLOAD;          //lastcommand has changed to DWLOAD_DUP ,so should reset to DWLOAD
		    answer_put_data(ANSWER_SUCCESS);
		}	
		else 
		   answer_put_data(ANSWER_INSTRUCTIONNOTRUN);
		return ;
	 }
	 dwload_dup++;
	 memset(&answer,0,sizeof(answer));
	 fseek(sendfileproperty.sendfile,lastpos,SEEK_SET);
	 num=fwrite(data->user_data, 1,data->nbytes, sendfileproperty.sendfile);
	 sendfileproperty.curcount=lastpos+num;
	 if(num!=data->nbytes)
	 {
	       answer_put_data(ANSWER_INSTRUCTIONNOTRUN);
      		 fclose(sendfileproperty.sendfile);
      		 memset(&sendfileproperty,0,sizeof(sendfileproperty));
      		 remove(tmpfilepath);
      		 return;
	 }	
	 answer_put_data(ANSWER_SUCCESS); 
}

/*--------------------------------------------------------------------------*
@Function			: com_retime - network timing
@Include      		: "com.h"
@Description			:
				data packet contains time data.the processing instruction is timing
@Return Value		: Success 0 Failure-1.
*---------------------------------------------------------------------------*/
int com_retime(_TData *data)
{
	int hour=0,min=0,sec=0,year=0,mon=0,day=0;
	char cmdbuf[1024];

	if(data==NULL)return -1;

	memset(cmdbuf,0,sizeof(cmdbuf));
	decrypt((char *)data->user_data,cmdbuf,(int)data->nbytes-1,1024);
	sscanf(cmdbuf,"%4d-%2d-%2d %2d:%2d:%2d",&year,&mon,&day,&hour,&min,&sec);//intercept time of server
	
	if(retime_all(year,mon,day,hour,min,sec)==0)
		answer_put_data(ANSWER_SUCCESS);
	else answer_put_data(ANSWER_INSTRUCTIONNOTRUN);
	return 0;
}

/*--------------------------------------------------------------------------*
@Function			: new_com_delete - delete file
@Include      		: "com.h"
@Description			:
				tmpbuff : data packet
				Remote deleting files
@Return Value		: void
*---------------------------------------------------------------------------*/
void new_com_delete(_TData *data)
{
	char tmpbuf[_USERDATALEN],buff[_USERDATALEN],*ptr,*out_ptr;

	if(sendfileproperty.filesendflag!=0)
	{
		if(sendfileproperty.sendfile)
			  fclose(sendfileproperty.sendfile);
		memset(&sendfileproperty,0,sizeof(sendfileproperty));
		remove(tmpfilepath);
	}

	memset(tmpbuf,0,sizeof(tmpbuf));
	decrypt((char *)data->user_data,tmpbuf,strlen((char *)data->user_data),sizeof(tmpbuf));
	ptr=tmpbuf;
	while((out_ptr=strtok(ptr,"\n"))!=NULL)
	{
		memset(buff,0,_USERDATALEN);
		strcpy(buff,out_ptr);
		if(new_pathtopath(buff)!=0)
			uppathtopath(buff);
		if(strstr(buff,"wdjl.wds"))
		{
		  	safe_rm(buff);               
		 }else 
			safe_rm(buff);
		  ptr=NULL;
	}
	answer_put_data(ANSWER_SUCCESS);
 return;
}

/*--------------------------------------------------------------------------*
@Function			: new_player - play audio remotely
@Include      		: "net_tcp.h"
@Description			: tmpbuff:data packet 
				  netfd: file description
@Return Value			: Success 0;
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int new_com_player(_TData *data)
{
	char tmp[1024],cmdbuf[1024];

	if(data==NULL)return -1;
	memset(tmp,0,sizeof(tmp));
	decrypt((char *)data->user_data,tmp,strlen((char *)data->user_data),sizeof(tmp));
	new_pathtopath(tmp);    //The path name format convert (windows->linux)
	cut(tmp);
	memset(cmdbuf,0,sizeof(cmdbuf));	
	sprintf(cmdbuf,"splay /%s &",tmp);
	system(cmdbuf);
	printf("splayer=%s\n",cmdbuf);
	answer_put_data(ANSWER_SUCCESS);
//	else answer_put_data(ANSWER_INSTRUCTIONNOTRUN);
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
int new_com_gpio(_TData *data)
{
	char cmdbuf[user_data_len];
	int flag=0;

	memset(cmdbuf,0,user_data_len);
	decrypt((char *)data->user_data,cmdbuf,strlen((char *)data->user_data),sizeof(cmdbuf));
	new_pathtopath(cmdbuf);    //The path name format convert (windows->linux)
	cut(cmdbuf);
printf("cmdbuf=%s\n",cmdbuf);
      	if(GpioOpt(atoi(cmdbuf))==TRUE)	flag=1;
	if(flag)
		answer_put_data(ANSWER_SUCCESS);
	else
		//answer_put_data(FAILURE);
	    answer_put_data(ANSWER_INSTRUCTIONNOTRUN); //modify by aduo 2013.6.29
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
int new_com_checkfp(_TData *data)
{
	char cmdbuf[user_data_len];
	int flag=0;

	memset(cmdbuf,0,user_data_len);
	decrypt((char *)data->user_data,cmdbuf,strlen((char *)data->user_data),sizeof(cmdbuf));
	new_pathtopath(cmdbuf);    //The path name format convert (windows->linux)
	cut(cmdbuf);
	flag=checkfile("./note/finger",cmdbuf);
	if(flag==0)
		flag=checkfile("./note/admi/finger",cmdbuf);
	if(flag)
		answer_put_data(ANSWER_SUCCESS);
	else
		//answer_put_data(FAILURE);
	    answer_put_data(ANSWER_INSTRUCTIONNOTRUN); //modify by aduo 2013.6.29
	return 0;
}
