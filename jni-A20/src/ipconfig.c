/**
 * @chinese
 * @file   ipconfig.c
 * @author 胡俊远
 * @date   Wed Jul 13 10:01:32 2011
 *
 * @brief  获得终端机ip,mask等信息模块
 * @endchinese
 *
 *
 * @english
 * @file   ipconfig.c
 * @author Hu Junyuan
 * @date   Wed Jul 13 10:01:32 2011
 *
 * @brief  get terminal's ip address,mask information etc.
 * @endenglish
 *
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/route.h>
#include <net/if_arp.h>
#include <pthread.h>
#include <ctype.h>

#include <netdb.h>
extern int h_errno;
#ifdef _ARM_A23
#include "../android/wedsa23.h"
#endif

#include "ipconfig.h"
#include "config.h"
#include "stringchar.h"
#include "debug.h"
#ifndef _ARM_FROCK
#include "wap.h"
#endif

/**
 * @chinese
 * 设置网络所有参数
 *
 * @param interface_name 网络接口
 * @param dhcp 是否自动获得IP [1:使用DHCP  0：不使用DHCP]
 * @param ip_str IP
 * @param netmask_str 掩码
 * @param gateway_str 网关
 * @param mac_str MAC地址
 *
 * @return 成功-SUCCESS, 失败-ERROR
 * @endchinese
 *
 * @english
 * set net all attr
 *
 * @param interface_name interface name
 * @param dhcp dhcp
 * @param ip_str IP
 * @param netmask_str netmask
 * @param gateway_str gateway
 * @param mac_str MAC
 *
 * @return success-SUCCESS, fail-ERROR
 * @endenglish
 *
 */
void set_ipconfig_all_attr(char *interface_name, int dhcp, char *ip_str,  char *netmask_str, char *gateway_str, char *mac_str)
{
    if(!interface_name)
    {
        return ;
    }

    if(dhcp==1)
    {
        set_dhcp();
    }
    else
    {
        set_ip(interface_name, ip_str);
        set_mac_addr(interface_name, mac_str);
        set_netmask(interface_name, netmask_str);
        set_gateway(interface_name, gateway_str);
    }
}

/**
 * @chinese
 * 设置网络状态
 *
 * @param interface_name 网络接口
 * @param flag NET_UP：开启 NET_DOWN：关闭
 *
 * @return 成功-SUCCESS, 失败-ERROR
 * @endchinese
 *
 * @english
 * set net state
 *
 * @param interface_name interface name
 * @param flag NET_UP：up NET_DOWN：down
 *
 * @return success-SUCCESS, fail-ERROR
 * @endenglish
 *
 */
int set_net_state(char *interface_name, int flag)
{
    int retval = 0;
    char *net_flag = "up";
    char tmp[128];

    if(!interface_name)
    {
        return ERROR;
    }

    memset(tmp,0,sizeof(tmp));
    if(flag==NET_DOWN)
    {
        net_flag ="down";
    }
#ifndef _ARM_A23
    sprintf(tmp,"ifconfig %s %s",interface_name,net_flag);
#else
    sprintf(tmp,"su -c \"busybox ifconfig %s %s\"",interface_name,net_flag);
#endif
    retval = system(tmp);
    if(retval == -1)
    {
        plog("set_net_state error!") ;
        return ERROR;
    }
    return SUCCESS;
}

// 检测网络连接

// routeName: 网络连接名称，如ppp0、eth0等

// 返回值: 网络正常返回1，异常返回-1

