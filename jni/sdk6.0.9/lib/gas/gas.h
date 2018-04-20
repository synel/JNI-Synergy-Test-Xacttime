/*
 * gas.h
 *
 *  Created on: 2014-7-1
 *      Author: aduo
 */

#ifndef GAS_H_
#define GAS_H_

struct _frame{
	unsigned char startcode;
	unsigned char len;
	unsigned char command;
	unsigned char data[252];
	unsigned char addr;
	unsigned char checksum;
};


/*
 *
 * +--------------------------------------------------+
 * | startcode | len | command | data | addr | checksum |
 * |                 |<-----len (Max 255 Bytes)------>|
 * +--------------------------------------------------+
 *
 */

int start_gas_test(int uart_port);
int read_gas_test(int uart_port,char *value);
int stop_gas_test(int uart_port);

#endif /* GAS_H_ */
