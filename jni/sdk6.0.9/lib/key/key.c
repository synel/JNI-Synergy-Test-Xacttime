#include "key.h"
#include "../_precomp.h"
#include "../public/initcom.h"
#include "../sound/sound.h"

int limit_len=-1/*limit_len=-1 is not limit*/;
int lastkey=-1,lastmode=0;
char keystr[126];
static time_t starttime;
KEYINFO keyinfo;

//KB-sructure 
 static char *szMode1[] =    
 {   
    "2abcABC", "3defDEF", "4ghiGHI", "5jklJKL", "6mnoMNO" ,   
      "9prsPRS", "8tuvTUV", "7wxyWXY" ,"0+()&", "1qzQZ", "#.-: "  
 } ;   

/*--------------------------------------------------------------------------*
@Function          :InitKeyBoard - Initialize keypad parameters
@Include      	:"key.h"
@Description		:void
@Return value	: Failure FALSE; Success TRUE	
*---------------------------------------------------------------------------*/
int InitKeyBoard(IN KEYINFO *key_info)
{
	keyinfo=*key_info;
	memset(keystr,0,sizeof(keystr));
	lastmode=key_info->mode;
	return TRUE; //add by aduo 2013.6.24
}
/*--------------------------------------------------------------------------*
@Function			: ThreadKeyBoard - get KB input(the sign showed in each key)
@Include      		: "key.h"
@Description			:
				:judge whether the data received from com1 came form keypad
				return the KB actual value
@Return Value		: Success the KB value, Failure -1.
*---------------------------------------------------------------------------*/
int ThreadKeyBoard(void) {

	int key=-1;
	
	if(keyinfo.enable==0)		return -1;

	if(com1value.instruction==KEYBOARD)	
	{
		key=(int)com1value.user_data[0];
		memset(&com1value,0,sizeof(com1value));
		if(key==16)	key=0;
		if (key!=-1) MenuSound("key");
		return (key);

	}
	return -1;
}

/*--------------------------------------------------------------------------*
@Function			: KeyMap_Capital - Read KB
@Include      		: "key.h"
@Description			: 
				char *str:current key value
				return alphanumeric value
@Return Value		: Success string recevied
*---------------------------------------------------------------------------*/
int KeyMap_Capital(char *str)
{
   int k,i=0,j=0,len=0;

   k=strlen(str);
   if(k<=0)	return FALSE;
   k=k-1;
for(i=0;i<=10;i++) //  11 times
{
	len=strlen(szMode1[i])-1;
	for(j=0;j<=len;j++)
	{
		if(str[k]==szMode1[i][j])		
		{
			if(j==len)
				str[k]=szMode1[i][0];
			else 
				str[k]=szMode1[i][j+1];
			break;
		}
	}
}

return TRUE;
}

/*--------------------------------------------------------------------------*
@Function			: KeyMap_Letters - Read KB
@Include      		: "key.h"
@Description			: 
				char *str:current key value
				return only letters
@Return Value		: Success return string received
*---------------------------------------------------------------------------*/
//return alphanumeric string
int KeyMap_Letters(char *str)
{
   int k,i=0,j=0,len=0;

   k=strlen(str);
   if(k<=0)	return FALSE;
    k=k-1;
for(i=0;i<=10;i++)
{
	len=strlen(szMode1[i])-1;
	for(j=0;j<=len;j++)
	{
		if(str[k]==szMode1[i][j])		
		{
			if(j==len)
				str[k]=szMode1[i][0];
			else 
				str[k]=szMode1[i][j+1];
			if(str[k]>='0'&&str[k]<='9')
				str[k]=szMode1[i][1];
			break;
		}
	}
}

return TRUE;
}
void SetLimitLen(int len)
{
limit_len=len;
}

int GetLimitLen()
{
return limit_len;
}
/*--------------------------------------------------------------------------*
@Function			: ReadKey - Read letter or number of KB
@Include      		: "key.h"
@Description			: 
				int key:current key value
				return string received,switch between number and letter keyinfo.mode
@Return Value		: Success string received
*---------------------------------------------------------------------------*/
char *ReadKey(IN int key)
{
   int len=0;
   static char buf[126];

	switch(key){
		case 0:case 1:case 2: case 3:case 4:
		case 5:case 6:case 7: case 8:
		case 9:case 14:
		   switch(lastmode)
		    {
			case 2:	//both letter and number should be received 
			if(GetLimitLen()>0&&strlen(keystr)>=GetLimitLen())return keystr;
				if(lastkey!=key||abs(time(NULL)-starttime) >= keyinfo.timeout)
				{
					lastkey=key;
					len=strlen(keystr);
					if(len==0)	len=0;
					if(key==14)
						keystr[len]='#';	
					else 
						keystr[len]=key+48;	
				}
				else
				{
					KeyMap_Capital(keystr);
				}
				time(&starttime);	
				break;
			case 1:	//receive letter only
				if(lastkey!=key||abs(time(NULL)-starttime) >= keyinfo.timeout)
				{
					lastkey=key;
					len=strlen(keystr);
					if(len==0)	len=0;
					if(key==14)
						keystr[len]='#';	
					else 
					{
						keystr[len]=key+48;						
						KeyMap_Letters(keystr);
					}
				}
				else
				{
					KeyMap_Letters(keystr);
				}
				time(&starttime);
				break;
			case 0:	//receive number only
				if(key==14)	break;
				len=strlen(keystr);
				keystr[len]=key+48;
			break;
			default:
			break;
		    }		
		break;
		case 10:
			memset(keystr,0,sizeof(keystr));
			lastmode=keyinfo.mode;
			SetLimitLen(-1);
		break;
		case 11:
			len=strlen(keystr);
			if(len!=0)
				keystr[len-1]='\0';
		break;
		case 12:
		break;
		case 13:
			lastmode=keyinfo.mode;
			memset(buf,0,sizeof(buf));
			strcpy(buf,keystr);
			memset(keystr,0,sizeof(keystr));
			SetLimitLen(-1);
			return buf;
		break;
		default:
			return NULL;
		break;
	}	
	return keystr;
}

/*--------------------------------------------------------
@Function			: SetKeyMode - change keyboard mode
@Include      		: "key.h"
@Description			: 
				int mode:current keyboard mode
				0--receive number only
				1--receive letter only
				2--receive both
@Return Value		: Success return True false return FALSE
---------------------------------------------------------*/
int SetKeyMode(IN int mode)
{
	if(mode>3||mode<0)	return FALSE;//modify by aduo 2013.7.12
	lastmode=mode;	//
	return TRUE;
}

//get keyboard mode
int GetKeyMode()
{
	return lastmode;
}



