#include "delay.h"
#include "sys.h"
#include "usart.h"	
#include "DEBUG.h"

/**********************************************************
打印调试等级：
DEBUG(调试):开发调试日志
INFO(通知)：记录系统运行状态等信息
WARN(警告): 发生问题但是不影响运行
ERROR(错误)：发生故障
[DEBUG]->(M) hello world  主任务
[DEBUG]->(P) hello world	取货口
[DEBUG]->(T) hello world	暂存柜
[ERROR]->(T) hello world
[WARN ]->(T) hello world
[INFO ]->(T) hello world

**********************************************************/




#ifdef DEBUG_Print
/*******************************************************************DEBUG****************************************************************/
/*****************************************************
 * 函数名: ONE_HEX_Print
 * 描述  ：打印一个十六进制数据
 * 输入  ：Hex 十六进制数据
 * 输出  ：无
 ****************************************************/
void ONE_HEX_Print(u8 Hex)
{
	u8 i = 0;
	u8 Temp = 0;
	u8 Str[4] = {0};
	
		Temp = Hex/16;
		if(Temp < 10)
			Str[0] = Temp+0x30;
		else
			Str[0] = Temp+0x37;
		Temp = Hex%16;
		if(Temp < 10)
			Str[1] = Temp+0x30;
		else
			Str[1] = Temp+0x37;
		Str[2] = ' '; 
	
	USART1_Send_Data(Str,3);
}

/*****************************************************
 * 函数名: Asterisk_Print
 * 描述  ：打印****
 * 输入  ：Num *号的数量
 * 输出  ：无
 ****************************************************/
void Asterisk_Print(u8 Num)
{
	u8 i = 0;
	
	for(i = 0;i < Num;i++)
	{
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);	  
		USART_SendData(USART1,0x2A);
	}
}

/*****************************************************
 * 函数名: Bang_Print
 * 描述  ：打印!!!
 * 输入  ：Num !号的数量
 * 输出  ：无
 ****************************************************/
void Bang_Print(u8 Num)
{
	u8 i = 0;
	
	for(i = 0;i < Num;i++)
	{
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);	  
		USART_SendData(USART1,0x21);
	}
}

/*****************************************************
 * 函数名: Pound_Print
 * 描述  ：打印##
 * 输入  ：Num #号的数量
 * 输出  ：无
 ****************************************************/
void Pound_Print(u8 Num)
{
	u8 i = 0;
	
	for(i = 0;i < Num;i++)
	{
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);	  
		USART_SendData(USART1,0x23);
	}
}

/*****************************************************
 * 函数名: CR_Print
 * 描述  ：打印换行
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CR_Print(void)
{
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);	  
		USART_SendData(USART1,0x0D);
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);	  
		USART_SendData(USART1,0x0A);
}

#endif

/*****************************************************
 * 函数名: DEBUG_Print_M
 * 描述  ：日志调试信息
 * 输入  ：Str：字符串
 * 输出  ：无
 ****************************************************/
void DEBUG_Print_M(char* Str)
{
	if(DEBUG)
		printf("[DEBUG]->(M):%s\r\n",Str);
}

/*****************************************************
 * 函数名: DEBUG_Print_PD
 * 描述  ：日志调试信息
 * 输入  ：Str：字符串
 * 输出  ：无
 ****************************************************/
void DEBUG_Print_PD(char* Str)
{
	if(DEBUG)
		printf("[DEBUG]->(P):%s\r\n",Str);
}

/*****************************************************
 * 函数名: DEBUG_Print_TST
 * 描述  ：日志调试信息
 * 输入  ：Str：字符串
 * 输出  ：无
 ****************************************************/
void DEBUG_Print_TST(char* Str)
{
	if(DEBUG)
		printf("[DEBUG]->(T):%s\r\n",Str);
}




/*****************************************************
 * 函数名: INFO_Print_M
 * 描述  ：日志状态
 * 输入  ：Str：字符串
 * 输出  ：无
 ****************************************************/
void INFO_Print_M(char* Str)
{
	#ifdef __INFO__
		printf("[INFO ]->(M):%s\r\n");
	#endif
}

/*****************************************************
 * 函数名: INFO_Print_PD
 * 描述  ：日志状态
 * 输入  ：Str：字符串
 * 输出  ：无
 ****************************************************/
void INFO_Print_PD(char* Str)
{
	#ifdef __INFO__
		printf("[INFO ]->(P):%s\r\n");
	#endif
}

/*****************************************************
 * 函数名: INFO_Print_TST
 * 描述  ：日志状态
 * 输入  ：Str：字符串
 * 输出  ：无
 ****************************************************/
void INFO_Print_TST(char* Str)
{
	#ifdef __INFO__
		printf("[INFO ]->(T):%s\r\n");
	#endif
}


/*****************************************************
 * 函数名: WARN_Print_PD
 * 描述  ：日志打印警告吗
 * 输入  ：Warn_Code：警告码
 * 输出  ：无
 ****************************************************/
void WARN_Print_PD(u16 Warn_Code)
{
	#ifdef __WARN__
	printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
	printf("[WARN ]->(P):!!!!!!!!!!!!!!!!WARN_Code:%d\r\n",Warn_Code);
	printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
	#endif
}	

/*****************************************************
 * 函数名: WARN_Print_TST
 * 描述  ：日志打印警告吗
 * 输入  ：Warn_Code：警告码
 * 输出  ：无
 ****************************************************/
void WARN_Print_TST(u16 Warn_Code)
{
	#ifdef __WARN__
	printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
	printf("[WARN ]->(T):!!!!!!!!!!!!!!!!WARN_Code:%d\r\n",Warn_Code);
	printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
	#endif
}	

/*****************************************************
 * 函数名: ERROR_Print_PD
 * 描述  ：日志打印错误码
 * 输入  ：Warn_Code：警告码
 * 输出  ：无
 ****************************************************/
void ERROR_Print_PD(u16 ERROR_Code)
{
	#ifdef __WARN__
	printf("#######################################################\r\n");
	printf("[ERROR ]->(P):#################ERROR_Code:%d\r\n",ERROR_Code);
	printf("#######################################################\r\n");
	#endif
}	


/*****************************************************
 * 函数名: ERROR_Print_TST
 * 描述  ：日志打印错误码
 * 输入  ：Warn_Code：警告码
 * 输出  ：无
 ****************************************************/
void ERROR_Print_TST(u16 ERROR_Code)
{
	#ifdef __WARN__
	printf("######################################################\r\n");
	printf("[ERROR ]->(T):###############ERROR_Code:%d\r\n",ERROR_Code);
	printf("######################################################\r\n");
	#endif
}	



