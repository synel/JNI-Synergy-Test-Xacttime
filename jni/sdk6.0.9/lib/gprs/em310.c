/*
 * em310.c
 *
 *  Created on: 2013-7-10
 *      Author: aduo
 */

#include "gprs.h"

//response
static char *ipdata="%IPDATA:";
static char *ipsendx="%IPSENDX:";
static char *GPRS_ERROR="ERROR";
static char *GPRS_OK="OK";
static char *ipclose="%IPCLOSE:";
//request
//apn,session_mode,open_pdp_context
static char init_ptr[][128]={"at+cgdcont=1,\"ip\",\"cmnet\"","at%iomode=,2","at%etcpip"};

void em310_check_sim(){
	unsigned char buf[32];

	memset(buf,0,sizeof(buf));
	strcpy((char *)buf,"at%tsim\r\n");
	my_write(gprs_fd,buf,strlen((char *)buf));
	sleep(1);
}

void em310_get_apn() {
	static FILE *file = NULL;
	unsigned char buf[128];

	if (!file)
		file = fopen("./apn.ini", "r");
	if (!file)
		return;
	memset(buf, 0, sizeof(buf));
	if (fgets((char*) buf, sizeof(buf), file) == NULL ) {
		fclose(file);
		file = NULL;
		return;
	}
	cut((char *) buf);
	if (strlen((char*) buf))
		strcpy(init_ptr[0], (char*) buf);
	memset((char*) buf, 0, sizeof(buf));
	if (fgets((char*) buf, sizeof(buf), file) == NULL ) {
		fclose(file);
		file = NULL;
	}
	cut((char *) buf);
	if (strlen((char*) buf))
		strcpy(init_ptr[2], (char*) buf);
}

void em310_open_pdp_context() {
	unsigned char buf[128];

	memset(buf, 0, sizeof(buf));
	sprintf((char *) buf, "%s\r\n", init_ptr[2]);
	my_write(gprs_fd, buf, strlen((char *) buf));
}


void em310_reg_apn(){
	unsigned char buf[128];
	int i;

	for (i = 0; i <= 2; i++) {
		memset(buf, 0, sizeof(buf));
		sprintf((char *) buf, "%s\r\n", init_ptr[i]);
		printf("aa %s\n", buf);
		my_write(gprs_fd, buf, strlen((char *) buf));
		usleep(2000000);
	}
}

int em310_open_tcp_connect(int tongdao,char *remote_ip,int remote_port){
	char buf[256];

	memset(buf,0,sizeof(buf));
	if(tongdao==1)
		sprintf(buf,"at%%ipopenx=%d,\"tcp\",\"%s\",%d\r\n",
						tongdao,gprsset.deputyip,gprsset.deputyport);
/*	else if(tongdao==2)
		sprintf(buf,"at%%ipopenx=%d,\"udp\",\"%s\",%d\r\n",tongdao,"192.168.1.255",3350+1);
	else if(tongdao==3)
		sprintf(buf,"at%%ipopenx=%d,\"udp\",\"%s\",%d\r\n",tongdao,"192.168.1.255",3350+3);
*/
	return my_write(gprs_fd,(unsigned char *)buf,strlen(buf));
}


/*--------------------------------------------------------------------------*
@Function            	_gprs_send - send data through serial port
@Include      	gprs.h
@Description
			data : data will be send
			len : length of data being send
			tongdao : Link No.
			at%ipsendx=Link No. opened; data send;
			all the data will be send must be placed in quotes ""
			send data to opened TCP/UDP connection in multi-link mode
@Return Value		Success reurn the number of bytes send; Failure return -1
@Create time		2009-06-15 08:23
*--------------------------------------------------------------------------*/
static int _gprs_send(unsigned char *data,unsigned int len,int tongdao){
	char *ptr="at%ipsendx=";
	unsigned char buf[20480],src[40960];
	int num,count=0;

	if(gprs_bianhao[tongdao-1]!=1)	return -1;
	memset(buf,0,sizeof(buf));
	memset(src,0,sizeof(src));
	num=hex_2_ascii(data,buf,len);

	sprintf((char *)src,"%s%d,\"%s\"\r\n",ptr,tongdao,buf);
	num=strlen((char*)src);

	count=my_write(gprs_fd,src,num);

	if(count!=num)
		return -1;
	else return len;
}


