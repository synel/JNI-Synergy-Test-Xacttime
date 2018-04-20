
#include "printer.h"
#include "s310.h"

struct 
{
	int flag;             //0-transmit,1-print
	int total;               //the total number of packet which should be print
	int curcount;                 //the cur number of packet have print
	FILE *fp;               //file pointer
}printmemory;

struct printdevinfo printdevinfo;

// 可打印字符串转换为字节数据
// 如："C8329BFD0E01" --> {0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01}
// 输入: pSrc - 源字符串指针
//nSrcLength - 源字符串长度
// 输出: pDst - 目标数据指针
// 返回: 目标数据长度
static int gsmString2Bytes(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
	int i=0;
	for ( i = 0; i < nSrcLength; i += 2)
	{
		// 输出高4位
		if ((*pSrc >= '0') && (*pSrc <= '9'))
		{
			*pDst = (*pSrc - '0') << 4;
		}
		else
		{
			*pDst = (*pSrc - 'A' + 10) << 4;
		}
		pSrc++;
		// 输出低4位
		if ((*pSrc>='0') && (*pSrc<='9'))
		{
			*pDst |= *pSrc - '0';
		}
		else
		{
			*pDst |= *pSrc - 'A' + 10;
		}
		pSrc++;
		pDst++;
	}

	return (nSrcLength / 2);
}


// 字节数据转换为可打印字符串
// 如：{0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01} --> "C8329BFD0E01"
// 输入: pSrc - 源数据指针
//nSrcLength - 源数据长度
// 输出: pDst - 目标字符串指针
// 返回: 目标字符串长度
/*
static int gsmBytes2String(unsigned char* pSrc, char* pDst, int nSrcLength)
{
	const char tab[]="0123456789ABCDEF";	// 0x0-0xf的字符查找表
	int i=0;

	for (i = 0; i < nSrcLength; i++)
	{
		*pDst++ = tab[*pSrc >> 4];	// 输出高4位
		*pDst++ = tab[*pSrc & 0x0f];	// 输出低4位
		pSrc++;
	}
	*pDst = '\0';

	return (nSrcLength * 2);
}
*/

//adaptive baudrate
static int atoi_speed(char *dev, int speed){
	/*
	int speed_arr[] = { B38400, B19200, B9600, B4800, B2400 };
	int name_arr[] = {38400,  19200,  9600,  4800,  2400 };
	int i=0;
	struct termios Opt;
	fd_set read_fd;
	unsigned char tmp[126]={0X10,0X04,0X01};
	struct timeval over_timer;

	for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
	{
		printdevinfo.fd = open( dev, O_RDWR|O_NOCTTY|O_NDELAY);        //| O_NOCTTY | O_NDELAY
		if (printdevinfo.fd == -1)
		{
			perror("Can't Open Serial Port");
			return FALSE;
		}
		tcgetattr(printdevinfo.fd, &Opt);
		tcflush(printdevinfo.fd, TCIOFLUSH);
		cfsetispeed(&Opt, speed_arr[i]);
		cfsetospeed(&Opt, speed_arr[i]);
//		options.c_cflag |= (CLOCAL | CREAD);
		tcsetattr(printdevinfo.fd, TCSANOW, &Opt);
		tcflush(printdevinfo.fd,TCIOFLUSH);
		WriteComm(tmp,3);
		FD_ZERO(&read_fd);
		FD_SET(printdevinfo.fd,&read_fd);
		over_timer.tv_sec=0;
		over_timer.tv_usec=100000*((12-i)/2);//delay time,change with the baud rate
		if((select(printdevinfo.fd+1,&read_fd,NULL, NULL, &over_timer)) <= 0)
		{
			close(printdevinfo.fd);
			continue;
		}
		if(speed == name_arr[i])	return TRUE;

		set_printspeed(speed);
		break;
	}
	close(printdevinfo.fd);
	sleep(2);
*/
	set_speed(dev,speed);
	return TRUE;
}


