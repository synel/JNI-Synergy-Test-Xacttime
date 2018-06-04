#ifndef __STRINGCHAR_H
#define __STRINGCHAR_H
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

#define CONST_2 2
#define CONST_3 3
#define CONST_8 8
#define CONST_9 9
#define CONST_10 10
#define CONST_16 16
#define CHAR_0 '0'
#define CHAR_9 '9'
#define CHAR_A 'A'
#define CHAR_a 'a'
#define CHAR_Z 'Z'
#define CHAR_SPACE ' '
#define STRING_0 "0"
#define STRING_Z "Z"


//卡号规则项
typedef struct cardrule_item
{
    int id;  //序号
    int argc1; //参数1
    char argc2[12]; //参数2
    char argc3[12]; //参数3
    char argc4[12]; //参数4
    char cmd[12]; //函数名
}_cardruleItem;

//卡号规则列表
typedef struct cardrule_info
{
    int itemNumber;  //执行过程个数
    int soureLen;  //原始卡号长度
    _cardruleItem item[24];  //执行过程

}_cardruleInfo;

extern _cardruleInfo cardruleInfo[64];
double OtherToDeci(int scale_in,char inputdata[]);
void DeciToOther(unsigned long deci,int scale_out,char outputdata[]);
void PutSource(char inputdata[])  ;
void decimalToBinary(int scale_in,char inputdata[],int scale_out,char outputdata[]);
int perversionCharacterString(char *InParam,char*OutParam);
int substring(char *InParam,char *type,int length,int stat,char *OutParam);
int stringBuilder(char *Inparam1,char *Inparam2,char *OutParam);
int openCardRuleFile(char *fileName);
int cardRuleAnalysis(char *InCardNo,char *OutCardNo);
#endif
