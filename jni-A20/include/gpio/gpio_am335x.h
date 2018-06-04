/*
 * gpio_am335x.h
 *
 *  Created on: 2014-8-8
 *      Author: aduo
 */

#ifndef GPIO_AM335X_H_
#define GPIO_AM335X_H_

int gpio_init_am335x();
int gpio_read_am335x(int fd, int pin);
int gpio_write_am335x(int fd, int pin ,int value);
int gpio_close_am335x(int fd);


#endif /* GPIO_AM335X_H_ */
