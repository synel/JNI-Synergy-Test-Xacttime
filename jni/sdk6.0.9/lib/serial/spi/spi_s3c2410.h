/**
 * @file   spi_s3c2410.h
 * @author 刘训
 * @date   Wed Jul 13 10:00:59 2011
 *
 * @brief
 *
 *
 */
#ifndef __SPI_S3C2410_H
#define __SPI_S3C2410_H
#define SPI_CPHA		0x01
#define SPI_CPOL		0x02
#define SPI_MODE_0		(0|0)
#define SPI_MODE_1		(0|SPI_CPHA)
#define SPI_MODE_2		(SPI_CPOL|0)
#define SPI_MODE_3		(SPI_CPOL|SPI_CPHA)

#ifdef __cplusplus
extern "C" {
#endif

int s3c2410_spi_open();
int s3c2410_spi_close(int fd);
int s3c2410_spi_recv_data(int fd, char *data, int len);
int s3c2410_spi_send_data(int fd, char *data, int len);
int s3c2410_spi_set_max_speed(int fd, unsigned long * max_speed_ptr);
int s3c2410_spi_get_max_speed(int fd, unsigned long * max_speed_ptr);
int s3c2410_spi_set_write_mode(int fd, unsigned char * mode_ptr);
int s3c2410_spi_set_read_mode(int fd, unsigned char * mode_ptr);

#ifdef __cplusplus
}
#endif
#endif
