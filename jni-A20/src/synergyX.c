/*
 * synergyX.c
 *
 *  Created on: May 14, 2018
 *      Author: chaol
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdint.h>
#include "_fp.h"
#include "arm9_finger.h"
#include "SynergyUtility.h"
#include "base64.h"
#include <dirent.h>
#include "com_synel_synergy_synergyX_presentation_controller_FPU.h"
/*
 * nPos is the array index in the fp memory
 * badgenum is the symbol in the fp data structure(FPINFO).
 */
#define DLOGI(...) fprintf(stderr, __VA_ARGS__);
#define APP_REPEAT		20
#define APP_DEF_FP_NUM	0
#define APP_CAP_TIMEOUT 500
#define JNI_FALSE  0
#define JNI_TRUE   1

struct timeval tv1,tv2;
static char templateLocation[128];
int fd ;
static char FP_LIB_PATH[128];
extern int gRegMax;
extern int fp_enable;
JavaVM* g_jvm = 0;
static jobject g_enrollHandler;
static jmethodID g_methodoFPR;// = (*env)->GetMethodID(env,enrollObj, "onFingerPrintRead", "(I)V");
static jmethodID g_methodsSC;// = (*env)->GetMethodID(env,enrollObj, "setStepCount", "(I)V");
static jmethodID g_methodoRFF; // = (*env)->GetMethodID(env,enrollObj, "onReadyForFinger", "(IZ)V");
//to debug jni code, call java function with: java -Xcheck:jni -jar MyApp.jar
//ref: https://www.ibm.com/developerworks/library/j-jni/index.html#listing2

int onReadyForFingerPrint(int step) {
	DLOGI("Please Place finger step %d\n",step);
	JNIEnv* env;
	if ((*g_jvm)->GetEnv(g_jvm, (void**) &env, JNI_VERSION_1_8) != JNI_OK) {
		DLOGI("JNIEnv find Err!\n");
		return -1;
	}
	(*env)->CallVoidMethod(env,g_enrollHandler, g_methodoRFF,step,JNI_FALSE);
	return 0;
}

int onFingerPrintRead(int step) {
	DLOGI("Please Remove finger step %d\n",step);
	JNIEnv* env;
	if ((*g_jvm)->GetEnv(g_jvm, (void**) &env, JNI_VERSION_1_8) != JNI_OK) {
		DLOGI("JNIEnv find Err!\n");
		return -1;
	}
	(*env)->CallVoidMethod(env,g_enrollHandler, g_methodoFPR,step,JNI_FALSE);
	return 0;
}

performBeforScan *formBeforScan = &onReadyForFingerPrint;
performAfterScan *formAfterScan = &onFingerPrintRead;

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JNIEnv* env;
	g_jvm = vm;

	if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_8) != JNI_OK) {
		DLOGI("JNI Onload Err!\n");
		return -1;
	}
	DLOGI("JNI_VERSION: %d\n",JNI_VERSION_1_8);
	strcpy(FP_LIB_PATH, "/usr/lib/fpu/fp.so");
	_initGpio();
	return JNI_VERSION_1_8;
}

// According to http://docs.oracle.com/javase/1.5.0/docs/guide/jni/spec/invocation.html#JNI_OnUnload
// The VM calls JNI_OnUnload when the class loader containing the native library is garbage collected.
void JNI_OnUnload(JavaVM *vm, void *reserved)
{
	JNIEnv* env;
	DLOGI("JNI unload function called..\n");

	if ((*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_8) != JNI_OK) {
		// Something is wrong but nothing we can do about this :(
		return;
	}
	(*env)->DeleteGlobalRef(env, g_enrollHandler);
	//release all Global References, if any.
}

JNIEXPORT void JNICALL Java_com_synel_synergy_synergyX_presentation_controller_FPU_REDON(JNIEnv * env, jclass class){
	DLOGI("RED LED On\n");
	//set_gpio_value(pin, value);
	set_gpio_value(1,1);
}

JNIEXPORT void JNICALL Java_com_synel_synergy_synergyX_presentation_controller_FPU_REDOFF(JNIEnv * env, jclass class){
	DLOGI("RED LED Off\n");
	set_gpio_value(1,0);
}

