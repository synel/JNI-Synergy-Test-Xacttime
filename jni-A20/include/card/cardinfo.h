/**
 * @file   readcard.h
 * @author 刘训
 * @date   Thu Jul 14 16:47:39 2011
 *
 * @brief
 *
 *
 */
#ifndef __CARD_INFO_H
#define __CARD_INFO_H
#include "_precomp.h"

// surport cpu
#define COS_PANCOS        (0xF1)     //panCos
#define COS_TFCOS         (0xF2)     //TFCos

#define COSTF26          (0x01)
#define COSFM20           (0x02)
// end

#define RET_SUCCESS       (0x00)   //错误码：成功
#define RET_OPERR         (0x01)   //指令操作失败
#define RET_CHECKERR      (0x10)   //校验错误
#define RET_INVALID       (0x11)   //不能识别的指令
#define RET_NOTCONDITION  (0x12)   //指令无法执行

#define WEDS_READER_ID     0XE1   //威尔ID卡头，帧头(0x02) 帧尾：0x03  20110806
#define SHNM201_READ       0XE2   //盛华SHNM201系列只读卡头
#define RFID_SIM_READER    0xE3   //国民技术
#define SHNM100_READONLY   0xE4   //盛华SHNM100卡头,只读用
#define YOOCAN_READONLY    0xE5   //中科讯联YooCan_Reader 只读用 20120619

#define WEDS_READER_CPU    0xf0   //威尔AAAB卡头,CPU
#define WEDS_READER        0xf1   //威尔AAAB卡头,IC
#define MINGWAH_READER     0xf2   //明华RD-ET-MX读写器
#define SHNM100_READER     0xf3   //盛华SHNM100卡头
#define YOOCAN_READER      0xf4   //中科讯联YooCan_Reader 20120616 包装选择应用及开通道1指令 20120619
#define MA102_READER       0xf5   //盛华MA102


#define TYPE_STD_ISSU_KEY  0x01   //标准版发行密钥类型（写管理扇区密钥）
#define TYPE_STD_DEAL_KEY  0x02   //标准版交易密钥类型
#define TYPE_STD_REQ_KEY   0x03   //标准版查询密钥类型

#define CARD_FINGER       (0x71)   //指纹
#define CARD_MAG          (0x91)   //磁条卡
#define CARD_BAR          (0x92)   //条码卡
#define CARD_ID           (0x93)   //ID卡
#define CARD_IC           (0x94)   //IC卡
#define CARD_CPU          (0x95)   //CPU卡
#define CARD_UIM          (0x96)   //RFID-UIM卡
#define CARD_SIM          (0x97)   //RFID_SIM 20120208
#define CARD_USIM         (0X98)   //联通2.4G卡 中科讯联配套 20120616
#define CARD_S70		  (0x9A) //S70 卡

#define ERR_OK                0 //正确

#define ERR_ISCARD           -101 //卡在位
#define ERR_NOCARD           -102 //无卡或卡未连接
#define ERR_NOCHARGE         -103 //充值序号相同，或充值金额为0
#define ERR_NORMALCARD       -104 //正常卡，不需恢复
#define ERR_TIMEOUT_N        -105 //超时 20110729

#define ERR_FALSE            -201 //错误
#define ERR_PARAM            -202 //参数错误
#define ERR_OPERR            -203 //指令操作失败、校验错误、不能识别的指令、指令无法执行
#define ERR_OVERRETRY        -204 //超过重试次数
#define ERR_INVALID          -205 //无效的返回
#define ERR_HEADINVALID      -206 //发送数据帧头无效
#define ERR_OVERBUF          -207 //数据超过缓冲区大小
#define ERR_COMM             -208 //UIM卡通讯错误
#define ERR_FUNCLOCK         -209 //功能被锁
#define ERR_TIMEOUT		 	 -210 //超时

#define ERR_SECTOR_LACK		 -211		// 卡片升级，扇区数不足

//psam
#define ERR_PSAM_INIT	-231 //PSAM操作失败

#define ERR_COMERR           -301 //串口操作失败
#define ERR_INVALIDHANDLE    -302 //无效的串口句柄


#define ERR_INVALIDREADER    -401 //不支持的读卡器
#define ERR_INVALIDCARDTYPE  -402 //不支持的卡类型
#define ERR_INVALIDCARD      -403 //无效卡、错误的卡格式
#define ERR_CHECKOVER        -404 //未通过校验的卡块数据域大于1
#define ERR_CRC              -405 //crc校验错
#define ERR_CARD_NO_RECOVER  -406 //卡片未恢复
#define ERR_DATA_FORMAT      -407 //数据格式错误


#define KEY_TYPE_A			1	//  A key
#define KEY_TYPE_B			2	//  B key

#define SCT_BLK_0		0
#define SCT_BLK_1		1
#define SCT_BLK_2		2
#define SCT_BLK_3		3
#endif
