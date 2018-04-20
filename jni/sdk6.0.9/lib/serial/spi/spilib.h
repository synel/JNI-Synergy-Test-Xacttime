/**
 * @file   spilib.h
 * @author 刘训
 * @date   Wed Jul 13 10:00:59 2011
 *
 * @brief
 *
 *
 */
#ifndef __SPILIB_H
#define __SPILIB_H

#include "../serial.h"

#ifdef __cplusplus
extern "C" {
#endif


int spi_open(int port, int rate);
int spi_close(int port);
int spi_recv_data(int port, char *data, int len);
int spi_send_data(int port, char *data, int len);
int spi_set_max_speed(int port, unsigned long * max_speed_ptr);
int spi_get_max_speed(int port, unsigned long * max_speed_ptr);
int spi_set_write_mode(int port, unsigned char * mode_ptr);
int spi_set_read_mode(int port, unsigned char * mode_ptr);
int spi_clear(int port);

#ifdef __cplusplus
}
#endif
#endif
