/*
 * power.c
 *
 *  Created on: 2013-7-11
 *      Author: aduo
 */


#include "../_precomp.h"
#include "../public/initcom.h"
#include "../public/protocol.h"
#include "power.h"

struct _battery battery;

int get_battery_power_status() {
	unsigned char status;

	battery.status = bs_unknown;

	switch (com1value.instruction) {
	case 0x8C:
		status = com1value.user_data[0];
	    if (status == 110){
			battery.status = bs_low_voltage;
		}else if (status == 120){
			battery.status = bs_none;
		}else if (status == 130){
			battery.status = bs_charging;
		}else if (status == 140){
			battery.status = bs_charged;
		}else if (status == 150){
			battery.status = bs_high_voltage;
		}else{
			battery.status = bs_discharging;
		}
		break;
	}
	return battery.status;
}

int get_battery_power_energy(){
	unsigned char status;

	switch (com1value.instruction) {
	case 0x8C:
		status = com1value.user_data[0];
		if (status > 0 && status < 100){
			battery.energy = status;
		}else{
			battery.energy = 0;
		}
		break;
	}
  return battery.energy;
}


int close_battery_power_report() {
	_TData asker;

	memset(&asker, 0, sizeof(asker));
	asker.address = 0X01;
	asker.instruction = 0x8C;
	asker.nbytes = 0X03;
	asker.itemnum = 0x01;
	asker.user_data[0] = 0xFF;

#if defined _2410
	if (_put_data(serialport1, &asker) != SUCCESS)
	return FALSE;
#elif defined _2416
	if (_put_data(serialport3 + 10, &asker) != SUCCESS)
		return FALSE;
#endif
	printf("ccc\n");
	return TRUE;
}

int open_battery_power_report() {
	_TData asker;

	memset(&asker, 0, sizeof(asker));
	asker.address = 0X01;
	asker.instruction = 0x8C;
	asker.nbytes = 0X03;
	asker.itemnum = 0x01;
	asker.user_data[0] = 0x01;

#if defined _2410
	if (_put_data(serialport1, &asker) != SUCCESS)
	return FALSE;
#elif defined _2416
	if (_put_data(serialport3 + 10, &asker) != SUCCESS)
		return FALSE;
#endif
	printf("ccc\n");
	return TRUE;
}
