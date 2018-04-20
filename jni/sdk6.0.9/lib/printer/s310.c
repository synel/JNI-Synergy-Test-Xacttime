

#include "s310.h"

//adaptive baudrate
int atoi_speed(char *dev, int speed){
	//int speed_arr[] = { B38400, B19200, B9600, B4800, B2400 };
	//int name_arr[] = {38400,  19200,  9600,  4800,  2400 };
	//int i=0;
	//struct termios Opt;
	//fd_set read_fd;
	//unsigned char tmp[126]={0X10,0X04,0X01};
	//struct timeval over_timer;
/*
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


//set baudrate of printer
int set_printspeed(int speed)
{
	unsigned char opensetbuf[126]={0X1d, 0X28, 0X45, 0X03, 0X00, 0X01, 0X49, 0X4E};
	unsigned char closesetbuf[126]={0X1d, 0X28, 0X45, 0X04, 0X00, 0X02, 0X4f, 0X55, 0X54};
	char buf[126]={0X1D, 0X28, 0X45, 0X07, 0X00, 0X0B, 0X01};

	sprintf(&buf[7],"%d",speed);
	WriteComm(opensetbuf,8);	// turn to printer function setting mode   function 1
	WriteComm((unsigned char*)buf,30);
	WriteComm(closesetbuf,9);	//  end printer function setting mode   function 2
	return TRUE;
}

int s310_Readstatus() {
	unsigned char str;
	int len = 0;

	SendOrder(PRINT_STATUS);
//while(1)
	{
		len = ReadComm(&str, 1);
		if (len == 1) {
			printf("str=%02X %d\n", str, 8 & (int) str);
			if (8 & (int) str)
				return FALSE; //buzy
		}
	}
	return TRUE;
}