JNIEXPORT void JNICALL Java_com_synel_synergy_synergyX_presentation_controller_FPU_GREENON(JNIEnv * env, jclass class){
	DLOGI("GREEN LED On\n");
	set_gpio_value(0,1);

}

JNIEXPORT void JNICALL Java_com_synel_synergy_synergyX_presentation_controller_FPU_GREENOFF(JNIEnv * env, jclass class){
	DLOGI("GREEN LED Off\n");
	set_gpio_value(0,0);
}

JNIEXPORT jint JNICALL Java_com_synel_synergy_synergyX_presentation_controller_FPU_FP_1OPENDEVICE
(JNIEnv *env, jclass jcls, jstring JtemplateLoc){
	DLOGI("%s: called \n",__func__);
	int nSensorType = 0x0;//0-3 is for smack bio etc. 0x33 is for suprema reader
	const char *tmpLocation =(*env)->GetStringUTFChars(env, JtemplateLoc, 0);
	if (tmpLocation == NULL || strcmp(tmpLocation, "DEBUG") == 0) {
		DLOGI("debug mode...");
		tmpLocation = (const char*)("/root/");
	} else {
		if (tmpLocation != NULL) {
			DLOGI("template location is: %s\n", tmpLocation);
		}
	}
	//save it to global:
	strncpy(templateLocation,tmpLocation,sizeof(tmpLocation));
	DLOGI("template location is: %s\n", templateLocation);
	int ret;
	do {
		DLOGI("now try sensor type: %d\n",nSensorType);
		ret = InitFp(nSensorType, FP_LIB_PATH);
		DLOGI("InitFp returned value %d\n", ret);
		if (ret != 1) {
			DLOGI("Initializing FPU Failed. Sleep 1 Sec and retry\n");
			nSensorType ++;
			sleep(1);
		}
	} while ( ret != 1);
	//this is to load all fingers to memory, one can also choose load it on demand for finer finger management
	int retcode = LoadAllFinger(templateLocation);
	DLOGI("loadAllfinger: %d",retcode);
	return ret;
}

JNIEXPORT jint JNICALL Java_com_synel_synergy_synergyX_presentation_controller_FPU_FP_1CLOSEDEVICE
(JNIEnv *env, jclass jcls){
	DLOGI("%s: called \n",__func__);
	return (int)hlpClose();
}


JNIEXPORT jint JNICALL Java_com_synel_synergy_synergyX_presentation_controller_FPU_FP_1ENROLE_1EMPLOYEE
(JNIEnv *env, jclass jcls,jstring badge, jint fingernum, jlong timeOut, jlong gapTime, jobject  enrollmentHandler ){
	//static char fileName[35];
	int nRet = 0;
	const char *nativeBadge =(*env)->GetStringUTFChars(env, badge, 0);

	long badgeL = atol(nativeBadge);
	DWORD ID;
	int step = 1;
	int fingerNumber = fingernum;
	jboolean readerError = JNI_FALSE ;

	jclass enrollObj = (*env)->GetObjectClass(env,enrollmentHandler);
	if((*env)->ExceptionOccurred(env)) {
		DLOGI("exception at %d\n",__LINE__);
		return JNI_ERR;
	}
	if (enrollObj == NULL) {
		DLOGI("exception at %d\n",__LINE__);
		return JNI_ERR;
	} else{
		g_enrollHandler = (jobject)(*env)->NewGlobalRef(env,enrollmentHandler);
	}
	//cache method IDs...
	g_methodoFPR = (*env)->GetMethodID(env,enrollObj, "onFingerPrintRead", "(I)V");
	if((*env)->ExceptionOccurred(env)) {
		DLOGI("exception at %d\n",__LINE__);
		return;
	}
	g_methodsSC = (*env)->GetMethodID(env,enrollObj, "setStepCount", "(I)V");
	if((*env)->ExceptionOccurred(env)) {
		DLOGI("exception at %d\n",__LINE__);
		return;
	}
	g_methodoRFF = (*env)->GetMethodID(env,enrollObj, "onReadyForFinger", "(IZ)V");
	if((*env)->ExceptionOccurred(env)) {
		DLOGI("exception at %d\n",__LINE__);
		return;
	}
	(*env)->CallVoidMethod(env,g_enrollHandler, g_methodsSC, 3);
	//

	DLOGI("at %d\n",__LINE__);
	int retcode = -1;
	retcode = Enroll( nativeBadge , fingerNumber,"/tmp/zw.bmp",templateLocation,
			formBeforScan,  formAfterScan);
	if((*env)->ExceptionOccurred(env)) {
		DLOGI("exception at %d\n",__LINE__);
		return -999;
	}
	DLOGI("at line: %d\n",__LINE__);
	DLOGI("return value: %d\n", retcode);

	return retcode;
}

