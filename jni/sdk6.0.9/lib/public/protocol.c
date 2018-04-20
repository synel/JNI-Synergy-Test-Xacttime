#include "protocol.h"
#include "../serial/serial.h"

/*------------------------------------------------------------------------
 * File description：
 * File name - protocol.c
 *
 * Function list ：
 *        _set_address                - set communication address
 *        _get_address                - get communication address
 *        _set_work_mode              - set work mode
 *        _get_work_mode              - get work mode
 *        _get_package                - get data packet
 *        _get_data                   - get data
 *        _put_data                   - send data
 *        _make_package               - make packet
 *        _analyze_package            - analyze data packet
 *-----------------------------------------------------------------------*/

/* communication between server and terminal ，source files of terminal */


/* the max length terminal can send or receive */
enum {_MAXBUFLEN = MAXLENGTH};
/*--------------------------------------------------------------------------
 * communication address of itself，used to check whether a packet was send by itself
 *-------------------------------------------------------------------------*/
static char _address;

/*--------------------------------------------------------------------------
 * server address（not used）
 *-------------------------------------------------------------------------*/
/*static char serveraddress;*/

/*--------------------------------------------------------------------------
 * work modes are terminal mode and server mode .start signs are _STY and _STX
 * _begin_putch：start sign used when send
 * _begin_getch：start sign used when receive
 *-------------------------------------------------------------------------*/

unsigned char _begin_putch[MAXCOM], _begin_getch[MAXCOM]; /* default : terminal mode */

/*--------------------------------------------------------------------------*
@Function					: cal_bcc - XOR Check
@Prototype        : char cal_bcc(void *data, short int nbytes)
@Include     			: "protocol.h"
@Description      : data：pointer to data block
                    nbytes：number of bytes in data
                    Calculation formula：BCC = DATA(0)^DATA(1)^DATA(2)^...^DATA(nbytes - 1)；
@Return Value     : XOR Check value
										if  data=NULL or nbytes<=0 return 0 
*----------------------------------------------------------------------------*/
unsigned char cal_bcc(void *data, short int nbytes) {

    short int i = 0;
    unsigned char bcc = 0, *p = (unsigned char *)data;

    if(data == NULL || nbytes <= 0)
        return 0;

    while(i < nbytes)
        bcc = (unsigned char)(bcc ^ (p[i++] & 0xFF));

    return bcc;
} /* cal_bcc */

/*--------------------------------------------------------------------------*
@Function					: cal_sum  - SUM Check
@Prototype        : char cal_sum(void *data, short int nbytes)
@Include     			: "protocol.h"
@Description      : data：pointer to data block
                    nbytes：number of bytes in data
                    Calculation formula：
                    SUM = LOWBATE(DATA(0)+DATA(1)+DATA(2)+...+DATA(nbytes - 1))；
                    in which LOWBATE means get the lowest bytes of data.
@Return Value     : SUM Check value
                    if  data=NULL or nbytes<=0 return 0 
*----------------------------------------------------------------------------*/
unsigned char cal_sum(void *data, short int nbytes) {

    short int i = 0;
    unsigned char *p = (unsigned char *)data;
    unsigned long int sum = 0UL;

    if(data == NULL || nbytes <= 0)
        return 0;

    while(i < nbytes)
        sum += (p[i++] & 0xFF);
    
    return (unsigned char)(sum & (unsigned long int)0xFF);     /* Only get the minimum bytes */
} /* cal_sum */

/*--------------------------------------------------------------------------*
@Function					: put_byte - send one byte
@Prototype        : TError put_byte(TCom com, unsigned char byte)
@Include     			: "serio.h "
@Description      : byte：
                    send one byte to serial.
                    None byte was send within the specified time means overtime
@Return Value     : if one byte was send within the specified time,return SUCCESS
                    else return FAILURE
*----------------------------------------------------------------------------*/
TError put_byte(TCom com, unsigned char byte) {

    TError error;
    //printf("put_byte %02X\n",byte);

    error = write_port(com, &byte, 1) == 1 ? SUCCESS : FAILURE;

     //printf("put_byte %02X\n",byte);
    return error;
} /* put_byte */

