/**
 * @chinese
 * @file   stringchar.c
 * @author 刘训
 * @date   Thu Oct 13 10:00:00 2011
 *
 * @brief  卡规则处理
 * @endchinese
 *
 * @english
 * @file   stringchar.c
 * @author Liu Xun
 * @date   Thu Oct 13 10:00:00 2011
 *
 * @brief string and char handling module
 * @endenglish
 */
#include <sys/times.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include "cardrule.h"
#include "config.h"
#include "debug.h"



_cardruleInfo cardruleInfo[64];
/*其他进制转换为十进制*/
double OtherToDeci(int scale_in,char inputdata[])
{
    int i,len,midint;
    double middbl,temp;

    len = strlen(inputdata);
    temp = 0.00;

    for(i = 0;i < len; i++)
    {
        midint = inputdata[i];

        if ( CHAR_0 <= midint && midint <= CHAR_9 )
        {
            midint = midint - CHAR_0;  /*0-9*/
        }
        else if ( CHAR_A <= midint && midint <= CHAR_Z )
        {
            midint = midint - CHAR_A + CONST_10;  /*A-Z*/
        }
        else
        {
            midint = midint - CHAR_a + CONST_10;  /*a-z*/
        }

        middbl = midint * pow(scale_in,len-i-1);
        temp = temp + middbl;
    }
    return temp;
}

/*十进制转换为其他进制*/
void DeciToOther(unsigned long deci,int scale_out,char outputdata[])
{
    int m,n;

    m = 0;
    n = 0;

    while (deci)
    {
        m = deci % scale_out;
        if (m > CONST_9)
        {
            outputdata[n] = m + CHAR_A - CONST_10;
        }
        else
        {
            outputdata[n] = m + CHAR_0;
        }
        deci = deci / scale_out;
        n++;
    }

    for (m = 0; m <= n / 2 - 1; m++)  /*反序得到目标数据*/
    {
        char t;
        t = outputdata[m];
        outputdata[m] = outputdata[n - 1 - m];
        outputdata[n - 1 - m] = t;
    }
    outputdata[n] = '\0';
}

void PutSource(char inputdata[])  /*输出源数据*/
{
    int k;
    if (!strcmp(inputdata,STRING_0))  /*源数据为0*/
    {
        printf("(The source data is: 0)\n");
    }
    else
    {
        printf("(The source data is: ");
        for( k = 0; k < strlen(inputdata); k++ )
        {
            printf("%c",inputdata[k]);
        }
        printf(")\n");
    }
}

/*进制转换*/
void decimalToBinary(int scale_in,char inputdata[],int scale_out,char outputdata[])
{
    int k;
    unsigned long deci;

    deci = 0;

    if (scale_in == scale_out) /*原数据进制和目标数据进制相同,不用转换 */
    {
        strcpy(outputdata,inputdata);
        PutSource(inputdata);
    }
    else
    {
        if (scale_in == CONST_10)
        {
            deci = atol(inputdata); /*源数据是十进制，直接转换为十进制数*/
        }
        else
        {
            deci = (unsigned long)OtherToDeci(scale_in,inputdata); /*其他进制转换为十进制*/
        }

        if (scale_out == CONST_10) /*如果目标进制是十进制，直接输出十进制数*/
        {
            printf("The object data is: \n%u \n",deci);
            sprintf(outputdata,"%u",deci);
                printf("****:%s\n",outputdata);
        }
        else
        {
            if (!deci)  /*源数据为0*/
            {
                printf("The object data is: \n0\n");
                sprintf(outputdata,"%u",deci);
            }
            else
            {
                DeciToOther(deci,scale_out,outputdata);  /*十进制转换为其他进制*/

                printf("The object data is: \n");  /*输出转换后结果*/
                for( k = 0; k < strlen(outputdata); k++ )
                {
                    printf("%c",outputdata[k]);
                }
                printf("\n");

            }
        }
        PutSource(inputdata);
    }
}

