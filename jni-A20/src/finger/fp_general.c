#include "public.h"
#include "fp_general.h"
#include "debug.h"

char *finger_path;


/** 
 * @chinese
 * 存储录入指纹模板
 * 
 * @param nID 人员ID
 * @param nFingerNum 人员手指号
 * @param gFeature 
 * 
 * @return 成功:0,失败:-1
 * @endchinese
 *
 * @english
 * saving finger templates of enrolling 
 * 
 * @param nID identify number of a person
 * @param nFingerNum finger number of ap person
 * @param gFeature 
 * 
 * @return success:0,fail:-1
 * @endenglish
 *
 */

int _save_template(int nID, int nFingerNum,void *gFeature)
{
	FILE * vFile;
	int vSize;
	char szFileName[256];

    memset(szFileName,0,sizeof(szFileName));
	sprintf(szFileName, "%s%d_%d.s10", finger_path,nID, (int)nFingerNum);
    creatdir(szFileName);
	vFile = fopen( szFileName, "wb" );
	if( vFile == NULL )
		return -1;

	vSize = fwrite( &gFeature, FPDATASIZE, 1,  vFile );
	fclose( vFile );
	if( vSize != 1 )
		return -1;
    return 0;
}