/*--------------------------------------------------------------------------*
@Function					: get_byte - receive one byte
@Prototype        : TError get_byte(TCom com, unsigned char *byte)
@Include     			: "serio.h"
@Description      : byte：
                    recieve one byte from serial.
                    None byte was received within the specified time means overtime
@Return Value     : if one byte was received within the specified time,return SUCCESS
                    else return FAILURE
*----------------------------------------------------------------------------*/
TError get_byte(TCom com, unsigned char *byte) {

    TError error;
    int r;

   	error = ((r = read_port(com, byte, 1)) == 1) ? SUCCESS : FAILURE;


    return error;
} /* get_byte */
/*--------------------------------------------------------------------------*
@Function					: do_put_data  - send data
@Prototype        : TError do_put_data(TCom com, const void *data, short int nbytes)
@Include     			: "serio.h"
@Description      : data：data will be send
                    nbytes：
                    less than nbytes were send within the specified time means overtime
                    and will return 
@Return Value     : nbytes were send completely return Success；Failure return  FAILURE or TIME_OUT。
*----------------------------------------------------------------------------*/
TError do_put_data(TCom com, const unsigned char  *data, short int nbytes) {

    int sended = 0, n;

    assert(data != NULL && nbytes > 0);
    //printf("do_put_data\n");
	if (com < 10) {
		while (sended != nbytes) {

			n = write_port(com, data + sended, nbytes - sended);

			//printf("put_data  %d\n",n);
			if (n == -1)
				return TIME_OUT;
			sended += n;
		}
	} else{
	    while(nbytes>sended)
	    {
	        n=write_port(com,data + sended,1);
	        if(n == -1)
	        {
	            return TIME_OUT;
	        }
	        //usleep(15);
	        sended=sended+n;
	    }
    }

    return SUCCESS; 
} /* do_put_data */

/*--------------------------------------------------------------------------*
@Function					: do_get_data - receive data
@Prototype        : TError do_get_data(TCom com, void *data, short int nbytes)
@Include     			: "serio.h"
@Description      : data：where to store data received
                    nbytes：number of byte will received
                    less than nbytes were received within the specified time means overtime
                    and will return 
@Return Value     : nbytes were received completely return Success；Failure return  FAILURE or TIME_OUT。
*----------------------------------------------------------------------------*/
TError do_get_data(TCom com, void *data, short int nbytes) {

    int received = 0, n;

    assert(data != NULL && nbytes > 0);
    //printf("do_get_data %d\n",nbytes);
    if (com < 10){
		while (received != nbytes) {
			n = read_port(com, (void *) ((char *) data + received),
					nbytes - received);
			if (n == -1)
				return TIME_OUT;
			received += n;
		}
	} else {
		while (nbytes > received) {
			n = 0;
			/*注意：spi驱动只支持每次取1个字节*/
			n = read_port(com, data + received, 1);
			if (n <= 0) {
				return TIME_OUT;
			}
			received = received + n;
		}
	}
    return SUCCESS;
} /* do_get_data */

/*--------------------------------------------------------------------------*
@Function					: _get_package  -  receive data packet
@Prototype        : static TError _get_package(TPackage package, char begin, char end)
@Include     			: only functions in this file can use this function
@Description      : package：store data packet
                    begin：start byte of data
                    end：end byte of data
                    receive a packet from serial port；
                    less than specified bytes were received within the specified time means overtime and will return 
@Return Value     : return BEGINERROR when no begin received；
										return NOTMYADDRESS when the address received was not it's；
										return LENGTHERROR when no byte which specified the length received
										return ENDERROR when there was no end received
                    you can refer to：
                 overtime receive specified bytes  return val      remarks
                      Y       Y                    no defined   it is impossible
                      N       Y                    SUCCESS
                                                             		maybe useful data has been received
                      Y       N                    TIME_OUT 		how to deal with it up to hign level function
                
                      N       N                    FAILURE  
                      data is NULL                 FAILURE 
*----------------------------------------------------------------------------*/
static TError _get_package(TCom com,TPackage package, char begin, char end) {

    short int nbytes;
    unsigned char ch = 0, *p;
    int i=0;
    int num=0;
    if(package == NULL)
       return FAILURE;
    lable:
       p = (unsigned char *)package;

   while(1)
   {
    if(num++>10)
     return TIME_OUT;
    if(get_byte(com,&ch) != SUCCESS)   /* receive begin */
        return TIME_OUT;
//printf("\n\n");
  
    if(ch != begin)
        continue;
    //printf("begin=%02X\n",ch);
    if(get_byte(com,&ch) != SUCCESS)   /* according to protocol,this byte is address */
        return TIME_OUT;
//printf("addr=%02X\n",ch);
    /* when work in terminal mode,check whether it is the local addr；while work in server mode,do not check */
    if(_get_work_mode(com) == _TERMINAL && ch != _get_address())
	return TIME_OUT;
     else break;
    }
    *p++ = ch;                     /* put addr byte to the first byte of package */
    if(get_byte(com,&ch) != SUCCESS)   /* according to protocol,this byte is length of data */
       return TIME_OUT;
//printf("bytes=%02X\n",ch);
    *p++ = ch;                     /* put data byte to the second byte of package */
    nbytes = (short int)ch + 2;    /* add 2 in order to receive BCC and SUM bits */

    if(do_get_data(com,p, nbytes) == TIME_OUT) /* receive bits */
        return TIME_OUT;
//for(i=0;i<nbytes;i++)
//	printf("%02X ",*p++);
//printf("\n");
    if(get_byte(com,&ch) != SUCCESS)  /* according to protocol,this byte should be end */
        return TIME_OUT;
//printf("end=%02X\n",ch);
    if(ch != end)
     goto lable;
//    printf("end=%02X\n",ch);
//printf("1111\n");
    return SUCCESS;
} /* _get_package */


