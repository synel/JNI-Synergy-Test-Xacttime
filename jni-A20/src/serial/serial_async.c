#include <unistd.h>
#include "serial_async.h"
#include "general/config.h"
#include "serial/device_protocol.h"
#include "serial/serial_devices.h"
#include "card/cardinfo.h"
#include "serial/serial.h"
//#include "android/wedsa23.h"

pthread_mutex_t serial_monitor_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_t thread_id_serial_service=0;
int serial_service_flag=0;
//判断数据是否可以覆盖的宏  1可以覆盖  0不可覆盖
#define COVER 1
//启动异步串行协议线程。
int start_serial_service()
{
    int retval = 0;
    //pthread_mutex_lock(&serial_monitor_lock);
    if(thread_id_serial_service==0)
    retval = pthread_create(&thread_id_serial_service, NULL,(void*)serial_monitor, NULL);
   // pthread_mutex_unlock(&serial_monitor_lock);
    return SUCCESS;
}


//初始化串口的操作信息。
int serial_init_async( int com, int baudrate, int workmode, int address )
{
    int ret=0;
   // LOGI("%s,%d\n",__func__,com);
    if(com>=MAXCOM)
        return ERROR;
    if(com>=10)
        ret=spi_open(com,baudrate,workmode,address);
    else
        ret=serial_init(com,baudrate,workmode,address);
    if(ret<=0)
        return ERROR;
    pthread_mutex_lock(&serial_monitor_lock);
    serial_data_array[com].com=com;
    serial_data_array[com].device_data=NULL;
    pthread_mutex_init(&(serial_data_array[com].serial_lock),NULL);
    pthread_mutex_init(&(serial_data_array[com].serial_data_lock),NULL);

    start_serial_service();//启动串口通讯服务。
    pthread_mutex_unlock(&serial_monitor_lock);
    return ret;
}


//检索
struct virtual_device_data * search_VirtualDeviceData(struct virtual_device_data * Pdevice,unsigned char instruction)
{
    struct virtual_device_data * tmp=Pdevice;
    while(tmp){
       // printf("%s 1 %02X %02X %d %d\n",__func__,tmp->instruction,instruction,tmp->end,tmp->head);
           if(tmp->instruction==instruction)
               return tmp;
           tmp=tmp->next;
       }
   // printf("%s 2\n",__func__);
       return NULL;
}


//分配并初始化虚拟设备,并把虚拟设备加入虚拟设备列表。
struct virtual_device_data * init_VirtualDeviceData(struct virtual_device_data ** Pdevice,TDATA *data)
{
    struct virtual_device_data *pdevice,*tmp;

    pdevice=malloc(sizeof(struct virtual_device_data));
    if(pdevice==NULL)
        return NULL;
    pdevice->end=-1;
    pdevice->head=-1;
    pdevice->instruction=data->instruction;
    pdevice->next=NULL;
    pdevice->num=0;
    pdevice->over=0;

    if(*Pdevice==NULL){
         *Pdevice=pdevice;
     }
     else {
         tmp=*Pdevice;
         *Pdevice=pdevice;
         pdevice->next=tmp;
     }
    return pdevice;

}


//模拟环形缓存区，采取覆盖策略
int put_data_to_devie(struct _serial_data *Pserialdata,TDATA *data)
{
    struct virtual_device_data * device_data=NULL;
    //检索具体指令（虚拟设备）队列是否存在
    int head=0;

    if(Pserialdata==NULL)
        return 0;
    device_data=search_VirtualDeviceData(Pserialdata->device_data,data->instruction);
    if(device_data==NULL){
            device_data=init_VirtualDeviceData(&(Pserialdata->device_data),data);
        }
    if(device_data==NULL)
        return FALSE;
//LOGI("%s 4 %02X %d,%d\n",__func__,data->instruction,device_data->head,device_data->end);
    //把数据加入队列内

    head= device_data->head;
    head+=1;
    head%=DEVICE_DATA_ARRAY;
    if(head==device_data->end)
    {
        device_data->over++;
        if(COVER==1)   //允许覆盖
        {
            device_data->end+=1;
            device_data->end%=DEVICE_DATA_ARRAY;
        }
        else
            return FALSE;       //不允许覆盖，直接丢弃。
    }
   // if(device_data->end==-1)
   //     device_data->end++;
    device_data->head+=1;
    device_data->head%=DEVICE_DATA_ARRAY;
    memcpy(&(device_data->device_array[device_data->head]),data,sizeof(TDATA));
    device_data->num++;
   // LOGI("%s %08X,%d,%d\n",__func__,device_data->instruction,device_data->num,device_data->over);
    return TRUE;

}


