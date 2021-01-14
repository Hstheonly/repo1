#ifndef __HS_BOX_IOT_H	 
#define __HS_BOX_IOT_H	 
#include "sys.h" 	

void IOT_Receive_Protocol(void);//IOT–≠“ÈΩ‚Œˆ
void IOT_Modbus_Read_TH(void);
void IOT_Read_4_20mA(void);
void IOT_STM32_ADC(void);

void RS485_1_Send_Data(u8* buf,u8 len);
void RS485_1_Receive_Data(u8* RSbuf,u8* rs_len);
#endif
