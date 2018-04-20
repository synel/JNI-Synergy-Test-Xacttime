/*
 * serial.h
 *
 *  Created on: 2013-7-2
 *      Author: aduo
 */

#ifndef SERIAL_H_
#define SERIAL_H_

#include "../version.h"

#define MAXCOM 20
#define ERROR	-1  /**< Ê§°Ü\r\n ERROR*/
#define OK 1   /**< ³É¹¦\r\n SUCCESS*/

extern int serial_fd[MAXCOM];

int open_port (int port, int baudrate, int databit,
                   char *stopbit, char parity);
int read_port (int port, void *data, int datalength);
int write_port (int port, char * data, int datalength);
void close_port (int port);
int clear_port(int port);

#endif /* SERIAL_H_ */
