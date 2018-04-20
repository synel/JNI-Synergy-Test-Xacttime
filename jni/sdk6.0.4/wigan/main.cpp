#include<stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../libfunc.h"

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

int main(int argc, char *argv[])
{
	char *p=0;
	int nContinue=1,vNum=0;
	struct mem_inout sno;

	if(InitWiganRead() == FALSE) // Open WG reading device
	{		
		perror("open wgout error");
		return -1;
	}
	printf( "\n" );
	printf( "-------Menu-------\n" );
	printf( "0  : Exit\n" );
    	printf( "1  : wg_26_0\n" );
    	printf( "2  : wg_26_1\n" );
    	printf( "3  : wg_26_2\n" );
    	printf( "4  : wg_26_3\n" );
	printf( "------------------\n" );

   	while( nContinue )
	{
		printf( "\n\n" );
		vNum = (int) _input_number("menu item", 100);

		switch(vNum)
		{
		case 0://Exit
	      		nContinue = 0;
			printf("Exit OK.\n");
			break;
		case 1:
			printf("Read 26_0\n");
			while(1){
				p=wg26_0();
				if(p!=NULL)
				{
					printf("card=%s\n",p);
					break;
				}
   			}
			break;
		case 2:
			printf("Read 26_1\n");
			while(1){
				p=wg26_1();
				if(p!=NULL)
				{
					printf("card=%s\n",p);
					break;
				}
   			}
			break;
		case 3:
			printf("Read 26_2\n");
			while(1){
				p=wg26_2();
				if(p!=NULL)
				{
					printf("card=%s\n",p);
					break;
				}
   			}
			break;
		case 4:
			printf("Read 26_3\n");
			while(1){
				p=wg26_3();
				if(p!=NULL)
				{
					printf("card=%s\n",p);
					break;
				}
   			}
			break;

		}
	}

	if(CloseWiganRead() == FALSE) // Close WG reading device
	{
		perror("close  error");
		return -1;
	}

	return 0;
}
