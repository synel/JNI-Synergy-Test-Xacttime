#ifndef __UARTLIB_H
#define __UARTLIB_H
/**
 * @chinese
 * @file   uartlib.h
 * @author 刘训
 * @date   Thu Oct 13 10:00:00 2011
 *
 * @brief  提供串口操作方面的函数
 * @endchinese
 *
 * @english
 * @file   uartlib.h
 * @author Liu Xun
 * @date   Thu Oct 13 10:00:00 2011
 *
 * @brief  functions of serail operating
 * @endenglish
 */

int uart_open(int port);
int uart_open_and_setattr(int port, int baudrate, int databit, char *stopbit, char parity);
int uart_clearattr(int fd);
int uart_set_baudrate(int fd, int baudrate);
int uart_set_databit(int fd, int databit);
int uart_set_parity(int fd, char parity);
int uart_set_stopbit(int fd, char *stopbit);
int uart_recv_data(int fd, char *data, int len);
int uart_send_data(int fd, char *data, int len);
int uart_clear(int fd);
int uart_close(int fd);

int UartWrite( int comfd, unsigned char *iBuf, int iLen );
int UsartRead( int comfd, unsigned char *oBuf, int oLen, unsigned int oTime);

#endif