JNIEXPORT jint JNICALL Java_com_synel_synergy_synergyX_presentation_controller_FPU_FP_1VALIDATE_1EMPLOYEE
(JNIEnv *env, jclass jcls,jstring badge, jint fingernum, jlong timeOut ,jint isSpecialEnrolled){
	DLOGI("%s: called \n",__func__);
	const char *nativeBadge =(*env)->GetStringUTFChars(env, badge, 0);
	long badgeL = atol(nativeBadge);
	int nRet=0;
	char nID[64];
	int ret;
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
	sprintf(templateLocation, "s_fp1n_%d.bmp", (int)time(NULL));
	sprintf(nID,  "%ld",badgeL);
	do {
		ret = OneToOneMatch(nID, fingernum, templateLocation);
	} while (ret == -8);

	if(ret == 1)
	{
		printf("OneToOneMatch : OK, found right finger print\n");
		ret = 0; //zero mean return success
	} else {
		DLOGI("OneToOneMatch: err code = %d\n", ret);
		if(ret == -103)
		{
			DLOGI("there is no such finger print\n");
		}
	}
	//	nRet = _validate_finger(timeOut);
	//	//special enrollment case when no finger pressed
	//	if(isSpecialEnrolled !=0 && nRet == -8){
	//		return -999;
	//	}
	//
	//	if(nRet !=0){
	//		return nRet;
	//	}
	//
	//	nRet = hlpVerify((long)badgeL, fingerNumber);
	//	dwTotalTime += (DWORD)_tick_end();
	//	if (nRet < 0){
	//		return -104;
	//	}

	return nRet;
}


JNIEXPORT jint JNICALL Java_com_synel_synergy_synergyX_presentation_controller_FPU_FP_1DELETE_1TEMPLATE(JNIEnv *env, jclass jcls,jstring jbadge, jint fingerNum ){
	const char *nativeBadge =(*env)->GetStringUTFChars(env, jbadge, 0);
	//static char fileName[35];
	long badgeL = atol(nativeBadge);
	long fingernum = (long)fingerNum;
	long nRet = 0;
	char nID[64];
	DWORD ID;
	(*env)->ReleaseStringUTFChars(env, jbadge, nativeBadge);  // release resources
	DLOGI("%s: called \n",__func__);
	if (hlpGetEnrollCount() == 0)
	{
		return -101;//NO Templates Loaded
	}
	sprintf(nID,"%ld",badgeL);

	if (templateLocation != NULL) {
		return hlpDeleteFlash(nID, fingernum, templateLocation);
	}else {
		return hlpDelete(nID, fingernum);
	}


	//	hlpSearchID((long*)&ID);
	//	if( fingernum < 0){
	//		if((nRet = hlpDeleteID(badgeL))< 0){
	//			return -111;//NO TEMPLATE FOR GIVEN USER
	//		}
	//	}
	//	if ((nRet = hlpDelete(badgeL, fingernum)) < 0)
	//	{
	//		return -102;//NO TEMPLATE FOR GIVEN USER
	//	}
	//	return 0;

}

JNIEXPORT jint JNICALL Java_com_synel_synergy_synergyX_presentation_controller_FPU_FP_1DELETE_1TEMPLATE_1ALL(JNIEnv *env, jclass jcls){
	int nRet = 0;
	if ((nRet = hlpDeleteAll()) < 0)
	{
		return -101;//NO TEMPLATE
	}
	return nRet;
}

