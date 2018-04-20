/****************************************************************************
****************************************************************************/

#ifndef SPCAV4L_H
#define SPCAV4L_H 
//extern "C" {
int init_camera(int image_width, int image_height, int brightness, int contrast); 
 //image_width*image_height 640*480 352*288  320*240     false->0  true->1   0<brightness<0x80  0<contrast<=6
int get_camera_jpeg(char* FileName);           //false->0  true->1
void close_camera();
//}
#endif /* SPCAV4L_H */
