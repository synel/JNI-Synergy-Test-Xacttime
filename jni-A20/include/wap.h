/*
 * wap.h
 *
 *  Created on: 2012-12-29
 *      Author: aduo
 */

#ifndef WAP_H_
#define WAP_H_


#include <sys/time.h>

//SIM 是否存在
#define NOSIM	0
#define HASSIM	1

//add by aduo 2013.1.4
//<!--
#define NOT_REGISTERED 0
#define REGISTERED 1

#define MAX_NUMBER_OF_INITIALIZATION 10
#define MAX_NUMBER_OF_ATTEMPT_CONNECTION 5
//尝试连接时间间隔(秒)
//#define INTERVAL_ATTEMPT_CONNECTION 30 //add by aduo 2013.6.25
#define INTERVAL_ATTEMPT_INITIALIZATION 5 //add by aduo 2013.7.15
//-->
//
#define CLOSED 0
#define OPENED 1

#define SEND_FAIL 0
#define SEND_OK 1
#define RECV_FAIL 2
#define RECV_OK 3

//add by aduo 2013.10.25
//<!--
#define WAITING 0
#define RUNNING 2

#define SUCCEED 1
#define TIMEOUTED 3
#define FAILED 4
//-->


//发送MAC的时间间隔(秒)
//#define INTERVAL_SEND_MAC	300
//接收结果码最长时间(秒)，超过这个时间则退出
//#define INTERVAL_RECV_FEEDBACK	300
//#define INTERVAL_RECV_FEEDBACK	10

/*
 *每个指令的内容，超时时间
 */
typedef struct _CMD_DESCR
{
	char *content;
        float timeout;
	int state;
	time_t askfor_time;
	time_t answer_time;
}CMD_DESCR;


/* -----------------------------------------------------
*                       AT指令列表
* -----------------------------------------------------*/
struct at_command
{
    //共用部分
	CMD_DESCR at;//输入"AT"指令
    char begin[3];//输入"AT"指令
    char end[3];//回车

    CMD_DESCR set_wireless_module_baud_rate;//设置波特率

    CMD_DESCR ate;
    CMD_DESCR atq;

    CMD_DESCR get_wireless_module_flow_control;
    CMD_DESCR set_wireless_module_flow_control; //设置 TE-TA 间流控

    CMD_DESCR get_wireless_module_error_report;
    CMD_DESCR set_wireless_module_error_report;

    CMD_DESCR wake_up_wireless_module; //唤醒模块 add by aduo 2012.12.27
    CMD_DESCR sleep_wireless_module;   //休眠模块 add by aduo 2012.12.27

    CMD_DESCR reset_wireless_module; //复位模块 add by aduo 2012.12.27

    CMD_DESCR get_gmm;//获得GSM模块型号
    CMD_DESCR get_remote_ip;//根据域名获得的IP

    CMD_DESCR get_csq;//检测信号强度
    CMD_DESCR chk_sim;//检测SIM卡是否存在

    //CMD_DESCR get_gsm_network_registration_status;//检测网络是否注册
    //CMD_DESCR set_gsm_network_registration_status;

    //CMD_DESCR get_gprs_network_registration_status;//检测网络是否注册
    //CMD_DESCR set_gprs_network_registration_status;

    CMD_DESCR get_network_registration_status;//检测网络是否注册
    CMD_DESCR set_network_registration_status;

    CMD_DESCR get_geographic_coordinates;//获取地理坐标

    CMD_DESCR def_pdp_context;//设置context

    CMD_DESCR set_bearer_type;//选择载体GPRS/CSD

    CMD_DESCR set_ap; //设置ap名称及ap用户名及ap密码
    CMD_DESCR set_ap_name;//设置接入点
    CMD_DESCR set_ap_user_passwd;//设置ap用户名及ap密码
    CMD_DESCR set_ap_user;//设置ap用户名
    CMD_DESCR set_ap_passwd;//设置ap密码

