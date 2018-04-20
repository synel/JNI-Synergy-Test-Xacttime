/*
 * synergy2416.c
 *
 *  Created on: May 31, 2014
 *      Author: chaol
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdint.h>
#include "_precomp.h"
#include "_fp.h"
#include "_fphlp.h"
#include "base64.h"
#include <dirent.h>
#include <jni.h>
#include "com_synel_synergy_synergy2416_presentation_controller_FPU.h"
#include "com_synel_synergy_synergy2416_presentation_controller_FPU_Light.h"
/*
 * nPos is the array index in the fp memory
 * badgenum is the symbol in the fp data structure(FPINFO).
 */

#define APP_REPEAT		20
#define APP_DEF_FP_NUM	0
#define APP_CAP_TIMEOUT 500
#define JNI_FALSE  0
#define JNI_TRUE   1
#define hlp_printf printf
#define app_printf printf

struct timeval tv1,tv2;
char *templateLocation;
int fd ;
static uint32_t *gpio;
off_t target = 0x56000000;

void _tick_stat()
{
	gettimeofday(&tv1, NULL);
}

int _tick_end()
{
	gettimeofday(&tv2, NULL);
	return (tv2.tv_sec-tv1.tv_sec)*1000000+(tv2.tv_usec-tv1.tv_usec);
}

void _tick_print(char* szPrompt, int nTime)
{
	app_printf("%s %d.%03ds\n",szPrompt,nTime/1000000,(nTime/1000)%1000);
}

int _validate_finger(int nMilliSecondTimeout)
//Same as _capture_finger except it return more information
{
	int nRet = -1;
	struct timeval tv3,tv4;

	gettimeofday(&tv3, NULL);
	while(1)
	{
		_tick_stat();
		nRet = SB_FP_CAPTUREFINGER();
		if(nRet == 0)
		{
			return 0;
		}

		gettimeofday(&tv4, NULL);

		if((tv4.tv_sec-tv3.tv_sec)*1000+(tv4.tv_usec-tv3.tv_usec)/1000 > nMilliSecondTimeout)
		{
			//app_printf("Timeout\n");
			break;
		}
	}

	return nRet;

}
BOOL _capture_finger(int nMilliSecondTimeout)
{
	int nRet;
	struct timeval tv3,tv4;

	gettimeofday(&tv3, NULL);
	while(1)
	{
		_tick_stat();
		nRet = SB_FP_CAPTUREFINGER();
		if(nRet == 0)
		{
			return TRUE;
		}

		gettimeofday(&tv4, NULL);

		if((tv4.tv_sec-tv3.tv_sec)*1000+(tv4.tv_usec-tv3.tv_usec)/1000 > nMilliSecondTimeout)
		{
			//app_printf("Timeout\n");
			break;
		}
	}

	return FALSE;
}


int _memory_setup(){
	//size_t free =getpagesize();
	//Obtain handle to physical memory
	if ((fd = open ("/dev/mem", O_RDWR | O_SYNC) ) < 0) {
		printf("Unable to open /dev/mem: %s\n", strerror(errno));
		return -1;
	}
	//map a page of memory to gpio at offset 0x56000000
	gpio = (uint32_t *)mmap(0, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, fd, target );
	if((int32_t)gpio < 0) {
		printf("Mmap failed: %s\n", strerror(errno));
		return -1;
	}
	//gpio = gpio + 0X1C;//Offset of GPH Register base is $target adding 1 to base pointer adds 4 to address in 32 bit memory offset of GPH is 70 0x70/0x4 = 0x1C
	return 0;
}

int _memory_close(){
	close(fd);
	//gpio = gpio - 0x1C;
	if(munmap(gpio, getpagesize())<0){
		return -1;
	}
	else{
		return 0;
	}
}

char* _ultostr(unsigned long num, char *str, int base)
{
	//hlp_printf("Convert unsigned long number to string..base %d.\n",base);
	unsigned long temp = num;
	unsigned int digit;
	if ( NULL == str || 1 > base ){
		return NULL;
	}
	//Calculate number of digits for the string representation of the number
	do {
		temp /= base;
		str += 1;     //move str_ptr to one digit space right
	} while ( temp > 0 );
	*str = '\0';
	//Now move backwards to fill the digits
	do {
		digit = num-base*(num/base);
		if ( digit < 10 ){
			*(--str) = '0' + digit;
		}
		else {
			*(--str) = 'A'-10 + digit;
		}
		num = num/base;
	} while (num > 0);
	return str;
}

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
  JNIEnv* env;
  if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_8) != JNI_OK)
    return -1;

  return JNI_VERSION_1_8;
}

