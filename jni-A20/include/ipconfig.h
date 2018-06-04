/**
 * @chinese
 * @file   ipconfig.h
 * @author 胡俊远
 * @date   Wed Jul 13 10:01:32 2011
 *
 * @brief  获得终端机ip,mask等信息模块
 * @endchinese
 *
 *
 * @english
 * @file   ipconfig.h
 * @author Hu Junyuan
 * @date   Wed Jul 13 10:01:32 2011
 *
 * @brief  get terminal's ip address,mask information etc.
 * @endenglish
 *
 *
 */
#ifndef __IPCONFIG_H__
#define __IPCONFIG_H__

#define NET_UP    1
#define NET_DOWN    0

int get_net_state(const char *interface_name);
int set_net_state(char *interface_name, int flag);
int get_ip(char *interface_name, char *ip_str);
int set_ip(char *interface_name, char *ip_str);
int get_mac_addr(char *interface_name, char *mac_str);
int set_mac_addr(char *interface_name, char *mac_str);
int get_netmask(char *interface_name, char *netmask_str);
int set_netmask(char *interface_name, char *netmask_str);
int get_gateway(char *interface_name, char *gateway_str);
int set_gateway(char *interface_name, char *gateway_str);
void set_dhcp();
//int get_netdevices_state(const char *interface_name);
int  dns2ip(char *dns,char *ip);



int get_rel_mac_addr(char *interface_name, char *mac_str);
//int set_rel_mac_addr(char *interface_name, char *mac_str);


//udhcp

typedef struct __dhcp_info{
    int isEnable;  //是否开启 0-未启用，1-启用
    int isBack;  //0前台1后台
    int runStatus; //1-后台运行,2-后台退出成功
    char interfaceName[64];
}_DhcpInfo;


extern _DhcpInfo DhcpInfo;

int init_udhcp_devices(char *interface_name,int bg_flag);
int open_udhcp_devices(char *interface_name);
int close_udhcp_devices();
int resetDhcpDevices();
int get_wifi_devices_status(char *interface_name);

#endif