    CMD_DESCR get_receive_mode; //获取接收模式，是否开启接收缓存
    CMD_DESCR set_receive_mode;
    CMD_DESCR get_transfer_mode;//获取传输模式
    CMD_DESCR set_transfer_mode;
    CMD_DESCR get_session_mode;//获取会话模式
    CMD_DESCR set_session_mode;
    CMD_DESCR set_encode_mode; //编码模式 add by aduo 2012.12.27
    CMD_DESCR get_tcp_connect_mode; //use ip address or domain name
    CMD_DESCR set_tcp_connect_mode; //use ip address or domain name

    CMD_DESCR get_pdp_context_status;
    CMD_DESCR open_pdp_context;//激活context

    CMD_DESCR get_local_ip; //模块分配到的IP
    //tcp
    CMD_DESCR get_tcp_session_status;//检测链接是否打开
    CMD_DESCR open_tcp_session;//打开链接

    CMD_DESCR send_tcp_data;//发送数据
    CMD_DESCR ack_tcp_data_for_send;
    CMD_DESCR recv_tcp_data;//接收数据
    CMD_DESCR close_tcp_session;//关闭链接

    //udp
    CMD_DESCR open_udp_session;//打开链接
    CMD_DESCR send_udp_data;//发送数据
    CMD_DESCR recv_udp_data;//接收数据
    CMD_DESCR close_udp_session;//关闭链接

    CMD_DESCR close_pdp_context;//关闭context

    CMD_DESCR RECV_DATA;//接收数据

    char RJK_CALL[10];//挂断电话

    //EM200
    char HCTCM[15];//关闭兼容CM320模块
}atc;

/* -----------------------------------------------------
*                       结果码
* -----------------------------------------------------*/
struct at_response
{
    //共用部分
    char OK[7];
    char ERR[10];

    char prompt_for_start_up[35]; //add by aduo 2012.12.28
    char prompt_for_shut_down[35]; //add by aduo 2012.12.28

    char normal_power_on[35]; //add by aduo 2012.12.28
    char normal_power_off[35]; //add by aduo 2012.12.28

    //basic configration
    char none_flow_control[35];
    char software_flow_control[35];//XON/XOFF
    char hardware_flow_control[35];//RTS/CTS

    char none_error_report[35];
    char numeric_error_report[35];
    char verbose_error_report[35];

    char GET_GMM[15];
    char local_ip[20];
    char remote_ip[20];

    char GET_CSQ[15];

    char HAS_SIM[25];//有SIM卡
    char NO_SIM[25];//没有SIM卡

    //add by aduo 2013.1.4
    //<!--
    char unregistered_network_registration_status[35];//检测网络是否注册，未注册
    char local_registered_network_registration_status[35];//检测网络是否注册，已注册
    char not_registered_network_registration_status[35];
    char denied_registered_network_registration_status[35];
    char unknown_registered_network_registration_status[35];
    char roaming_registered_network_registration_status[35];//检测网络是否注册，已注册
    //-->

    char geographic_coordinates[35];
    //tcp configration
    char normal_receive_mode[35];
    char buffer_receive_mode[35];

    char normal_transfer_mode[35];
    char transparent_transfer_mode[35];

    char normal_encode_mode[35]; //add by aduo 2012.12.27
    char compress_encode_mode[35];//add by aduo 2012.12.27

    char tcp_connect_ip_mode[35];
    char tcp_connect_dn_mode[35];

    char single_session_mode[35];
    char multiple_session_mode[35];

    //pdp context
    char pdp_context_closed[35];//非请求反馈

    char pdp_context_connect_connecting[35];//用于状态查询
    char pdp_context_connect_opened[35];//用于状态查询
    char pdp_context_connect_closed[35];//用于状态查询

    char pdp_context_connect_ok[35];//打开ppp链接成功
    char pdp_context_connect_fail[35];
    char pdp_context_connect_already_open[35];

    char pdp_context_disconnect_ok[35];
    char pdp_context_disconnect_fail[35];//add by aduo 2012.12.28
    char pdp_context_connect_already_close[35];
    //tcp
    char tcp_channel_1_closed[35];//非请求反馈
    char tcp_data_arrival[20];//非请求反馈 不使用接收缓存
    char tcp_data_coming[20];//非请求反馈 使用接收缓存

