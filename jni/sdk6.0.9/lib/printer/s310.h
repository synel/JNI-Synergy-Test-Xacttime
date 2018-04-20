#ifndef PRINTCOM_H__
#define PRINTCOM_H__

#include "printer.h"

#define CUT_PAPER		"1D564200"		// cut paper
#define INIT_PRINT		"1B40"		// initialize printer
#define PRINT_ENTER		"0A"		// print and feed line
#define SETCHAR_SIZE(a)	"1B21"#a	// font setting
#define SET_LEFT(a,b)	"1D4C"#a#b	// The value of the left margin is [(nL + nH x 256) x basic calculation pitch] inches
#define SET_ALIGNMENT(a)	"1B61"#a	// Aligning the : n=0 Left end alignment,n=1 Centering,n=2 Right end alignment
#define PRINT_STATUS	"100401"	// status of printer

int s310_Readstatus();

#endif // !defined(COMM_H__)
