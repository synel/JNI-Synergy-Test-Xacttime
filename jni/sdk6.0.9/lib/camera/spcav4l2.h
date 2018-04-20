#ifndef SPCAV4L2_H
#define SPCAV4L2_H

struct _globals {

    /* global JPG frame, this is more or less the "database" */
    unsigned char *buf;
    int size;
};
typedef struct _globals globals;

int init_camera(int image_width, int image_height, int brightness, int contrast);
int get_camera_jpeg(char* FileName);
void close_camera();

#endif