    char tcp_connect_connecting[35];//用于状态查询
    char tcp_connect_opened[35];//用于状态查询 链接已打开
    char tcp_connect_closed[35];//用于状态查询 链接未打开

    char tcp_channel_1_connect_ok[35];//打开链接成功
    char tcp_channel_1_connect_error[35];
    char tcp_channel_1_connect_fail[35];//服务器端出现问题
    char tcp_channel_1_connect_denied[35];
    char tcp_channel_1_connect_timeout[35];//链接超时
    char tcp_channel_1_connect_already_open[35];//链接已打开

    char prompt_for_tcp_data_input[10];
    char tcp_data_send_ok[35];//发送成功
    char tcp_data_send_ack_ok[35];
    char tcp_data_send_text_too_long[35];
    char tcp_data_send_buffer_full[35];//发送缓存满
    char tcp_data_send_fail[35];//服务器端出现问题

    char tcp_data_recv_ok[35];//接收成功 使用接收缓存
    char tcp_data_recv_invalid_index[35];
    char tcp_data_recv_buffer_full[35];//接收缓存满

    char tcp_channel_1_disconnect_ok[35];//关闭链接成功
    char tcp_channel_1_disconnect_fail[35];
    char tcp_channel_1_connect_already_close[35];//链接已关闭

    //udp
    char udp_connect_opened[35];//用于状态查询 链接已打开
    char udp_connect_closed[35];//用于状态查询 链接未打开

    char udp_channel_1_connect_ok[35];//打开链接成功
    char udp_channel_1_connect_error[35];
    char udp_channel_1_connect_fail[35];//服务器端出现问题
    char udp_channel_1_connect_denied[35];
    char udp_channel_1_connect_timeout[35];//链接超时
    char udp_channel_1_connect_already_open[35];//链接已打开

    char udp_channel_1_closed[35];
    char udp_data_arrival[20];//非请求反馈 不使用接收缓存
    char udp_data_coming[20];//非请求反馈 使用接收缓存

    char prompt_for_udp_data_input[10];
    char udp_data_send_ok[35];//发送成功
    char udp_data_send_text_too_long[35];
    char udp_data_send_buffer_full[35];//发送缓存满
    char udp_data_send_fail[35];

    char udp_channel_1_disconnect_ok[35];//关闭链接成功
    char udp_channel_1_disconnect_fail[35];
    char udp_channel_1_connect_already_close[35];//链接已关闭

    //sms
    char sms_text_mode[35];
    char sms_pdu_mode[35];

}MARK;

/* -----------------------------------------------------
*                     返回结果码转译信号
*                   GPRS模块返回信息的类型
* -----------------------------------------------------*/
enum __T_MESSAGE
{
    /*预留:用于判断读取信息是否完毕;是否出现错误(比如超时)*/
    T_OVER=1,//完毕
    T_NOVER=2,//未完毕
    T_CRASH=3,//获得结果码超时,认定死机
    T_FAILURE=4,//出现错误

    /*常规信息类型*/
    T_OK=5,//正确
    T_ERROR,//不可修复错误
    T_RETRY,//出现错误，但是可以重新执行

    //特殊
    //T_NEED_INIT,//需要进行初始化的错误
    //T_NEED_LINK,//需要进行链接的错误

    /**/
    T_PDP_CONTEXT_CONNECT_OPENED,
    T_PDP_CONTEXT_CONNECT_CLOSED,

    T_PDP_CONTEXT_CONNECT_OK,
    T_PDP_CONTEXT_CONNECT_ALREADY_OPEN,
    T_PDP_CONTEXT_CONNECT_FAIL,

    T_PDP_CONTEXT_DISCONNECT_OK,
    T_PDP_CONTEXT_CONNECT_ALREADY_CLOSE,

    T_SINGLE_SESSION_MODE,
    T_MULTIPLE_SESSION_MODE,

    T_NORMAL_TRANSFER_MODE,
    T_TRANSPARENT_TRANSFER_MODE,

