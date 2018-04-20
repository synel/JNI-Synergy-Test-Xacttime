#include "initcom.h"
#include "../_precomp.h"
#include "../public/public.h"
#include "../printer/printer.h"

#include "../serial/serial.h"

TCom serialport0=COM1,serialport1=COM2,serialport2=COM3,serialport3=COM4;

int comovertime=30;
_TData com1value;

/*
//test use
int opencomm(int mode,int baud)
{
	char id=1,i=1;


		if(start_com_io((enum _TCom)i,baud)==0)
		{
			printf("set serialport1 error=%d\n",i);
			return FALSE;
		}
		_set_work_mode((enum _TCom)i,(enum _TWorkMode)mode);
	
		if(_set_address(id) == 0)
		{
			printf("set address error=%d\n",i);
			return FALSE;
		}
		serialport1=(enum _TCom)i;

	return TRUE;
}
*/
/*--------------------------------------------------------------------------*
@Function			:OpenCom - Initialize serial port
@Include      		: "com.h"
@Description			: port£ºserial port No.
				baudrate : serial band rate
				overtime : overtime  used during communication
				set the serial port
@Return Value		: Success 0 .Failure -1
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int OpenCom(IN COMINFO *com_info)
{
	char id;
	int bianhao=1,i=0,j;
	int baud[]= {2400,4800,9600,19200,38400,57600,115200};
#if defined _2410
	for (j = 0;j < 3; j++){
	  printf("serial %d peripheral type %d\r\n",j,com_info[j].workmode);
	  printf("serial %d default serial type %d\r\n",j,com_info[j].type);
	}
#elif defined _2416
	for (j = 0;j < 4; j++){
	  //printf("serial %d peripheral type %d\r\n",j,com_info[j].workmode);
	  //printf("serial %d peripheral model %d\r\n",j,com_info[j].workmode);
	  printf("serial %d default serial type %d\r\n",j,com_info[j].type);
	}
#endif

	if(com_info[i].enable==1)
	{
		if(com_info[i].workmode == 1)	//printer  port
		{
			OpenDev(i,baud[com_info[i].baudrate],8,1,'N');	//  initialize port
		}
		else
		{

#if defined _2410
			set_speed("/dev/ttyS0",baud[com_info[i].baudrate]);
#elif defined _2416
			set_speed("/dev/ttySAC0",baud[com_info[i].baudrate]);
#endif
			UnPrintCom();

		}
		printf("serial %d peripheral type %d\r\n",i,com_info[i].workmode);
		printdevinfo.printflag=com_info[i].workmode;	//com1
	}
	
#if defined _2410
	for(i=1;i<=2;i++)
	{
		if(com_info[i].enable==0)	continue;
		if(start_com_io((enum _TCom)i,baud[com_info[i].baudrate])==0)
		{
			printf("set comport error=%d\n",i);
		}
		_set_work_mode((enum _TCom)i,(enum _TWorkMode)com_info[i].workmode);
	
		bianhao=read_terminalno();
		id=bianhao;
		if(id==0X00||id==0XFF)	id=1;
		if(_set_address(id) == 0)
		{
			printf("set address error=%d\n",id);
		}
		if(i==2)		//485
		{
			serialport2=(enum _TCom)i;
			comovertime=com_info[i].overtime;
		}
		else if(i==1)		//card
			serialport1=(enum _TCom)i;
	}
#elif defined _2416
	printf("===================\n");
	for(i=1;i<=3;i++)
	{
		if(com_info[i].enable==0)	continue;
		if (com_info[i].type == 1) { //spi;
			if (start_com_io((enum _TCom) i + 10, baud[com_info[i].baudrate]) == 0) {
				printf("set comport error=%d\n", i + 10);
			}
			_set_work_mode((enum _TCom) i + 10,
					(enum _TWorkMode) com_info[i].workmode);
			bianhao = read_terminalno();
			id = bianhao;
			if (id == 0X00 || id == 0XFF)
				id = 1;
			if (_set_address(id) == 0) {
				printf("set address error=%d\n", id);
			}
		} else {
			if (start_com_io((enum _TCom) i, baud[com_info[i].baudrate]) == 0) {
				printf("set comport error=%d\n", i);
			}
			_set_work_mode((enum _TCom) i,
					(enum _TWorkMode) com_info[i].workmode);
			bianhao = read_terminalno();

			printf("read addr %d\n",bianhao);
			id = bianhao;
			if (id == 0X00 || id == 0XFF)
				id = 1;
			printf("my id %d\n",id);
			if (_set_address(id) == 0) {
				printf("set address error=%d\n", id);
			}
			if (i == 2) //485
					{
				serialport2 = (enum _TCom) i;
				comovertime = com_info[i].overtime;
			} else if (i == 3) //card
				serialport3 = (enum _TCom) i;
		}
	}
#endif
	return TRUE;

}

/*--------------------------------------------------------------------------*
@Function			: UnCom - close serial port
@Include      		: "com.h"
@Description			: release resource
						
@Return Value		: void		
*---------------------------------------------------------------------------*/
void UnCom(IN int comnum)
{

	switch(comnum){
		case 0:
			UnPrintCom();
		break;
		case 1:
		case 2:
		case 3:
			end_com_io((enum _TCom)comnum);
		break;
		default:
		break;
	}
}