JNIEXPORT jobjectArray JNICALL Java_com_synel_synergy_synergyX_presentation_controller_FPU_FP_1GET_1BADGES(JNIEnv *env, jclass jcls){
	int i=0;
	long idsLength = hlpGetEnrollCount();
	DWORD* ids = NULL;
	char* badgeString = malloc(sizeof(DWORD));
	jobjectArray badges = (*env)->NewObjectArray(env, idsLength, (*env)->FindClass(env,"java/lang/String"), (*env)->NewStringUTF(env,""));
	ids = hlpIdIterator();
	if (NULL == ids){
		DLOGI("error! no badges found!\n");
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
JNIEXPORT jint JNICALL Java_com_synel_synergy_synergyX_presentation_controller_FPU_FP_1GET_1BADGE_1STATUS(JNIEnv *env, jclass jcls,jstring jbadge, jint fingernum ){
	const char *nativeBadge =(*env)->GetStringUTFChars(env, jbadge, 0);
	long badgeL = atol(nativeBadge);
	long fingerNumber = (long)fingernum;
	int nRet = 0;
	(*env)->ReleaseStringUTFChars(env, jbadge, nativeBadge);  // release resources
	DLOGI("%s: badgeL is %ld and fingerNumber is %ld \n",__func__,badgeL,fingerNumber);
	if ((nRet = (int)hlpCheckFingerNum(badgeL, fingerNumber)) < 0){
		return -102;
	}
	return 0;
}

JNIEXPORT jint JNICALL Java_com_synel_synergy_synergyX_presentation_controller_FPU_FP_1SET_1TEMPLATE(JNIEnv *env, jclass jcls,jstring jbadge, jint fingernum, jstring jtemplate ){
	//TODO: to be tested
	const char *nativeBadge =(*env)->GetStringUTFChars(env, jbadge, 0);
	const char *nativeTemplate = (*env)->GetStringUTFChars(env, jtemplate, 0);
	long badgeL = atol(nativeBadge);
	long fingerNum = (long)fingernum;
	unsigned char* templateDecoded=(unsigned char *)base64_decode(nativeTemplate);
	(*env)->ReleaseStringUTFChars(env, jbadge, nativeBadge);  // release resources
	if(SetFpDataOne(badgeL,fingerNum,templateDecoded)){
		(*env)->ReleaseStringUTFChars(env, jtemplate, nativeTemplate);  // release resources
		return -100;
	}
	else {
		(*env)->ReleaseStringUTFChars(env, jtemplate, nativeTemplate);  // release resources
		return 0;
	}
}

JNIEXPORT jstring JNICALL Java_com_synel_synergy_synergyX_presentation_controller_FPU_FP_1GET_1TEMPLATE(JNIEnv *env, jclass jcls,jstring jbadge, jint fingernum ){
	const char *nativeBadge =(*env)->GetStringUTFChars(env, jbadge, 0);
	long fingerNum = (long)fingernum;
	unsigned char* template; //;= (unsigned char*)calloc(FPDATASIZE,sizeof(char));
	template = GetFpDataOne(nativeBadge, fingernum);
	(*env)->ReleaseStringUTFChars(env, jbadge, nativeBadge);  // release resources
	if (template != NULL){
		DLOGI("have template %s\n",template);
		char* templateEncoded=(char *)base64_encode((char*)template,FPDATASIZE);
		//free(template);
		jstring jTemplate = (*env)->NewStringUTF(env, templateEncoded);
		free(templateEncoded);
		return jTemplate;
	}
	return (*env)->NewStringUTF(env, NULL);
}
JNIEXPORT jint JNICALL Java_com_synel_synergy_synergyX_presentation_controller_FPU_FP_1GET_1ENROLECOUNT(JNIEnv *env, jclass jcls){
	return (int)hlpGetEnrollCount();
}

JNIEXPORT jint JNICALL Java_com_synel_synergy_synergyX_presentation_controller_FPU_FP_1IDENTIFY_1EMPLOYEE(JNIEnv *env,jclass jcls){
	int nRet;
	DWORD ID, FingerNum;

	if (hlpGetEnrollCount() == 0)
	{
		return -101;
	}
	nRet = SB_FP_CAPTUREFINGER();

	if(nRet == 0)
	{
		nRet = OneToNMatch("/tmp/sw.bmp");
		//nRet = hlpIdentify((long*)&ID, (long*)&FingerNum);
		return (long) nRet;
	}
	DLOGI("%d: nRet is %d\n",__LINE__,nRet);

	if (nRet < 0)
		return -107;
	else
		return nRet;
}

