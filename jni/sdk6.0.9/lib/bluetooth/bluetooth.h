/*
 * bluetooth.h
 *
 *  Created on: 2014-6-14
 *      Author: aduo
 */

#ifndef BLUETOOTH_H_
#define BLUETOOTH_H_

int init_bluetooth(char *model,int port,int rate);
int open_bluetooth();
int scan_bluetooth(char *addr_list);
int connect_bluetooth();
int disconnect_bluetooth();
int close_bluetooth();

#endif /* BLUETOOTH_H_ */
