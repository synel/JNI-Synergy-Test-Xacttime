#ifndef C_CAMERAINFO_H
#define C_CAMERAINFO_H

#include "../version.h"

extern int camera_enable;
int OpenCamera(void);
int GetCameraImage(IN const char *path);
int CameraClose(void);

int reset_camera();
void overtime(int signo);

#endif
