/*
 * em310.h
 *
 *  Created on: 2013-7-10
 *      Author: aduo
 */

#ifndef EM310_H_
#define EM310_H_

void em310_check_sim();
void em310_get_apn();
void em310_reg_apn();
void em310_open_pdp_context();
int em310_open_tcp_connect(int tongdao);
int em310_gprs_send(unsigned char *data,unsigned int len,int tongdao);
int em310_gprs_recv(unsigned char *data,int len,int tongdao);
void em310_close_tcp_connect(int tongdao);

void em310_close_all_tcp_connect();

#endif /* EM310_H_ */
