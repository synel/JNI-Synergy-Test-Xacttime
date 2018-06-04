#include "sb_type.h"
#include "sfe.h"
#include "fp_general.h"
#include "usb_finger.h"
//#include "arm9_finger.h"

static	FPINFO gFeature;
BYTE	gImg256[256 * 256];
static	SFE_OPER_FUNC_TYPE sfp = NULL;

/**
 *
 * ��ʼ��ָ��
 * @param nSensorType ָ���豸�ͺ�
 * @param fp_path ָ���豸·��
 *
 * @return �ɹ�-TRUE,ʧ��-��������
 */
int init_finger_usb(int nSensorType,char *fp_path)
{
    int i,nRet;
    void* g_hSFE = NULL;

	g_hSFE = dlopen(fp_path, RTLD_NOW);
	if (g_hSFE == NULL) {
		plog("Can't load library.\n");
		goto l_exit;
	}

	sfp = (SFE_OPER_FUNC_TYPE)dlsym(g_hSFE, "fp");
	if (sfp == NULL) {
		plog("Can't find the fp in the library.\n");
		goto l_exit;
	}
    for(i=0;i<3;i++){
        nRet = sfp(FP_OPEN, nSensorType, 0, 0);
        if (nRet == 0) {
            plog("Can't open the SBUM device.\n");
            return TRUE;
        }
    }
l_exit:
   return FALSE;
}


int CaptureGoodFinger()
{
	int nRet = 0;
	int sMax = 0;
	int sFpCapArea = 0;

	while (1) {
/*
		if (getch() == 'q') {
			plog("\tProcess stopped.\n");
			nRet = -1;
			break;
		}
*/
		if (sfp(FP_SEN_CAPTURE, 0, 0, 0) != 0) {
			plog("\tCapture fail.\n");
			nRet = -2;
			break;
		}

		sFpCapArea = sfp(FP_SEN_ISFINGER, 0, 0, 0);
		if (sFpCapArea < 0) continue;

		if (sfp(FP_SEN_GETIMG, (int)gImg256, 0, 0) != 0) {
			plog("\tGet image fail. Please check USB Scanner!\n");
			nRet = -3;
			break;
		}
		//DispImg();

		if (sFpCapArea < sMax + 2) break;
		if (sFpCapArea > sMax) sMax = sFpCapArea;
		if (sFpCapArea > 45) break;

	}

	if (nRet < 0) {
		memset(gImg256, 0, sizeof(gImg256));
	}
    return nRet;
}

/**
 *
 *
 * @param nID ��Ա����
 * @param FingerNum  ָ�ƶ�Ӧ��ָ��
 * @param tpath ָ��ͼ���洢·��
 * @param dpath ָ��ģ���洢·��
 * @param formBeforScan �ص�����,������ǰ¼���Ĵ���������������Ʋ�����ʾ����
 * @param formAfterScan �ص�����,������ǰ¼���Ĵ������������������ʾָ��ͼ��
 *
 * @return �ɹ�-TRUE��ʧ��-��������
 */
int enroll_finger_usb( char * nID ,int FingerNum,char *tpath,char *dpath,
				performBeforScan formBeforScan,performAfterScan formAfterScan )
{
	int i;
	int nRet;

	plog("\nEnroll\n");
	nRet = sfp(FP_ENROLLSTART, 0, 0, 0);
	if (nRet != 0) {
		plog("\tStarting enrollment fails.\n");
		return FALSE;
	}

  	for (i = 1; i < 4; i++) {
		plog("\tPlace the finger %dth, please! (Press 'q' to stop)\n", i);

		nRet = CaptureGoodFinger();
		if (nRet < 0) {
			break;
		}
//        ImageWrite2BmpFile("/dev/shm/zw.bmp",256,256,gImg256,8);
		plog("\tPlease take off the finger.\n");
		if (sfp(FP_SEN_GETFEATURE, (long)&gFeature, 0, 0) != 0) {
			nRet = -1;
			plog("\tCan't extract feature.\n");
			break;
		}

		nRet = sfp(FP_ENROLLNTHFPDATA, (long)&gFeature, (long)i, 0);

        if (nRet > 0) {
			plog("\tIs Already enrolled: Pos = %d\n", nRet - 1);
			nRet = -1;
			break;
		}
		else if (nRet < 0) {
			plog("\tError code = %d occured\n", nRet);
			nRet = -1;
			break;
		}
		else {
			plog("\t%dth success!\n", i);
		}

		nRet = 0;
		if (i != 3) {
			plog("\tPlease take off the finger.\n");
		    while (1) {
				if (sfp(FP_SEN_CAPTURE, 0, 0, 0) != 0) {
					plog("\tCapture fail. Please check USB Scanner!\n");
					nRet = -1;
					break;
				}
				if (sfp(FP_SEN_ISFINGER, 0, 0, 0) < 0)  break;
      		}
			if (nRet < 0) {
				break;
			}
		}
	}

	if (nRet == 0) {
		nRet = sfp(FP_ENROLLMERGE, (long)&gFeature, 0, 0);
		if (nRet < 0) {
			plog("\tFp Merge Error!\n");
		}
		else {
			gFeature.ID = (DWORD)nID*10+FingerNum;
			gFeature.FingerNum = (BYTE)FingerNum;
			if ((nRet = sfp(FP_GETEMPTYPOS, 0, 0, 0)) >= 0) {
				sfp(FP_SETFPDATA, (long)&gFeature, nRet, 0);
                //_save_template(nID, FingerNum);
			}
			else {
				plog("\tDatabase is full.\n");
			}
		}
	}
	return TRUE;
}