//根据指令类型，或则数据

int get_data_from_devie_type(TCOM com,TDATA *data,unsigned char instruction)
{
    struct virtual_device_data * device_data=NULL;
    if(com>=MAXCOM)
        return -1;
    device_data=search_VirtualDeviceData(serial_data_array[com].device_data,instruction);
    if(device_data==NULL)
        return -1;

    if(device_data->end==device_data->head)//缓存全内没有数据
        return -1;
    pthread_mutex_lock(&(serial_data_array[com].serial_data_lock));
    // LOGI("%s,%d,%d\n",__func__,device_data->head,device_data->end);
    device_data->end+=1;
    device_data->end%=DEVICE_DATA_ARRAY;
    memcpy(data,&(device_data->device_array[device_data->end]),sizeof(TDATA));
    pthread_mutex_unlock(&(serial_data_array[com].serial_data_lock));
    return com;
}

//按类型清空数据
int clear_data_from_device_type(TCOM com,unsigned char instruction)
{
    struct virtual_device_data * device_data=NULL;
    if(com>=MAXCOM)
        return -1;
    device_data=search_VirtualDeviceData(serial_data_array[com].device_data,instruction);
    if(device_data==NULL)
        return -1;
    pthread_mutex_lock(&(serial_data_array[com].serial_data_lock));
    device_data->end=-1;
    device_data->head=-1;
    pthread_mutex_unlock(&(serial_data_array[com].serial_data_lock));
    return com;
}



//清空串口内所有卡类型的数据
int clear_data_from_device(TCOM com)
{
    int i=0;
    for(i=1;i<255;i++){
        clear_data_from_device_type(com,i);
    }
    return TRUE;
}


//清空串口内所有卡类型的数据
int clear_card_from_device(TCOM com)
{
    int i=0;
    for(i=1;i<255;i++){
        switch(i){//非卡头数据
        case RK3288_GPIO_OUT:   // gpio 输出应答
        case RK3288_GPIO_IN:    // IO 输入状态变化
        case WIEGAND_CARD:      // wgout 应答
        case PSAMCMD:		// 透传指令应答 20170922
            continue;
        }
        clear_data_from_device_type(com,i);
    }
    return TRUE;
}

//获取所有数据
int get_all_data_from_devie(TCOM com,TDATA *data)
{
    int i=0;
    int ret=FALSE;
    for(i=1;i<255;i++){
        if((ret=get_data_from_devie_type(com,data,i))==TRUE)
            break;
    }
    return FALSE;
}


//获取所有卡号数据
int get_all_card_from_devie(TCOM com,TDATA *data)
{
    int i=0;
    int ret=FALSE;
    for(i=1;i<=0xFF;i++){
        switch(i){//非卡头数据
        case RK3288_GPIO_OUT:   // gpio 输出应答
        case RK3288_GPIO_IN:    // IO 输入状态变化
        case WIEGAND_CARD:      // wgout 应答
        case PSAMCMD:		// 透传指令应答 20170922
            continue;
        }
        if((ret=get_data_from_devie_type(com,data,i))==TRUE)
            break;
    }
    return FALSE;
}


//独立线程函数
void * serial_monitor()
{
    int num=0;
    TDATA			tdata;
    int retval;
    int flag=0;
    while(thread_id_serial_service)
    {
        flag=0;
        for(num=0;num<MAXCOM;num++)
            if(check_uart_cache(serial_data_array[num].com)==TRUE)
            {
            pthread_mutex_lock(&(serial_data_array[num].serial_lock));
            retval = serial_package_processor_async(serial_data_array[num].com , &tdata, RECV_PACKAGE );
            pthread_mutex_unlock(&(serial_data_array[num].serial_lock));
            if(retval==SUCCESS)
            {
                flag=1;
                pthread_mutex_lock(&(serial_data_array[num].serial_data_lock));
                if(serial_async_func[tdata.instruction]){
                    serial_async_func[tdata.instruction](num,&tdata);
                }
                else
                put_data_to_devie(&serial_data_array[num],&tdata);
                pthread_mutex_unlock(&(serial_data_array[num].serial_data_lock));
            }

        }
        if(flag==0)
        usleep(5000);
    }

}


//读取卡号

