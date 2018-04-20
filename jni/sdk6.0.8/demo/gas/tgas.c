/*
 * tgas.c
 *
 *  Created on: 2014-7-2
 *      Author: aduo
 */

/*
 * æ∆æ´≤‚ ‘“« FAR-Q8
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//////////////////////////////////////////////////////////////////////////
int _input_number(char* szPrompt, int dwDefault)
{
	int	vKey, vCnt = 0;
	char vStr[16] = {0}, *vStr2;
	int vRet = dwDefault;

	printf("Please input %s [default = %u] ", szPrompt, (unsigned int)dwDefault);

	while(1)
	{
		vKey = getchar();
		if ( vKey >= '0' && vKey <= '9')
		{
			vStr[vCnt] = (char)vKey;
			vCnt++;
			vStr[vCnt] = 0;

			vRet = (int)strtoul(vStr, &vStr2, 10);

			if(vCnt > 10)
				break;
		}
		else if ( vKey == '\n' )
		{
			if (vCnt == 0)
				vRet = dwDefault;
			goto RET;
		}
	}

	while(1)
	{
		vKey = getchar();
		if ( vKey == 0x0a )
			break;
	}

RET:
	return vRet;
}

int main() {
	int ret = 0;
	int len = 0;
	int i = 0;
	int nContinue = 1,vNum=0;
	unsigned char data[16];
	unsigned char hex[16];

	ret = open_port(g_serial_port, 9600, 8, "1", 'N');

   	while( nContinue )
	{
		printf("\n");
		printf("-------Menu-------\n");
		printf("0  : Exit\n");
		printf("1  : start\n");
		printf("2  : get\n");
		printf("3  : stop\n");
		printf("------------------\n");

		printf("\n\n");
		vNum = (int) _input_number("menu item", 0);

		switch (vNum) {
		case 0: //Exit
			nContinue = 0;
			printf("Exit OK.\n");
			break;
		case 1: //start
			ret = start_gas_test(g_serial_port);
			if (ret == 1) {
				printf("start gas test success \n");
			} else {
				printf("start gas test fail \n");
			}
			break;
		case 2:
			while (1) {
				memset(data, 0, sizeof(data));

				len = read_gas_test(g_serial_port,data);
				if (len > 0) {
					printf("The content of gas %s mg/100ml\n", data);
					break;
				}
			}
			break;
		case 3: //stop
			ret = stop_gas_test(g_serial_port);
			if (ret == 1) {
				printf("stop gas test success \n");
			} else {
				printf("stop gas test fail \n");
			}
			break;
		}
	}

	ret = close_port(g_serial_port);

	return 0;
}

