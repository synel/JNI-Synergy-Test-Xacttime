#include <stdio.h>

#include "waterLoad.h"
#include "card_sectors.h"
#include "../general/stringchar.h"


// 20170506 接口调用说明
/*
   联机交互部分流程不变:
   读到卡号后，发送联机消费指令到后台，收到成功应答后，此时仍要保留冲正状态。
   继续调用圈存接口，传入必要的参数，执行圈存写卡操作，接口返回成功后，才认为
   本次联机交易成功，可以清除冲正状态，保存交易记录，显示成功界面信息。
   成功界面可显示出，水控钱包的圈存前余额和圈存后的余额。
   如果圈存接口返回失败状态-1，则，冲正当前交易
   如果圈存接口返回未决状态-2，则，保留当前接口返回的圈存前金额，继续调用圈存接口，查询当前余额，确认是否
   已圈存成功，如成功，继续显示成功界面，如未成功，则冲正
*/

#undef DEBUG
//#define DEBUG
#ifdef DEBUG
#define PRINTF( x ) printf x
#else
#define PRINTF( x ) ( (void)0 )
#endif



// 20150420 Load m710  凯路水控钱包初始化、圈存
// KEYA: 7F 12 23 44 55 AB

static unsigned int WaterSct = 15;	//	定义水控钱包的扇区，默认为15扇区

static _CARD_MONEY_INFO CARD_MONEY_INFO;


// 20170506 M1卡，圈存初始化接口
static int MF1_init(int uart_port, unsigned int waterSct)
{
	WaterSct = waterSct;	
//printf("WaterSct =========================== %d\n", WaterSct);	
	pit_init_card_cfg();	// must before
	statInfo.readerType = 0xF1;
	return sdk_get_comfd(uart_port);
}


// 物理卡号4bytes
// key: 6bytes
// type:
// cardno: 物理卡号
// 卡号翻转
// 计算水控钱包扇区密钥

static int calcWaterKey(unsigned int cardno, unsigned char *key)
{
	unsigned char tmpkey[6];
	unsigned char *addr = NULL;

// 设置专用密码  "wedsqc"
	unsigned char password[6] = {'w', 'e', 'd', 's', 'q', 'c'};

	addr = (unsigned char *)&cardno;

	{
		tmpkey[0] = password[0] + addr[0];
		tmpkey[1] = password[1] + addr[1];
		tmpkey[2] = password[2] | addr[2];
		tmpkey[3] = password[3] ^ addr[3];
		tmpkey[4] = password[4] ^ addr[0];
		tmpkey[5] = password[5] & addr[3];
	}

	memcpy(key, tmpkey, 6);

//	leeDebugData(key, 6, 6, 0);

	return 0;

}



