/*
 * supremainc.h
 *
 *  Created on: 2014-5-5
 *      Author: aduo
 */

#ifndef SUPREMAINC_H_
#define SUPREMAINC_H_

#include "finger.h"

#define START_CODE	0
#define COMMAND		1
#define PARAM		2
#define SIZE		6
#define FLAG		10
#define CHECK_SUM	11
#define END_CODE	12

#define CMOSTYPE_OP5 0x33

typedef struct _packet232{
	unsigned char startcode;
	unsigned char command;
	unsigned int param;
	unsigned int size;
	unsigned char flag;
	unsigned char checksum;
	unsigned char endcode;
}packet232;

typedef struct _packet485{
	unsigned char startcode;
	unsigned short terminalid;
	unsigned char command;
	unsigned int param;
	unsigned int size;
	unsigned char flag;
	unsigned char checksum;
	unsigned char endcode;
}packet485;

enum image_format{Gray,Binary,FourBitGray};

#define BUFFER_LENGTH 200*1024
typedef struct _image {
	int width;  //Width of fingerprint images (4byte)
	int height; //Height of fingerprint images (4byte)
	int compressed; //Compression status (4byte)
	int encrypted; //Encryption status (4byte)
	int binary; //Image Format : 0 ¨C gray, 1 ¨C binary, 2 ¨C 4bit gray (4byte)
	int img_len; //Size of the fingerprint image received from the  sensor = width * height (4byte)
	int template_len; //Size of the fingerprint template (4byte)
	char buffer[BUFFER_LENGTH]; //Actual raw image data
} image_t;

/*========================================================================
                    Error Code
==========================================================================*/
#define SUCCESS			0x61
#define SCAN_SUCCESS	0x62
#define SCAN_FAIL		0x63
#define NOT_FOUND		0x69
#define NOT_MATCH		0x6A
#define TRY_AGAIN		0x6B
#define TIME_OUT		0x6C
#define MEM_FULL		0x6D
#define EXIST_ID		0x6E
#define FINGER_LIMIT	0x72
#define CONTINUE		0x74
#define INVALID_ID		0x76
#define TIMEOUT_MATCH	0x7A
#define BUSY			0x80
#define EXIST_FINGER	0x86
#define REJECTED_ID		0x90
#define DURESS_FINGER	0x91
#define ENTRANCE_LIMIT	0x94
#define FAKE_DETECTED	0xB0

int sfm_InitFp(int port,int baud_rate);
long sfm_FpDataOneToNMatch(char *FileName);
int sfm_LoadFpData(char *nID, int FingerNum, char *FileName);
int sfm_Enroll(char * nID, int FingerNum, char *tpath, char *dpath);
long sfm_OneToNMatch(char *tpath);
long sfm_OneToOneMatch(char *nID,char *tpath);
int sfm_DeleteFpOne(char *nID, int FingerNum);
int sfm_DeleteFpAll(void);
int sfm_get_enroll_count();
int sfm_UninitFp(void);

#endif /* SUPREMAINC_H_ */
