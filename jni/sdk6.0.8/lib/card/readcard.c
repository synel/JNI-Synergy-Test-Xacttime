#include<ctype.h>
#include "readcard.h"
#include "../public/protocol.h"
#include "../public/initcom.h"
#include "../public/public.h"
#include "../weigand/weigand.h"

struct card_INFO cardinfo;		//card-reader info
struct R_card_INFO ReadCardInfo;		//card-reader info
struct W_card_INFO WriteCardInfo;		//card-reader info
//int track1valid = 0; //added by chaol to track mag card track record


/*--------------------------------------------------------------------------*
@Function          :InitCard - initialize card-reader parameters
@Include      	:"readcard.h"
@Description		:void
@Return value	: Failure FALSE; Success TRUE	
*---------------------------------------------------------------------------*/
int InitCard(struct card_INFO *card_info)
{
	cardinfo=*card_info;
	return TRUE;	
}

/*--------------------------------------------------------------------------*
@Function          :InitCard_Read - initialize card-reader parameters
@Include      	:"readcard.h"
@Description		:void
@Return value	: Failure FALSE; Success TRUE	
*---------------------------------------------------------------------------*/
int InitCard_Read(struct R_card_INFO *R_cardinfo)
{
	ReadCardInfo=*R_cardinfo;
	return TRUE;	
}

/*--------------------------------------------------------------------------*
@Function          :InitCard_Write - initialize card-reader parameters
@Include      	:"readcard.h"
@Description		:void
@Return value	: Failure FALSE; Success TRUE	
*---------------------------------------------------------------------------*/
int InitCard_Write(struct W_card_INFO *W_cardinfo)
{
	WriteCardInfo=*W_cardinfo;
	return TRUE;	
}