/*顺序两两颠倒*/
int perversionCharacterString(char *InParam,char*OutParam)
{
    int i=0;
    int len=0;
    int index=0;

    memset(OutParam,0,sizeof(OutParam));
    len=strlen(InParam)-1;
    if(len<=0)
    {
        OutParam[0]=InParam[0];
        return 1;
    }

    for(i=0;i<=len;i++)
    {
        if(i==len)
            index = 0;
        else
            index = len-i-1;
        OutParam[i]=InParam[index];
        if(i>=len)
            break;
        OutParam[i+1]=InParam[len-i];
        i+=1;
    }
    return 1;
}


/*
    方法：
        卡号截取
    参数：
        char *InParam - 入口参数
        char *type - 截取方式 R-右截取，L-左截取
        int length - 截取位数
        int stat - 是否补0，0-不补，1-左边补0
        char *OutParam - 出口参数
    返回值：
        1 -成功 -1 -失败
*/
int substring(char *InParam,char *type,int length,int stat,char *OutParam)
{
    int sLen=0;
    int pLen=0;  //补齐长度
    int pos=0; //截取位置
    int i=0;

    sLen=strlen(InParam);
    if(sLen<0)
        return -1;

    if(stat==1)  //不足位左补'0'
    {
        if((length-sLen)>0)
            pLen = abs(length-sLen);
        else
            pLen=0;
        for(i=0;i<pLen;i++)
        {
            OutParam[i]='0';
        }
    }
    if (strcmp(type,"L")==0||strcmp(type,"l")==0)
    {
        pos=0;
    }
    else if (strcmp(type,"R")==0||strcmp(type,"r")==0)
    {
        pos=sLen-length;
        if(pos<0)
            pos=0;
    }

    i=0;
    for(i;i<length;i++)
    {
        OutParam[pLen+i]=InParam[pos+i];
    }
    return 1;
}

//字符串拼接
int stringBuilder(char *Inparam1,char *Inparam2,char *OutParam)
{
    sprintf(OutParam,"%s%s",Inparam1,Inparam2);
    return 1;
}


//读取规则文件
int openCardRuleFile(char *fileName)
{
    FILE *file=NULL;
    char buf[256],dest[128];
    char *sep=",";//分割符
    char *ptr=NULL;
    int index=0,itemIndex=0;
    int i = 0;

    for(i=0;i < 64;i++)
    {
        memset(&cardruleInfo[i],0,sizeof(_cardruleInfo));
        cardruleInfo[i].soureLen = -1;
        cardruleInfo[i].itemNumber = 0;
    }
    if (access(fileName,F_OK) !=0)
    {
        goto error;
    }

    file=fopen(fileName,"r");
    if(!file)
    {
        goto error;
    }
    memset(buf,0,sizeof(buf));
    while((ptr=fgets(buf,sizeof(buf),file)))
    {
        cut_rn(buf);
        if(strlen(buf)<=0)
        {
            memset(buf,0,sizeof(buf));
            continue;
        }
        printf("buf:%s\n",buf);
        //原始卡号长度
        memset(dest,0,sizeof(dest));
        string_get_item_with_sep(buf,sep,0,dest,sizeof(dest));
        index = atoi(dest);
        cardruleInfo[index].soureLen= atoi(dest);
        //执行过程数
        cardruleInfo[index].itemNumber+=1;

        //序号
        memset(dest,0,sizeof(dest));
        string_get_item_with_sep(buf,sep,1,dest,sizeof(dest));
        itemIndex = atoi(dest);
        cardruleInfo[index].item[itemIndex].id=atoi(dest);

        //函数名
        string_get_item_with_sep(buf,sep,2,cardruleInfo[index].item[itemIndex].cmd,sizeof(cardruleInfo[index].item[itemIndex].cmd));

        //参数1
        memset(dest,0,sizeof(dest));
        string_get_item_with_sep(buf,sep,3,dest,sizeof(dest));
        cardruleInfo[index].item[itemIndex].argc1 = atoi(dest);

        //参数2
        string_get_item_with_sep(buf,sep,4,cardruleInfo[index].item[itemIndex].argc2,sizeof(cardruleInfo[index].item[itemIndex].argc2));

        //参数3
        string_get_item_with_sep(buf,sep,5,cardruleInfo[index].item[itemIndex].argc3,sizeof(cardruleInfo[index].item[itemIndex].argc3));

        //参数4
        string_get_item_with_sep(buf,sep,6,cardruleInfo[index].item[itemIndex].argc4,sizeof(cardruleInfo[index].item[itemIndex].argc4));

        memset(buf,0,sizeof(buf));

    }
    error:
    if(file!=NULL)
        fclose(file);
    /*
int j=0;
for(i=0;i < 64;i++)
{
printf("s:%d,in:%d\n",cardruleInfo[i].soureLen ,cardruleInfo[i].itemNumber );
for(j=1;j<=cardruleInfo[i].itemNumber;j++)
{
int id;  //序号
int argc1; //参数1
char argc2[12]; //参数2
char argc3[12]; //参数3
char argc4[12]; //参数4
char cmd[12]; //函数名
printf("%d,%d,%s,%s,%s,%s\n",cardruleInfo[i].item[j].id,cardruleInfo[i].item[j].argc1,cardruleInfo[i].item[j].argc2,cardruleInfo[i].item[j].argc3,cardruleInfo[i].item[j].argc4,cardruleInfo[i].item[j].cmd);
}
}
*/
    return 1;
}