    /*打开链接*/
    T_NETWORK_OUT,//从网络注销
    T_LINK_TIMEOVER,//链接超时

    /*发送数据*/
    //T_LINK_OUT,//链接断开
    T_SEND_NO_SPACE,//发送缓存满
    T_PROMPT_FOR_TCP_DATA_INPUT,
    T_TCP_DATA_SEND_OK,
    T_TCP_DATA_SEND_FAIL,
    T_TCP_DATA_SEND_ACK_OK,

    T_PROMPT_FOR_UDP_DATA_INPUT,
    T_UDP_DATA_SEND_OK,
    T_UDP_DATA_SEND_FAIL,

    /*接收数据*/
    T_TCP_DATA_ARRIVAL,
    T_UDP_DATA_ARRIVAL,

    T_TCP_DATA_COMING,//模块接收数据
    T_UDP_DATA_COMING,//模块接收数据
    //T_DATA_COMING,//模块接收数据
    T_NO_DATA,//接收缓存没有数据
    T_RECV_NO_SPACE,//接收缓存已满

    /*SIM卡*/
    T_SIM0,//没有SIM卡
    T_SIM1,//有SIM卡

    /*是否注册网络*/
    T_NETWORK_REGED,//已注册
    T_NETWORK_UNREG,//未注册

    /*检测链接是否打开*/
    T_LINK_OPENED,//链接已打开
    T_LINK_UNOPEN,//链接未打开

    T_TCP_CHANNEL_1_CONNECT_OK,//链接已打开
    T_TCP_CHANNEL_1_CONNECT_FAIL,//服务端未开启软件
    T_TCP_CHANNEL_1_CONNECT_ALREADY_OPEN,//链接已打开

    T_TCP_CHANNEL_1_DISCONNECT_OK,//链接关闭成功
    T_TCP_CHANNEL_1_CONNECT_ALREADY_CLOSE,//链接已关闭

    T_TCP_CHANNEL_1_CLOSED,//链接关闭

    //add by aduo 2013.1.25
    T_UDP_CHANNEL_1_CONNECT_OK,//链接已打开
    T_UDP_CHANNEL_1_CONNECT_FAIL,//服务端未开启软件
    T_UDP_CHANNEL_1_CONNECT_ALREADY_OPEN,//链接已打开

    T_UDP_CHANNEL_1_DISCONNECT_OK,//链接关闭成功
    T_UDP_CHANNEL_1_CONNECT_ALREADY_CLOSE,//链接已关闭

    T_UDP_CHANNEL_1_CLOSED,//链接关闭
    //<--
    //-->


    /*有来电*/
    T_CALL,

    /*串口和模块波特率不一样*/
    T_BAUD_RATE_ERROR,

    T_UNKNOWN
};

/* -----------------------------------------------------
*
* 执行某个AT指令，通过分析信息类型，根据当前指令和状态机返回的类型
*
* -----------------------------------------------------*/
enum __R_MESSAGE
{
    R_FAILURE=-1,   //执行失败
    R_DEFAULT=1,
    R_SUCCESS,      //执行成功
    R_CONTINUE,
    R_RETRY,        //执行过程中出现了错误，并且将错误排除后，需要重新执行该指令
    R_IGNORE,       //出现的错误可以忽略
    R_WAIT,         //暂缓执行指令，当前不符合执行某指令的条件
    R_LOCKED,       //GPRS正在执行其他指令
    R_CLOSED,       //GPRS模块没有开启
    R_SIM0,         //没有SIM卡
    R_SIM1,         //有SIM卡
    R_NETWORK_REGED,//网络已注册
    R_NETWORK_UNREG,//网络未注册
    R_LINK_OPENED,  //链接已打开
    R_LINK_UNOPEN,  //链接未打开
    R_TCP_CHANNEL_1_CLOSED,
    R_UDP_CHANNEL_1_CLOSED,
    R_STOP_SEND_DATA,//当发送窗口已满时，返回停止发送数据信号
    R_DATA_COMING,
    R_TCP_DATA_COMING,
    R_UDP_DATA_COMING,
    R_NODATA,       //接收缓存没有数据
    R_NOSPACE       //发送缓存没有剩余空间
};