int get_net_state(const char *interface_name)
{
    int get_net_state_fd = -1,ret=0;
    struct ifreq ifr;

    if(!interface_name)
    {
        return ERROR;
    }

    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, interface_name);

    get_net_state_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (get_net_state_fd < 0)
    {
        get_net_state_fd = -1;
        plog("get net state error\n");
        return ERROR;
    }

    if( 0!=(ioctl(get_net_state_fd, SIOCGIFFLAGS, (char*)&ifr)) )
    {
            close(get_net_state_fd);
            get_net_state_fd = -1;
            return ERROR;
    }
    ret=close(get_net_state_fd);
    get_net_state_fd = -1;

    if ((ifr.ifr_flags & IFF_UP) && (ifr.ifr_flags & IFF_RUNNING))
    {
        plog("get net state success\n");
        return SUCCESS;
    }
    else
    {
        return ERROR;
    }

}
/**
 * @chinese
 * 获得IP地址
 *
 * @param interface_name 网络接口
 * @param ip_str 保存IP地址
 *
 * @return 成功-SUCCESS, 失败-ERROR
 * @endchinese
 *
 * @english
 * get ip
 *
 * @param interface_name interface name
 * @param ip_str save ip
 *
 * @return success-SUCCESS, fail-ERROR
 * @endenglish
 *
 */