/*卡号处理过程*/
int cardRuleAnalysis(char *InCardNo,char *OutCardNo)
{
    int i=0;
    int sLen=0;
    char buf[128][128];
    char dest[128];

    sLen=strlen(InCardNo);
    if(sLen>=64)
    {
        strcpy(OutCardNo,InCardNo);
        return -1;
    }
    if(cardruleInfo[sLen].soureLen<0)
    {
        strcpy(OutCardNo,InCardNo);
        return -1;
    }

    if(cardruleInfo[sLen].itemNumber <=0){
        strcpy(OutCardNo,InCardNo);
        return -1;
    }
    memset(buf,0,sizeof(buf));
    strcpy(buf[0],InCardNo);
    for(i=1;i<=cardruleInfo[sLen].itemNumber;i++)
    {
        printf("cmd:%s\n",cardruleInfo[sLen].item[i].cmd);
        if(strcmp(cardruleInfo[sLen].item[i].cmd,"convert")==0)
        {
            decimalToBinary(atoi(cardruleInfo[sLen].item[i].argc2),buf[cardruleInfo[sLen].item[i].argc1],atoi(cardruleInfo[sLen].item[i].argc3),buf[i]);
            printf("convert:%s\n",buf[i]);
        }
        if(strcmp(cardruleInfo[sLen].item[i].cmd,"perversion")==0)
        {
            perversionCharacterString(buf[cardruleInfo[sLen].item[i].argc1],buf[i]);
            printf("perversion:%s\n",buf[i]);
        }

        if(strcmp(cardruleInfo[sLen].item[i].cmd,"cut")==0)
        {

            substring(buf[cardruleInfo[sLen].item[i].argc1],cardruleInfo[sLen].item[i].argc2,atoi(cardruleInfo[sLen].item[i].argc3),atoi(cardruleInfo[sLen].item[i].argc4),buf[i]);
            printf("cut:%s\n",buf[i]);
        }

        if(strcmp(cardruleInfo[sLen].item[i].cmd,"link")==0)
        {

            stringBuilder(buf[atoi(cardruleInfo[sLen].item[i].argc2)],buf[atoi(cardruleInfo[sLen].item[i].argc3)],buf[i]);
            printf("link:%s\n",buf[i]);
        }
    }
    i-=1;
    if(i==0) i=1;
    strcpy(OutCardNo,buf[i]);
    return 1;
}