int device_recv_data_async( int port, char *value )
{
        static int		closebt = 0, deg=0;
        TDATA			tdata;
        int				len	   = 0;
        unsigned char	*p	   = NULL;
        int				i	   = 0, j = 1;
        int				retval = FALSE;
        char			data[16];
        memset( &tdata, 0, sizeof( tdata ) );

        //retval = serial_package_processor( port, &tdata, RECV_PACKAGE );
        retval=get_all_card_from_devie(port,&tdata);
        if( retval < 0 )
        {
            return retval;
        }

        memset( value, 0, sizeof( value ) );
        len				   = (int)tdata.nbytes;
        p				   = tdata.user_data;
        tongdao_itemnum	   = tdata.itemnum;

        //LOGI("%s,%d len=%d,%02X,%02X,%02X,%02X,%02X\n",__func__,port,len,tdata.instruction,p[0],p[1],p[2],p[3]);
        switch( tdata.instruction )
        {
                case READHIDCARD:
                        if( (int)tdata.user_data[0] == 0x00 && (int)tdata.user_data[1] == 0x00 )
                        {
                                strcpy( value, "CARDERROR" );
                                return -10;
                                break;
                        }else if( (int)tdata.user_data[0] == 0x00 && (int)tdata.user_data[1] == 0x01 )
                        {
                                strcpy( value, "EFFIACCYERROR" );
                                return -11;
                                break;
                        }else if( (int)tdata.user_data[0] == 0x00 )
                        {
                                return -5;
                        }

//        value[0]=len;
//        memcpy(value+1,tdata.user_data,len);

                        for( i = 0; i < len; i++ )
                        {
                                sprintf( value + i * 2, "%02X", tdata.user_data[i] );
                        }
                        printf( "HID ... = %d, %s\n", len, value );
                        break;
                case READPF: // iclass
                        if( (int)tdata.itemnum == 0x00 )
                        {
                                if( (int)tdata.user_data[0] == 0x00 )
                                {
                                        return -13;
                                        break;
                                }else if( (int)tdata.user_data[0] == 0x01 )
                                {
                                        return -14;
                                        break;
                                }else
                                {
                                        return -5;
                                        break;
                                }
                        }

                        for( i = 0; i < len; i++ )
                        {
                                sprintf( value + i * 2, "%02X", tdata.user_data[i] );
                        }
                        break;

                /*键盘*/
                case KEYBOARD:
                        if( (int)( *p ) == 16 )
                        {
                                sprintf( value, "%d", 0 );
                        } else
                        {
                                sprintf( value, "%d", p[0] );
                        }
                        switch( terminal_type )
                        {
                                case F_TYPE:
                                        if( atoi( value ) == 14 )
                                        {
                                                memset( value, 0, sizeof( value ) );
                                                sprintf( value, "%d", 28 );
                                        }
                                        break;
                        }
                        break;
                case READEMCARD /*读取id卡序列号*/:
                        if( len == 1 )
                        {
                                if( (int)tdata.user_data[0] == 0x00 )
                                {
                                        strcpy( value, "DECODERROR" );
                                        return -8;
                                        break;
                                }else if( (int)tdata.user_data[0] == 0x01 )
                                {
                                        strcpy( value, "CARDERROR" );
                                        return -9;
                                        break;
                                }else
                                {
                                        return -5;
                                }
                        }
                        hex2string( (char *)p, value, len );
                        break;
                /*读取MF1 S50卡序列号*/
                case MF1_S50_CARDNO:
                        if( len == 1 )
                        {
                                if( (int)tdata.user_data[0] == 0x00 )
                                {
                                        strcpy( value, "DECODERROR" );
                                        return -6;
                                        break;
                                }else if( (int)tdata.user_data[0] == 0x01 )
                                {
                                        strcpy( value, "CARDERROR" );
                                        return -7;
                                        break;
                                }else
                                {
                                        return -5;
                                }
                        }
                        hex2string( (char *)p, value, len );
                        strcpy( card_number, value );
                        //清空读取扇区内容的控制变量
                        cardHexToUInt( len, tdata.user_data );
                        //设置卡片和卡头类型
                        setCardType( tdata.instruction, WEDS_READER );

                        break;
                /*读取MF1 S70卡序列号*/
                case MF1_S70_CARDNO:
                        if( len == 1 )
                        {
                                if( (int)tdata.user_data[0] == 0x00 )
                                {
                                        strcpy( value, "DECODERROR" );
                                        break;
                                }else if( (int)tdata.user_data[0] == 0x01 )
                                {
                                        strcpy( value, "CARDERROR" );
                                        break;
                                }
                        }
                        hex2string( (char *)p, value, len );
                        strcpy( card_number, value );
                        //清空读取扇区内容的控制变量
                        cardHexToUInt( len, tdata.user_data );
                        //设置卡片和卡头类型
                        setCardType( tdata.instruction, WEDS_READER );

                        break;
                case CPU_CARDNO:
                        hex2string( (char *)p, value, len );
                        break;
                case TRANSIT:
                        hex2string( (char *)p, value, len );
                        break;
                /*读取磁条卡序列号*/
                case READMAGNETIC:
                        if( track_number == 0 ) // 任何轨都返 20170526
                        {
                                ;
                        }     else if( (int)tdata.address != track_number )
                        {
                                break;
                        }
//		printf("22\n");

                        //读卡错误
                        if( (int)tdata.nbytes == 1 )
                        {       //printf("33\n");
                                if( (int)tdata.user_data[0] == 0x00 )
                                {   //printf("44\n");
                                        strcpy( value, "DECODERROR" );
                                        return -4;
                                        break;
                                }else if( (int)tdata.user_data[0] == 0x01 )
                                {   //printf("55\n");
                                        strcpy( value, "CARDERROR" );
                                        return -1;
                                        break;
                                }else
                                {
                                        return -5;
                                }
                        }
                        i = 0;
                        while( isxdigit( tdata.user_data[i] ) )
                        {
                                value[i] = (int)tdata.user_data[i];
                                i++;
                        } //printf("66\n");
                        break;
                /*读取条码卡序列号*/
                case READBARCODE:
                        len = len - 1;
                        if( len >= 0 )
                        {
                                if( (int)tdata.user_data[0] == 0x00 && (int)tdata.user_data[1] == 0x00 )
                                {
                                        strcpy( value, "DECODERROR" );
                                        return -2;
                                        break;
                                }else if( (int)tdata.user_data[0] == 0x00 && (int)tdata.user_data[1] == 0x01 )
                                {
                                        strcpy( value, "CARDERROR" );
                                        return -3;
                                        break;
                                }

                                for( i = 0; i < len; i++ )
                                {
                                        value[i] = (int)tdata.user_data[i + 1];
                                }
                        }else
                        {
                                strcpy( value, "ERROR" );
                        }
                        break;
                /*身份证*/
                case ID_CARDNO:
                        if( len == 1 )
                        {
                                if( (int)tdata.user_data[0] == 0x00 )
                                {
                                        strcpy( value, "DECODERROR" );
                                        break;
                                }else if( (int)tdata.user_data[0] == 0x01 )
                                {
                                        strcpy( value, "CARDERROR" );
                                        break;
                                }
                        }
                        hex2string( (char *)p, value, len );
                        break;

                case WIEGANDINPUT:
                        retval = resolve_wiegand_input( (char *)p, value );
                        deg++;
                        //LOGI( "%s wg data %d ===%s\n",__func__, deg, value );
                        //printf( "wg data2 ===%s\n", value );
                        break;
                /*电池电量信息*/
                case BATTERY_INFO:


                        /*
                                150: High voltage alarm
                                110:Low voltage alarm
                                160:Chip failure
                                120:No battery
                                130:Battery charging
                                140:Battery ful
                                0~~100:battery level %
                         */
#ifndef _ARM_A23
                        value[0] = p[0];
#else       //android 没有无符合类型，只能支持-128-127;
                        switch( p[0] )
                        {
                                case 110: value[0] = 101; break;
                                case 120: value[0] = 102; break;
                                case 130: value[0] = 103; break;
                                case 140: value[0] = 104; break;
                                case 150: value[0] = 105; break;
                                case 160: value[0] = 106; break;
                                case 170: value[0] = 107; break;
                                case 180: value[0] = 108; break;
                                default: value[0]  = p[0]; break;
                        }
#endif
                        if( value[0] <= 160 && value[0] >= 10 )
                        {
                                //根据机型发送告知单片机是否发送电池电量情况
//            printf("terminal_type.... = %d\n", terminal_type);
                                switch( terminal_type )
                                {
                                        case K_TYPE:
//				printf("xxxxxxxxxxxxxxxxxxx\n");
//				if(closebt)
                                        {
                                                retval	   = send_close_battery_info( "BATTERY_INFO" );
                                                closebt	   = 1;
                                        }

                                        break;
                                }
                                if( retval < 0 )
                                {
                                        printf( "send_close_battery_info error!!\n" );
                                }
                        }
                        return tdata.instruction;
                        break;
						
				case LIGHT_SENSE:	// 20180115 光感
				case HUMAN_SENSE:	// 20180115 人体感应
					//value[0] = p[0];
					memset(value, 0, sizeof(value));
					sprintf(value, "%d", p[0]);
					break;

                case CONTROL_P:     // mcu 发出关机指令 0x0F	,应用层可取消
                case CONTROL_PL:    // 应用层不可取消	0x1F
                case REBOOT_CMD:    // mcu返回重启信息，将在5s后重启	0x0E,应用层可取消掉
                case LIGHT_CMD:     // 背光控制指令	0x0D
                        value[0] = p[0];
                        break;

                /*错误反馈信息*/
                case FEEDBACK:
                        value[0] = p[0];
                        break;
                case GET_VERSION:
                        value[0] = tdata.instruction;
                        memcpy( value + 1, tdata.user_data, len );
                        return SUCCESS;
                        break;
                case SPI_TEST:
                        value[0]   = p[0];
                        value[1]   = p[1];
                        break;
                case SEARCH_CARD:
                        //printf("card type %02x\n",p[0]);
                        //printf("card len %d\n",len);
                        if( p[0] == 0x04 )
                        {
                                tdata.instruction = 0x95;
                                hex2string( (char *)p + 1, value, 4 );
                        }else
                        {
                                tdata.instruction = p[0];
                                hex2string( (char *)p + 1, value, len - 1 );
                        }
                        break;
                case SEARCH_CARD_IC:
                        tdata.instruction = 0x94;
                        if( len > 1 )
                        {
                                hex2string( (char *)p + 1, value, len - 1 );
                        }

                case UPDATEUARTCARD: // 0x3A 需要升级
                        memcpy( &synel_tdata, &tdata, sizeof( tdata ) );
                        memcpy( value, tdata.user_data, tdata.nbytes );
                        break;

                default:
                        memcpy( value, tdata.user_data, len );
                        break;
        }
        /*添加上此处以后可能会造成按键丢数，不添加的时候会造成操作快，读数据迟钝的显现,可以开到应用层处理*/
        if( port < SPI_COM )
        {
            clear_card_from_device(port);
                //serial_clear( port );
        }
        return tdata.instruction;
}