/*--------------------------------------------------------------------------*
@Function          :ReadCard - get data
@Include      	:"readcard.h"
@Description		:
			char *cardnum:card No.
			judge what type of data was received form com1
			(as data form keypad and all card-reader are all transferred through com1,so it's necessary to distinguish)
@Return value	:
Failure 			0;
read magnetic failure	-1;
read barcode crypt failure	-2;
read barcode failure		-3;
read magnetic crypt failure	-4;
unknown error			-5;
read EM crypt failure	-6;
read EM failure		-7;
read MF1 crypt failure	-8;
read MF1 failure		-9;
read HID crypt failure	-10;
read HID failure		-11;

Success card type	>0
*---------------------------------------------------------------------------*/
int ReadCard(char *cardnum)
{
	unsigned char *card=NULL,buf[32];
	static char sno[128];
	unsigned int wgdata=0;
	int i=0,len=0;
	
	memset(sno, 0, sizeof(sno));
	switch(com1value.instruction)
	{
		case READHID:
		case 0x97: //add by aduo 2013.7.9
			if((int)com1value.itemnum==0x0)
			{
				if(com1value.user_data[0]==0x00)
					return -10;
				else if(com1value.user_data[0]==0x01)
					return	-11;
				else return -5;
			}
			card = com1value.user_data;
			len=com1value.nbytes;
			cardnum[0]=len;
			memcpy(cardnum+1,com1value.user_data,len);

			//for (i = 0;i < len;i++){
				//printf("%02x ",com1value.user_data[i]);
			//}
			//printf("\n");

			return com1value.instruction;

		case READEMCARD:
			if((int)com1value.itemnum==0x0)
			{
				if(com1value.user_data[0]==0x00)
					return -8;
				else if(com1value.user_data[0]==0x01)
					return	-9;
				else return -5;
			}
			card = com1value.user_data;
			sprintf(sno, "%02X%02X%02X%02X%02X",*card++,*card++,*card++,*card++,*card++);
//			p=card_handle(sno);
		break;
		case READMF1CARD:
			if((int)com1value.itemnum==0x0)
			{
				if(com1value.user_data[0]==0x00)
					return -6;
				else if(com1value.user_data[0]==0x01)
					return	-7;
				else return -5;
			}
			card = com1value.user_data;
			memset(buf,0,sizeof(buf));
			sprintf(buf, "%02X%02X%02X%02X",*card++,*card++,*card++,*card++);
			//sprintf(sno, "%02X%02X%02X%02X",*card++,*card++,*card++,*card++);
			//printf("card no: %s\r\n",buf);
			//sprintf((char *)sno,"%l",strtoul((const char *)buf,NULL,16));
			sprintf((char *)sno,"%u",strtoul((const char *)buf,NULL,16));//modify by aduo 2013.7.8
			//printf("sno %s\r\n",sno);
//			
		break;
		case READMAGNETIC:
			if((int)com1value.address==cardinfo.track){
			//debug info by chaol
			//printf("num of tracks : %d\n",cardinfo.track);
			//printf("card type : %d\n",cardinfo.cardtype);
			//printf("card _right: %d\n",cardinfo.card_right);
			//printf("card _10: %d\n",cardinfo.card_10);
			//printf("card _fs: %d\n",cardinfo.card_fs);
			//printf("card _right10: %d\n",cardinfo.card_right10);
			//printf("address: 0x%02X\n",com1value.address);
			//printf("nbytes: %d\n",com1value.nbytes);
			//printf("instruction: %d\n",com1value.instruction);
			//printf("itemnum: %d\n",com1value.itemnum);
				if((int)com1value.itemnum==0x0)
				{
					if(com1value.user_data[0]==0x00)
						return -4;
					else if(com1value.user_data[0]==0x01)
						return	-1;
					else return -5;
				}
				memset(sno, 0, sizeof(sno));
				len=(int)com1value.nbytes;
				//added by chaol
				/*if (com1value.address == 0x1) {
						track1valid = 1;
				} */
				
				//if ((com1value.address == 0x1)  || (com1value.address == 0x2 && track1valid != 1)){
					//printf("track1valid: %d, address #: %d , nbytes: %d\n",track1valid, com1value.address, len);
					int i = 0;
					int j;
					if (com1value.address == 0x1) {
						j = i+1; //first track, skip the format code FC
					} else {
						j = i; //second track has no format code FC
					}
					while(isxdigit(com1value.user_data[j]))
					{
						sno[i]=(int)com1value.user_data[j];
						i++;
						j++;
					}
				}// else {
					//clear the flag on second pass:
				//	track1valid = 0;
				//	return 0;
				//}
		break;
		case READBARCODE:
				if((int)com1value.user_data[0]==0x0&&(int)com1value.user_data[1]==0x0)
					return	-2;
				else if((int)com1value.user_data[0]==0x0&&(int)com1value.user_data[1]==0x1)
					return -3;
				memset(sno, 0, sizeof(sno));
				len=(int)com1value.nbytes;
				i=0;
				len--;
				while(len--)
				{
					sno[i]=(int)com1value.user_data[i+1];
					i++;
				}
				break; //add by aduo 2013.6.22
		default:
/*			wgdata=ReadWigan();
			if(wgdata<=0)	break;
			sprintf(sno,"%06X",(wgdata<<7)>>8);	//cut the first and the last bytes
			if(cardinfo.card_right!=0) 	//cut the card number according to setting
			{
				memset(sno,0,sizeof(sno));
				sprintf(sno,"%10X",(wgdata<<7)>>8);//cut the first and the last bytes
				right(sno,cardinfo.card_right);
			}
			com1value.instruction=WEIGANDCARD;
*/
			break;
	}
	if(sno==NULL||strlen(sno)==0)	return com1value.instruction;
	//printf("p=%s\n",sno);
	memcpy(cardnum,sno,strlen(sno));
	return com1value.instruction;
}

//8+6 bits  -->3+5 Bytes
int ReadWigan26(struct mem_inout* wg_data)
{
unsigned int count1=0,count2=0;
unsigned int i,begin=0,end=0,c=0;
//read
if(ReadWigan(26,wg_data)!=0)return -1;
//printf("wg26  success\n");
//even-odd check
for(i=0;i<26;i++)
{
c=1<<i;
if(c&wg_data->d[0]){
		if(i==0)begin=1;
		else if(i<13)count1++;
		else if(i<25)count2++;
		else if(i==25)end=1;
	}

}

//cut Hight and Low bit
wg_data->d[0]=(wg_data->d[0]&0X1FFFFFE)>>1;

if(count1%2!=begin&&count2%2==end)return 0;
else return -1;
}
int ReadWigan27(struct mem_inout* wg_data)
{

//read
if(ReadWigan(27,wg_data)!=0)return -1;
return -1;
//check

}