/*--------------------------------------------------------------------------*
@Function		: set_speed - set baudrate...
@Include      	: "comm.h"
@Description		:
			fd：file operation handler of opened file
			speed ： com baudrate
@Return Value	: Success TREU, Failure FALSE.
@Create time		: 2009-06-15 08:23
*---------------------------------------------------------------------------*/
int set_speed(char *dev, int speed){

	int  status,i;
	struct termios  Opt;
	int speed_arr[] = { B38400, B19200, B9600, B4800, B2400 };
	int name_arr[] = {38400,  19200,  9600,  4800,  2400 };

	printdevinfo.fd = open( dev, O_RDWR|O_NOCTTY|O_NDELAY);        //| O_NOCTTY | O_NDELAY
	if (printdevinfo.fd == -1)
	{
		perror("Can't Open Serial Port");
		return FALSE;
	}

	tcgetattr(printdevinfo.fd, &Opt);
	for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
	{
		if  (speed == name_arr[i])
		{
			tcflush(printdevinfo.fd, TCIOFLUSH);
			cfsetispeed(&Opt, speed_arr[i]);
			cfsetospeed(&Opt, speed_arr[i]);
			status = tcsetattr(printdevinfo.fd, TCSANOW, &Opt);
			if  (status != 0)
			{
				perror("tcsetattr fd1");
				return	  FALSE;
			}
			tcflush(printdevinfo.fd,TCIOFLUSH);
		}
	}
	return TRUE;
}

/**
  set serial port baudrate,databits，stopbits,paritybit
  fd    :file handler
  databits :7 or 8
  stopbits :1 or 2
  parity  :N,E,O,,S
*/
static int set_Parity(int fd,int databits,int stopbits,int parity)
{
	struct termios options;

	if  ( tcgetattr( fd,&options)  !=  0)
	{
		perror("SetupSerial 1");
		return  FALSE;
	}
	options.c_cflag &= ~CSIZE;
	switch (databits) /*set databits*/
	{
		case 7:
			options.c_cflag |= CS7;
			break;
		case 8:
			options.c_cflag |= CS8;
			break;
		default:
			fprintf(stderr,"Unsupported data size\n");
			return FALSE;
	}
	switch (parity)
	{
		case 'n':
		case 'N':
			options.c_cflag &= ~PARENB;  /* Clear parity enable */
			options.c_iflag &= ~INPCK;    /* Enable parity checking */
			break;
		case 'o':
		case 'O':
			options.c_cflag |= (PARODD | PARENB); /* set odd check*/
			options.c_iflag |= INPCK;            /* Disnable parity checking */
			break;
		case 'e':
		case 'E':
			options.c_cflag |= PARENB;    /* Enable parity */
			options.c_cflag &= ~PARODD;  /* convert to even*/
			options.c_iflag |= INPCK;      /* Disnable parity checking */
			break;
		case 'S':
		case 's':  /*as no parity*/
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			break;
		default:
			fprintf(stderr,"Unsupported parity\n");
			return FALSE;
	}

	/* set stopbits*/
	switch (stopbits)
	{
		case 1:
			options.c_cflag &= ~CSTOPB;
			break;
		case 2:
			options.c_cflag |= CSTOPB;
			break;
		default:
			fprintf(stderr,"Unsupported stop bits\n");
			return FALSE;
	}
//	options.c_cflag |= CNEW_RTSCTS; /* you can call it CRTSCTS */
	/* Set input parity option */
	if (parity != 'n')
		options.c_iflag |= INPCK;
	tcflush(fd,TCIFLUSH);
	options.c_cc[VTIME] = 150; /* overtime 15 seconds*/
	options.c_cc[VMIN] = 0; /* Update the options and do it NOW */
	if (tcsetattr(fd,TCSANOW,&options) != 0)
	{
		perror("SetupSerial 3");
		return FALSE;
	}
	return TRUE;
}



/*--------------------------------------------------------------------------*
@Function					:	ReadComm - read data form com
@Include      		: "comm.h"
@Description			: None
@param				: data: data will be read
						  datalength: length of data to be read
@Return Value			: Success  the number of bytes read; Failure FALSE
@Create time			: 2009-06-15 08:23
*---------------------------------------------------------------------------*/
int ReadComm (unsigned char *buf,int len)
{
	struct timeval over_timer;
	fd_set read_fd;
	unsigned char *ptr;
	int num=0;

	if(printdevinfo.fd<=0)	return FALSE;
	FD_ZERO(&read_fd);
	FD_SET(printdevinfo.fd,&read_fd);
	over_timer.tv_sec=0;
	over_timer.tv_usec=15000;
	while(len!=num)
	{
		over_timer.tv_sec=0;
		over_timer.tv_usec=100000;
		if(select(printdevinfo.fd+1,&read_fd,NULL, NULL, &over_timer) <= 0)
		{
		    return num;
		}
		ptr=buf+num;
		num+=read(printdevinfo.fd,ptr,len-num);
	}
	return num;
}