/*--------------------------------------------------------------------------*
@Function					: _make_package - make packete
@Prototype        : static TError _make_package(TPackgae package, 
                  :  _TData *data, short int *total)
@Include     			: only functions in this file can use this function
@Description      : data：nbytes=0 means no data，itemnu will be set to 0 automaticly,
                    and ignore use_data
                    total：return number of total effectual bytes
@Return Value     : always success
                    Notice：it do not check any parameters
*----------------------------------------------------------------------------*/
static TError _make_package(TPackage package, _TData *data, short int *total) {

    unsigned char *p = (unsigned char *)package;

    p[_ADDRESS] = data->address;
    p[_INSTRUCTION] = data->instruction; // the third byte of data is instruction
    if(data->nbytes == 0) {         /* nbytes = 0，means no data */
        p[_ITEMNUM] = data->itemnum = 0;   
        memset(p + _ITEMNUM + 1, 0, _MAXBUFLEN - 4);
        p[_LENGTH] = 2;   
    }
    else {
         p[_ITEMNUM] = data->itemnum;
         memcpy(p + _ITEMNUM + 1, data->user_data, data->nbytes);
         p[_LENGTH] = data->nbytes + 2;
    }

    p[p[_LENGTH] + 2] = cal_bcc(p, p[_LENGTH] + 2);
    p[p[_LENGTH] + 3] = cal_sum(p, p[_LENGTH] + 2);
    /* 2：there is two bytes in header of packet：address and length；
     * 2：total bytes of instruction and data
     * 2：BCC、SUM are total two bytes */
    *total = 2+ 2 + data->nbytes + 2;
    return SUCCESS;
} /* _make_package */

/*--------------------------------------------------------------------------*
@Function					: _analyze_package   -  analyze data packet
@Prototype        : static TError _analyze_package(TPackage package, _TData *data)
@Include     			: only functions in this file can use this function
@Description      : data：
                    first deal with the tail of the packete:
                        1。bcc check，compare with bcc in package
                        2。sum check，compare with sum in package
                    if ok,assign corresponding value to parameters
                    Notice：it do not check any parameters
@Return Value     : return value are followings
                    bcc check ok  sum check ok   return val
                          Y             Y       SUCCESS
                          Y             N       SUMERROR
                          N            	Y       BCCERROR
                          N             N       BCCERROR | SUMERROR
*----------------------------------------------------------------------------*/
static int  _analyze_package(TPackage package, _TData *data) {

    //TError error = SUCCESS;
      int error=SUCCESS;
    unsigned char *p = (unsigned char *)package;
	//printf("bcc %02X,%02X\n",p[p[_LENGTH] + 2],cal_bcc(p, p[_LENGTH] + 2));
    if(p[p[_LENGTH] + 2] != cal_bcc(p, p[_LENGTH] + 2))
        error |= BCCERROR;
        //printf("sum %02X,%02X\n",p[p[_LENGTH] + 3],cal_sum(p, p[_LENGTH] + 2));
    if(p[p[_LENGTH] + 3] != cal_sum(p, p[_LENGTH] + 2))
        error |= SUMERROR;
    if(error != SUCCESS)
        return error;
    /* assign correspongding value to fields of  data */
    data->instruction = p[_INSTRUCTION];
    //printf("_INSTRUCTION %2x\r\n",p[_INSTRUCTION]);
    data->address = p[_ADDRESS];
    data->nbytes = p[_LENGTH] - 2;
    data->itemnum = p[_ITEMNUM];
    memcpy(data->user_data, p + 4, p[_LENGTH] - 2);
    //printf("aaaaa\n");
    return SUCCESS;
} /* _analyze_package */


