/*
 * LV3095.h
 *
 *  Created on: 2013-6-3
 *      Author: sherry
 */

#ifndef LV3095_H_
#define LV3095_H_

unsigned int crc_cal_by_bit(unsigned char* ptr, unsigned int len);
int open_Lv3095_light();
int close_Lv3095_light();
int read_LV3095_card(int uart_port,char *value);
int set_LV3095_card(int uart_port,char *value ,int send_len);
int set_LV3095_mode(int mode, int port_number);
#endif /* LV3095_H_ */
