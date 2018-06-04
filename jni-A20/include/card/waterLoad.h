#ifndef WATERLOAD_H
#define WATERLOAD_H

#include <stdio.h>



typedef struct __WATER_KEY
{
	unsigned char keyA[6];
	unsigned char access[4];
	unsigned char keyB[6];
}_WATER_KEY;

typedef union __WATER_SECTOR_DATA
{
	unsigned char block[16];
	_WATER_KEY Key;
}_WATER_SECTOR_DATA;

typedef struct __CARD_MONEY_INFO
{
//	int scmOldAmt;
//	int scmNewAmt;
	unsigned int waterOldAmt;
	unsigned int waterNewAmt;
	unsigned int QcAmt;

}_CARD_MONEY_INFO;


int DoWaterLoadEvent(int uart_port, unsigned int waterSct, unsigned int optValue, char *phycard, char *oldAmt, char *newAmt );


#endif








