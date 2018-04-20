/*
 * Synergy2416JNI.c
 *
 *  Created on: Nov 3, 2014
 *      Author: chaol
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "./include/libfunc.h"
#include "./include/SynergyUtility.h"

#include "./include/com_synel_synergy_synergy2416_jni_Synergy2416JNI.h"
static uint32_t *gpio = NULL; //base address for memory-mapped gpio

/*
 * Class:     com_synel_synergy_synergy2416_jni_Synergy2416JNI
 * Method:    helpInfo
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_synel_synergy_synergy2416_jni_Synergy2416JNI_helpInfo
(JNIEnv *, jobject) {
	printf("This is the java native library api for synergy2416HW \n");
	printf("It includes hw control api for the Power, the WebCam, the HID card etc.\n");
	return;
}

/*
 * Class:     com_synel_synergy_synergy2416_jni_Synergy2416JNI
 * Method:    getBatteryStatusCode
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_synel_synergy_synergy2416_jni_Synergy2416JNI_getBatteryStatusCode
(JNIEnv *env, jobject obj) {
	COMINFO cominfo[4];
	int  status = -1; //-1 indicateds com read error
	const int BAUDRATE = 115200;
	cominfo[0].enable = 0;
	cominfo[2].enable = 0;//rs485
#if defined _2410
	cominfo[1].enable = 1;//reader
	cominfo[1].baudrate = BAUDRATE;	//2400,4800,9600,19200,38400,57600,115200
	cominfo[1].workmode = 1;
#else // defined _2416
	cominfo[1].enable = 0;//gsm
	cominfo[3].type = 1; //0 = uart 1 = spi reader
	cominfo[3].enable = 1;
	cominfo[3].baudrate = BAUDRATE;	//2400,4800,9600,19200,38400,57600,115200
	cominfo[3].workmode = 1; //answer
#endif

	if(OpenCom(cominfo) == FALSE) // Initialize serial port
	{
		perror("com init error");
		return -1;
	}

	while(1)
	{
		if(ReadCom1()==FALSE)	continue;

		status = get_battery_power_status();

		switch(status){
		case 0:
			printf("no battery power\n");
			break;
		case 1:
			printf("battery charging\n");
			break;
		case 2:
			printf("battery charged\n");
			break;
		case 3:
			printf("battery discharging\n");
			break;
		case 4:
			printf("low voltage\n");
			break;
		case 5:
			printf("high voltage\n");
			break;
		default:
			printf("unknown\n");
			break;
		}
		break;
	}

#if defined _2410
	UnCom(1);// Close serial device
#else // defined _2416
	UnCom(3 + 10);
#endif
	return status;
}

/*
 * Class:     com_synel_synergy_synergy2416_jni_Synergy2416JNI
 * Method:    setWatchDog
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_synel_synergy_synergy2416_jni_Synergy2416JNI_setWatchDog
(JNIEnv *env, jobject obj, jint jmode) {
	int return_code = -1;
	//printf("Usage: watchdogtest status\r\n");
	//printf("status: 0 = close 255 = shutdown 1-254 = interval\r\n");
	int mode = jmode;
	COMINFO cominfo[3];
	cominfo[0].enable=0;
	cominfo[2].enable=0;
#if defined _2410
	cominfo[1].enable=1;
	cominfo[1].baudrate=6;
	cominfo[1].workmode=1;
#else // defined _2416
	cominfo[1].enable=0;
	cominfo[3].type=1;
	cominfo[3].enable=1;
	cominfo[3].baudrate=6;
	cominfo[3].workmode=1;
#endif

	if(OpenCom(cominfo) == FALSE) // Initialize serial port
	{
		perror("com init error");
		return return_code;
	}
	printf("set machine mode\r\n");
	return_code = Set_Machine_Mode(mode);
	printf("return_code %d\r\n",return_code);
#if defined _2410
	UnCom(1);// Close serial device
#elif defined _2416
	UnCom(3 + 10);// Close serial device
#endif
	return return_code;
}


/*
 * Class:     com_synel_synergy_synergy2416_jni_Synergy2416JNI
 * Method:    takePicture
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_synel_synergy_synergy2416_jni_Synergy2416JNI_takePicture
(JNIEnv *env, jobject obj, jstring jfilename) {
	const char* nativefilename = env->GetStringUTFChars(jfilename, 0);
	//char buf[126];

	if (OpenCamera()==FALSE) // open camera device and initialize it at the same time
	{
		perror("Pixel setting error!");
		return -1;
	}
	sleep(5);


	//sprintf(buf,nativefilename);
	if(GetCameraImage(nativefilename) == FALSE) // get image and save it to buf.
	{
		perror("camera error!");
		return -1;
	}
	env->ReleaseStringUTFChars(jfilename, nativefilename);
	//usleep(12000); // 12ms delay

	CameraClose();	// close camera device ,it will call close_camera()
	return 0;
}

/*
 * Class:     com_synel_synergy_synergy2416_jni_Synergy2416JNI
 * Method:    takePictures
 * Signature: (Ljava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_com_synel_synergy_synergy2416_jni_Synergy2416JNI_takePictures
(JNIEnv *env, jobject obj, jstring jbasename, jint numOfPictures) {
	const char* nbasename = env->GetStringUTFChars(jbasename, 0);
	char filename[126];
	int picNum = 0; //return number of pictures taken.

	if (OpenCamera()==FALSE) // open camera device and initialize it at the same time
	{
		perror("Pixel setting error!");
		return -1;
	}
	sleep(5);

	for (int i=0; i < numOfPictures; i++) {
		sprintf(filename,"%s_%i",nbasename,i);
		if(GetCameraImage(filename) ==FALSE) // get image and save it to buf.
		{
			perror("camera error!");
		}
		picNum++;
		usleep(12000); // 12ms delay
	}
	env->ReleaseStringUTFChars(jbasename, nbasename);
	CameraClose();	// close camera device ,it will call close_camera()
	return picNum;
}

/*
 * Class:     com_synel_synergy_synergy2416_jni_Synergy2416JNI
 * Method:    initVolume
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_synel_synergy_synergy2416_jni_Synergy2416JNI_initVolume
(JNIEnv *env, jobject obj) {
	//TODO
	SOUNDINFO soundinfo;
	soundinfo.sound_kq=1;
	soundinfo.sound_kq=1;
	soundinfo.sound_type=1;
	soundinfo.sound_key=1;
	soundinfo.sound_menu=1;   //
	InitVolume(&soundinfo);
}

/*
 * Class:     com_synel_synergy_synergy2416_jni_Synergy2416JNI
 * Method:    setVolume
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_synel_synergy_synergy2416_jni_Synergy2416JNI_setVolume
(JNIEnv *env, jobject obj, jint vol, jstring jsoundfilename) {
	const char* nsoundname = env->GetStringUTFChars(jsoundfilename, 0);
	char* nsound = (char*)malloc(strlen(nsoundname)+1);
	if(nsound) {
		strcpy(nsound, nsoundname);
	}
	SOUNDINFO soundinfo;
	soundinfo.sound_kq=1;
	soundinfo.sound_kq=1;
	soundinfo.sound_type=1;
	soundinfo.sound_key=1;
	soundinfo.sound_menu=1;   //
	InitVolume(&soundinfo);
	SetVolume(vol);
#if defined _2410
	MenuSound(nsound);
#else //if defined _2416
	MenuSound(nsound);
#endif
	free(nsound);
	env->ReleaseStringUTFChars(jsoundfilename, nsoundname);

}

/*
 * Class:     com_synel_synergy_synergy2416_jni_Synergy2416JNI
 * Method:    printData
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_synel_synergy_synergy2416_jni_Synergy2416JNI_printData
(JNIEnv *env, jobject obj, jstring jdata){
	//TODO
	return FALSE;
}

/*
 * Class:     com_synel_synergy_synergy2416_jni_Synergy2416JNI
 * Method:    readMAC
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_synel_synergy_synergy2416_jni_Synergy2416JNI_readMAC
(JNIEnv *env, jobject obj) {
	//TODO
	return NULL;
}

/*
 * Class:     com_synel_synergy_synergy2416_jni_Synergy2416JNI
 * Method:    setMAC
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_synel_synergy_synergy2416_jni_Synergy2416JNI_setMAC
(JNIEnv *env, jobject obj, jstring jmac) {
	//TODO
}

/*
 * Class:     com_synel_synergy_synergy2416_jni_Synergy2416JNI
 * Method:    readWigand
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_synel_synergy_synergy2416_jni_Synergy2416JNI_readWigand
(JNIEnv *env, jobject obj, jint jtype) {
	//TODO
	return NULL;
}

/*
 * Class:     com_synel_synergy_synergy2416_jni_Synergy2416JNI
 * Method:    readCardSector
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_synel_synergy_synergy2416_jni_Synergy2416JNI_readCardSector
(JNIEnv *env, jobject obj) {
	//TODO
	return NULL;
}

/*
 * Class:     com_synel_synergy_synergy2416_jni_Synergy2416JNI
 * Method:    writeCardSector
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_synel_synergy_synergy2416_jni_Synergy2416JNI_writeCardSector
(JNIEnv *env, jobject obj, jstring jcardNum) {
	//TODO
	return FALSE;
}

/*
 * Class:     com_synel_synergy_synergy2416_jni_Synergy2416JNI
 * Method:    writeCardKey
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_synel_synergy_synergy2416_jni_Synergy2416JNI_writeCardKey
(JNIEnv *env, jobject obj, jstring jcardKey) {
	//TODO
	return FALSE;
}

/*
 * Class:     com_synel_synergy_synergy2416_jni_Synergy2416JNI
 * Method:    redON
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_synel_synergy_synergy2416_jni_Synergy2416JNI_redON
(JNIEnv *env, jobject obj) {
	printf("debug: initial gpio address is %02x \n",gpio);
	if(_memory_setup(&gpio) < 0){
		printf("Memory Setup Failed Critical Error\n");
	}
	else{
#if defined _2410
		*gpio = *(gpio) | 0x200000; //RED on
#else //defined _2416
		printf("debug: gpio address is %02x \n",gpio);
		*gpio = *(gpio) & (~(0x1<<16)); //Set RED LED Pin as output
		*(gpio+0x1)= *(gpio+0x1) | (0x1<<16); //RedON-->Synergy2416 GPA16
#endif
		if(_memory_close(&gpio) < 0){
			printf("Memory Failed to Close-May Cause Memory Leak\n");
		}
	}
}

/*
 * Class:     com_synel_synergy_synergy2416_jni_Synergy2416JNI
 * Method:    redOFF
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_synel_synergy_synergy2416_jni_Synergy2416JNI_redOFF
(JNIEnv *env, jobject obj) {
	if(_memory_setup(&gpio) < 0){
		printf("Memory Setup Failed Critical Error\n");
	}
	else{
#if defined _2410
		*(gpio)= *(gpio) & 0xDFFFFF; //RedOFF
#else
		*gpio = *(gpio) & (~(0x1<<16)); //Set RED LED Pin as output
		*(gpio+0x1)= *(gpio+0x1) & (~(0x1<<16)); //RedOFF-->Synergy2416 GPA16
#endif
		if(_memory_close(&gpio) < 0){
			printf("Memory Failed to Close-May Cause Memory Leak\n");
		}
	}
}

/*
 * Class:     com_synel_synergy_synergy2416_jni_Synergy2416JNI
 * Method:    greenON
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_synel_synergy_synergy2416_jni_Synergy2416JNI_greenON
(JNIEnv *env, jobject obj) {
	if(_memory_setup(&gpio) < 0){
		printf("Memory Setup Failed Critical Error\n");
	}
	else{
#if defined _2410
		*(gpio)= *(gpio) | 0x80000; //GREENON
#else
		*gpio = *(gpio) & (~(0x1<<15)); //Set RED GREED Pin as output
		*(gpio+0x1)= *(gpio+0x1) | (0x1<<15); //GREEN ON-->Synergy2416 GPA15
#endif
		if(_memory_close(&gpio) < 0){
			printf("Memory Failed to Close-May Cause Memory Leak\n");
		}
	}
}

/*
 * Class:     com_synel_synergy_synergy2416_jni_Synergy2416JNI
 * Method:    greenOFF
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_synel_synergy_synergy2416_jni_Synergy2416JNI_greenOFF
(JNIEnv *env, jobject obj) {
	if(_memory_setup(&gpio) < 0){
		printf("Memory Setup Failed Critical Error\n");
	}
	else{
#if defined _2410
		*(gpio)= *(gpio) & 0xF7FFFF; //GREENOFF
#else
		*gpio = *(gpio) & (~(0x1<<15)); //Set GREEN LED Pin as output
		*(gpio+0x1)= *(gpio+0x1) & (~(0x1<<15)); //GREEN OFF-->Synergy2416 GPA15
#endif
		if(_memory_close(&gpio) < 0){
			printf("Memory Failed to Close-May Cause Memory Leak\n");
		}
	}
}

/*
 * Class:     com_synel_synergy_synergy2416_jni_Synergy2416JNI
 * Method:    modemON
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_synel_synergy_synergy2416_jni_Synergy2416JNI_modemON
(JNIEnv *env, jobject obj) {
	//TOTEST
	if(_memory_setup(&gpio) < 0){
			printf("Memory Setup Failed Critical Error\n");
		}
		else{
	#if defined _2410
			//*(gpio)= *(gpio) & 0xF7FFFF
	#else
			*gpio = *(gpio) & (~0x1000000); //Set GREEN LED Pin as output
			*(gpio+0x1)= *(gpio+0x1) & (~(0x1<<24)); //Synergy2416 GPA24 pull down for 2 seconds
			usleep(2000);
			*(gpio+0x1)= *(gpio+0x1) | (0x1<<24); //Synergy2416 GPA24 pull high
	#endif
			if(_memory_close(&gpio) < 0){
				printf("Memory Failed to Close-May Cause Memory Leak\n");
			}
		}
}

/*
 * Class:     com_synel_synergy_synergy2416_jni_Synergy2416JNI
 * Method:    modemOFF
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_synel_synergy_synergy2416_jni_Synergy2416JNI_modemOFF
(JNIEnv *env, jobject obj) {
	//TODO
	if(_memory_setup(&gpio) < 0){
				printf("Memory Setup Failed Critical Error\n");
			}
			else{
		#if defined _2410
				//*(gpio)= *(gpio) & 0xF7FFFF
		#else
				*gpio = *(gpio) & (~(0x1<<24)); //Set GREEN LED Pin as output
				*(gpio+0x1)= *(gpio+0x1) | (0x1<<24); //Synergy2416 GPA24 pull high for 2 seconds
				//usleep(2000);
				//*(gpio+0x1)= *(gpio+0x1) & (0x1000000); //Synergy2416 GPA24 pull high data port is gpio+0x1 (equal to +4 to address)
		#endif
				if(_memory_close(&gpio) < 0){
					printf("Memory Failed to Close-May Cause Memory Leak\n");
				}
			}
}

/*
 * Class:     com_synel_synergy_synergy2416_jni_Synergy2416JNI
 * Method:    doorbellON
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_synel_synergy_synergy2416_jni_Synergy2416JNI_relayON
(JNIEnv *env, jobject obj) {
	// GPK CON IS 0x560000e0
	if(_memory_setup(&gpio) < 0){
					printf("Memory Setup Failed Critical Error\n");
				}
				else{
			#if defined _2410
					//*(gpio)= *(gpio) & 0xF7FFFF
			#else
					*(gpio+0x38) = *(gpio+0x38) & ~(0x3<<24); //Set GPK12 Pin as output: set mask
					*(gpio+0x38) = *(gpio+0x38) | (0x1<<24); //Set GPK12 Pin as output

					//*(gpio+0x38) = *(gpio+0x38) & ~(0x3<<24); //Synergy2416 GPK12 pull up enable: set mask
					//*(gpio+0x3a)= *(gpio+0x3a) | (0x2<<24); //Synergy2416 GPK12 pull up enable: 10

					*(gpio+0x39)= *(gpio+0x39) | (0x1<<12); //Synergy2416 GPK12 pull high
					//usleep(2000);
					//*(gpio+0x1)= *(gpio+0x1) & (0x1000000); //Synergy2416 GPA24 pull high data port is gpio+0x1 (equal to +4 to address)
			#endif
					if(_memory_close(&gpio) < 0){
						printf("Memory Failed to Close-May Cause Memory Leak\n");
					}
				}
}

/*
 * Class:     com_synel_synergy_synergy2416_jni_Synergy2416JNI
 * Method:    doorbellOFF
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_synel_synergy_synergy2416_jni_Synergy2416JNI_relayOFF
(JNIEnv *env, jobject obj) {
	// GPK12 OUT
	if(_memory_setup(&gpio) < 0){
					printf("Memory Setup Failed Critical Error\n");
				}
				else{
			#if defined _2410
					//*(gpio)= *(gpio) & 0xF7FFFF
			#else
					*(gpio+0x38) = *(gpio+0x38) & ~(0x3<<24); //Set GPK12 Pin as output: set mask
					*(gpio+0x38) = *(gpio+0x38) | (0x1<<24); //Set GPK12 Pin as output

					//*(gpio+0x38) = *(gpio+0x38) & ~(0x3<<24); //Synergy2416 GPK12 pull up enable: set mask
					//*(gpio+0x3a)= *(gpio+0x3a) | (0x1<<24); //Synergy2416 GPK12 pull down enable: 01

					*(gpio+0x39)= *(gpio+0x39) & (~(0x1<<12)); //Synergy2416 GPK12 pull down
					//*(gpio+0x1)= *(gpio+0x1) & (0x1000000); //Synergy2416 GPA24 pull high data port is gpio+0x1 (equal to +4 to address)
			#endif
					if(_memory_close(&gpio) < 0){
						printf("Memory Failed to Close-May Cause Memory Leak\n");
					}
				}
}

/*
 * Class:     com_synel_synergy_synergy2416_jni_Synergy2416JNI
 * Method:    doorBellPressed
 * Signature: ()Z
 */

JNIEXPORT jboolean JNICALL Java_com_synel_synergy_synergy2416_jni_Synergy2416JNI_doorBellPressed
  (JNIEnv *env, jobject obj) {
	// GPK13 IN
	    int res = FALSE;
		if(_memory_setup(&gpio) < 0){
						printf("Memory Setup Failed Critical Error\n");
		} else{
				#if defined _2410
						//*(gpio)= *(gpio) & 0xF7FFFF
				#else
						*(gpio+0x38) = *(gpio+0x38) & ~(0x3<<26); //Set GPK13 Pin mask
						//*(gpio+0x38) = *(gpio+0x38) | (0x0<<26); //Set GPK13 Pin as input 00
						res = (*(gpio+0x39) & (0x1<<13))>>13; //Synergy2416 GPK13
				#endif
						if(_memory_close(&gpio) < 0){
							printf("Memory Failed to Close-May Cause Memory Leak\n");
						}
					}
		return res;
 }

