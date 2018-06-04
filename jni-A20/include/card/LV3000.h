/*
 * LV3000.h
 *
 *  Created on: 2013-5-14
 *      Author: sherry
 */

#ifndef LV3000_H_
#define LV3000_H_

int read_LV3000_card(int uart_port,char *value);
int set_LV3000_card(int uart_port,char *value);
int set_LV3000_mode(int mode, int port_number);
#endif /* LV3000_H_ */
