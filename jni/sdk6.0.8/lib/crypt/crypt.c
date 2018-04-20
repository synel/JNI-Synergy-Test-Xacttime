/*------------------------------------------------------------------------
 * Description£º
 * File name - crypt.c 
 *
 * Function list£º
 *        encrypt        - Encryption function
 *        decrypt        - Decryption function
 *-----------------------------------------------------------------------*/

/*------------------------------------------------------------------------
 *
 * Reference materials may be used are list here
 *
 *-----------------------------------------------------------------------*/

/* Header files used are followings*/

#include "crypt.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

/*--------------------------------------------------------------------------*
@Function		: encrypt - Encryption function
@Prototype		: int encrypt(const unsigned char *src,unsigned char *dest, int src_len£¬
                             int dest_len)
@Include           : "crypt.h"
@Description	   	: src£ºdata will be encrypted£»
                 	dest£ºwhere to store the encrypted data£»
                 	src_len£ºthe length of the encrypted data£»
                 	dest_len£ºthe buffer size pointed by dest(according to Base64 coding)
		    						dest_len = ((src_len - 1) / 3 + 1) * 4; functin will check it
                    when dest_len < ((src_len - 1) / 3 + 1) * 4£¬function will return
@Return Value     : Success  length of data; Failure 0
*--------------------------------------------------------------------------*/
int encrypt( char *src,
             char *dest, int src_len, int dest_len) {

//const char EnBase64Tab[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const char EnBase64Tab[] = "#$abcdefghijklmNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMnopqrstuvwxyz";
   unsigned char c1, c2, c3; /* 3bytes */
    int n_div, n_mod, i, length = 0;

    /*assert(src != NULL);
    assert(dest != NULL);
    assert(src_len > 0);
    assert(dest_len > 0);
    assert(((src_len - 1) / 3 + 1) * 4 <= dest_len);*/
if((src==NULL)||(dest==NULL)||(src_len<=0)||(dest_len<=0)||(((src_len - 1) / 3 + 1) * 4 > dest_len))
        return 0;

    n_div = src_len / 3;
    n_mod = src_len % 3;
    for(i = 0; i < n_div; i++) {
        c1 = *src++;
        c2 = *src++;
        c3 = *src++;

        *dest++ = EnBase64Tab[c1 >> 2];
        *dest++ = EnBase64Tab[((c1 << 4) | (c2 >> 4)) & 0x3F];
        *dest++ = EnBase64Tab[((c2 << 2) | (c3 >> 6)) & 0x3F];
        *dest++ = EnBase64Tab[c3 & 0x3F];
        length += 4;
	if(length > dest_len)
            return 0;
	}
        if(n_mod == 1) { /* Coding to the remaining one byte  */
            c1 = *src++;
            *dest++ = EnBase64Tab[c1 >> 2];
            *dest++ = EnBase64Tab[(c1 & 0x03) << 4];
            *dest++ = '=';
            *dest++ = '=';
            length += 4;
 	} else if(n_mod == 2) { /* Coding to the remaining two bytes */
            c1 = *src++;
            c2 = *src++;
            *dest++ = EnBase64Tab[c1 >> 2];
            *dest++ = EnBase64Tab[((c1 << 4) | (c2 >> 4)) & 0x3F];
            *dest++ = EnBase64Tab[(c2 << 2) & 0x3F];
            *dest++ = '=';
            length += 4;
        }
        *dest++ = '\0';

        if(length > dest_len)
            return 0;
        return length;
}

/*--------------------------------------------------------------------------*
@Function		: decrypt - Decryption function
@Prototype   	: int decrypt(const char *src, char *dest, int src_len£¬
                              int dest_len)
@Include		: "crypt.h"
@Description	      : src£ºData waiting to be decrypted£»
                  	dest£ºstore the decrypted data£»
                  	src_len£ºlenght of the decrypted data(must be in multiples of 4)
                    dest_len£ºthe buffer size pointed by dest(according to Base64 coding)
                    dest_len = (src / 4) * 3 ; functin will check it
                    when dest_len < (src / 4) * 3£¬then return
@Return Value     : Success  length of data; Failure 0
*--------------------------------------------------------------------------*/

int decrypt( char *src, char *dest,
            int src_len, int dest_len) {

    const char DeBase64Tab[] = { 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 28, 29, 30, 31,32, 33,
34, 35, 36, 37, 0, 0, 0, 0, 0, 0, 0, 38, 39, 40, 41, 42, 43, 44,
45, 46, 47, 48, 49, 50, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
27, 0, 0, 0, 0, 0, 0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
13, 14, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63
};
	char p1, p2, p3, p4; /* the corresponding locations of 4 source bytes in DeBase64Tab */
	int div, i;

	/*assert(src != NULL);
	assert(dest != NULL);
	assert(src_len > 0 && src_len % 4 == 0);
	assert(dest_len >= (src_len >> 2) * 3);*/
    if(src == NULL || dest == NULL || src_len <= 0 || src_len % 4 != 0 \
       || dest_len < (src_len >> 2) * 3)
		return -1;

    div = src_len >> 2;

    for(i = 0; i < div; i++) {
        p1 = DeBase64Tab[(int)*src++];
        p2 = DeBase64Tab[(int)*src++];
        p3 = DeBase64Tab[(int)*src++];
        p4 = DeBase64Tab[(int)*src++];

        *dest++ = (unsigned char)((p1 << 2) | (p2 >> 4));
        *dest++ = (unsigned char)((p2 << 4) | (p3 >> 2));
        *dest++ = (unsigned char)(((p3 & 0x03) << 6) | p4);
    }
    *dest++ = '\0';
    return 0;
} /* decrypt */

int _encrypt( char *src,
             char *dest, int src_len, int dest_len){
	return encrypt(src,dest,src_len,dest_len);
}

int _decrypt( char *src,
char *dest, int src_len, int dest_len){
	return decrypt(src,dest,src_len,dest_len);
}
