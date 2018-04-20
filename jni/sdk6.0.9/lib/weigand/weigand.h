#ifndef C_WEIGAND_H
#define C_WEIGAND_H
#include "../_precomp.h"
#include "../version.h"

#define WGOUT26 0x5010  
#define SET_READWG 0x4700
#define SET_WRITEWG 0x4701

struct mem_inout{
unsigned int d[3];
};

 int InitWiganRead(void);
 int SetWiganBit(IN int wgbit);
 int ReadWigan(int swg,struct mem_inout*wg_data);
// char * ReadWigan(void);
 int CloseWiganRead(void);


 int InitWiganOut(char *path);
 int SendDataWigan(char *buf) ;
 int  CloseWiganOut(void);


extern int EnableOutPut();
extern int CloseOut();
extern int SetPulse(int msec);

extern int StandbyMode();
extern int PowerSupply();
extern int LowPowerWarning(int value);

#endif