//get com data
int ReadCom1(void)
{
	memset(&com1value,0,sizeof(com1value));
#if defined _2410
	if (_get_data(serialport1,&com1value)==SUCCESS)
	{
		clear_port(serialport1);	//clear com1 buffer
		return TRUE;
	} 
#elif defined _2416
	if (_get_data(serialport3 + 10,&com1value)==SUCCESS)
	{
		clear_port(serialport3 + 10);	//clear com1 buffer
		return TRUE;
	}
#endif
	return FALSE;
}

//sett terminal mode
int Set_Machine_Mode(IN int mode)
{		
	_TData answer;
	
	if(mode>255||mode<0)	return FALSE;
	memset(&answer,0,sizeof(answer));
	answer.address=0X01;
	
	answer.instruction=0X0F;		
	answer.nbytes=0X01;
	answer.itemnum=1;
	answer.user_data[0]=mode;
#if defined _2410
	answer.instruction=0X8F;
	if(_put_data(serialport1,&answer)!=SUCCESS)	return FALSE;
	//tcdrain(serialport1);
#elif defined _2416
	answer.instruction=0X0F;
	if(_put_data(serialport3 + 10,&answer)!=SUCCESS)	return FALSE;
	//tcdrain(serialport1 + 10);
#endif
//	ComPort_Clear(serialport1);	//clear com1 buffer
	return TRUE;		
}


int read_mac_com(char * mac)
{
//unsigned char data[10]={0x8d};
//int i=0;
_TData answer;
memset(&answer,0,sizeof(answer));
answer.address=0X01;
answer.instruction=0X8D;		
answer.nbytes=0X0;
answer.itemnum=1;
//printf("aaa\n");
_get_data(serialport1,&answer);
//printf("bbb\n");
if(_put_data(serialport1,&answer)!=SUCCESS)	return FALSE;
//printf("ccc\n");
usleep(500000);
memset(&answer,0,sizeof(answer));
if(_get_data(serialport1,&answer)==SUCCESS&&answer.instruction==0X8D)
{

sprintf((char *)mac,"%02X:%02X:%02X:%02X:%02X:%02X",answer.user_data[0],answer.user_data[1],answer.user_data[2],answer.user_data[3],answer.user_data[4],answer.user_data[5]);

return TRUE;

}
//printf("ddd  %02X\n",answer.instruction);
return FALSE;
}


int write_mac_com(char *mac)
{
//unsigned char data[10];
char tmp[3];
int i;
_TData answer;
memset(&answer,0,sizeof(answer));
answer.address=0X01;
answer.instruction=0X8E;		
answer.nbytes=0X6;
answer.itemnum=1;
printf("write_mac_com %s\n",mac);
for(i=0;i<6;i++)
{
memset(tmp,0,sizeof(tmp));
memcpy(tmp,mac+i*3,2);
answer.user_data[i]=strtoul(tmp,NULL,16);
}
//printf("%02X:%02X:%02X:%02X:%02X:%02X",answer.user_data[0],answer.user_data[1],answer.user_data[2],answer.user_data[3],answer.user_data[4],answer.user_data[5]);
if(_put_data(serialport1,&answer)==SUCCESS){
	//printf("_put_data TRUE\n");
}
//else printf("_put_data FALSE\n");
	tcdrain(serialport1);
if(_put_data(serialport1,&answer)==SUCCESS){
	printf("_put_data TRUE\n");
}
else {
	return FALSE;
	printf("_put_data FALSE\n");
	}
	tcdrain(serialport1);
return TRUE;
}


