/**
 * @file   dx-RF-SIM.h
 * @author 刘训
 * @date   Wed Jul 13 09:35:09 2011
 *
 * @brief
 *
 *
 */
#ifndef LV1000_H__
#define LV1000_H__
#include "_precomp.h"


#define INPUT_SEND "$$$$"
#define INPUT_RECV "@@@@"
#define SIGNOUT_SEND "%%%%"
#define SIGNOUT_RECV "^^^^"

#define	LV1000					12
#define	OPEN_SET_MODE			"#99900031;"	//启动设置码//两种方法可以对识读引擎进行设置
#define	MANUAL_READ_MODE		"#99900110;"	//手动识读模式1
#define	AUTO_READ_MODE			"#99900111;"	//自动识读模式2
#define	INTERVAL_READ_MODE		"#99900112;"	//间歇识读模式3
#define	INDUCT_READ_MODE		"#99900113;"	//感应识读模式4
#define	CONTINUOUS_READ_MODE	"#99900114;"	//连续识读模式5
#define	AIMID_PRIFIX_YES		"#99904031;"	//允许添加全部AIMID前缀字符
#define	AIMID_PRIFIX_NO			"#99904030;"	//禁止添加AIMID前缀字符

#define	TIME_READ_DATA			"#99900150;"	//设置读码时间长度
#define	AUTOMODE_RESTART_TIMER	"#99900157;"	//识读出条码后重新开始计时//自动识读模式2
#define	AUTOMODE_RESTART_TIMER_NO	"#99900160;"
#define	AUTOMODE_READ_SAMEDATA_NO	"#99900155;"
#define	AUTOMODE_READ_SAMEDATA_YES	"#99900156;"

#define	INTERVAL_TIME_READ_DATA	"#99900151;"	//设置间歇模式的间歇时长
#define	ADD_SUFFIX_YES	"#99904101;"
#define	ADD_SUFFIX_NO	"#99904100;"
#define	SET_SUFFIX	"#99904102;"
#define	RESET	"#99900030;"

#define	DATA_ZERO	"#99900000;"	//0
#define	DATA_ONE	"#99900001;"	//1
#define	DATA_TWO	"#99900002;"	//2
#define	DATA_THREE	"#99900003;"	//3
#define	DATA_FOUR	"#99900004;"	//4
#define	DATA_FIVE	"#99900005;"	//5
#define	DATA_SIX	"#99900006;"	//6
#define	DATA_SEVEN	"#99900007;"	//7
#define	DATA_EIGHT	"#99900010;"	//8
#define	DATA_NINE	"#99900011;"	//9
#define DATA_C		"#99900014;"	//C

#define BAUD_1200	"#99902101;"
#define BAUD_2400	"#99902102;"
#define BAUD_4800	"#99902103;"
#define BAUD_9600	"#99902104;"
#define BAUD_14400	"#99902105;"
#define BAUD_19200	"#99902106;"
#define BAUD_38400	"#99902107;"
#define BAUD_57600	"#99902110;"
#define BAUD_115200	"#99902111;"



#define	SAVE_DATA_PARAM			"#99900020;"	//保存数据参数

int init_LV1000_card(int uart_port);
int quit_LV1000_card(int uart_port);
int set_LV1000_card(int uart_port,char *value);
int read_LV1000_card(int uart_port,char *value);
int set_LV1000_mode(int mode, int port_number, int data1, int data2, int psame,int restart, int baud);

#endif // !defined(COMM_H__)

