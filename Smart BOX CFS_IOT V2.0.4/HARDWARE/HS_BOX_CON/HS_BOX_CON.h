#ifndef __HS_BOX_CON_H	 
#define __HS_BOX_CON_H	 
#include "sys.h"


#define CON_Replenish_Mid_Fault   4001
#define CON_Replenish_right_Fault 4002
#define CON_Replenish_left_Fault  4003

void CON_Receive_Protocol(void);


void CON_Send_Data(u8* buf,u8 len);
void CON_Receive_Data(u8* RSbuf,u8* rs_len);

void CON_SCAN_Device_State(void);
void CON_REP_TEST(void);

#endif