int set_track(int track) {
//unsigned char data[10]={0x33};
	int i = 0;
	_TData ask,answer;
	memset(&ask, 0, sizeof(ask));
	ask.address = 0X01;
	ask.instruction = 0X91;
	ask.nbytes = 0X03;
	ask.itemnum = 0x01;
	switch(track){
	case 1:
		ask.user_data[0] = 0x01;
		break;
	case 2:
		ask.user_data[0] = 0x02;
		break;
	default:
		ask.user_data[0] = 0x03;
		break;
	}

	/*
	 #if defined _2410
	 _get_data(serialport1, &answer);
	 #elif defined _2416
	 _get_data(serialport1 + 10, &answer);
	 #endif
	 */
#if defined _2410
	if (_put_data(serialport1, &ask) != SUCCESS)
	return FALSE;
#elif defined _2416
	if (_put_data(serialport3 + 10, &ask) != SUCCESS)
	return FALSE;
#endif
//printf("ccc\n");
	usleep(500000);
	memset(&answer, 0, sizeof(answer));
#if defined _2410
	printf("recv response\r\n");

	if (_get_data(serialport1, &answer) == SUCCESS
			&& answer.instruction == 0x91) {
			printf("answer.instruction==%02X\n",answer.instruction);
		for (i = 0; i < answer.nbytes; i++)
			printf(" %02X ",answer.user_data[i]);
		//sprintf(ver + i, "%c", answer.user_data[i]);
		//sprintf((char *)ver,"%02X:%02X:%02X:%02X:%02X:%02X",answer.user_data[0],answer.user_data[1],answer.user_data[2],answer.user_data[3],answer.user_data[4],answer.user_data[5]);
	}
#elif defined _2416
	if (_get_data(serialport3 + 10, &answer) == SUCCESS
			&& answer.instruction == 0x91) {
		//	printf("answer.instruction==%02X\n",answer.instruction);
		for (i = 0; i < answer.nbytes; i++)
			printf(" %02X ",answer.user_data[i]);
		//sprintf(ver + i, "%c", answer.user_data[i]);
		//sprintf((char *)ver,"%02X:%02X:%02X:%02X:%02X:%02X",answer.user_data[0],answer.user_data[1],answer.user_data[2],answer.user_data[3],answer.user_data[4],answer.user_data[5]);
	}
#endif

	return TRUE;
}



int get_keyboardVer(char *ver) {
//unsigned char data[10]={0x33};
	int i = 0;
	_TData answer;
	memset(&answer, 0, sizeof(answer));
	answer.address = 0X01;
	answer.instruction = 0X33;
	answer.nbytes = 0X03;
	answer.itemnum = 0x01;
	answer.user_data[0] = 0x33;
	/*
	 #if defined _2410
	 _get_data(serialport1, &answer);
	 #elif defined _2416
	 _get_data(serialport1 + 10, &answer);
	 #endif
	 */
#if defined _2410
	if (_put_data(serialport1, &answer) != SUCCESS)
	return FALSE;
#elif defined _2416
	if (_put_data(serialport3 + 10, &answer) != SUCCESS)
	return FALSE;
#endif
//printf("ccc\n");
	usleep(500000);
	memset(&answer, 0, sizeof(answer));
#if defined _2410
	if (_get_data(serialport1, &answer) == SUCCESS
			&& answer.instruction == 0x33) {
		//	printf("answer.instruction==%02X\n",answer.instruction);
		for (i = 0; i < answer.nbytes; i++)
		sprintf(ver + i, "%c", answer.user_data[i]);
		//sprintf((char *)ver,"%02X:%02X:%02X:%02X:%02X:%02X",answer.user_data[0],answer.user_data[1],answer.user_data[2],answer.user_data[3],answer.user_data[4],answer.user_data[5]);
	}
#elif defined _2416
	if (_get_data(serialport3 + 10, &answer) == SUCCESS
			&& answer.instruction == 0x33) {
		//	printf("answer.instruction==%02X\n",answer.instruction);
		for (i = 0; i < answer.nbytes; i++)
		sprintf(ver + i, "%c", answer.user_data[i]);
		//sprintf((char *)ver,"%02X:%02X:%02X:%02X:%02X:%02X",answer.user_data[0],answer.user_data[1],answer.user_data[2],answer.user_data[3],answer.user_data[4],answer.user_data[5]);
	}
#endif

	return TRUE;
}



//get card serial number
int mifs_request()
{
	static _TData data;

#if defined _2410
	if (_get_data(serialport1,&data)==SUCCESS)
#elif defined _2416
	if (_get_data(serialport3 + 10,&data)==SUCCESS)
#endif
	{
		if(data.instruction==READMF1CARD)
		{
			return TRUE;
		}
	}
	return FALSE;
}