/*--------------------------------------------------------------------------*
@Function					:	WriteComm - write data to com
@Include      		: "comm.h"
@Description			: None
@param				: data: data will be written
						  datalength: length of data to be written
@Return Value			: Success  the number of bytes been written
@Create time			: 2009-06-15 08:23
*---------------------------------------------------------------------------*/
int WriteComm(unsigned char * buf, int len) {

	struct timeval over_timer;
	fd_set write_fd;
	unsigned char *ptr;
	int num=0,count=0;

	if(printdevinfo.fd<=0)		return FALSE;
	FD_ZERO(&write_fd);
	FD_SET(printdevinfo.fd,&write_fd);
	over_timer.tv_sec=0;
	over_timer.tv_usec=1250000;
	while(len!=num)
	{
		over_timer.tv_sec=0;
		over_timer.tv_usec=150000;
		if(select(printdevinfo.fd+1,NULL,&write_fd, NULL, &over_timer) <= 0)
		{
		    return num;
		}
		ptr=buf+num;
		count=write(printdevinfo.fd,ptr,len-num);
		if(count<=0)		return num;
		num+=count;
	}
	return num;
}


/*
* 'open_port()' - Open serial port 1.
*
* Returns the file descriptor on success or -1 on error.
*/
int OpenDev(int comport,int speed,int databits,int stopbits,int parity )
{
	char Dev[126];

	memset(Dev,0,sizeof(Dev));
#if defined _x86
	sprintf(Dev,"/dev/ttyS%d",comport);
#elif defined _2410
	sprintf(Dev,"/dev/ttyS%d",comport);
#elif defined _2416
	sprintf(Dev,"/dev/ttySAC%d",comport);
#endif

	printf("printer port %s\r\n",Dev);
	printf("printer baud rate %d\r\n",speed);

	atoi_speed(Dev,speed);
	if (set_Parity(printdevinfo.fd, databits, stopbits,parity) == FALSE)  {
		printf("Set Parity Error\n");
		return FALSE;
	}
	printdevinfo.printflag = 1;
	return TRUE;
}

/*--------------------------------------------------------------------------*
@Function			:	UnPrintCom - release resource
@Include      		: "comm.h"
@Description			: None
@param				: void
@Return Value			: Success TREU, Failure FALSE.
@Create time			: 2009-06-15 08:23
*---------------------------------------------------------------------------*/
int UnPrintCom(void)
{
	if(printdevinfo.fd>0)
		 close(printdevinfo.fd);
	else
		return FALSE;
	printdevinfo.fd=-1;
	return TRUE;
}

//clear com0 buffer
void PrintClear(void)
{
	if(printdevinfo.fd >0)
		tcflush(printdevinfo.fd,TCIOFLUSH);
}

//api

int GetFileInfo(char *filename)
{
	struct stat filestat;

	if(printdevinfo.printflag==0)	return FALSE;
	if(ReadComStatus()==FALSE||printmemory.flag!= 0) 
	{
		return FALSE;	
	}
	memset(&printmemory,0,sizeof(printmemory));

	printmemory.fp=fopen(filename,"r");
	if(printmemory.fp==NULL)
	{
		   return FALSE;
	 }
	 printmemory.flag=1;	
	 fstat(fileno(printmemory.fp),&filestat); 
	 printmemory.total=filestat.st_size;

	return TRUE;
}

