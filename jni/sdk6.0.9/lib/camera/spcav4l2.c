#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/videodev2.h>
#include <sys/types.h>
#include <sys/stat.h>


#include "spcav4l2.h"
#include "v4l2uvc.h"
#include "huffman.h"

struct vdIn *videoIn;
globals *pglobal;

static int capture(struct vdIn *videoIn, char *filename)
{
    unsigned char *frame = NULL;
    int frame_size;
    int fd=0;
    char buffer[1024];

    pglobal->buf = malloc(videoIn->framesizeIn);
    if(pglobal->buf == NULL)
    {
        printf("could not allocate memory\n");
        return -1;
    }

    //printf("%d\n", videoIn->framesizeIn);
    if(uvcGrab(videoIn) < 0)
    {
        printf("grabbing frames error;\n");
//        return -1;
        goto err;
    }


    pglobal->size = memcpy_picture(pglobal->buf, videoIn->tmpbuffer, videoIn->buf.bytesused);

    if((frame = malloc(512 * 1024)) == NULL)
    {
        printf("not enough memory\n");
//        return -1;
        goto err;
    }

    frame_size = pglobal->size;
    memcpy(frame, pglobal->buf, frame_size);

    memset(buffer, 0, sizeof(buffer));

    strcpy(buffer, filename);

    /* open file for write */
#if 1
    if( (fd = open(buffer, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) < 0 ) {
        printf("could not open the file %s\n", buffer);
//        return -1;
        goto err;
    }

    /* save picture to file */
    if( write(fd, frame, frame_size) < 0 ) {
        printf("could not write to file %s\n", buffer);
        goto err;
//        close(fd);
//        return -1;
    }
#endif

    close(fd);
    free(pglobal->buf);
    free(frame);
    pglobal->buf = NULL;
    frame = NULL;
    return 0;
    err:

    if(fd > 0)
    {
        close(fd);
        fd = -1;
    }
    if(pglobal->buf != NULL)
    {
        free(pglobal->buf);
        pglobal->buf = NULL;
    }
    if(frame != NULL)
    {
        free(frame);
        frame = NULL;
    }
    return -1;
}

int setbrightness(int val)
{
    return v4l2SetControl(videoIn, V4L2_CID_BRIGHTNESS, val);
}

static int cam_init(int width, int height)
{
    char *dev = "/dev/video0";
    int fps = 1;
    int format = V4L2_PIX_FMT_MJPEG;
    int grabmethod = 1;

    pglobal = malloc(sizeof(globals));
    if(pglobal == NULL)
    {
        printf("not enough memory for videoIn\n");
        return(-1);
    }
    memset(pglobal,0,sizeof(globals));
    videoIn = malloc(sizeof(struct vdIn));
    if(videoIn == NULL)
    {
        printf("not enough memory for videoIn\n");
        return(-1);
    }
    memset(videoIn,0,sizeof(struct vdIn));
    if(init_videoIn(videoIn, dev, width, height, fps, format, grabmethod) < 0)
    {
        printf("init_videoIn failed\n");
        free(videoIn) ;
        free(pglobal);
        videoIn = NULL;
        pglobal = NULL;
        return(-1);
    }

    return 0;
}

static int getimage(char *filename)
{
    int retval = 0;

    //input_cmd(videoIn, IN_CMD_BRIGHTNESS_PLUS, 0);
    capture(videoIn, filename);
    if(retval < 0)
    {
        return retval;
    }
    return 0;
}

static int cam_close()
{    
    close_v4l2(videoIn);
    free(videoIn);
    free(pglobal);
    videoIn = NULL;
    pglobal = NULL;
    return 1;
}

//image_width*image_height 640*480 352*288  320*240  0<brightness<0x80  0<contrast<=6
//false->0  true->1
int init_camera(int image_width, int image_height, int brightness, int contrast){
	int return_code;
	return_code = cam_init(image_width,image_height);
	if (return_code == 0){
		return 1;
	}else{
		return 0;
	}
}
//false->0  true->1
int get_camera_jpeg(char* FileName){
	int return_code;
	return_code = getimage(FileName);
	if (return_code == 0){
		return 1;
	}else{
		return 0;
	}
}

void close_camera(){
	cam_close();
}