/* -----------------------------------------------------
*                       状态机
* -----------------------------------------------------*/
enum __STATE
{
    ms_power_off,
    ms_power_on,
    ms_shut_down,
    ms_start_up,
    ms_busy,
    ms_free//空闲
};

enum _operating_state
{
    os_empty,
    os_prepare,
    os_initialize,
    os_open_session,
    os_send_data,
    os_recv_data,
    os_close_session,
    os_finalize
};

/* -----------------------------------------------------
*                       模块型号
* -----------------------------------------------------*/
enum __MODEL
{
    NONE,
    EM310,//华为EM310
    EM200,//华为EM200
    //MC323,//华为MC323
    M20,//移远
    M35,
    MC8331A,//中兴
    MC8332, //add by aduo 2013.7.12
    U10//移远
};

#define MAX_PROXY_HOSTS  2
struct _proxy_host_list //add by aduo 2012.12.29
{
	unsigned host[MAX_PROXY_HOSTS][128];
	int count;
};

#define MAX_REMOTE_HOSTS  2
struct _remote_host_list //add by aduo 2012.12.29
{
	unsigned host[MAX_REMOTE_HOSTS][128];
	int count;
};

#define MAX_DNS_SERVERS  2
struct _dns_server_list //add by aduo 2012.12.29
{
	unsigned server[MAX_DNS_SERVERS][20];
	int count;
};

/* -----------------------------------------------------
*                       GPRS的设置信息
* -----------------------------------------------------*/
struct __gprsset
{
	int rate,port;//,overtime,protocol;//rate:GPRS和串口的波特率;port:串口ID;overtime:读取数据和发送数据不能超过这个时间否则返回错误
	char deputyip[63];//代理服务器IP/domain name
	//char deputydn[63];//代理服务器 domain name
	int deputyport;//代理服务器PORT

	//add by aduo 2012.12.29
	//<!--
	struct _proxy_host_list proxy_host;//代理服务器IP/domain name
	int proxy_port;//代理服务器PORT

	struct _remote_host_list remote_host;//远程服务器IP/domain name
	int remote_port;//远程服务器PORT

	struct _dns_server_list dns_server;

	//-->

	char APN[100];//接入点
	char UserName[50];//用户名
	char Password[50];//密码
	char module_model[40];//用户设置的模块型号
};

/* -----------------------------------------------------
*                       GPRS状态信息
* -----------------------------------------------------*/
struct __gprsinfo
{
	int baud_rate;
    int TSIM;//是否存在SIM卡
    int network_registration_status; //add by aduo 2013.1.4
    int pdp_context_status; //add by aduo 2014.5.23
    int rssi;//信号强度 Receive signal strength indicator
    int ber; //信道误码率 Bit error rate
    int sql;//信号格数 signal quality level 0..4
    char module_model[40]; //程序自动获取的模块型号
    char local_ip[40];//模块分配到的IP
    //char *Mac;//MAC地址
};

/* -----------------------------------------------------
*                       保存接收数据
* -----------------------------------------------------*/
/*
struct __data_recv
{
    char data[40960];
    int len;
};
*/

#define MAX_TCP_SESSIONS  10
#define MAX_UDP_SESSIONS  10

//保存接收到的但是没有传送给协议层的数据
struct _tcp_data_recv
{
	unsigned char data[MAX_TCP_SESSIONS][40960];
    int len[MAX_TCP_SESSIONS];
}tcp_data_recv;

struct _udp_data_recv
{
	unsigned data[MAX_UDP_SESSIONS][40960];
    int len[MAX_UDP_SESSIONS];
}udp_data_recv;