/*--------------------------------------------------------------------------*
@Function			:PrintFile - print files
@Include			: "print.h"
@Description			: filename: the file that will be print
@Return Value		: Success TREU, Failure FALSE.
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/
int PrintFile(void)
{
	int len=0;
	char  buf[50];  
	int overtime; 
	struct timeval tv1,tv2;

	if(printdevinfo.printflag==0)	return FALSE;
	 if(printdevinfo.fd<=0||printmemory.flag != 1)
	 {
		return FALSE;
	 }

	gettimeofday(&tv1,NULL);
	overtime=(tv1.tv_sec % 10000) *1000+tv1.tv_usec/1000;
	while(1){
		 memset(buf,0,64);
		 if(ReadComStatus()==FALSE)	return FALSE;
		 if(fseek(printmemory.fp,printmemory.curcount,SEEK_SET)<0)
		 {
			printmemory.flag=0;
			return FALSE;			
		 }
		 len=fread(buf, 1, sizeof(buf), printmemory.fp);
	 	 if(WriteComm( (unsigned char*)buf, strlen(buf)) == FALSE)	return FALSE;
		 printmemory.curcount+=len;
		if(printmemory.curcount==printmemory.total)
		{
			printmemory.flag=0;
			SendOrder(CUT_PAPER);
			return TRUE;
		}
		gettimeofday(&tv2,NULL);
		if (abs( (tv2.tv_sec % 10000) *1000+tv2.tv_usec/1000 - overtime) > 100)
			break;
	}
	return TRUE;	
}


/*--------------------------------------------------------------------------*
@Function			:PrintData - print data
@Include			: "print.h"
@Description			: buf: data that will be print
@Return Value		: Success TREU, Failure FALSE.
@Create time			: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/

int PrintData(char *buf)
{
	int len,i=0;
	unsigned char *ptr;
	
	printf("printdevinfo.printflag %d\n",printdevinfo.printflag);
	if(printdevinfo.printflag==0)	return FALSE;
	ptr=(unsigned char*)buf;
	//if(ReadComStatus()==FALSE||printmemory.flag != 0||ptr==NULL)
	//modify by aduo 2013.7.19
	//<!--
/*
	if(Readstatus()==FALSE||printmemory.flag != 0||ptr==NULL) //modify by aduo 2013.7.19
	{
		return FALSE;	
	}
*/
	//-->
	len=strlen(buf);
	do{
		printf("ptr=%s\n",ptr);
		//if(ReadComStatus()==FALSE)	return FALSE;
		//modify by aduo 2013.7.19
			//<!--
/*
		if(Readstatus()==FALSE)	return FALSE;//modify by aduo 2013.7.19
*/
	//-->
		//WriteComm( ptr, 50);
		WriteComm( ptr, len);//modify by aduo 2013.7.19
		//i+=50;
		i+=len;//modify by aduo 2013.7.19
		ptr+=i;
		usleep(100);
	}while(i<len);
	SendOrder(PRINT_ENTER);
	//SendOrder(CUT_PAPER);//modify by aduo 2013.7.19
	return TRUE;	
}



/*--------------------------------------------------------------------------*
@Function		: Readstatus - read status of printer (1DH 72H n)
@Include		: "print.h"
@Description		: tranfer printer's status realtime
			   n=1,transfer printer status:
			bit	status	hex		function
			0	0	00		fix to 0
			1	1	02		fix to 1
			2	0/1	00/04	have/no cashbox open
			3	0/1	00/08	serial port busy/not
			4	1	10		tix to 1
			5				undefine
			6	0/1	00/40	printer ok/error
			7	0	0		fix to 0
			
@Return Value	: Success TREU, Failure FALSE.
@Create time		: 2009-06-15 08:23		
*---------------------------------------------------------------------------*/

int Readstatus() {
	int err_code;
	//switch (printdevinfo.model) {
	//case s310:
		err_code = s310_Readstatus();
	//	break;
	//}

	return err_code;
}

//get printer's status  TRUE -not busy  FALSE-busy
int ReadComStatus()
{
	int bytes, status;

	ioctl(printdevinfo.fd, FIONREAD, &bytes);
	ioctl(printdevinfo.fd, TIOCMGET, &status);
	printf("printer status %d\n", status & TIOCM_CTS);
	if(status & TIOCM_CTS)
			return TRUE;
	return FALSE;
}

int SendOrder(char *command)
{
	int len=0,flag=0;
	unsigned char  buf[126];  
	
	if(command == NULL)		return FALSE;
	memset(buf,0,sizeof(buf));
    printf("print command %s\n",command);
	len=gsmString2Bytes(command,buf,strlen(command));
	printf("len of command %d\r\n",len);
	flag=WriteComm( buf, len);
	if(flag == 1)	
		return TRUE;
	else
		return FALSE;
}
