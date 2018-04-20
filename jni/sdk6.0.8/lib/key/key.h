#ifndef C_KEY_H
#define C_KEY_H
#include "../_precomp.h"
#include "../version.h"

 typedef struct _key_INFO
 {
	int enable;		//0 disabled		1 enable
	int mode;		//0 alpha-numeric  1 numeric	
	int timeout;		//key time out
	int keydisplay;	//0 immediately	1 until ent pressed
 }KEYINFO;

 int InitKeyBoard(IN KEYINFO *key_info);
 int ThreadKeyBoard(void);
 char *ReadKey(IN int key);
 int SetKeyMode(IN int mode);
 int GetKeyMode();

 int KeyMap_Letters(char *str);
 int KeyMap_Capital(char *str);

#endif
