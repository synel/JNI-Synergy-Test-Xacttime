/**
 * @file   spi_s3c2416.h
 * @author 刘训
 * @date   Wed Jul 13 10:00:59 2011
 *
 * @brief
 *
 *
 */
#ifndef __SPI_S3C2416_H
#define __SPI_S3C2416_H

#ifdef __cplusplus
extern "C" {
#endif
int s3c2416_spi_open();
int s3c2416_spi_close(int fd);
int s3c2416_spi_recv_data(int fd, char *data, int len);
int s3c2416_spi_send_data(int fd, char *data, int len);
int s3c2416_spi_read_data(int fd, char *data, int len);
int s3c2416_spi_write_data(int fd, char *data, int len);
int s3c2416_spi_set_max_speed(int fd, unsigned long * max_speed_ptr);
int s3c2416_spi_get_max_speed(int fd, unsigned long * max_speed_ptr);
int s3c2416_spi_set_write_mode(int fd, unsigned char * mode_ptr);
int s3c2416_spi_set_read_mode(int fd, unsigned char * mode_ptr);
int s3c2416_spi_set_bits(int fd, unsigned char * bits_ptr);
int s3c2416_spi_get_bits(int fd, unsigned char * bits_ptr);

#ifdef __cplusplus
}
#endif
#endif
