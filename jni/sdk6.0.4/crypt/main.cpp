#include<stdio.h>
#include <errno.h>
#include <string.h>
//#include <unistd.h>
#include "../libfunc.h"

int main(int argc, char *argv[])
{
  char buf[128]="abcdefg",tmp[128];

  printf("Usage: crypttest\n");

	_encrypt(buf,tmp,strlen(buf),sizeof(tmp));
	printf("encrypt:%s\n",tmp);
	memset(buf,0,sizeof(buf));
	_decrypt(tmp,buf,strlen(tmp),sizeof(buf));
	printf("decrypt:%s\n",buf);


  return 0;
}