static int initWaterCard()
{
	int ret = 0, i = 0;
	unsigned int keymode = 1;	// A 密钥
	unsigned char key[6];
	unsigned char orignalkey[6];
	unsigned char tmpbuf[16];

	unsigned int cardNo;
	unsigned char sum = 0;

	_WATER_SECTOR_DATA uWATER_DATA;

	memset(orignalkey, 0xFF, 6);

	ret = readDataBlock2(keymode, orignalkey, WaterSct, 0, 16, tmpbuf);
	if (ret != ERR_OK)
	{
		PRINTF((" initWaterCard Err 0\n"));
		return -1;	// 异常卡
	}

// block0
	memset(&uWATER_DATA, 0, sizeof(uWATER_DATA));

	cardNo = (statInfo.card2 & 0xFFFFFF);		// 发卡号
	uWATER_DATA.block[0] = cardNo / 0xFFFF;
	uWATER_DATA.block[1] = cardNo & 0xFF;
	uWATER_DATA.block[2] = (cardNo >> 8) & 0xFF;
	uWATER_DATA.block[3] = 0x01;					// 卡类,固定为1类卡 20140813 注释

	for(i = 0; i < 15; i++)
		sum += uWATER_DATA.block[i];

	uWATER_DATA.block[15] = sum;

	ret = writeDataBlock2(keymode, orignalkey, WaterSct, 0, 16, uWATER_DATA.block);
	if (ret != ERR_OK)
	{
		PRINTF((" initWaterCard Err 1\n"));
		return -1;
	}

// block1
	memset(&uWATER_DATA, 0, sizeof(uWATER_DATA));
	ret = writeDataBlock2(keymode, orignalkey, WaterSct, 1, 16, uWATER_DATA.block);
	if (ret != ERR_OK)
	{
		PRINTF((" initWaterCard Err 2\n"));
		return -1;
	}

// block2
	ret = writeDataBlock2(keymode, orignalkey, WaterSct, 2, 16, uWATER_DATA.block);
	if (ret != ERR_OK)
	{
		PRINTF((" initWaterCard Err 3\n"));
		return -1;
	}

// KEY
	calcWaterKey(statInfo.card2, key);
	ret = readDataBlock2(keymode, orignalkey, WaterSct, 3, 16, uWATER_DATA.block);
	if (ret != ERR_OK)
	{
		PRINTF((" initWaterCard Err 4\n"));
		return -1;
	}

	memcpy(uWATER_DATA.Key.keyA, key, 6);
	memcpy(uWATER_DATA.Key.keyB, orignalkey, 6);

//leeDebugData(uWATER_DATA.block, 16, 16, 0);

	ret = modiSectorKey( keymode, orignalkey, WaterSct, uWATER_DATA.Key.keyA, uWATER_DATA.Key.keyB, uWATER_DATA.Key.access);
	if (ret != ERR_OK)
	{
		PRINTF((" initWaterCard Err 5\n"));
		return -1;
	}

	return ERR_OK;

}



// 水控钱包同步及恢复
static int waterCardRecover(unsigned char *NewWaterBlock)
{
	unsigned int keymode = 1;		// A 密钥
	unsigned char key[6];	// 初始化为默认密钥
	int ret, i = 0;
	_WATER_SECTOR_DATA uWATER_DATA;
	_WATER_SECTOR_DATA uWATER_DATA2;
	unsigned char sum = 0, sum2 = 0;

	memset(key, 0xFF, 6);

	calcWaterKey(statInfo.card2, key);

	ret = readDataBlock2(keymode, key, WaterSct, 1, 16, uWATER_DATA.block);
	if (ret != ERR_OK)
	{PRINTF(("waterCardRecover not init\n"));
		ret = initWaterCard();
		if (ret != ERR_OK){
			return -1;
		}

		//
		ret = readDataBlock2(keymode, key, WaterSct, 1, 16, uWATER_DATA.block);
		if (ret != ERR_OK)
		{
			PRINTF(("waterCardRecover not init 2\n"));
			return -1;
		}
	}

	ret = readDataBlock2(keymode, key, WaterSct, 2, 16, uWATER_DATA2.block);
	if (ret != ERR_OK)
	{
		PRINTF(("waterCardRecover Err 0\n"));
		return -1;
	}

// 校验
	for(i = 0; i < 15; i++)
	{
		sum += uWATER_DATA.block[i];
		sum2 += uWATER_DATA2.block[i];
	}

	if(sum == uWATER_DATA.block[15]){
		if(memcmp(uWATER_DATA.block, uWATER_DATA2.block, 16)){
			ret = writeDataBlock2(keymode, key, WaterSct, 2, 16, uWATER_DATA.block);
			if (ret != ERR_OK)
			{
				PRINTF(("waterCardRecover Err 1\n"));
				return -1;
			}
		}
		memcpy(NewWaterBlock, uWATER_DATA.block, 16);
	}else
	{
		if (sum2 != uWATER_DATA2.block[15]){
			PRINTF(("waterCardRecover Err xxxx\n"));
			return -1;
		}

		ret = writeDataBlock2(keymode, key, WaterSct, 1, 16, uWATER_DATA2.block);
		if (ret != ERR_OK)
		{
			PRINTF(("waterCardRecover Err 2\n"));
			return -1;
		}

		memcpy(NewWaterBlock, uWATER_DATA2.block, 16);
	}

	CARD_MONEY_INFO.waterOldAmt = HexToUInt(NewWaterBlock, 3, 0);
	CARD_MONEY_INFO.waterNewAmt = CARD_MONEY_INFO.waterOldAmt;

	return 0;

}


