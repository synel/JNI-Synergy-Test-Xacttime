#ifndef _SFE_H_
#define _SFE_H_

#define SBFP_SUCCESS				0
#define KIT_REG_MAX				    10000
#define FEATURE_SIZE		        1404
#define	SENSOR_OV7648_16		0
#define	SENSOR_OV7648_20		1
#define	SENSOR_PAS6311			2
#define	SENSOR_HV7131R			3
#define	SENSOR_EB6048			4	//support in SB1001U
#define	SENSOR_GC0307			5	//support in SB1001U

//function code
#define FP_OPEN								1
#define FP_CLOSE							2
#define	FP_GETENROLLCOUNT					5
#define	FP_GETEMPTYPOS						6
#define	FP_ISENROLLEDPOS					7
#define FP_SETFPDATA						11
#define FP_GETFPDATA						12
#define FP_FPCHECK256						14
#define FP_PROCESS256						22
#define FP_ENROLLSTART					    31
#define FP_ENROLLNTH256						33
#define FP_ENROLLNTHFPDATA					34
#define FP_ENROLLMERGE						35
#define FP_IDENTIFYFPDATA					43
#define FP_VERIFYFPDATA						44
#define FP_IDENTIFYIMAGE256					45
#define FP_VERIFYIMAGE256					46
#define FP_DELETE							51
#define FP_DELETEALL						52

#define	FP_SEN_ADJUST						60
#define	FP_SEN_CAPTURE						61
#define	FP_SEN_ISFINGER						62
#define	FP_SEN_GETIMG						63
#define	FP_SEN_GETFEATURE					64

#define	FP_GETLIBVER						100

//error code
#define IMAGE_ERR							-1
#define FPDATA_ERR							-2
#define ID_ERR								-3
#define OVER_ERR							-4
#define BUFFER_ERR							-6
#define SENSOR_ERR							-7
#define NTH_ERR								-8
#define MERGE_ERR							-9
#define NOT_FINGER							-11

#define DEV_ERR								-100

typedef struct tagPC_ATTR2 {			//1404byte
	unsigned int	ID;					// ID
	unsigned char   Valid;				// Valid
	unsigned char   Manager;			// Manager
	unsigned char   FingerNum;			// Number of finger
	unsigned char   Reserved[1397];
} FPINFO, *P_FPINFO;                      

typedef	int  (*SFE_OPER_FUNC_TYPE)(int FuncNo, long Param1, long Param2, long Param3);

#endif