int ReadWigan32(struct mem_inout* wg_data)
{

//read
if(ReadWigan(32,wg_data)!=0)return -1;
return -1;
//check

}


int ReadWigan33(struct mem_inout* wg_data)
{

//read
if(ReadWigan(33,wg_data)!=0)return -1;
return -1;
//check

}

int ReadWigan34(struct mem_inout* wg_data)
{

//read
if(ReadWigan(34,wg_data)!=0)return -1;
return -1;
//check
}

int ReadWigan36(struct mem_inout* wg_data)
{

//read
if(ReadWigan(36,wg_data)!=0)return -1;
return -1;
//check

}


int ReadWigan37(struct mem_inout* wg_data)
{

//read
if(ReadWigan(37,wg_data)!=0)return -1;
return -1;
//check

}

int ReadWigan40(struct mem_inout* wg_data)
{

//read
if(ReadWigan(40,wg_data)!=0)return -1;
return -1;
//check

}

int ReadWigan86(struct mem_inout* wg_data)
{

//read
if(ReadWigan(86,wg_data)!=0)return -1;
return -1;
//check

}

char * wg26_0()
{
static  char buf[9];
#if defined _2410
struct mem_inout data;
if(ReadWigan26(&data)!=0)return NULL;
memset(buf,0,sizeof(buf));
data.d[0]=data.d[0]&0xFFFFFF;
sprintf((char *)buf,"%03d%05d",(data.d[0]&0XFF0000)>>16,data.d[0]&0XFFFF);
#elif defined _2416
  char card_num[9];
  char temp1[3],temp2[5];
  int err_code = 0;
  int a,b;
  memset(card_num,0,sizeof(card_num));
  err_code = read_wiegand(card_num);
  if (err_code == 1){
	  memset(temp1,0,sizeof(temp1));
	  memset(temp2,0,sizeof(temp2));
	  memcpy(temp1,card_num,2);
	  memcpy(temp2,card_num + 2,strlen(card_num) - 2);
	  a = strtoull(temp1,NULL,16);
	  b = strtoull(temp2,NULL,16);
	  sprintf((char *)buf,"%03d%05d",a,b);
  }
#endif
return buf;
}

//3+3+18 bits -->1+1+6 Byte
 char * wg26_1()
{
static  char buf[9];
#if defined _2410
struct mem_inout data;
if(ReadWigan26(&data)!=0)return NULL;
memset(buf,0,sizeof(buf));
data.d[0]=data.d[0]&0xFFFFFF;
sprintf(buf,"%d%d%06d",(data.d[0]&0XE00000)>>21,(data.d[0]&0X1C0000)>>18,data.d[0]&0X3FFFF);
#elif defined _2416
char card_num[9];
char temp1[3],temp2[5];
int err_code = 0;
int a;
memset(card_num,0,sizeof(card_num));
err_code = read_wiegand(card_num);
if (err_code == 1){
	  a = strtoull(card_num,NULL,16);
	  sprintf(buf,"%d%d%06d",(a&0XE00000)>>21,(a&0X1C0000)>>18,a&0X3FFFF);
}
#endif
return buf;
}


//4+20 bits --> 1+7 Byte   ??  4bits ->1111 -->15(2Byte)

char * wg26_2()
{
static  char buf[9];
#if defined _2410
struct mem_inout data;

memset(buf,0,sizeof(buf));
if(ReadWigan26(&data)!=0)return NULL;
memset(buf,0,sizeof(buf));
data.d[0]=data.d[0]&0xFFFFFF;
sprintf(buf,"%02u%07u",(data.d[0]&0xF00000)>>20,data.d[0]&0xFFFFF);
#elif defined _2416
char card_num[9];
char temp1[3],temp2[5];
int err_code = 0;
int a;
memset(card_num,0,sizeof(card_num));
err_code = read_wiegand(card_num);
if (err_code == 1){
	  a = strtoull(card_num,NULL,16);
	  sprintf(buf,"%02u%07u",(a&0xF00000)>>20,a&0xFFFFF);
}
#endif
return buf;
}