extern int GPRS_FD;//GPRS文件标识符,其实就是和GPRS通信的串口,这是发送指令和接收数据的通道
extern enum __STATE GPRS_STATE;//记录GPRS当前的状态，服务于状态机机制
extern enum _operating_state operating_state;
extern enum __MODEL MODEL;//记录当前的模块型号
extern int GPRS_INIT_NUM;//模块初始化次数
extern int number_of_attempt_connection; //add by aduo 2013.1.4
extern struct __gprsset gprsset;//设置GPRS通信参数设置的结构体
extern struct __gprsinfo gprsinfo;//保存模块信息：信号强度
extern time_t LastTimeOfSend, LastTimeOfRecv;//保存最近发送和接收数据的时间
extern char CUR_CMD[256];//保存正在执行的指令
extern char *CMD_ZERO;//表示没有指令正在执行的标志位
extern int NumSpeHandle;//当出现假死现象时，特殊处理器处理的次数.
extern int DNIP; ////0 = use IP address as the address to establish TCP/UDP session. 1 = use domain name as the address to establish a TCP/UDP session
extern int tcp_channel_status[MAX_TCP_SESSIONS];
extern int udp_channel_status[MAX_UDP_SESSIONS];
/*内部函数*/
void* thread_2g_link();
int open_watch_link();
int close_watch_link();
/*API函数*/
int mobile_comm_init(char *Model,char *APN,char *UserName,char *Password,int port,int rate);
int mobile_comm_local_ip(char *ip_addr);
int mobile_comm_open(char *deputyip,int deputyport);
int mobile_comm_send_data(unsigned char *data,unsigned int len);
int mobile_comm_recv_data(unsigned char *data,unsigned int len);
int mobile_comm_connection_status();
int mobile_comm_close();
int mobile_comm_uninit();
//udp
int mobile_comm_open_udp(char *deputyip,int deputyport);
int mobile_comm_send_udp_data(unsigned char *data,unsigned int len);
int mobile_comm_recv_udp_data(unsigned char *data,unsigned int len);
int mobile_comm_close_udp();

int gprs_get_sql_api();
int gprs_get_csq_api();
int gprs_check_sim_api();

/*接口函数*/
int initialize_at_command();
int power_on_wireless_module();
int reset_wireless_module();
int initialize_wireless_module();
int get_local_ip(char *ip_addr);
int gprs_check_link();
int gprs_open_link();
int gprs_send_data(unsigned char *data,unsigned int len);
int gprs_recv_data(unsigned char *data,unsigned int len);
int gprs_close_link();

//udp
int gprs_open_udp_link();
int gprs_send_udp_data(unsigned char *data,unsigned int len);
int gprs_recv_udp_data(unsigned char *data,unsigned int len);
int gprs_close_udp_link();

int finalize_wireless_module();
int power_off_wireless_module();


/*底层函数*/
int SEND_COMMAND(int fd,unsigned char *buf,int len);
int RECV_FEEDBACK(int fd,unsigned char *buf,int len);
int HANDLE_FEEDBACK(unsigned char *buf, int len);
int CORE_PROCESSOR(enum __T_MESSAGE T_MESSAGE,enum __STATE STATE);
//int SPE_PROCESSOR();
void SET_CURCMD(char *buf);
int CHK_CURCMD(char *buf);


/*工具函数*/
int ascii_2_hex(unsigned char *data,unsigned char *buffer, int len);
int hex_2_ascii(unsigned char *O_data,unsigned char *N_data, int len);
int comp_char(unsigned char *buf1,int len1,unsigned char *buf2,int len2);
int diffutime(struct timeval newtimer,struct timeval oldtimer);

void gprs_set_recv_time();
void gprs_set_send_time();

/*功能函数*/
int is_wireless_module_power_on();
int initial_preparation();
int autobauding_synchronization();
int set_wireless_module_baud_rate(int baudrate);
int set_ate();
int set_atq();
int set_wireless_module_flow_control(int dce_by_dte,int dte_by_dce);
int set_wireless_module_error_report(int n);
int get_csq();
int check_sim();
int get_network_registration_status();//add by aduo 2013.1.4
int set_network_registration_status(int n);//add by aduo 2013.1.4
int get_gmm();//response

int get_data_from_inbuffer(unsigned char *data, int start,int len);

#endif /* WAP_H_ */
