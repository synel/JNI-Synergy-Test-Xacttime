#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include "./sb_type.h"

#pragma pack(1)

typedef struct tagBITMAPFILEHEADER {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{
        DWORD      biSize;
        LONG       biWidth;
        LONG       biHeight;
        WORD       biPlanes;
        WORD       biBitCount;
        DWORD      biCompression;
        DWORD      biSizeImage;
        LONG       biXPelsPerMeter;
        LONG       biYPelsPerMeter;
        DWORD      biClrUsed;
        DWORD      biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct tagRGBQUAD {
        BYTE    rgbBlue;
        BYTE    rgbGreen;
        BYTE    rgbRed;
        BYTE    rgbReserved;
} RGBQUAD;

#pragma pack()

#define		DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')
#define		LM_SIZEOF_4(pp_value) ((pp_value + 3) / 4 * 4)
#define		LM_MOD_OF_4(pp_value) ((pp_value % 4)? (4 - (pp_value % 4)): 0)

//--+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8
//:=*===================*===============*=======================================
int SetBMPHeder(
	int					p_X				//:IN :
,	int					p_Y				//:IN :
,	int					p_inf			//:IN :
,	BITMAPFILEHEADER *	p_BMP_FILE		//:OUT:
,	BITMAPINFOHEADER *	p_BMP_INFO		//:OUT:
){		
	int						w_sts			= 0 ;

	//*------ Header --------------------
	p_BMP_INFO->biSize			= sizeof(BITMAPINFOHEADER);//The size of the header.
	p_BMP_INFO->biWidth			= p_X				;//The image width.
	p_BMP_INFO->biHeight		= p_Y				;//The image height.
	p_BMP_INFO->biPlanes		= 1					;//The number of planes for the target device. 
	if( p_inf == 24 ) {
		p_BMP_INFO->biBitCount	= 24				;//The number of bits-per-pixel. 
		p_BMP_FILE->bfOffBits	= (ULONG)(14 + p_BMP_INFO->biSize );// The bmp start position.
	}
	else {
		p_BMP_INFO->biBitCount	= 8;				//The number of bits-per-pixel.
		p_BMP_FILE->bfOffBits	= (ULONG)(14 + p_BMP_INFO->biSize + (256 * sizeof(RGBQUAD)));	// The bmp start position.
	}
	p_BMP_INFO->biCompression	= 0;				// Non compress.
	p_BMP_INFO->biSizeImage		= 0;				// The image size.
	p_BMP_INFO->biXPelsPerMeter	= 0;				// The horizontal resolution, in pixels-per-meter.
	p_BMP_INFO->biYPelsPerMeter	= 0;				// The vertical resolution, in pixels-per-meter.
	p_BMP_INFO->biClrUsed		= 0;				// The number of color indexes in the color table.
	p_BMP_INFO->biClrImportant	= 0;				// The number of color indexes that are required for displaying the bitmap.

	p_BMP_FILE->bfType			= (USHORT)(0x4d42);	// "BM"
	p_BMP_FILE->bfReserved1		= 0;				// Reserved
	p_BMP_FILE->bfReserved2		= 0;				// Reserved

	p_BMP_FILE->bfSize			= (unsigned long)
					(p_BMP_FILE->bfOffBits
					+ (
					(
					LM_SIZEOF_4(p_BMP_INFO->biWidth)
					* p_BMP_INFO->biHeight)
					* (p_BMP_INFO->biBitCount / 8)
					));	// File Size.
	return 0 ;
}
//--+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8
//:=*===================*===============*=======================================
int	BMPSetPlet(								//:CAL:
	FILE *				p_fp			//:IN :
,	int					p_inf			//:IN :
){
	RGBQUAD			pals				;
	int						i					;
	int						w_sts = 0			;
	//.In the 24-bit color. 
	if ( p_inf != 24 ) {
		for(i=0;i<256;i++){
			pals.rgbRed   	= (UCHAR)(i);
			pals.rgbGreen 	= (UCHAR)(i);
			pals.rgbBlue  	= (UCHAR)(i);
			pals.rgbReserved= 0x00;
			if( fwrite(&pals, sizeof(RGBQUAD), 1, p_fp) != 1 ){
				w_sts = -1;
				goto L_EXIT;
			}
		}
	}
L_EXIT:
	return w_sts ;
}
//--+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8
//:=*===================*===============*=======================================
int	ImgToBmp(							//:CAL:
	int					p_X				//:IN :
,	int					p_Y				//:IN :
,	int					p_inf			//:IN :
,	char*				p_bmp			//:OUT:
,	char*				p_img			//:IN :
) {
	int					y					;
	char				*p_R				;
	char				*p_G				;
	char				*p_B				;
	char				*p_mono				;
	int					i,idx				;
	int					w_1men_buff_size	;
	int					w_buff_point		;
	int					w_mod_x				;
	//Color image.
	if ( p_inf == 24 ) {
		w_1men_buff_size = p_Y * p_X ;
		w_mod_x = LM_MOD_OF_4(p_X * 3) ;
		for( idx = 0, y = p_Y; y > 0 ; y -= 1 ) {
			w_buff_point = p_X * (y -1) ;
			p_R = &p_img[(w_1men_buff_size * 0) + w_buff_point] ;
			p_G = &p_img[(w_1men_buff_size * 1) + w_buff_point] ;
			p_B = &p_img[(w_1men_buff_size * 2) + w_buff_point] ;
			for( i = 0 ; i < p_X ; idx++, i++){
				p_bmp[3 * idx +0 + ((p_Y - y) * w_mod_x)] = p_B[i] ;
				p_bmp[3 * idx +1 + ((p_Y - y) * w_mod_x)] = p_G[i] ;
				p_bmp[3 * idx +2 + ((p_Y - y) * w_mod_x)] = p_R[i] ;
			}
		}
	}
	//Gray image.
	else { //.if ( p_inf == 8 ) {
		for( idx = 0,y = p_Y ; y > 0 ; y -= 1 ) {
			w_buff_point = p_X * (y -1) ;
			p_mono = &p_img[w_buff_point] ;
			for( i = 0 ; i < p_X ; idx++, i++){
				p_bmp[idx] = p_mono[i];
			}
			for( i = 0 ; i < LM_MOD_OF_4(p_X) ; idx++, i++){
				p_bmp[idx] = (UCHAR)255;
			}
		}
	}
	return 0 ;
}
//--+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8
//:=*===================*===============*=======================================
int	ImageWrite2BmpFile(						//:CAL:Save bmp-data as file
	const char*			p_FileName		//:IN :File name.
,	int					p_X				//:IN :width
,	int					p_Y				//:IN :height
,	UCHAR*				p_buff			//:IN :Bitmap data.
,	int					p_inf			//:IN :8=grayA24=color
){										//:
	int						w_sts	=	0			;
	BITMAPFILEHEADER		w_BMP_FILE				;
	BITMAPINFOHEADER		w_BMP_INFO				;
	FILE *					w_fp	=	NULL		;
	int						w_y,w_x					;
	int						w_inf					;
	char					*w_pBuff = NULL			;
	int						w_size					;
	////////////////////////////////////////////////////////////////////////////
	//.check
	if ( p_FileName == NULL )	w_sts =  -1 ;
	if ( p_X <= 0 )				w_sts =  -2 ;
	if ( p_Y <= 0 )				w_sts =  -3 ;
	if ( p_buff == NULL )		w_sts =  -4 ;
 	if ( w_sts != 0 ) goto L_EXIT ;

	w_x = p_X ;
	w_y = p_Y ;
	w_inf = p_inf;

	//.Open the file
	w_fp = fopen(p_FileName, "wb" );
	if( w_fp == NULL ){
		w_sts = -5;
		goto L_EXIT ;
	}
	w_sts = SetBMPHeder(w_x,w_y,w_inf,&w_BMP_FILE,&w_BMP_INFO);
	if ( w_sts != 0 ) {
		w_sts = -6	;
		goto L_EXIT ;
	}
		
	if ( fwrite(&w_BMP_FILE, sizeof(w_BMP_FILE), 1, w_fp) != 1 ) {
		w_sts = -7 ;
		goto L_EXIT ;
	}
	if ( fwrite(&w_BMP_INFO, sizeof(w_BMP_INFO), 1, w_fp) != 1 ) {
		w_sts = -8 ;
		goto L_EXIT ;
	}
	if (BMPSetPlet(w_fp,w_inf) != 0) {
		w_sts = -9 ;
		goto L_EXIT ;
	}

	w_size = w_y * LM_SIZEOF_4(w_x) * ((w_inf == 24)? 3 : 1) ;
	w_pBuff = (char*)malloc(w_size);
	if ( w_pBuff == NULL ) {
		w_sts = -10 ;
		goto L_EXIT ;
	}
	w_sts = ImgToBmp(w_x,w_y,w_inf,w_pBuff,(char*)p_buff) ;
	if ( w_sts != 0 ) {
		w_sts = -11	;
		goto L_EXIT ;
	}
	if ( fwrite(w_pBuff, w_size, 1, w_fp) != 1 ) {
		w_sts = -12 ;
		goto L_EXIT ;
	}
L_EXIT:
	if (w_pBuff != NULL ) free(w_pBuff);
	if ( w_fp != NULL ) fclose(w_fp);
	return w_sts	;
}

int	ImageReadFromBmpFile(
	const char*			p_FileName		//:IN :File name.
,	int					p_X				//:IN :width
,	int					p_Y				//:IN :height
,	UCHAR*				p_buff			//:OUT:Image data.
)
{
	int						w_sts	=	0			;
	BITMAPFILEHEADER		w_BMP_FILE				;
	BITMAPINFOHEADER		w_BMP_INFO				;
	FILE *					w_fp	=	NULL		;
	int						w_x, k					;
	char					*w_pBuff = NULL			;
	////////////////////////////////////////////////////////////////////////////
	//.check
	if ( p_FileName == NULL )	w_sts =  -1 ;
	if ( p_X <= 0 )				w_sts =  -2 ;
	if ( p_Y <= 0 )				w_sts =  -3 ;
	if ( p_buff == NULL )		w_sts =  -4 ;
 	if ( w_sts != 0 ) goto L_EXIT ;

	//.Open the file
	w_fp = fopen(p_FileName, "rb" );
	if( w_fp == NULL ){
		w_sts = -5;
		goto L_EXIT ;
	}

	if (fread(&w_BMP_FILE, 1, sizeof(w_BMP_FILE), w_fp) != sizeof(w_BMP_FILE)) {
		w_sts = -6;
		goto L_EXIT ;
	}
	if (w_BMP_FILE.bfType != DIB_HEADER_MARKER) {
		w_sts = -7;
		goto L_EXIT ;
	}

	if (fread(&w_BMP_INFO, 1, sizeof(w_BMP_INFO), w_fp) != sizeof(w_BMP_INFO)) {
		w_sts = -7;
		goto L_EXIT ;
	}

	if (w_BMP_INFO.biWidth != p_X || 
		w_BMP_INFO.biHeight != p_Y || 
		w_BMP_INFO.biBitCount != 8) 
	{
		w_sts = -8;
		goto L_EXIT ;
	}

	if (fseek(w_fp, w_BMP_FILE.bfOffBits, SEEK_SET) != 0) {
		w_sts = -9;
		goto L_EXIT ;
	}

	w_x = LM_SIZEOF_4(p_X);
	w_pBuff = (char*)malloc(w_x);
	for (k = p_Y - 1; k >= 0; k--) {
		if (fread(w_pBuff, 1, w_x, w_fp) != w_x) {
			w_sts = -10;
			goto L_EXIT ;
		}
		memcpy(&p_buff[k * p_X], w_pBuff, p_X);
	}

	
L_EXIT:
	if ( w_fp != NULL ) fclose(w_fp);
	if (w_pBuff != NULL ) free(w_pBuff);
	return w_sts	;
}
