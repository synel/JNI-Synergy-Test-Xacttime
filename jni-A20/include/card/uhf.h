/*
 * uhf.h
 *
 *  Created on: 2014-5-5
 *      Author: aduo
 */

#ifndef UHF_H_
#define UHF_H_

int read_uhf_card(int uart_port,char *value);
int HR915M_Init(int port);
int HR915MGetCard_TimeMode(char *cards);

extern unsigned int mj_stat[5][3];

int mj_ext_event(int port, char* value);
int mj_ext_set(unsigned int index, unsigned int value);
int mj_ext_set_ex(unsigned int index, unsigned int value, unsigned int delay);


#endif /* UHF_H_ */