JNIEXPORT void JNICALL Java_com_synel_synergy_synergy2416_presentation_controller_FPU_REDON(JNIEnv * env, jclass class){
	if(_memory_setup() < 0){
		printf("Memory Setup Failed Critical Error\n");
	}
	else{
		*gpio = *(gpio) & (~0x10000); //Set RED LED Pin as output
		*(gpio+0x1)= *(gpio+0x1) | 0x10000; //RedON-->Synergy2416 GPA16
		if(_memory_close() < 0){
			printf("Memory Failed to Close-May Cause Memory Leak\n");
		}
	}
}

JNIEXPORT void JNICALL Java_com_synel_synergy_synergy2416_presentation_controller_FPU_REDOFF(JNIEnv * env, jclass class){
	if(_memory_setup() < 0){
		printf("Memory Setup Failed Critical Error\n");
	}
	else{
		*gpio = *(gpio) & (~0x10000); //Set RED LED Pin as output
		*(gpio+0x1)= *(gpio+0x1) & (~0x10000); //RedOFF-->Synergy2416 GPA16
		if(_memory_close() < 0){
			printf("Memory Failed to Close-May Cause Memory Leak\n");
		}
	}
}

JNIEXPORT void JNICALL Java_com_synel_synergy_synergy2416_presentation_controller_FPU_GREENON(JNIEnv * env, jclass class){
	if(_memory_setup() < 0){
		printf("Memory Setup Failed Critical Error\n");
	}
	else{
		*gpio = *(gpio) & (~0x8000); //Set RED GREED Pin as output
		*(gpio+0x1)= *(gpio+0x1) | 0x8000; //GREEN ON-->Synergy2416 GPA15
		if(_memory_close() < 0){
			printf("Memory Failed to Close-May Cause Memory Leak\n");
		}
	}
}

JNIEXPORT void JNICALL Java_com_synel_synergy_synergy2416_presentation_controller_FPU_GREENOFF(JNIEnv * env, jclass class){
	if(_memory_setup() < 0){
		printf("Memory Setup Failed Critical Error\n");
	}
	else{
		*gpio = *(gpio) & (~0x8000); //Set GREEN LED Pin as output
		*(gpio+0x1)= *(gpio+0x1) & (~0x8000); //GREEN OFF-->Synergy2416 GPA15
		if(_memory_close() < 0){
			printf("Memory Failed to Close-May Cause Memory Leak\n");
		}
	}
}

JNIEXPORT jint JNICALL Java_com_synel_synergy_synergy2416_presentation_controller_FPU_FP_1OPENDEVICE
(JNIEnv *env, jclass jcls, jstring JtemplateLoc){
	hlp_printf("%s: called \n",__func__);
	int nSensorType = CMOSTYPE_OV7648;
	char *nativeString = "3k";
	int nRet=0;//10k 3k CMOSTYPE_HV7131R CMOSTYPE_EB6048
	templateLocation = (char*)(*env)->GetStringUTFChars(env, JtemplateLoc, 0);
	putenv("HOME=/root");
	hlp_printf("%s: templateLocation is %s, nSensorType (%s,%d) \n",__func__,templateLocation,nativeString,nSensorType);
	nRet=hlpOpen(nativeString, nSensorType);
	if(nRet < 0)
	{
		nativeString = "10k";
		nSensorType = CMOSTYPE_OV7648;
		nRet=hlpOpen(nativeString, nSensorType);
		hlp_printf("%s: nRet %d, try next nSensorType (%s,%d) \n",__func__,nRet,nativeString,nSensorType);
		if(nRet >= 0){
			hlpLoadFingersDB(templateLocation);
			return nRet;
		}

		nativeString = "10k";
		nSensorType = CMOSTYPE_HV7131R;
		nRet=hlpOpen(nativeString, nSensorType);
		hlp_printf("%s: nRet %d, try next nSensorType (%s,%d) \n",__func__,nRet,nativeString,nSensorType);
		if(nRet >= 0){
			hlpLoadFingersDB(templateLocation);
			return nRet;
		}
		nativeString = "10k";
		nSensorType = CMOSTYPE_EB6048;
		hlp_printf("%s: nRet %d, try next nSensorType (%s,%d) \n",__func__,nRet,nativeString,nSensorType);
		nRet=hlpOpen(nativeString, nSensorType);
		if(nRet >= 0){
			hlpLoadFingersDB(templateLocation);
			return nRet;
		}
		nativeString = "3k";
		nSensorType = CMOSTYPE_HV7131R;
		//hlp_printf("%s: nRet %d, try next nSensorType (%s,%d) \n",__func__,nRet,nativeString,nSensorType);
		nRet=hlpOpen(nativeString, nSensorType);
		hlp_printf("%s: nRet %d, try next nSensorType (%s,%d) \n",__func__,nRet,nativeString,nSensorType);
		if(nRet >= 0){
			hlpLoadFingersDB(templateLocation);
			return nRet;
		}

		nativeString = "3k";
		nSensorType = CMOSTYPE_EB6048;
		nRet=hlpOpen(nativeString, nSensorType);
		hlp_printf("%s: nRet %d, try next nSensorType (%s,%d) \n",__func__,nRet,nativeString,nSensorType);
		if(nRet >= 0){
			hlpLoadFingersDB(templateLocation);
			return nRet;
		}
		return -1;
	}
	hlpLoadFingersDB(templateLocation);
	(*env)->ReleaseStringUTFChars(env, JtemplateLoc, templateLocation);  // release resources
	//(*env)->ReleaseStringUTFChars(env, sensorLib, nativeString);
	return nRet;
}

