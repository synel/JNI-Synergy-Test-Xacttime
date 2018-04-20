#include <unistd.h> //sleep usleep
#include <sys/stat.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../libfunc.h"


int main(int argc, char *argv[])
{
	char buf[126];
	int i=0;

	if (OpenCamera()==FALSE) // open camera device and initialize it at the same time
	  {
		perror("Pixel setting error!");
	      return -1; 
	   }
	sleep(5);
	while(i<10)
	{
		sprintf(buf,"/tmp/%d.jpg",i++);
	     if(GetCameraImage((const char*)buf) ==FALSE) // get image and save it to buf.
		{
			perror("camera error!");
		      return -1; 
		}
		memset(buf,0,sizeof(buf));
		usleep(12000); // 12ms delay
	}

    CameraClose();	// close camera device ,it will call close_camera()
	return 0;
}
