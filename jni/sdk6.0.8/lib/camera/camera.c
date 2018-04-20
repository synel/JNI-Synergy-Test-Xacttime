
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <dirent.h>
#include <signal.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../_precomp.h"

#include "camera.h"
#include "../public/public.h"
#include "../gpio/gpio.h"

#if defined _2410
#include "v4l.h"
#elif defined _2416
#include "spcav4l2.h"
#endif

sigjmp_buf catpicture_loop; //env-variable structure
int err_num=0;		//video err num
int camera_enable=0;


/*--------------------------------------------------------------------------*
@Function          :OpenCamera - Open Camera Device
@Include      	:"camera.h"
@Description		:Initialize camera device£¬the default pixel is 320*240, brightness  : 96
@Return value	: Failure 0; Success 1	
*---------------------------------------------------------------------------*/
int OpenCamera(void)
{

	if(access("/dev/v4l/video0",F_OK)!=0)
	{
		perror("Camera equipment, file does not exist!");
		camera_enable=0;
		return FALSE;
	}

	if (init_camera(320,240,96,6)==0) // open camera device and initialize it at the same time
	{

		GpioOpt(10);//power off the  camera
		sleep(1);	//this delay is needed 
		GpioOpt(9);	//power on the camera again
		sleep(4);	//waiting for driver identify the device
		if (init_camera(320,240,96,6)==0) 
		{
			camera_enable=0;
			perror("Pixel setting error!");
			return FALSE;
		}
	}

	camera_enable=1;
	return TRUE;
}

/*--------------------------------------------------------------------------*
@Function		: GetCameraImage - Get image
@Incldue		:"camera.h"
@Description		: 
			IN   char *FileName£ºPath to save images£»
@Return Value	: Failure FALSE; Sucess TURE
*---------------------------------------------------------------------------*/
int GetCameraImage(IN const char *FileName)
{
	int i=0;
	
	if(camera_enable==0)	return FALSE;
	creatdir((char *)FileName);

	alarm(10);
	signal(SIGALRM,overtime);
	if(sigsetjmp(catpicture_loop,1)==0)//
        {
		errno=0;
		i=get_camera_jpeg((char *)FileName);
		alarm(0);
		if(i==1)
		{
			err_num=0;
			return TRUE;
		}
		else
		{
			err_num++;
			if(err_num>=2)	return FALSE;
			reset_camera();
			alarm(10);
			i=get_camera_jpeg((char *)FileName);
			alarm(0);
			if(i==1)	err_num=0;
			return TRUE;
		}
	}
	else
	{
		err_num++;
		if(err_num>=2)	return FALSE;
		reset_camera();
		alarm(0);
		return TRUE;
	}
	alarm(0);

	return FALSE;

}

/*--------------------------------------------------------------------------*
@Function		      :CameraClose - Close Camera Device
@Incldue      		: "camera.h"
@Description			: void
@Return Value		: Failure FAlSE; Success TRUE
*---------------------------------------------------------------------------*/
int CameraClose(void)
{
	if(camera_enable==0)	return FALSE;
	close_camera();
	camera_enable=0;
	return TRUE;
}

//used for overtime skip,eg,when can not find camera device.
void overtime(int signo) 
{
	printf("overtime %d\n",signo);
	siglongjmp(catpicture_loop,1);//jump
}

//restart the camera when fail to take picture
int reset_camera()
{
	CameraClose();
	sleep(1);
	GpioOpt(10);
	sleep(1);
	GpioOpt(9);
	sleep(4);
	if(init_camera(320,240,96,6)==0)  //load camera again
	{
		camera_enable=0;
		return FALSE;
	}
	camera_enable=1;
	return TRUE;
}