JNIEXPORT jint JNICALL Java_com_synel_synergy_synergy2416_presentation_controller_FPU_FP_1CLOSEDEVICE
(JNIEnv *env, jclass jcls){
	hlp_printf("%s: called \n",__func__);
	return (int)hlpClose();
}


JNIEXPORT jint JNICALL Java_com_synel_synergy_synergy2416_presentation_controller_FPU_FP_1ENROLE_1EMPLOYEE
(JNIEnv *env, jclass jcls,jstring badge, jint fingernum, jlong timeOut, jlong gapTime, jobject  enrollmentHandler ){
	//static char fileName[35];
	int nRet = 0;
	const char *nativeBadge =(*env)->GetStringUTFChars(env, badge, 0);
	long badgeL = atol(nativeBadge);
	DWORD ID;
	int step = 1;
	long fingerNumber = (long)fingernum;
	jboolean readerError = JNI_FALSE ;

	jclass enrollObj = (*env)->GetObjectClass(env,enrollmentHandler);
	jmethodID methodoFPR = (*env)->GetMethodID(env,enrollObj, "onFingerPrintRead", "(I)V");
	jmethodID methodsSC = (*env)->GetMethodID(env,enrollObj, "setStepCount", "(I)V");
	(*env)->CallVoidMethod(env,enrollmentHandler, methodsSC, 3);

	jmethodID methodoRFF = (*env)->GetMethodID(env,enrollObj, "onReadyForFinger", "(IZ)V");
	(*env)->CallVoidMethod(env,enrollmentHandler, methodoRFF,step,readerError);

	if(step ==1){
		if (hlpGetEnrollCount() == gRegMax)
		{
			readerError = JNI_TRUE;
			return -108;
		}

		hlpSearchID((long*)&ID);
		ID =(DWORD) badgeL ;

		if ((nRet = hlpEnrollPrepare(badgeL, fingerNumber, 0)) < 0)
		{
			readerError = JNI_TRUE;
			return -109;
		}

		if((nRet = SB_FP_ENROLLSTART()) < 0)
		{
			readerError = JNI_TRUE;
			return -103;
		}

		if (!_capture_finger(timeOut))
		{
			readerError = JNI_TRUE;
			return -107;
		}

		nRet = SB_FP_ENROLLNTH256(step);

		if (nRet < 0)
		{
			readerError = JNI_TRUE;
			return -103;
		}
		else if(nRet > 0)
		{
			readerError = JNI_TRUE;
			return -106;
		}
		else
		{
			(*env)->CallVoidMethod(env,enrollmentHandler, methodoFPR ,step);
		}
	}
	step++;
	usleep(gapTime*1000);
	(*env)->CallVoidMethod(env,enrollmentHandler, methodoRFF,step,readerError);

	while(SB_FP_ISPRESSFINGER() == 0);

	if (!_capture_finger(timeOut))
	{
		readerError = JNI_TRUE;
		return -107;
	}

	nRet = SB_FP_ENROLLNTH256(step);

	if (nRet < 0)
	{
		readerError = JNI_TRUE;
		return -103;
	}
	else if(nRet > 0)
	{
		readerError = JNI_TRUE;
		return -106;
	}

	if(step<3)
	{
		(*env)->CallVoidMethod(env,enrollmentHandler, methodoFPR ,step);
		step++;
		usleep(gapTime*1000);
		(*env)->CallVoidMethod(env,enrollmentHandler, methodoRFF,step,readerError);
	}

	while(SB_FP_ISPRESSFINGER() == 0);

	if (!_capture_finger(timeOut))
	{
		readerError = JNI_TRUE;
		return -107;
	}

	nRet = SB_FP_ENROLLNTH256(step);

	if (nRet < 0)
	{
		readerError = JNI_TRUE;
		return -103;
	}
	else if(nRet > 0)
	{
		readerError = JNI_TRUE;
		return -106;
	}
	else{
		hlpDelete(badgeL,fingernum);
	}


	if ((nRet = hlpEnrollEnd(badgeL, fingerNumber, 0)) < 0)
	{
		readerError = JNI_TRUE;
		return -103;
	}
	return nRet;
}

