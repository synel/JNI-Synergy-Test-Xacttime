#include<stdio.h>
#include <errno.h>
#include <string.h>
//#include <unistd.h>
#include "../libfunc.h"

int main(int argc, char *argv[])
{
	char buf[126];
	int error = 0;

	printf("Usage: datetimetest date time\n");
	printf("date format: YYYY-MM-DD\n");
	printf("time format: hh:mm:ss\n");

	if (argc < 3)
		return -1;
	if (GetDate(buf, RTC_ENABLE) == FALSE) //get date/time of currect
	{
		perror("get date error");
		return -1;
	}
	printf("currect time=%s\n", buf);

	// set date
	error = MenuSetDate(argv[1], RTC_ENABLE);
	if (error == FALSE) {
		perror("set date error\n");
	} else if (error == ERR_FORMAT) {
		perror("date format error\n");
	} else {
		printf("set date ok\n");
	}

	// set time
	error = MenuSetTime(argv[2], RTC_ENABLE);
	if (error == FALSE) {
		perror("set time error\n");
		return -1;
	} else if (error == ERR_FORMAT) {
		perror("time format error\n");
	} else {

		printf("set time ok\n");
	}
	return 0;
}