//24bits  -->24Byte
 char * wg26_3()
{
static  char buf[9];
#if defined _2410
struct mem_inout data;
if(ReadWigan26(&data)!=0)return NULL;
memset(buf,0,sizeof(buf));
data.d[0]=data.d[0]&0xFFFFFF;
sprintf(buf,"%08d",data.d[0]);
#elif defined _2416
char card_num[9];
char temp1[3],temp2[5];
int err_code = 0;
int a;
memset(card_num,0,sizeof(card_num));
err_code = read_wiegand(card_num);
if (err_code == 1){
	  a = strtoull(card_num,NULL,16);
	  sprintf((char *)buf,"%08d",a);
}
#endif
return buf;
}

char * wg27_0()
{
static  char buf[10];
struct mem_inout data;
if(ReadWigan27(&data)!=0)return NULL;

memset(buf,0,sizeof(buf));
data.d[0]=data.d[0]&0x1FFFFFF;
sprintf(buf,"%1d%03d%05d",(data.d[0]&0X1000000)>>24,(data.d[0]&0xFF0000)>>16,data.d[0]&0XFFFF);
return buf;
}

//15+15 bits -->5+5=10 Bytes
char * wg32_0()
{
static  char buf[12];
struct mem_inout data;
if(ReadWigan27(&data)!=0)return NULL;

memset(buf,0,sizeof(buf));
data.d[0]=data.d[0]&0x3FFFFFFF;
sprintf(buf,"%05d%05d",data.d[0]>>15,data.d[0]&0x7FFF);
return buf;
}

//9+21  bits --->3+7=10 Bytes
char * wg32_1()
{
static  char buf[12];
struct mem_inout data;
if(ReadWigan27(&data)!=0)return NULL;

memset(buf,0,sizeof(buf));
data.d[0]=data.d[0]&0x1FFFFFF;
sprintf(buf,"%03d%07d",data.d[0]>>21,data.d[0]&0X1FFFFF);
return buf;
}
//7+24 bits --> 2+8 Byte
char * wg33_0()
{
static  char buf[16];
struct mem_inout data;
if(ReadWigan33(&data)!=0)return NULL;

memset(buf,0,sizeof(buf));
data.d[0]=data.d[0]&0x7FFFFFFF;
sprintf(buf,"%02d%08d",data.d[0]>>24,data.d[0]&0X7F);
return buf;
}

//10+21 bits -->4+7 Byte
char * wg33_1()
{
static  char buf[16];
struct mem_inout data;
if(ReadWigan33(&data)!=0)return NULL;

memset(buf,0,sizeof(buf));
data.d[0]=data.d[0]&0x7FFFFFFF;
sprintf(buf,"%04d%07d",data.d[0]>>21,data.d[0]&0X3FF);
return buf;
}
//32 bits --> 10 Byte
char * wg34_0()
{
static  char buf[16];
struct mem_inout data;
if(ReadWigan34(&data)!=0)return NULL;

memset(buf,0,sizeof(buf));
sprintf(buf,"%010d",data.d[0]);
return buf;
}

//16+16bits --> 5+5 Byte 
char * wg34_1()
{
static  char buf[16];
struct mem_inout data;
if(ReadWigan34(&data)!=0)return NULL;

memset(buf,0,sizeof(buf));
sprintf(buf,"%05d%05d",data.d[0]>>16,data.d[0]&0XFFFF);
return buf;
}

//8+4+20 bits -->3+2+7 Byte
char * wg34_2()
{
static  char buf[16];
struct mem_inout data;
if(ReadWigan34(&data)!=0)return NULL;

memset(buf,0,sizeof(buf));
sprintf(buf,"%03d%02d%07d",data.d[0]>>24,(data.d[0]>>20)&0XF,data.d[0]&0XFFFFF);
return buf;
}

