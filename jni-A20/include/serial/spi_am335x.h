/*
 * spi_am335x.h
 *
 *  Created on: 2014-8-7
 *      Author: aduo
 */

#ifndef SPI_AM335X_H_
#define SPI_AM335X_H_

#define SPI_CPHA		0x01
#define SPI_CPOL		0x02
#define SPI_MODE_0		(0|0)
#define SPI_MODE_1		(0|SPI_CPHA)
#define SPI_MODE_2		(SPI_CPOL|0)
#define SPI_MODE_3		(SPI_CPOL|SPI_CPHA)

#ifdef __cplusplus
extern "C" {
#endif

int am335x_spi_open();
int am335x_spi_close(int fd);
int am335x_spi_recv_data(int fd, char *data, int len);
int am335x_spi_send_data(int fd, char *data, int len);
int am335x_spi_set_max_speed(int fd, unsigned long * max_speed_ptr);
int am335x_spi_get_max_speed(int fd, unsigned long * max_speed_ptr);
int am335x_spi_set_write_mode(int fd, unsigned char * mode_ptr);
int am335x_spi_set_read_mode(int fd, unsigned char * mode_ptr);

#ifdef __cplusplus
}
#endif


#endif /* SPI_AM335X_H_ */