int serial_package_processor_async( TCOM com, TDATA *tdata, ACTION action )
{
    int ret;
    pthread_mutex_lock(&(serial_data_array[com].serial_data_lock));
    ret=_serial_package_processor_async(  com, tdata,action );
    pthread_mutex_unlock(&(serial_data_array[com].serial_data_lock));
    return ret;
}

// psam卡透传指令: send and recv
int rk3288_psam_apdu_data_async( unsigned char* data, int bytes, unsigned char *obuf )
{
        int						ret = 0;
        TDATA					tdata;
        struct timeval			oldtimer, newtimer;
       // struct serial_devices	*p1 = NULL;

        if( RK3288_51port < 0 )
        {
                return -1;
        }
        ret = device_send_data( RK3288_51port, 0X01, PSAMCMD, 0X01, data, bytes );
        gettimeofday( &oldtimer, NULL );
        gettimeofday( &newtimer, NULL );
        while( 1 )
        {
                memset( data, 0, sizeof( data ) );
               // ret = rk3288_device_recv_data( RK3288_51port, &tdata );
                ret= get_data_from_devie_type(RK3288_51port,&tdata,PSAMCMD);
                if( ret > 0 )
                {
                        if( ret == PSAMCMD )
                        {
                                obuf[0] = tdata.nbytes; // 首字节要赋值为长度
                                memcpy( obuf + 1, tdata.user_data, tdata.nbytes );
                                return tdata.nbytes;
                        }
                }

                gettimeofday( &newtimer, NULL );
                if( ( newtimer.tv_sec - oldtimer.tv_sec ) * 1000
                    + ( newtimer.tv_usec - oldtimer.tv_usec ) / 1000 > 800 ) // 800 ms, 部分运算耗时较长
                {
//			printf("now rk3288_psam_apdu_data time out ~~~~~~~~~~~~\n");
                        return -1;
                }
        }

        return -1;
}







int set_async_func(unsigned char instruction,int (*serial_async_fun)(int com, TDATA *data))
{
    printf("%s %d\n",__func__,instruction);
    if(instruction>=255)return -1;
    serial_async_func[instruction]=serial_async_fun;
    return 0;
}