JNIEXPORT jint JNICALL Java_com_synel_synergy_synergy2416_presentation_controller_FPU_FP_1VALIDATE_1EMPLOYEE
(JNIEnv *env, jclass jcls,jstring badge, jint fingernum, jlong timeOut ,jint isSpecialEnrolled){
	//TODO make it thread safe, adding mutex.
	hlp_printf("%s: called \n",__func__);
	const char *nativeBadge =(*env)->GetStringUTFChars(env, badge, 0);
	long badgeL = atol(nativeBadge);
	int nRet=0;
	DWORD dwTotalTime = 0;
	long fingerNumber = (long)fingernum;

	if(isSpecialEnrolled!=0){

		if (hlpGetEnrollCount() == 0)
		{
			//empty fp template
			return -101;
		}

		if ((nRet = hlpCheckFingerNum(badgeL, fingerNumber)) < 0)
		{
			//bageNum not in fp reader
			return -102;
		}
	}
	//wait for fingerprint to be pressed.
	//while(SB_FP_ISPRESSFINGER() == 0);

	nRet = _validate_finger(timeOut);
	//special enrollment case when no finger pressed
	if(isSpecialEnrolled !=0 && nRet == -8){
		return -999;
	}

	if(nRet !=0){
		return nRet;
	}

	nRet = hlpVerify((long)badgeL, fingerNumber);
	dwTotalTime += (DWORD)_tick_end();
	if (nRet < 0){
		return -104;
	}

	return nRet;
}


JNIEXPORT jint JNICALL Java_com_synel_synergy_synergy2416_presentation_controller_FPU_FP_1DELETE_1TEMPLATE(JNIEnv *env, jclass jcls,jstring jbadge, jint fingerNum ){
	const char *nativeBadge =(*env)->GetStringUTFChars(env, jbadge, 0);
	//static char fileName[35];
	long badgeL = atol(nativeBadge);
	long fingernum = (long)fingerNum;
	long nRet = 0;
	DWORD ID;
       	(*env)->ReleaseStringUTFChars(env, jbadge, nativeBadge);  // release resources 
	hlp_printf("%s: called \n",__func__);
	if (hlpGetEnrollCount() == 0)
	{
		return -101;//NO Templates Loaded
	}
	hlpSearchID((long*)&ID);
	if( fingernum < 0){
		if((nRet = hlpDeleteID(badgeL))< 0){
			return -111;//NO TEMPLATE FOR GIVEN USER
		}
	}
	if ((nRet = hlpDelete(badgeL, fingernum)) < 0)
	{
		return -102;//NO TEMPLATE FOR GIVEN USER
	}
	return 0;

}

JNIEXPORT jint JNICALL Java_com_synel_synergy_synergy2416_presentation_controller_FPU_FP_1DELETE_1TEMPLATE_1ALL(JNIEnv *env, jclass jcls){
	int nRet = 0;
	if ((nRet = hlpDeleteAll()) < 0)
	{
		return -101;//NO TEMPLATE
	}
	return nRet;
}