int  GetLocalIp(char *interface_name, char *ipaddr)
{

    int sock_get_ip;
    //char ipaddr[50];

    struct   sockaddr_in *sin;
    struct   ifreq ifr_ip;

    if ((sock_get_ip=socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
         printf("socket create failse...GetLocalIp!/n");
         return ERROR;
    }

    memset(&ifr_ip, 0, sizeof(ifr_ip));
    strncpy(ifr_ip.ifr_name, interface_name, sizeof(ifr_ip.ifr_name) - 1);

    if( ioctl( sock_get_ip, SIOCGIFADDR, &ifr_ip) < 0 )
    {
        close(sock_get_ip);
         return ERROR;
    }
    sin = (struct sockaddr_in *)&ifr_ip.ifr_addr;
    strcpy(ipaddr,inet_ntoa(sin->sin_addr));

    //printf("local ip:%s \n",ipaddr);
    close( sock_get_ip );

    return SUCCESS;
}
int  GetLocalNetMask(char *interface_name, char *netmask_addr)
{
    int sock_netmask;
   // char netmask_addr[50];

    struct ifreq ifr_mask;
    struct sockaddr_in *net_mask;

    sock_netmask = socket( AF_INET, SOCK_STREAM, 0 );
    if( sock_netmask == -1)
    {
        perror("create socket failture...GetLocalNetMask/n");
        return ERROR;
    }

    memset(&ifr_mask, 0, sizeof(ifr_mask));
    strncpy(ifr_mask.ifr_name, interface_name, sizeof(ifr_mask.ifr_name )-1);

    if( (ioctl( sock_netmask, SIOCGIFNETMASK, &ifr_mask ) ) < 0 )
    {
        printf("mac ioctl error/n");
        close( sock_netmask );
        return ERROR;
    }

    net_mask = ( struct sockaddr_in * )&( ifr_mask.ifr_netmask );
    strcpy( netmask_addr, inet_ntoa( net_mask -> sin_addr ) );

//    printf("local netmask:%s/n",netmask_addr);

    close( sock_netmask );
    return SUCCESS;
}



int get_ip(char *interface_name, char *ip_str)
{
    FILE *file = NULL;
    int up_flag = 0,len=0;
    char buff[128], *ptr = NULL,*p="inet addr:";
    int retval = 0,i=0;


#ifndef _ARM_FROCK
#ifndef _PC44
    if(strcmp(interface_name,"gprs") == 0)
    {
        retval = mobile_comm_local_ip(ip_str);
        if(retval < 0)
        {
            return ERROR;
        }
        return SUCCESS;
    }

    retval=GetLocalIp(interface_name,ip_str);
    return retval;

#endif
#endif
    /*获取其他网络设备类型ip地址*/
#ifndef _ARM_A23
    file = popen( "ifconfig", "r" );
#else
    file = popen( "su -c \"busybox ifconfig\"", "r" );

#endif
    if( !file )
    {
        return ERROR;
    }
    while( fgets( buff, sizeof( buff ), file ) )
    {
        if(strncmp(interface_name,buff,strlen((char*)interface_name)) == 0)
        {
            up_flag=1;
            i=0;
        }

        if(up_flag == 1)
        {
            ptr = strstr( buff, p ); //查找是否含有"inet addr:"
            if( ptr )
            {
                ptr=ptr+strlen(p);
                len = strcspn(ptr," ");
                strncpy( ip_str, ptr , len );
                ip_str[len]=0;
                ip_str[len+1]=0;
                ip_str[len+2]=0;
                pclose( file );
                return SUCCESS;
            }
            if((++i) >= 3)
            {
                pclose(file);
                return ERROR;
            }
        }
    }
    pclose( file );
    return ERROR;
}

/**
 * @chinese
 * 设置IP地址
 *
 * @param interface_name 网络接口
 * @param ip_str IP地址 如"192.168.10.110"
 *
 * @return 成功-SUCCESS, 失败-ERROR
 * @endchinese
 *
 * @english
 * set ip
 *
 * @param interface_name interface name
 * @param ip_str ip
 *
 * @return success-SUCCESS, fail-ERROR
 * @endenglish
 *
 */
int set_ip(char *interface_name, char *ip_str)
{
    int retval = 0;
    char tmp[128];

    if(!interface_name)
    {
        return ERROR;
    }

    memset(tmp,0,sizeof(tmp));
#ifndef _ARM_A23
    sprintf(tmp,"ifconfig %s %s up",interface_name,ip_str);
#else
    sprintf(tmp,"su -c \"busybox ifconfig %s %s up\"",interface_name,ip_str);
#endif
    retval = system(tmp);
    if(retval == -1)
    {
        plog("set_net_state error!") ;
        return ERROR;
    }
    return SUCCESS;
}

/**
 * @chinese
 * 获得MAC地址
 *
 * @param interface_name 网络接口
 * @param mac_str 保存MAC地址
 *
 * @return 成功-SUCCESS, 失败-ERROR
 * @endchinese
 *
 * @english
 * get mac
 *
 * @param interface_name interface name
 * @param mac_str save mac
 *
 * @return success-SUCCESS, fail-ERROR
 * @endenglish
 *
 */


int get_mac_addr(char *interface_name, char *mac_str)
{
    FILE *file=NULL;
    char buff[64],*ptr=NULL;
#ifdef _PC44
    file=popen("ifconfig","r");
#else
#ifndef _ARM_A23
    file=fopen("/weds/mac.sh","r");
#else
    file=fopen("/mnt/app/mac.sh","r");
#endif
#endif
    if(!file || mac_str == NULL)
    {
        return ERROR;
    }

#ifndef _PC44
    memset(buff,0,sizeof(buff));
    fgets(buff,sizeof(buff),file);
    memset(buff,0,sizeof(buff));
    fgets(buff,sizeof(buff),file);
    ptr=strstr(buff,"ether");
    if(ptr)
    {
        memset(mac_str,0,sizeof(mac_str));
        strncpy(mac_str,ptr+6,17);
        cut_rn(mac_str);
    }
    fclose(file);
#else
    while(fgets(buff,sizeof(buff),file))
    {
        cut_rn(buff);
        ptr=strstr(buff,"HWaddr");
        if(ptr)
        {
            strncpy(mac_str,ptr+7,18);
            break;
        }
    }
    pclose(file);
#endif
    return SUCCESS;
}



int get_rel_mac_addr(char *interface_name, char *mac_str)
{
    FILE *file=NULL;
    char buff[64],*ptr=NULL;
    char command[32];
    int flag=0;
    sprintf(command,"busybox ifconfig %s",interface_name);
    file=popen(command,"r");

    if(!file || mac_str == NULL)
    {
        return ERROR;
    }

    while(fgets(buff,sizeof(buff),file))
    {
        cut_rn(buff);
        ptr=strstr(buff,"HWaddr");
       // printf("%s %s\n",__func__,buff);
        if(ptr)
        {
            flag=1;
            strncpy(mac_str,ptr+7,18);
            mac_str[18]=0;
           // printf("%s %s\n",__func__,mac_str);
            break;
        }
    }
    pclose(file);
    if(flag==0)
        return ERROR;
    return SUCCESS;
}

/**
 * @chinese
 * 设置MAC地址
 *
 * @param interface_name 网络接口
 * @param mac_str MAC地址 如"1A:3C:02:01:02:2A"
 *
 * @return 成功-SUCCESS, 失败-ERROR
 * @endchinese
 *
 * @english
 * set mac
 *
 * @param interface_name interface name
 * @param mac_str mac
 *
 * @return success-SUCCESS, fail-ERROR
 * @endenglish
 *
 */
int set_mac_addr(char *interface_name, char *mac_str)
{
    int retval;
    char str[15]={0};
    char mac[6],tmp[128];

    if(!interface_name || !mac_str)
    {
        return ERROR;
    }

    sscanf(mac_str, "%2s:%2s:%2s:%2s:%2s:%2s", str, str+2, str+4, str+6, str+8, str+10);
    if(strlen(str)<12)
    {
        return ERROR;
    }

    string2hex(str,mac,strlen(str));

#ifdef _ARM_2410
    set_net_state(interface_name, NET_DOWN);
#endif

#ifdef _ARM_2416
    set_net_state(interface_name, NET_DOWN);
#endif

    memset(tmp,0,sizeof(tmp));
#ifndef _ARM_A23
    sprintf(tmp,"ifconfig %s hw ether %s ",interface_name,mac_str);
#else
    sprintf(tmp,"su -c \"busybox ifconfig %s hw ether %s\"",interface_name,mac_str);
#endif
    retval = system(tmp);

#ifdef _ARM_2410
    set_net_state(interface_name, NET_UP);
#endif

#ifdef _ARM_2416
    set_net_state(interface_name, NET_UP);
#endif

    if(retval<0)
    {
        return ERROR;
    }

    return SUCCESS;
}

/**
 * @chinese
 * 获得掩码
 *
 * @param interface_name 网络接口
 * @param netmask_str 保存掩码
 *
 * @return 成功-SUCCESS, 失败-ERROR
 * @endchinese
 *
 * @english
 * get net mask
 *
 * @param interface_name interface name
 * @param netmask_str save net mask
 *
 * @return success-SUCCESS, fail-ERROR
 * @endenglish
 *
 */
int get_netmask(char *interface_name, char *netmask_str)
{
    FILE *file = NULL;
    int up_flag = 0,len=0,i=0;
    char buff[128], *ptr = NULL,*p="Mask:";
    int ret=0;

    ret=GetLocalNetMask(interface_name,netmask_str);
    return ret;
#ifndef _ARM_A23
    file = popen( "ifconfig", "r" );
#else
    file = popen( "su -c \"busybox ifconfig\"", "r" );
#endif
    if( !file )
    {
        return ERROR;
    }

    while( fgets( buff, sizeof( buff ), file ) )
    {
        if(strncmp(interface_name,buff,strlen((char*)interface_name)) == 0)
        {
            up_flag=1;
            i=0;
        }

        if(up_flag == 1)
        {
            ptr = strstr( buff, p );
            if( ptr )
            {
                ptr=ptr+strlen(p);
                len = strcspn(ptr,"\r\n");
                strncpy( netmask_str, ptr , len );
                pclose( file );
                return SUCCESS;
            }
            if((++i)>=3)
            {
                pclose( file );
                return ERROR;
            }
        }
    }
    pclose( file );
    return SUCCESS;

}

/**
 * @chinese
 * 设置掩码
 *
 * @param interface_name 网络接口
 * @param netmask_str 掩码
 *
 * @return 成功-SUCCESS, 失败-ERROR
 * @endchinese
 *
 * @english
 * set net mask
 *
 * @param interface_name interface name
 * @param netmask_str net mask
 *
 * @return success-SUCCESS, fail-ERROR
 * @endenglish
 *
 */
int set_netmask(char *interface_name, char *netmask_str)
{
    char tmp[128];
    int retval =0;

    if(!interface_name)
    {
        return ERROR;
    }
    memset(tmp,0,sizeof(tmp));
#ifndef _ARM_A23
    sprintf(tmp,"ifconfig %s netmask %s ",interface_name,netmask_str);
#else
    sprintf(tmp,"su -c \"busybox ifconfig %s netmask %s\"",interface_name,netmask_str);
#endif
    retval = system(tmp);
    if(retval == -1)
    {
        plog("set_net_state error!") ;
        return ERROR;
    }
    return SUCCESS;

}

/**
 * @chinese
 * 获得网关
 *
 * @param interface_name 网络接口
 * @param gateway_str 保存网关
 *
 * @return 成功-SUCCESS, 失败-ERROR
 * @endchinese
 *
 * @english
 * get gateway
 *
 * @param interface_name interface name
 * @param gateway_str save gateway
 *
 * @return success-SUCCESS, fail-ERROR
 * @endenglish
 *
 */

int procip2ip(char *src,char *dst_ip)
{
char dst[6];
gsmString2Bytes(src,dst,8);
sprintf(dst_ip,"%d.%d.%d.%d",dst[3],dst[2],dst[1],dst[0]);
}

int proc_get_gatway(char *interface_name,char * gate_way)
{
FILE *file;
char buf[128];
char *tmp,*str;
int num=0;
file=fopen("/proc/net/route","r");
if(file==NULL)return -1;

    while(fgets(buf, sizeof(buf), file) != NULL)
    {
        tmp=buf;
        num=0;
        while(str=strtok(tmp,"	"))
        {
                tmp=NULL;
                num++;
                if(num==1)
                {
                        if(strcmp(str,interface_name)!=0)
                        break;
                }else if(num==2)
                {
                        if(strcmp(str,"00000000")!=0)
                        break;
                }
                else if(num==3)
                {
                        //strcpy(gate_way,str);
                        procip2ip(str,gate_way);
                        fclose(file);
                        return 1;
                }

        }
    }
fclose(file);
return -1;
}


int get_gateway(char *interface_name, char *gateway_str)
{
#if 1
    FILE *fp;
    char buf[512];
    char cmd[128];
    char gateway[30];
    char *tmp;
    int ret;

    ret= proc_get_gatway(interface_name,gateway_str);
    return ret;

    //这种方案不是最佳，但目前位能找到更好的解决方案
#ifndef _ARM_A23
    strcpy(cmd, "route -n");
#else
    strcpy(cmd, "su -c \"busybox route -n\"");
#endif
    fp = popen(cmd, "r");

    if(NULL == fp)
    {
        perror("popen error");
        return ERROR;
    }
    while(fgets(buf, sizeof(buf), fp) != NULL)
    {

        tmp =buf;
        while(*tmp && isspace(*tmp))
            ++ tmp;
        if((strncmp(tmp, "0.0.0.0", strlen("0.0.0.0")) == 0) &&
                strstr(tmp,interface_name) != 0)
        {
            sscanf(buf, "%*s%s", gateway);
            strcpy(gateway_str,gateway);
            break;
        }
    }
    pclose(fp);
    return SUCCESS;
#endif

}

/**
 * @chinese
 * 设置网关
 *
 * @param interface_name 网络接口
 * @param gateway_str 网关
 *
 * @return 成功-SUCCESS, 失败-ERROR
 * @endchinese
 *
 * @english
 * set gateway
 *
 * @param interface_name interface name
 * @param gateway_str gateway
 *
 * @return success-SUCCESS, fail-ERROR
 * @endenglish
 *
 */
int set_gateway(char *interface_name, char *gateway_str)
{
#if 1
    int retval = 0;
    char tmp[128];
    if(!interface_name)
    {
        return ERROR;
    }

    memset(tmp,0,sizeof(tmp));
#ifndef _ARM_A23
    sprintf(tmp,"route del %s",interface_name);
#else
    sprintf(tmp,"su -c \"busybox route del default\"");
#endif
    retval = system(tmp);

    memset(tmp,0,sizeof(tmp));
#ifndef _ARM_A23
    sprintf(tmp,"route add %s gw %s",interface_name,gateway_str);
#else
    sprintf(tmp,"su -c \"busybox route  add default gw %s dev %s\" ",gateway_str,interface_name);

#endif
    retval = system(tmp);
    if(retval == -1)
    {
        plog("set_gateway error!") ;
        return ERROR;
    }

    return SUCCESS;
#endif
    return SUCCESS;
}

/**
 * @chinese
 * DHCP
 *
 * @return 无
 * @endchinese
 *
 * @english
 * DHCP
 *
 * @return none
 * @endenglish
 *
 */
void set_dhcp()
{
#ifndef _ARM_A23
    char *command="udhcpc -b";
#else
    char *command="su -c \"busybox udhcpc -b\"";
#endif
    system(command);
}





/*
  get ip by dns
  in_dns: Domain Name
  out_ip: ip address
  return 0 ,success;return -1,fail;
*/

int  dns2ip(char *dns,char *ip)
{
     struct hostent *host;
     struct sockaddr_in host_addr;
     if(dns==NULL)
         return -1;
     if ((host=gethostbyname(dns))==NULL)
     {
         return -1;
     }
     host_addr.sin_family=AF_INET;
     host_addr.sin_addr = *((struct in_addr *)host-> h_addr);
     bzero(&(host_addr.sin_zero),8);
   // printf("%s,%s\n",host->h_name,inet_ntoa(host_addr.sin_addr));
    strcpy(ip,inet_ntoa(host_addr.sin_addr));
    return 0;

}

/*
 udhcpc
 */

_DhcpInfo DhcpInfo;

//检测网络节点设备状态
int get_wifi_devices_status(char *interface_name)
{
    FILE *file = NULL;
    int up_flag = 0;
    char buff[256],command[32], *ptr = NULL;

    if(!interface_name)
    {
        return ERROR;
    }
#ifdef _ARM_A23
    memset(command,0,sizeof(command));
    sprintf(command,"su -c \"busybox ifconfig %s\"",interface_name);
    file = popen(command, "r" );
#else
    file = popen( "ifconfig", "r" );
#endif
    if( !file )
    {
        return ERROR;
    }
    up_flag = 0;
    while( fgets( buff, sizeof( buff ), file ) )
    {
        if(strncmp(interface_name,buff,strlen(interface_name)) == 0)
        {
            up_flag=1;
        }
        if(up_flag == 1)
        {
            ptr = strstr( buff, "UP" );
            if( ptr )
            {
                pclose( file );
                return SUCCESS;
            }
        }
    }
    pclose( file );
    return ERROR;
}


int udhcp_flag=0;

//初始化功能
int init_udhcp_devices(char *interface_name,int bg_flag)
{
    int retval = 0,pid=0,num=10;
    pthread_t udhcp_thread_id=0;
    static char buf[256];
#ifdef _A20_UBUNTU
    char *dhcp="dhclient";
#elif _ARM_A23
    char *dhcp="dhcpcd";
#else
    char *dhcp="udhcpc";
#endif
    if(interface_name == NULL||strlen(interface_name)==0)
    {
        return ERROR;
    }
    //如果DHCP 已经启动，且节点名称未改变，不再启动新的服务进程。
    if(DhcpInfo.isEnable==1&&strcmp(interface_name,DhcpInfo.interfaceName)==0)
        return SUCCESS;

    DhcpInfo.isEnable = 1;
    DhcpInfo.isBack = bg_flag;
    strcpy(DhcpInfo.interfaceName,interface_name);
    //前台运行
    if(DhcpInfo.isBack == 0)
    {
        while(num-->0){
            pid=pid_get(dhcp);
            if(pid>0)
            {
#ifdef _ARM_A23
                sprintf(buf,"su -c \"kill %d\"",pid);
                pox_system(buf);
#else
                retval = killp(pid);
#endif
               usleep(1000000);
            }else break;
        }
        set_ip(interface_name,"0.0.0.0");
#ifdef _A20_UBUNTU
        sprintf(buf,"dhclient %s ",interface_name);
#elif _ARM_A23
        set_ip(interface_name,"0.0.0.0");
        sprintf(buf,"su -c \"dhcpcd -b %s \" &",interface_name);
#else
        sprintf(buf,"udhcpc -b -A 10 -i %s ",interface_name);
#endif
        retval = pox_system(buf);
        usleep(500000);
        if(retval < 0)
        {
            return ERROR;
        }
        return SUCCESS;
    }
    //后台执行
    if(DhcpInfo.runStatus == 1)
    {
        return SUCCESS;
    }
    memset(buf,0,sizeof(buf));
    strcpy(buf,interface_name);

    retval = pthread_create(&udhcp_thread_id, NULL,(void*)open_udhcp_devices, (void*)buf);
    if (retval != 0)
    {
        plog("init_udhcp_devices_create");
        return FALSE;
    }
    pthread_detach(udhcp_thread_id);

    return SUCCESS;
}

//初始化功能
int open_udhcp_devices(char *interface_name)
{
    char buf[128];
    int retval=0,pid=0;

#ifdef _A20_UBUNTU
char *dhcp="dhclient";
#elif _ARM_A23
char *dhcp="dhcpcd";
#else
char *dhcp="udhcpc";
#endif
    memset(buf,0,sizeof(buf));
#ifdef _A20_UBUNTU
sprintf(buf,"dhclient %s ",interface_name);
#elif _ARM_A23
sprintf(buf,"su -c \"dhcpcd -b %s \"",interface_name);
#else
sprintf(buf,"udhcpc -b -A 10 -i %s ",interface_name);
#endif
    DhcpInfo.runStatus = 1;
    while(DhcpInfo.runStatus) //
    {
        sleep(3);
        retval = get_wifi_devices_status((char *)interface_name);
        if (retval == ERROR)
        {
            pid=pid_get(dhcp);
            if(pid>0)
            {
#ifdef _ARM_A23
     sprintf(buf,"su -c \"kill %d\"",pid);
     pox_system(buf);
#else
     retval = killp(pid);
#endif
            }
            continue;
        }
        pid=pid_get(dhcp);
        if(pid>0)
        {
            continue;
        }
        retval = pox_system(buf);
    }
 pid=pid_get(dhcp);
 if(pid>0)
 {

#ifdef _ARM_A23
     sprintf(buf,"su -c \"kill %d\"",pid);
     pox_system(buf);
#else
     retval = killp(pid);
#endif
 }
    DhcpInfo.runStatus = 2;
    pthread_exit(NULL);
}

//关闭udhcp设备
int close_udhcp_devices()
{
    int pid = 0,retval = 0,num=5;
    char buf[64];
    unsigned t=time(NULL);
    time_t last_time = 0;
#ifdef _A20_UBUNTU
char *dhcp="dhclient";
#elif _ARM_A23
char *dhcp="dhcpcd";
#else
char *dhcp="udhcpc";
#endif
    if(DhcpInfo.isEnable ==0)
        return 0;
    DhcpInfo.runStatus=0;
    //前台运行
    if(DhcpInfo.isBack==0)
    {
        while(num-->0){
            pid=pid_get(dhcp);
            if(pid>0)
            {
                //retval = killp(pid);
#ifdef _ARM_A23
                sprintf(buf,"su -c \"kill %d\"",pid);
                pox_system(buf);
#else
                retval = killp(pid);
#endif
                usleep(1000000);
            }else break;
        }
        DhcpInfo.isEnable=0;
        return ERROR;
    }
    time(&last_time);
    while(DhcpInfo.runStatus !=2)
    {
        if(abs((int)difftime(time(NULL),last_time)) > 15)
        {
            DhcpInfo.isEnable=0;
            return ERROR;
        }
        sleep(1);
    }
    DhcpInfo.isEnable=0;
    return SUCCESS;
}

//重启DHCP
int resetDhcpDevices()
{
    if(DhcpInfo.isEnable == 0)
    {
        return 0;
    }

    close_udhcp_devices();
    init_udhcp_devices(DhcpInfo.interfaceName,DhcpInfo.isBack);
    return 1;
}

