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

#ifdef __cplusplus
extern "C" {
#endif

//#ifndef _ARM_A23
int spi_open(int port, int rate,  int workmode,  int address);
int spi_close(int fd);
int spi_recv_data(int fd, char *data, int len);
int spi_send_data(int fd, char *data, int len);
int spi_set_max_speed(int fd, unsigned long * max_speed_ptr);
int spi_get_max_speed(int fd, unsigned long * max_speed_ptr);
int spi_set_write_mode(int fd, unsigned char * mode_ptr);
int spi_set_read_mode(int fd, unsigned char * mode_ptr);
int spi_clear(int fd);
//#endif
#ifdef __cplusplus
}
#endif
#endif