JNIEXPORT jobjectArray JNICALL Java_com_synel_synergy_synergy2416_presentation_controller_FPU_FP_1GET_1BADGES(JNIEnv *env, jclass jcls){
	int i=0;
	long idsLength = hlpGetEnrollCount();
	DWORD* ids = NULL;
	char* badgeString = malloc(sizeof(DWORD));
	jobjectArray badges = (*env)->NewObjectArray(env, idsLength, (*env)->FindClass(env,"java/lang/String"), (*env)->NewStringUTF(env,""));
	ids = hlpIdIterator();
	if (NULL == ids){
		hlp_printf("error! no badges found!\n");
		return NULL;
	}
	while ( i < idsLength){
		_ultostr(ids[i],badgeString,10);
		(*env)->SetObjectArrayElement(env,badges, i, (*env)->NewStringUTF(env,badgeString));
		i++;
	}
	free(badgeString);
	return badges;
}
JNIEXPORT jint JNICALL Java_com_synel_synergy_synergy2416_presentation_controller_FPU_FP_1GET_1BADGE_1STATUS(JNIEnv *env, jclass jcls,jstring jbadge, jint fingernum ){
	const char *nativeBadge =(*env)->GetStringUTFChars(env, jbadge, 0);
	long badgeL = atol(nativeBadge);
	long fingerNumber = (long)fingernum;
	int nRet = 0;
       	(*env)->ReleaseStringUTFChars(env, jbadge, nativeBadge);  // release resources 
	hlp_printf("%s: badgeL is %ld and fingerNumber is %ld \n",__func__,badgeL,fingerNumber);
	if ((nRet = (int)hlpCheckFingerNum(badgeL, fingerNumber)) < 0){
		return -102;
	}
	return 0;
}

JNIEXPORT jint JNICALL Java_com_synel_synergy_synergy2416_presentation_controller_FPU_FP_1SET_1TEMPLATE(JNIEnv *env, jclass jcls,jstring jbadge, jint fingernum, jstring jtemplate ){
	//static char fileName[35];
	const char *nativeBadge =(*env)->GetStringUTFChars(env, jbadge, 0);
	const char *nativeTemplate = (*env)->GetStringUTFChars(env, jtemplate, 0);
	long badgeL = atol(nativeBadge);
	long fingerNum = (long)fingernum;
	unsigned char* templateDecoded=(unsigned char *)base64_decode(nativeTemplate);
	(*env)->ReleaseStringUTFChars(env, jbadge, nativeBadge);  // release resources
	if(hlpSetFpDataOne(badgeL,fingerNum,templateDecoded)){
		(*env)->ReleaseStringUTFChars(env, jtemplate, nativeTemplate);  // release resources
		return -100;
	}
	else {
		(*env)->ReleaseStringUTFChars(env, jtemplate, nativeTemplate);  // release resources
		return 0;
	}
}

JNIEXPORT jstring JNICALL Java_com_synel_synergy_synergy2416_presentation_controller_FPU_FP_1GET_1TEMPLATE(JNIEnv *env, jclass jcls,jstring jbadge, jint fingernum ){
	const char *nativeBadge =(*env)->GetStringUTFChars(env, jbadge, 0);
	long badgeL = atol(nativeBadge);
	long fingerNum = (long)fingernum;
	unsigned char* template = (unsigned char*)calloc(FPDATASIZE,sizeof(char));
	(*env)->ReleaseStringUTFChars(env, jbadge, nativeBadge);  // release resources
	if(hlpGetFpDataOne(badgeL, fingerNum, template)){
		char* templateEncoded=(char *)base64_encode((char*)template,FPDATASIZE);
		free(template);
		jstring jTemplate = (*env)->NewStringUTF(env, templateEncoded);
		free(templateEncoded);
		return jTemplate;
	}
	return (*env)->NewStringUTF(env, NULL);
}
JNIEXPORT jint JNICALL Java_com_synel_synergy_synergy2416_presentation_controller_FPU_FP_1GET_1ENROLECOUNT(JNIEnv *env, jclass jcls){
	return (int)hlpGetEnrollCount();
}

JNIEXPORT jint JNICALL Java_com_synel_synergy_synergy2416_presentation_controller_FPU_FP_1IDENTIFY_1EMPLOYEE(JNIEnv *env,jclass jcls){
	int nRet;
	DWORD ID, FingerNum;

	if (hlpGetEnrollCount() == 0)
	{
		return -101;
	}
	nRet = SB_FP_CAPTUREFINGER();

	if(nRet == 0)
	{
		nRet = hlpIdentify((long*)&ID, (long*)&FingerNum);
		return (long) nRet;
	}

	if (nRet < 0)
		return -107;
	else
		return nRet;
}

