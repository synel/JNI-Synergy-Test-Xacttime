#include <stdio.h>  
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char * readmac(char *p)
{
FILE *file;
char buf[256],*ptr;
file=fopen("/proc/cmdline","r");
if(file==NULL)return NULL;
memset(buf,0,sizeof(buf));
fgets(buf,sizeof(buf),file);
ptr=strstr(buf,"mac=");
if(ptr==NULL)ptr=strstr(buf,"MAC=");
if(ptr==NULL)return NULL;
ptr+=4;
strcpy(p,ptr);
*(p+17)='\0';
fclose(file);
return ptr;
}

int main(int argc, char *argv[]){
char buf[256],cmd[256];
memset(buf,0,sizeof(buf));
if(readmac(buf)){
memset(cmd,0,sizeof(cmd));
sprintf(cmd,"ifconfig eth0 down\nifconfig eth0 hw ether %s\nifconfig eth0 up\n",buf);
system(cmd);
}
}
