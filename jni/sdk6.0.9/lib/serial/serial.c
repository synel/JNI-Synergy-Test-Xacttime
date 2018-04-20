/*
 * serial.c
 *
 *  Created on: 2013-7-2
 *      Author: aduo
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>

#include "serial.h"
#include "uart/uartlib.h"
#include "spi/spilib.h"

int serial_fd[MAXCOM];

//return -1 or 1
int open_port (int port, int baudrate, int databit,
                   char *stopbit, char parity){
	int err_code;

    if (port < 10){
    	err_code = uart_open_and_setattr(port,baudrate,databit,stopbit,parity);
    }else{
    	err_code = spi_open(port,baudrate);
    }

    return err_code;

}

int read_port(int port, void *data, int datalength) {
	int len = 0;


	//printf("fd %d \r\n",serial_fd[port]);
	if (serial_fd[port] < 0) {
		return -1;
	}

	if (port < 10) {
		len = uart_recv_data(port, data, datalength);
	} else {
		len = spi_recv_data(port, data, datalength);
	}

	return len;
}

int write_port (int port, char * data, int datalength){
	int len = 0;

	if (serial_fd[port] < 0) {
		return -1;
	}

	if (port < 10){
		len = uart_send_data(port,data,datalength);
	}else{
		len = spi_send_data(port,data,datalength);
	}
	return len;
}

void close_port (int port){
	if (port < 10){
		uart_close(port);
	}else{
		spi_close(port);
	}
}

int clear_port(int port)
{
    if(serial_fd[port]<=0)
    {
        return 0;
    }

    if(port < 10)
    {
        return uart_clear(serial_fd[port]);
    }
    else
    {
        return spi_clear(serial_fd[port]);
    }

    return 1;
}
