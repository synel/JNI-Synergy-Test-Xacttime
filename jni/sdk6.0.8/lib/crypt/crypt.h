/*------------------------------------------------------------------------
 * Description£º
 * File name - crypt.h 
 *
 * the exported list £º
 *         encrypt        - Encryption function
 *         decrypt        - Decryption function
 *-----------------------------------------------------------------------*/

#ifndef __CRYPT_H
#define __CRYPT_H

/* Header files used are following */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


/* Edit code here */
int encrypt( char *src,
             char *dest, int src_len, int dest_len);
int decrypt( char *src,
char *dest, int src_len, int dest_len);

int _encrypt( char *src,
             char *dest, int src_len, int dest_len);
int _decrypt( char *src,
char *dest, int src_len, int dest_len);


#endif /* crypt.h */