/*--------------------------------------------------------------------------*
@Function            	get_gprs_data - receive data
@Include      	gprs.h
@Description
			data : data received
@Return Value		Success reurn the number of bytes received; Failure return -1
@Create time		2009-06-15 08:23
*--------------------------------------------------------------------------*/
static int _gprs_recv(unsigned char *data){
	int num=0,i=0,j=0,/*status=0,*/count1=0,count2=0;
	static unsigned char  buf[40960];
	static int pos=0;	//datasize in buffer

	if(sizeof(buf)-pos>0)	//if buffer is not full,read data.

	num=my_read(gprs_fd,buf+pos,sizeof(buf)-pos);

	pos+=num;
	if(pos<=0)		return -1;
	num=strcspn((char*)buf,"\r\n");
	if(num==pos)
	{
		if(pos!=(count1=strcspn((char *)buf,"\""))&&
			(pos-count1-1)!=(count2=strcspn((char *)buf+count1+1,"\"")))// "" appear
		{
			num=count1+count2+2;
			memcpy(data,buf,num);   //one line data read
			for(i=num,j=0;i<pos;i++,j++)  //move data of public buffer
				buf[j]=buf[i];
			pos=pos-num;			//datasize in buffer
			return num;
		}
		return -1;
	}

	//if still no "\r\n" are read.then
	memcpy(data,buf,num+2);   //one line data read
	data[num]='\0';		//cut "\r\n"

	for(i=num+2,j=0;i<pos;i++,j++)  //move data of public buffer
		buf[j]=buf[i];
	pos=pos-num-2;			//datasize in buffer
	return num;
}


/*--------------------------------------------------------------------------*
@Function            	gprs_recv - receive data
@Include      	gprs.h
@Description
			data : save data reveived
			len : lenght of data
			tongdao : Link No.
			read data of buffer through serial port and check the type
@Return Value		Success reurn the number of bytes received; Failure return -1
								No data read return 0
@Create time		2009-06-15 08:23
*--------------------------------------------------------------------------*/
int em310_gprs_recv(unsigned char *data,int len,int tongdao)
{
	char *gprs_connect="CONNECT";
	char *gprs_error="ERROR:";
	char *gprs_csq="+CSQ:";
	char *gprs_tsim="%TSIM";

	unsigned char buf[20480],tmp[20480];
	static char buffer[3][10240];//Receiving buffer of 3 channels
	static int pos[3];
	int num=0,n=0;

	memset(buf,0,sizeof(buf));
	if(_gprs_recv(buf)>0)
	{
		memset(tmp,0,sizeof(tmp));
//		printf("buf= %.25s\n",(char*)buf);
		if(strncmp((char *)buf,ipclose,strlen(ipclose))==0)
		{
			strcpy((char *)data,(char *)buf);
			n=atoi((char*)buf+strlen(ipclose));
			switch(n)
			{
				case 5:
					gprs_tongdao_count=-1;
					memset(gprs_bianhao,0,sizeof(gprs_bianhao));
					pos[0]=0;
					break;
				case 1:
					gprs_bianhao[0]=0;
					gprs_tongdao_count=1;
					pos[0]=0;
					break;
				case 2:
					gprs_bianhao[1]=0;
					gprs_tongdao_count=2;
					break;
				case 3:
					gprs_bianhao[2]=0;
					gprs_tongdao_count=3;
					break;
			}
			oldtimer.tv_sec=0;
			return -1;
		}
		if(strncmp((char*)buf,(char *)gprs_error,strlen((char *)gprs_error))==0)//ERROR
		{
			strcpy((char *)data,(char *)buf);
			if(strlen((char*)buf)==strlen((char*)gprs_error))
				return -1;
			n=atoi((char*)buf+strlen((char *)gprs_error));
			switch(n)
			{
				case 0:
					em310_get_apn();
					em310_reg_apn();
					break;
				case 1:
					gprs_tongdao_count=-1;
					oldtimer.tv_sec=0;
					break;
				case 21:
					gprs_tongdao_count=-1;
					oldtimer.tv_sec=0;
					break;
				case 5:
				case 4:{
					oldtimer.tv_sec=0;
					break;
					}
				case 2:{
					switch(gprs_tongdao_count)
					{
						case 1:
							gprs_bianhao[0]=1;
							gprs_write_mac();
							break;
						case 2:
							gprs_bianhao[1]=1;
							break;
						case 3:
							gprs_bianhao[2]=1;
							break;
					}
					oldtimer.tv_sec=0;
					break;
				}
				case 20:
					return -1;
			}
		}//if
		if(strncmp((char *)buf,gprs_connect,strlen(gprs_connect))==0)//CONNECT
		 {
		 	gprs_bianhao[gprs_tongdao_count-1]=1;
		 	if(gprs_tongdao_count==1)
			{
				gprs_write_mac();
			}
			sleep(1);
			oldtimer.tv_sec=0;
			return 0;
		 }

		if(strncmp((char *)buf,GPRS_OK,strlen(GPRS_OK))==0){
			if(gprs_tongdao_count==0)// fail to register
			{
				gprs_tongdao_count=2;  //register the first channel
				oldtimer.tv_sec=0;
				sleep(1);
				return -1;
			}
			if(tongdao==0&&data)
				strcpy((char *)data,(char *)buf);
			else return -1;
			return strlen(ipsendx);
		}

		if(strncmp((char *)buf,ipsendx,strlen(ipsendx))==0){
			if(tongdao==0&&data)
				strcpy((char *)data,(char *)buf);
			else return -1;
			return strlen(ipsendx);
		}

		if(strncmp((char *)buf,ipdata,strlen(ipdata))==0)  //data come
		{
			reset_recv_time();
			memset(tmp,0,sizeof(tmp));
			if(sscanf((char*)buf,"%*[^:]:%d,%d,\"%[^\"]",&n,&num,tmp)==3)
			{
				if(n==1||n==2||n==3)
				{
					gprs_bianhao[n-1]=1;
					pos[tongdao-1]+=ascii_2_hex(tmp,
						(unsigned char*)buffer[n-1]+pos[tongdao-1],num*2);
				}
			}
			//printf("buffer=%s\n",buffer);
		}
		//check signal intensity£¬ MAX 31£¬MIN 0
		if(strncmp((char*)buf,gprs_csq,strlen(gprs_csq))==0)
		{
			sscanf((char*)buf,"%*[^:]:%d,%d",&num,&n);
			gprsinfo.rssi=num;
			memset(buf,0,sizeof(buf));
			sprintf((char *)buf,"%d",num);
		}
		//check whether SIM exit
		if(strncmp((char*)buf,gprs_tsim,strlen(gprs_tsim))==0)
		{
			gprsinfo.tsim=1;
		}
}//if(get_gprs_data(buf)>0)

	if(pos[0]>0&&strncmp(buffer[0],"MAC=OK",6)==0)
	{
		memmove(buffer[0],buffer[0]+6,pos[0]-6);
		pos[0]=pos[0]-6;
	}

	if(tongdao>=1&&tongdao<=3)
	{
		num=(pos[tongdao-1]>len?len:pos[tongdao-1]);
		if(num>0)
		{
			memcpy(data,buffer[tongdao-1],num);
			memmove(buffer[tongdao-1],buffer[tongdao-1]+num,pos[tongdao-1]-num);
			pos[tongdao-1]=pos[tongdao-1]-num;
			return num;
		}
	}
	return 0;
}