//
char * wg36_0()
{
static  char buf[16];
struct mem_inout data;
if(ReadWigan36(&data)!=0)return NULL;
return NULL;
return buf;
}
//16+16+2 bits -->5+5+1=11 Byte
char * wg36_1()
{
static  char buf[16];
struct mem_inout data;
unsigned int tmp=0;
if(ReadWigan36(&data)!=0)return NULL;
data.d[1]=data.d[1]&0x3;
tmp=(data.d[1]<<14)&(data.d[0]>>18);
memset(buf,0,sizeof(buf));
sprintf(buf,"%05d%05d%01d",tmp,(data.d[0]>>2)&0XFFFF,data.d[0]&0X3);
return buf;
}
//14+20 bits --> 5+7 =12 Byte
char * wg36_2()
{
static  char buf[16];
struct mem_inout data;
unsigned int tmp;
if(ReadWigan36(&data)!=0)return NULL;
data.d[1]=data.d[1]&0x3;
tmp=(data.d[1]<<12)&(data.d[0]>>20);
memset(buf,0,sizeof(buf));
sprintf(buf,"%05d%07d",tmp,data.d[0]&0XFFFFF);
return buf;
}


char *wg37_0()
{
return NULL;
}

char *wg37_1()
{
return NULL;
}


char *wg37_2()
{
return NULL;
}

char *wg37_3()
{
return NULL;
}
char *wg37_4()
{
return NULL;
}
char *wg37_5()
{
return NULL;
}
char *wg37_6()
{
return NULL;
}
char *wg37_7()
{
return NULL;
}


char *wg40_0()
{
return NULL;
}

char *wg40_1()
{
return NULL;
}
char *wg40_2()
{
return NULL;
}


char *wg86_0()
{
return NULL;
}

//linear(13 Decimal char)
int card_linearhandle(char *cardsno,char*param)
{
//	char sno[128]
	int len=0;
	char tmpsno[128];
	unsigned long long cardno=0,cardno1;

	if(cardsno==NULL||strlen(cardsno)==0)	return FALSE;
	len=strlen(cardsno);
	memcpy(tmpsno,cardsno,2);
	cardno1=strtoull(tmpsno,NULL,16);
	cardno=strtoull(&cardsno[2],NULL,16);
	sprintf(param,"%03llu%010llu",cardno1,cardno);
//	memset(sno,0,sizeof(sno));
//	sprintf(sno,"%014llo",cardno);
//	printf("tmp=%s\n",sno);
	return TRUE;
}
//syner card no.(14 Decimal char)
int card_synelhandle(char *cardsno,char*param)
{
	char sno[128];
	int i;
//	char tmpsno[128];
//	int len=0;
	unsigned long long cardno=0;

	if(cardsno==NULL||strlen(cardsno)==0)	return FALSE;

	cardno=strtoull(cardsno,NULL,16);
	memset(sno,0,sizeof(sno));
	sprintf(sno,"%014llo",cardno);

	for(i=0;i<14;i++)	
	{
		param[i]=sno[13-i];
	}
	return TRUE;
}

//ReadOnly-card
char *card_handle(char *p)
{
	static char sno[126];
	char tmpsno[126];
	unsigned long cardno=0;


	if(p==NULL||strlen(p)==0)	return NULL;

	memset(sno, 0, sizeof(sno));
	strcpy(sno,p);
//	sprintf(sno, "%02X%02X%02X%02X",*p++,*p++,*p++,*p++);
	if(cardinfo.card_fs==0)			//first cut ,then convert  (disable)
	{  
		if(cardinfo.card_10!=0)		//decimal 
		{
			memset(tmpsno,0,sizeof(tmpsno));
			right(sno,8);
			strcpy(tmpsno,sno);
			memset(sno,0,sizeof(sno));
			sprintf(sno,"%lu",strtoul(tmpsno,NULL,16));
		}
		if(cardinfo.card_right!=0) //cut the card number according to setting
		{
			if(cardinfo.card_10)
			{
				cardno=strtoul(sno,NULL,10);
				memset(sno,0,sizeof(sno));
				sprintf(sno,"%0*lu",cardinfo.card_right,cardno);
			}
			else 
			{
				cardno=strtoul(sno,NULL,16);
				memset(sno,0,sizeof(sno));
				sprintf(sno,"%0*lX",cardinfo.card_right,cardno);
			}
			right(sno,cardinfo.card_right);
		}
	}
	else	// enable
	{
		if(cardinfo.card_right!=0) //cut the card number according to setting
		{
			right(sno,cardinfo.card_right);
		}
		if(cardinfo.card_10!=0)	// convert to decimal
		{
			memset(tmpsno,0,sizeof(tmpsno));
			right(sno,8);
			strcpy(tmpsno,sno);
			memset(sno,0,sizeof(sno));
			sprintf(sno,"%lu",strtoul(tmpsno,NULL,16));
			if(cardinfo.card_right10!=0)	// decimal bits num
			{	
				right(sno,cardinfo.card_right10);
				cutzero(sno);
			}
		}
	}

	return sno;

}



