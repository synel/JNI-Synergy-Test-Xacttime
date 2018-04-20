#include<stdio.h>
#include "../libfunc.h"

int main(int argc, char *argv[])
{
  FILE *fp;
  char buf[1024];

  printf("Usage: filetest \n");

  safe_cp("aa/","bb/");//copy file
  safe_rm("bb/"); // delete file
  fp=creatdirfile("/weds/kq42/");//get file list
  while(fgets(buf,sizeof(buf),fp))
  {
	printf("buf=%s\n",buf);
  }
  return 0;
}