/*
//_Adr --data address
this function will get 16bytes data when card pass verification
Sector:sector No.
Block:block No.
Akey:A-key
Bkey:B-key
out
data:
*/
int mifs_read(char *cardsno,char Sector, char Block, unsigned char *Data, char mode,char *key)
{
	_TData data;
	_TData answer;
	//char *ptr,buf[3];
	int i=0,j=0;
	struct timeval oldtime,newtime;
	int oldsec;
//printf("key=%s\n",key);
//	if(Sector>15||Sector<0||Block>3||Block<0)	return 1;
	memset(&answer,0,sizeof(answer));
	answer.address=0X01;
	if(key==NULL)
	{
		answer.instruction=0X82;
		answer.nbytes=2+4;
	}
	else
	{
		answer.instruction=0X80;
		answer.nbytes=14+4-6+1;
	}
	answer.itemnum=1;
	j=gsmString2Bytes((const char*)cardsno,&answer.user_data[i],strlen(cardsno));
	i+=j;
	answer.user_data[i++]=Sector;
	answer.user_data[i++]=Block;
	if(key!=NULL)
		answer.user_data[i++]=mode;
	if(key!=NULL)
	{
		j=0;
printf("key=%s,%d,%d,%s\n",key,Sector,Block,cardsno);
		j=gsmString2Bytes((const char*)key,&answer.user_data[i],strlen(key));
	}
#if defined _2410
	if(_put_data(serialport1,&answer)!=SUCCESS)	return FALSE;
#elif defined _2416
	if(_put_data(serialport3 + 10,&answer)!=SUCCESS)	return FALSE;
#endif
//usleep(50000);
	gettimeofday(&oldtime,NULL);
	oldsec=(oldtime.tv_sec % 10000) *1000+oldtime.tv_usec/1000;
	while(1){
		gettimeofday(&newtime,NULL);
		if (abs(((newtime.tv_sec % 10000) *1000+newtime.tv_usec/1000)-oldsec)>300)
		{
			return FALSE;//there is no card in 200ms
		}
		memset(&data,0,sizeof(data));
#if defined _2410
		if (_get_data(serialport1,&data)==SUCCESS)	//read data in block
#elif defined _2416
		if (_get_data(serialport3 + 10,&data)==SUCCESS)	//read data in block
#endif
		{
printf("read instruction:%02X\n",data.instruction);
			if(data.instruction==0X80)
			{
				memcpy(Data,data.user_data,16);
				return TRUE;
			}
		}
	}
	return FALSE;
}
//Ð´¿¨
int mifs_write(char *cardsno,char Sector, char Block, unsigned char *Data, char mode,char *key)
{
	_TData answer,data;
	//char *ptr,buf[3];
	int i=0,j=0;
	struct timeval oldtime,newtime;
	int oldsec;

//	if(strlen((char*)Data)!=16||Sector>15||Sector<0||Block>3||Block<0)	return 1;
	memset(&answer,0,sizeof(answer));
	answer.address=0X01;
	if(key==NULL)
	{
		answer.instruction=0X83;
		answer.nbytes=18+4;
	}
	else
	{
		answer.instruction=0X81;
		answer.nbytes=30+4-6+1;
	}
	answer.itemnum=1;
	j=gsmString2Bytes((const char*)cardsno,&answer.user_data[i],strlen(cardsno));
	i+=j;
	answer.user_data[i++]=Sector;
	answer.user_data[i++]=Block;
	if(key!=NULL)
		answer.user_data[i++]=mode;

	if(key!=NULL)
	{
		j=0;
		j=gsmString2Bytes((const char*)key,&answer.user_data[i],strlen(key));
		i+=j;
	}
//printf("%s,%d,%d,%d,%s\n",cardsno,Sector,Block,mode,key);
	memcpy(&answer.user_data[i],Data,16);
#if defined _2410
	if(_put_data(serialport1,&answer)!=SUCCESS)	return FALSE;
#elif defined _2416
	if(_put_data(serialport3 + 10,&answer)!=SUCCESS)	return FALSE;
#endif

	gettimeofday(&oldtime,NULL);
	oldsec=(oldtime.tv_sec % 10000) *1000+oldtime.tv_usec/1000;
	while(1){
		gettimeofday(&newtime,NULL);
		if (abs(((newtime.tv_sec % 10000) *1000+newtime.tv_usec/1000)-oldsec)>300)
		{
			printf("c\n");
			return FALSE;//there is no card in 200ms
		}
		memset(&data,0,sizeof(data));
#if defined _2410
		if (_get_data(serialport1,&data)==SUCCESS)	//read data in block
#elif defined _2416
		if (_get_data(serialport3 + 10,&data)==SUCCESS)	//read data in block
#endif
		{
//printf("write instruction:%02X,%02X\n",data.instruction,data.user_data[0]);
			if(data.instruction==0X00&&data.user_data[0]==0X00)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}
