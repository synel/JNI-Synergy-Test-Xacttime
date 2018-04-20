/*
 * tfingerprint.c
 *
 *  Created on: 2014-6-13
 *      Author: aduo
 */





//////////////////////////////////////////////////////////////////////////
int _input_number(char* szPrompt, int dwDefault)
{
	int	vKey, vCnt = 0;
	char vStr[16] = {0}, *vStr2;
	int vRet = dwDefault;

	printf("Please input %s [default = %u] ", szPrompt, (unsigned int)dwDefault);

	while(1)
	{
		vKey = getchar();
		if ( vKey >= '0' && vKey <= '9')
		{
			vStr[vCnt] = (char)vKey;
			vCnt++;
			vStr[vCnt] = 0;

			vRet = (int)strtoul(vStr, &vStr2, 10);

			if(vCnt > 10)
				break;
		}
		else if ( vKey == '\n' )
		{
			if (vCnt == 0)
				vRet = dwDefault;
			goto RET;
		}
	}

	while(1)
	{
		vKey = getchar();
		if ( vKey == 0x0a )
			break;
	}

RET:
	return vRet;
}

int main() {

	int ret = 0;
	int nContinue = 1,vNum=0,fingernum=0;
	char nID[128];
	char uid[16];
	int port = 3;
    int baud_rate = 115200;
    int n = 0;
    int i = 0;
    char file_name[128],template[384];
    FILE *file;
//    pit api
//    InitFp(sensor_type, '/weds/arm/lib/fp.so')
	open_port(port,baud_rate,8,"1",'N');

//	ret = get_module_baud();
//	if (ret == 1){
//	printf("get_module_baud success\r\n");
//	}else{
//		printf("get_module_baud fail\r\n");
//	}

	//set_module_baud(19200);

	/*
	 * //			ret = list_user_id();
//			if (ret == 1) {
//				printf("list_user_id success\r\n");
//			} else {
//				printf("list_user_id fail\r\n");
//			}
		ret = get_module_sensor_type();
		if (ret == 1){
		printf("get_module_sensor_type success\r\n");
		}else{
			printf("get_module_sensor_type fail\r\n");
		}

		ret = get_module_image_quality();
		if (ret == 1){
		printf("get_module_image_quality success\r\n");
		}else{
			printf("get_module_image_quality fail\r\n");
		}

	*/

//	ret = get_module_image_format(&g_image_format);
//	if (ret == 1){
//	  switch(g_image_format){
//	  case 0x30:
//		  printf("Gray image\r\n");
//		  break;
//	  case 0x31:
//		  printf("Binary image\r\n");
//		  break;
//	  case 0x32:
//		  printf("4 bit gray image\r\n");
//		  break;
//	  }
//	  printf("get_module_image_format success\r\n");
//	}else{
//	  printf("get_module_image_format fail\r\n");
//	}

//	ret = set_module_image_format(Binary);
//	if (ret == 1){
//		printf("set_module_image_format success\r\n");
//	}else{
//		printf("set_module_image_format fail\r\n");
//	}
//
//	ret = get_module_available_finger(&gRegMax);
//	if (ret == 1){
//	  printf("gRegMax = %d\r\n",gRegMax);
//	  printf("get_module_available_finger success\r\n");
//	}else{
//	  printf("get_module_available_finger fail\r\n");
//	}
//
//	ret = get_module_template_size(&g_template_size);
//	if (ret == 1){
//	  printf("g_template_size = %d\r\n",g_template_size);
//	  printf("get_module_template_size success\r\n");
//	}else{
//	  printf("get_module_template_size fail\r\n");
//	}
//
//	ret = get_module_enroll_mode(&g_enroll_mode);
//	if (ret == 1){
//	  printf("g_enroll_mode = %d,0x%02X\r\n",g_enroll_mode,g_enroll_mode);
//	  printf("get_module_enroll_mode success\r\n");
//	}else{
//		printf("get_module_enroll_mode fail\r\n");
//	}
//
//	ret = get_module_send_scan_success(&g_send_scan_success);
//	if (ret == 1){
//	  printf("g_send_scan_success = %d,0x%02X\r\n",g_send_scan_success,g_send_scan_success);
//	  printf("get_module_send_scan_success success\r\n");
//	}else{
//		printf("get_module_send_scan_success fail\r\n");
//	}

   	while( nContinue )
	{
   		printf( "\n" );
   		printf( "-------Menu-------\n" );
   		printf( "0  : Exit\n" );
   	    printf( "1  : Enroll\n" );
   		printf( "2  : 1:N Matching\n" );
   		printf( "3  : 1:1 Matching\n" );
   		printf( "4  : load fingerprint data\n" );
   		printf( "5  : Delete finger one \n" );
   		printf( "6  : Delete one All\n" );
   		printf( "7  : Delete All\n" );
   		printf( "8  : Get Enroll Count\n" );
   		printf( "------------------\n" );

		memset(nID, 0, sizeof(nID));
		printf("\n\n");
		vNum = (int) _input_number("menu item", 100);

		switch (vNum) {
		case 0: //Exit
			nContinue = 0;
			printf("Exit OK.\n");
			break;
		case 1: //Enroll
			printf("===== Enroll\n");
//			pit api
//			Enroll(nID,FingerNum,tpath,dpath,FingerSoundHandle, FingerInfoHandle)
			sprintf(nID, "%d", _input_number("Input staff No.:", 123));
			fingernum = _input_number("Input Finger No.:", 0);

			if (fingernum < 0 || fingernum > 9){
				printf("param error\r\n");
				break;
			}

			memset(uid,0,sizeof(uid));
			sprintf(uid,"%d",atoi(nID) * 10 + fingernum);

			ret = _enroll_by_scan(atoi(uid));
			if (ret == 1) {
				printf("Enroll success\r\n");
			} else {
				printf("Enroll fail\r\n");
			}

			ret = _read_image();
			if (ret == 1) {
				printf("read fingerprint image success\r\n");

				memset(file_name,0,sizeof(file_name));
				strcpy(file_name,"./zw.bmp");
				ret = save_image_to_file(file_name);
				if (ret == 1) {
					printf("save image to file success\n");
				} else {
					printf("save image to file fail\n");
				}
			} else {
				printf("read fingerprint image fail\r\n");
			}

//			ret = _read_template(atoi(uid));
			ret = _read_template(0); //latest fingerprint template created
			if (ret == 1) {
				printf("read fingerprint template success\r\n");

				memset(file_name,0,sizeof(file_name));
				sprintf(file_name,"./%s_%d.s10",nID,fingernum);
				ret = save_template_to_file(file_name);
				if (ret == 1){
					printf("save template to file success\n");
				}else{
					printf("save template to file fail\n");
				}


			} else {
				printf("read fingerprint template fail\r\n");
			}

			break;
		case 2: //1:N Matching
			printf("===== 1:N Matching\n");
//			pit api
//			OneToNMatch(tpath)
			while (1) {

				ret = _identify_by_scan();

				if (ret > 0) {
					printf("onetoN=%ld\n", ret);
					break;
				}
			}

			//
			ret = _read_image();
			if (ret == 1) {
				printf("read fingerprint image success\r\n");

				memset(file_name,0,sizeof(file_name));
				strcpy(file_name,"./zw.bmp");
				ret = save_image_to_file(file_name);
				if (ret == 1) {
					printf("save image to file success\n");
				} else {
					printf("save image to file fail\n");
				}

			} else {
				printf("read fingerprint image fail\r\n");
			}

			break;
		case 3: //1:1 Matching
			printf("===== 1:1 Matching\n");
//			pit api
//			hlpOneToOneMatch(nID,finger_str,fppath, tpath)
			sprintf(nID, "%d", _input_number("Input staff No:", 123));

			while (1) {
				ret = _verify_by_scan(atoi(nID)*10+0);

				if (ret > 0) {
					printf("onetoone=%ld\n", ret);
					break;
				}
			}

			break;
		case 4: //load fingerprint data
			printf("===== load fingerprint data\n");
//			pit api
//			LoadFingerTemplate(nID,finger_str,fpath)
			//load_finger("./note/admi/finger/");

//			ret = get_module_template_size(&n);
//			if (ret == 1){
//				printf("n = %d\r\n",n);
//				printf("get template size success\n");
//			}else{
//				printf("get template size fail\n");
//			}

			memset(file_name,0,sizeof(file_name));
			strcpy(file_name,"./123_0.s10");
			printf("file_name %s\r\n",file_name);

		    file = fopen( file_name, "r" );
		    if(!file)
		    {
		        return 0;
		    }

		    memset(template,0,sizeof(template));
		    ret  = fread( template, 384, 1,  file );
		    if(ret != 1)
		    {
		        fclose( file );
		        return 0;
		    }

			ret = _enroll_by_template(1230,template,384);
			if (ret == 1) {
				printf("_enroll_by_template success\r\n");
			} else {
				printf("_enroll_by_template fail\r\n");
			}

			break;
		case 5: //Delete
			printf("===== Delete\n");
//			pit api
//			hlpDelete(nID,FingerNum)
			sprintf(nID, "%d", _input_number("Input staff No.:", 123));
			fingernum = _input_number("Input Finger No.:", 0);

			memset(uid,0,sizeof(uid));
			sprintf(uid,"%d",atoi(nID) * 10 + fingernum);

			_delete_template(atoi(uid));
			break;
		case 6:
			printf("===== Delete one all\n");
//			pit api
//			hlpDeleteID(nID)
			sprintf(nID, "%d", _input_number("Input staff No.:", 123));

			for (fingernum = 0; fingernum < 10; fingernum++) {
				memset(uid, 0, sizeof(uid));
				sprintf(uid, "%d", atoi(nID) * 10 + fingernum);

				ret = check_user_id(atoi(uid));
				if (ret == 0) { //not exists
					continue;
				}
				_delete_template(atoi(uid));
			}
			break;
		case 7: //DeleteAll
			printf("===== Delete All\n");
//			pit api
//			hlpDeleteAll()
			_delete_all_templates();
			break;
		case 8:
			printf("===== Get Enroll Count\n");
//			pit api
//			hlpGetEnrollCount()
			ret = get_module_enrolled_finger(&n);
			if (ret == 1) {
				printf("n = %d\r\n",n);
				printf("get_module_enrolled_finger success\r\n");
			} else {
				printf("get_module_enrolled_finger fail\r\n");
			}
			break;
		}
	}

//   	pit api
//   	hlpClose()
	close_port(port);
	return 0;

}
