/*
 * m35.h
 *
 *  Created on: 2013-7-10
 *      Author: aduo
 */

#ifndef M35_H_
#define M35_H_

void m35_check_sim();
void m35_get_apn();
void m35_reg_apn();
void m35_open_pdp_context();
int m35_open_tcp_connect(int tongdao);
int m35_gprs_send(unsigned char *data,unsigned int len,int tongdao);
int m35_gprs_recv(unsigned char *data,int len,int tongdao);
void m35_close_tcp_connect(int tongdao);

void m35_close_all_tcp_connect();

#endif /* M35_H_ */
