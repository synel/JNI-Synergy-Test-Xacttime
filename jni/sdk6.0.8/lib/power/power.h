/*
 * power.h
 *
 *  Created on: 2013-7-11
 *      Author: aduo
 */

#ifndef POWER_H_
#define POWER_H_

#include "../version.h"

typedef enum _battery_status{
	bs_none,
	bs_charging,
	bs_charged,
	bs_discharging,
	bs_low_voltage,
	bs_high_voltage,
	bs_unknown
} battery_status;

struct _battery{
	battery_status status;
	int energy;
};

extern struct _battery battery;

int get_battery_power_status();
int get_battery_power_energy();
int open_battery_power_report();
int close_battery_power_report();

#endif /* POWER_H_ */
