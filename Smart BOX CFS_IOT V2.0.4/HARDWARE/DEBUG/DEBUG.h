#ifndef __DEBUG_H
#define __DEBUG_H	

#include "sys.h"
//#include "usart.h"	
#define DEBUG 1
#define INFO  1
#define WARN  1
#define ERROR 1

//#define DEBUG_Print 1

void ONE_HEX_Print(u8 Hex);
void Asterisk_Print(u8 Num);
void CR_Print(void);

void DEBUG_Print_PD(char* Str);
void DEBUG_Print_M(char* Str);
void DEBUG_Print_TST(char* Str);

void INFO_Print_PD(char* Str);
void INFO_Print_M(char* Str);
void INFO_Print_TST(char* Str);

void WARN_Print_PD(u16 Warn_Code);
void WARN_Print_TST(u16 Warn_Code);

void ERROR_Print_PD(u16 ERROR_Code);
void ERROR_Print_TST(u16 ERROR_Code);



#endif