/*--------------------------------------------------------------------------*
@Function		: start_com_io - open serial
@Prototype         : int start_com_io(int com, TBaudRate baud_rate,
                                   TWrodLength word_length, TStopBits stop_bits,
                                   TParity parity)
@Include     	: "serio.h"
@Description       : com：serial port .Olny COM1 and COM2 can use at the present time
                    baud_rate_index：baudrate index defined in chip.h
                    trigger：time needed to receive or send a byte
                    databits,stopbits and parity are set by function automaticly
                    It's work is to open serial port,set timer and communication address
@Return Value     : Success 1 ; failure 0
*----------------------------------------------------------------------------*/
int start_com_io(TCom com, long baud_rate) {
	printf("open com port %d\r\n",com);
	if (open_port(com, (int)baud_rate, 8, "1", 'N') == -1)
			return 0;
    return 1;
} /* start_com_io */

/*--------------------------------------------------------------------------*
@Function					: end_com_io   - close serial port
@Prototype        : void end_com_io(TCom com)
@Include     			: "serio.h"
@Description      : end communication,close serial port and destroy timer
@Return Value     : None
*----------------------------------------------------------------------------*/
void end_com_io(TCom com) {
    close_port(com);
} /* end_com_io */

void ComPort_Clear(int ComPort){
   clear_port(ComPort);
}

/*--------------------------------------------------------------------------*
@Function					: _set_address  - set communication address
@Prototype        : int _set_address(char address)
@Include     			: "terpro.h"
@Description      : address：The communication address used to mark a receiver.
@Return Value     : Success return non-zero;else 0
*----------------------------------------------------------------------------*/
int _set_address(unsigned char address) {

    return _address = (address == _SERVERADDR || address == _BROADCASTADDR ? 0 : address);
} /* _set_address */

/*--------------------------------------------------------------------------*
@Function					: _get_address  - get address
@Prototype        : int _get_address(void)
@Include     			: "terpro.h"
@Description      : get address
@Return Value     : return address
*----------------------------------------------------------------------------*/
char _get_address(void) {

    return _address;
} /* _get_address */

/*--------------------------------------------------------------------------*
@Function					: _set_work_mode  - set work mode
@Prototype        : void _set_work_mode(TWorkMode work_mode)
@Include     			: "terpro.h"
@Description      : work_mode：it's value is _TERMINAL or _SERVER
@Return Value     : None
*----------------------------------------------------------------------------*/
void _set_work_mode(TCom com,TWorkMode work_mode) {

    switch(work_mode) {
        case _TERMINAL:
            _begin_putch[com] = _STY;
            _begin_getch[com] = _STX;
            break;
        case _SERVER:
            _begin_putch[com] = _STX;
            _begin_getch[com] = _STY;
            break;
        default:
            _begin_putch[com] = _STY; /* default : terminal mode */
            _begin_getch[com] = _STX;
            break;
    }
} /* _set_work_mode */

/*--------------------------------------------------------------------------*
@Function					: _get_work_mode  - get work mode
@Prototype        : TWorkMode _get_work_mode(void)
@Include     			: "terpro.h"
@Description      : get work mode
@Return Value     : current work mode
*----------------------------------------------------------------------------*/
TWorkMode _get_work_mode(TCom com) {

    return (_begin_putch[com] == _STX && _begin_getch[com] == _STY) ? _SERVER : _TERMINAL;
} /* _get_work_mode */