static int writeWaterCard(unsigned char *block)
{
	int ret = 0;
	unsigned int keymode = 1;	// A 密钥
	unsigned char key[6];

	calcWaterKey(statInfo.card2, key);

BLK_1:
	ret = writeDataBlock2(keymode, key, WaterSct, 1, 16, block);
	if (ret != ERR_OK)
	{
		PRINTF((" writeWaterCard block1 Err\n"));
		return -1;		
	}

BLK_2:
	ret = writeDataBlock2(keymode, key, WaterSct, 2, 16, block);

	return 0;

}




// 水控圈存
// uart_port:串口号
// waterSct:水控扇区号
// optValue:圈存金额，单位: 分
// phycard:物理卡号，字符串格式，如 "11223344"
// oldAmt:圈存前余额
// newAmt:圈存后余额
// return: -9 水控钱包超限
//	if(CARD_MONEY_INFO.waterNewAmt > 10000)	// 凯路水控实际支持不大于100元，否则水控器上刷卡会报错	，2017-09-11
// 2017-09-11-2 具体不确定，最好应用层可设置，并将参数传入
int DoWaterLoadEvent(int uart_port, unsigned int waterSct, unsigned int optValue, char *phycard, char *oldAmt, char *newAmt )
{
	static int init = 0;
	int ret = -1, i = 0;
	unsigned char sum = 0;	// 和校验
	char activeCard[32 + 1];
	unsigned char NewWaterBlock[16];

	if(init == 0)
	{
		init = 1;
		MF1_init(uart_port, waterSct);
	}

// 再做一次主动寻卡:must
	memset(&activeCard, 0, sizeof(activeCard));
	ret = card_active( WEDS_READER, activeCard );
	if (ret != CARD_IC && ret != CARD_S70)
	{//printf("w1\n");
		return -1;
	}

	if(strcmp(phycard, activeCard) != 0)
	{//printf("w2\n");
		return -1;	// 两次获取的卡号不一致
	}

	memset(&CARD_MONEY_INFO, 0, sizeof(CARD_MONEY_INFO));

	memset(&NewWaterBlock, 0, sizeof(NewWaterBlock));
	ret = waterCardRecover(NewWaterBlock);
	if (-1 == ret)
	{//printf("w3\n");
		return -1;
	}

	if(optValue == 0)	// 查询余额
	{
		sprintf(oldAmt, "%d", CARD_MONEY_INFO.waterOldAmt);
		sprintf(newAmt, "%d", CARD_MONEY_INFO.waterNewAmt);
		return 0;
	}

	CARD_MONEY_INFO.QcAmt = optValue;
	CARD_MONEY_INFO.waterNewAmt = CARD_MONEY_INFO.waterOldAmt + optValue;
	if(CARD_MONEY_INFO.waterNewAmt > 0xFFFFFF)	// 水控钱包超限
	{
		return -9;
	}

	UIntToHex(CARD_MONEY_INFO.waterNewAmt, NewWaterBlock, 3, 0);	// 小端
	for(i = 0; i < 15; i++)
	{
		sum += NewWaterBlock[i];
	}
	NewWaterBlock[15] = sum;	

	sprintf(oldAmt, "%d", CARD_MONEY_INFO.waterOldAmt);
	sprintf(newAmt, "%d", CARD_MONEY_INFO.waterNewAmt);	// must here

	ret = writeWaterCard(NewWaterBlock);
	if (-1 == ret)
	{
		return -2;
	}	

	return 0;

}