/*
sno:card number
rightnum:numbers of bits to cut
*/
void right(char * sno,int rightnum)
{
	char tmpsno[1024];
	int len,j;
	char *p1;

	if(sno==NULL||rightnum<0)		return;
	memset(tmpsno,0,sizeof(tmpsno));
	len=strlen(sno);
	strncpy(tmpsno,sno,1024);
	p1=tmpsno;
	j=len-rightnum;
	if(j<0)
		return;
	p1+=j;
	if(len>1024)len=1024;
	memset(sno,0,len);
	strncpy(sno,p1,len);
	return;
}
//cut '0' character
void cutzero(char *src)
{
	char *ptr,*str;
      int flag=0;

	ptr=str=src;
	while(*str)
	{
		if((flag)||(*str!='0'))
		{
			*ptr++=*str++;
			flag=1;
		}
		else str++;
	}
	*ptr='\0';
}


//read sector value
int MF1Read(char *cardsno,char *param)
{
	int i,n,j,flag=1,len=0;
	unsigned char buf[1024];

	if(ReadCardInfo.enable==0)	return -1;
	if(ReadCardInfo.beg_sectors<0||ReadCardInfo.end_sectors>15||ReadCardInfo.beg_block<0||ReadCardInfo.end_block>2)	
		return -1;

	memset(buf,0,sizeof(buf));
	for(n=ReadCardInfo.beg_sectors;n<=ReadCardInfo.end_sectors;n++)
	{
		for(j=ReadCardInfo.beg_block;j<=ReadCardInfo.end_block;)
		{
			if(n==0&&j==0)	{j++;continue;}
//			usleep(overtime);
			if(flag)
			{
				if(mifs_read(cardsno,n,j,&buf[len],ReadCardInfo.keymode,ReadCardInfo.Bkey)==FALSE)
					return -1;
				flag=0;
				j++;
			}
			else
			{
				if(mifs_read(cardsno,n,j,&buf[len],ReadCardInfo.keymode,NULL)==FALSE)	return -1;
				flag=0;
				j++;
			}
			len=len+16;
		}
	}
	for(i=0;i<len;i++)
	{
		param[i]=(int)buf[i];
	}
//	gsmBytes2String(buf,sno,len);
	return len;
}


//write data to sector
int MF1Write(char *cardsno, char*value)
{
	int n,j,flag=1,len=0;
	char tmp[2048];
	unsigned char buf[26];

	if(WriteCardInfo.enable==0)	return FALSE;
	if(WriteCardInfo.beg_sectors<0||WriteCardInfo.end_sectors>15||WriteCardInfo.beg_block<0||WriteCardInfo.end_block>2)	
		return FALSE;
	strncpy(tmp,value,1024);
	for(n=WriteCardInfo.beg_sectors;n<=WriteCardInfo.end_sectors;n++)
	{
		for(j=WriteCardInfo.beg_block;j<=WriteCardInfo.end_block;)
		{
			if(n==0&&j==0)	{j++;continue;}
			memset(buf,0,sizeof(buf));
			memcpy(buf,&tmp[len],16); 
			if(flag)
			{
				if(mifs_write(cardsno,n,j,buf,WriteCardInfo.keymode,WriteCardInfo.Bkey)==1)
					return FALSE;
				printf("\nwrite rigth\n");
				flag=0;
				j++;
			}
			else
			{
				if(mifs_write(cardsno,n,j,buf,WriteCardInfo.keymode,NULL)==FALSE)	return FALSE;
				printf("\nwrite rigth\n");
				flag=0;
				j++;
			}
			len=len+16;
		}
	}
	return TRUE;
}