/*--------------------------------------------------------------------------*
@Function					: _put_data   -  send data
@Prototype        : TError _put_data(_TData *data)
@Include     			: "terpro.h"
                    data：nbytes=0 means no data，itemnu will be set to 0 automaticly,
                    and ignore use_data
@Return Value    	: Failure return TIME_OUT;else return SUCCESS。
										if data=NULL，return  FAILURE
*----------------------------------------------------------------------------*/
TError _put_data(TCom com,_TData *data) {

    TPackage package;
    short int total;
    unsigned char *ptr;
    //int i;

    ptr=(unsigned char *)data;
    //printf("%02X,%02X,%02X,%02X\n",*ptr++,*ptr++,*ptr++,*ptr++);
    if(data ==NULL || data->nbytes > 253)
        return FAILURE;
    _make_package(package, data, &total);
//for(i=0;i<total;i++)
//printf("%02X ",package[i]);
//printf("\n");
    if(put_byte(com,_begin_putch[com]) == TIME_OUT)  /* communication start */
        return TIME_OUT;
    if(do_put_data(com,(unsigned char *)package, total) == TIME_OUT)
        return TIME_OUT;
    if(put_byte(com,_ETX) != SUCCESS)     /* communication end */
        return TIME_OUT;
//printf("send ok\n");
    return SUCCESS;
} /* _put_data */

/*--------------------------------------------------------------------------*
@Function		: _get_data  -  receive data
@Prototype        	: TError _get_data(_TData *data)
@Include     	: "terpro.h"
@Description      	: data：store data
                    after the data has been received,analyse it and assign the parameters
                    according to the result.
@Return Value     	: Failure return error code；
			If the error is discovered after analysis,return code that stands for anlysis error.others return SUCCESS.if data=NULL return  FAILURE
*----------------------------------------------------------------------------*/
int _get_data(TCom com,_TData *data) {

    TError error;
    TPackage package;

    if(data == NULL)
        return FAILURE;
//    printf(" _begin_getch[com] %d %02X\r\n", com,_begin_getch[com]);
    if((error = _get_package(com,package, _begin_getch[com], _ETX)) != SUCCESS)
        return error;

    return _analyze_package(package, data);
} /* _get_data */



/*--------------------------------------------------------------------------*
	synergy serial communication protocol
*----------------------------------------------------------------------------*/
static TError comm_get_package(TCom com,TComPackage package, char begin, char end) {

    short int nbytes;
    unsigned char ch = 0, *p;
    int i=0;

    if(package == NULL)
       return FAILURE;
    lable:
       p = (unsigned char *)package;

   while(1)
   {
    if(get_byte(com,&ch) != SUCCESS)   /* receive begin */
        return TIME_OUT;
//printf("\nbegin=%02X,%02X\n",ch,begin);
//printf("\nbegin=%02X\n",ch);
    if(ch != begin)
        continue;
    printf("\nbegin=%02X\n",ch);
    if(get_byte(com,&ch) != SUCCESS)   /* according to protocol,this byte is address */
        return TIME_OUT;
printf("addr=%02X\n",ch);
    /* when work in terminal mode,check whether it is the local addr；while work in server mode,do not check */
    /*if(_get_work_mode(com) == _TERMINAL && ch != _get_address())
	return TIME_OUT;
     else break;
    }*/
printf("my addr %02X\n",_get_address());

   if(ch != _get_address())
	return TIME_OUT;
    else break;
    }
    *p++ = ch;                     /* put addr byte to the first byte of package */
    if(get_byte(com,&ch) != SUCCESS)   /* according to protocol,this byte is length of data */
       return TIME_OUT;
//printf("bytes=%02X\n",ch);
    *p++ = ch;    //len high byte                /* put data byte to the second byte of package */
    nbytes = (short int)ch * 256;    /* add 2 in order to receive BCC and SUM bits */
    if(get_byte(com,&ch) != SUCCESS)   /* according to protocol,this byte is length of data */
       return TIME_OUT;
//printf("bytes=%02X\n",ch);
    *p++ = ch;      //len low byte               /* put data byte to the second byte of package */

    nbytes += (short int)ch + 2;    /* add 2 in order to receive BCC and SUM bits */
printf("nbytes=%d\n",nbytes);
    if(do_get_data(com,p, nbytes) == TIME_OUT) /* receive bits */
        return TIME_OUT;
//for(i=0;i<nbytes;i++)
//	printf("%02X ",*p++);
//printf("\n");
    if(get_byte(com,&ch) != SUCCESS)  /* according to protocol,this byte should be end */
        return TIME_OUT;
printf("end=%02X\n",ch);
    if(ch != end)
     goto lable;
//printf("1111\n");
    return SUCCESS;
} /* _get_package */