int em310_gprs_send(unsigned char *data,unsigned int len,int tongdao)
{
	struct timeval oldtimer,newtimer;
	unsigned char buf[128];
	int num=0;

	gettimeofday(&oldtimer,NULL);
	gettimeofday(&newtimer,NULL);
	if(_gprs_send(data,len,tongdao)<=0)	return -1;
	while(abs(newtimer.tv_sec-oldtimer.tv_sec)<= 30)
	{
		gettimeofday(&newtimer,NULL);
		if((em310_gprs_recv(buf,0,0))==0)	continue;
		printf("gprs_send %.25s\n",(char*)buf);
		if(strncmp((char *)buf,ipsendx,strlen(ipsendx))==0)
		{
			sscanf((char *)buf,"%*[^:]:%*[^,],%d",&num);
			if(num<10)		sleep(1);//when the capacity is less than10,slow sending speed
			return len;
		}
		if(strncmp((char *)buf,GPRS_ERROR,strlen(GPRS_ERROR))==0)		{return -1;}
		if(strncmp((char *)buf,ipclose,strlen(ipclose))==0)			{return -1;}
	}
	return -1;
}


void em310_close_all_tcp_connect(){
	char buf[256];
	int i=0;

	memset(buf,0,sizeof(buf));
	for(i=3;i>=1;i--)
	{
		sprintf(buf,"at%%ipclose=%d\r\n",i);
		my_write(gprs_fd,(unsigned char *)buf,strlen(buf));
		usleep(200000);
	}
}


void em310_close_tcp_connect(int tongdao)
{
	char buf[256];

	memset(buf,0,sizeof(buf));
	sprintf(buf,"at%%ipclose=%d\r\n",tongdao);
	my_write(gprs_fd,(unsigned char *)buf,strlen(buf));
}
