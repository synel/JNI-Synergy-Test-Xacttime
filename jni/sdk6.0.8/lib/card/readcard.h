#ifndef READCARD_H
#define READCARD_H
#include "../_precomp.h"

//#define READONLYCARD  0X94
//card-reader info
struct card_INFO
{
	char cardtype;			//card-reader type(now unuse)
	int track;				//numbers of magnetic tracks 
	int card_right,card_10,card_fs,card_right10;  //how many bits of card number will be cut from right;whether to convert card number to decimal system
};

struct R_card_INFO
{
	int enable;
	char keymode;			//key type
	char Akey[24],Bkey[24];		//A-key£¬B-key
	int beg_sectors,end_sectors,beg_block,end_block;
};


struct W_card_INFO
{
	int enable;
	char keymode;	
	char Akey[24],Bkey[24];		//A-key£¬B-key
	int beg_sectors,end_sectors,beg_block,end_block;
};

 int InitCard(struct card_INFO *card_info);
 int InitCard_Read(struct R_card_INFO *R_cardinfo);
 int InitCard_Write(struct W_card_INFO *W_cardinfo);
 int ReadCard(char *cardnum);
 int MF1Read(char *cardsno,char *param);
 int MF1Write(char *cardsno, char*value);

char *card_handle(char *p);
int card_synelhandle(char *cardsno,char*param);
int card_linearhandle(char *cardsno,char*param);

char *wg26_0();
char *wg26_1();
char *wg26_2();
char *wg26_3();
char *wg27_0();
char *wg32_0();
char *wg32_1();
char *wg33_0();
char *wg33_1();
char *wg34_0();
char *wg34_1();
char *wg34_2();
char *wg36_0();
char *wg36_1();
char *wg36_2();
char *wg37_0();
char *wg37_1();
char *wg37_2();
char *wg37_3();
char *wg37_4();
char *wg37_5();
char *wg37_6();
char *wg37_7();
char *wg40_0();
char *wg40_1();
char *wg40_2();
char *wg86_0();

void right(char * sno,int rightnum);
void cutzero(char *src);

#endif