/*--------------------------------------------------------------------------*
@Function					: _make_package - make packete
@Prototype        : static TError _make_package(TPackgae package,
                  :  _TData *data, short int *total)
@Include     			: only functions in this file can use this function
@Description      : data：nbytes=0 means no data，itemnu will be set to 0 automaticly,
                    and ignore use_data
                    total：return number of total effectual bytes
@Return Value     : always success
                    Notice：it do not check any parameters
*----------------------------------------------------------------------------*/
static TError comm_make_package(TComPackage package, struct package *data, short int *total)
{
    unsigned char *p = (unsigned char *)package;

    p[0] = _get_address();
    p[1] = 0X04;
    p[2] = 0X00;
    memcpy(&p[3], data->command, sizeof(data->command));
    memcpy(&p[3+sizeof(data->command)], &data->datalen, sizeof(data->datalen));
    memcpy(&p[3+sizeof(data->command)+sizeof(data->datalen)], data->data, sizeof(data->data));

    p[1024 + 3] = cal_bcc(p, 1024 + 3);
    p[1024 + 4] = cal_sum(p, 1024 + 3);

    *total = 1024 +2 + 3;
    return SUCCESS;
} /* _make_package */

/*--------------------------------------------------------------------------*
@Function					: _analyze_package   -  analyze data packet
@Prototype        : static TError _analyze_package(TComPackage package, _TData *data)
@Include     			: only functions in this file can use this function
@Description      : data：
                    first deal with the tail of the packete:
                        1。bcc check，compare with bcc in package
                        2。sum check，compare with sum in package
                    if ok,assign corresponding value to parameters
                    Notice：it do not check any parameters
@Return Value     : return value are followings
                    bcc check ok  sum check ok   return val
                          Y             Y       SUCCESS
                          Y             N       SUMERROR
                          N            	Y       BCCERROR
                          N             N       BCCERROR | SUMERROR
*----------------------------------------------------------------------------*/
static int  comm_analyze_package(TComPackage package, struct package *data)
{

    int error=SUCCESS;
    int len=0;
    unsigned char *p = (unsigned char *)package;

    len=((int)p[1])*256+(int)p[2];

    if(p[len + 3] != cal_bcc(p, len+3))	error |= BCCERROR;
    if(p[len + 4] != cal_sum(p, len+3))	error |= SUMERROR;
    if(error != SUCCESS)	return error;

    memcpy((char*)data,&p[3],len);

    return SUCCESS;
} /* _analyze_package */

/*--------------------------------------------------------------------------*
@Function					: _put_data   -  send data
@Prototype        : TError _put_data(_TData *data)
@Include     			: "terpro.h"
                    data：nbytes=0 means no data，itemnu will be set to 0 automaticly,
                    and ignore use_data
@Return Value    	: Failure return TIME_OUT;else return SUCCESS。
										if data=NULL，return  FAILURE
*----------------------------------------------------------------------------*/
int comm_put_data(TCom com,struct package *data) {

    TComPackage package;
    short int total=0;
    //int i;

    if(data ==NULL)	return FAILURE;

    comm_make_package(package, data, &total);
//printf("send begin\n");
//for(i=0;i<total;i++)
//printf("%02X ",package[i]);
//printf("\n");
    if(put_byte(com,_begin_putch[com]) == TIME_OUT)  /* communication start */
        return TIME_OUT;
    if(do_put_data(com,(unsigned char *)package, total) == TIME_OUT)
        return TIME_OUT;
    if(put_byte(com,_ETX) != SUCCESS)     /* communication end */
        return TIME_OUT;
printf("send ok\n");
    return SUCCESS;
} /* _put_data */

/*--------------------------------------------------------------------------*
@Function		: _get_data  -  receive data
@Prototype        	: TError _get_data(_TData *data)
@Include     	: "terpro.h"
@Description      	: data：store data
                    after the data has been received,analyse it and assign the parameters
                    according to the result.
@Return Value     	: Failure return error code；
			If the error is discovered after analysis,return code that stands for anlysis error.others return SUCCESS.if data=NULL return  FAILURE
*----------------------------------------------------------------------------*/
int comm_get_data(TCom com,struct package *data) {

    TError error;
    TComPackage package;

//    if(data == NULL)
//        return FAILURE;

    if((error = comm_get_package(com,package, _begin_getch[com], _ETX)) != SUCCESS)
        return error;

    return comm_analyze_package(package, data);
} /* _get_data */
