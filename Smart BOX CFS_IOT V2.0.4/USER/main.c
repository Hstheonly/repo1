/*********************************************************************************
 * 文件名  ：main.c
 * 描述    ：1. 
 * 硬件连接：---------------------------------------------
 *  UART    | PA9  - USART1(Tx) PA10 - USART1(Rx)         |
 *          | PA2  - USART2(Tx) PA3  - USART2(Rx)         |
 *          | PB10 - USART3(Tx) PB11 - USART3(Rx)         |
 *          | PC10 - UART4(Tx)  PC11 - UART4(Rx)          |
 *          | PC12 - UART5(Tx)  PD2  - UART5(Rx)          |
 *           --------------------------------------------- 
 *  485   ： ----------------------------------------------------------------------
 *         | RS485-1 USART3  RS485_RE1 -->PE15     |
 *          -----------------------------------------------------------------------
 *  DI    ： ----------------------------------------------------------------------
 *         | IN1 -->PE2 IN2 -->PE3 IN3 -->PE4 IN4 -->PE6 IN5 -->PE6  IN6 -->PE7    |
 *          -----------------------------------------------------------------------
 *  D0    ： ----------------------------------------------------------------------
 *         | LED2 -->PD5  LED3 -->PD6 LED4 -->PD6                                  |
 *         | Q1 --> PE8  Q2 -->PE9       	                                         |
 *         | WDI --> PE10                                                          |
 *         | M1 --> PE1 M2 -->PE0   M3 -->PB9  M4 -->PB8                           |
 *          -----------------------------------------------------------------------
 * IIC   ： ----------------------------------------------------------------------
 *         | ADS_SDA -->PC7  ADS_SCL -->PC8                                        |
 *         | SDA -->PB7      SCL -->PB6                                            |
 *          -----------------------------------------------------------------------
 * SPI   ： -----------------------------------------------------------------------
 *         | Flash WQ_CS-->PB12 WQ_CLK-->PB13 WQ_MISO-->PB14 WQ_MOSI-->PB15        |
 *          -----------------------------------------------------------------------
 * 网络配置：CON:USART2--> TTL1 Client 9002           IOT:UART4--> TTL2 Client 9003
 * 8.7
 * 今日任务：补货位：电机旋转位置检测逻辑
 *           堆垛车状态检测
 *           配合堆垛车安全门动作和开关检测
 *           集装箱门开关检测和查询
 *           buffer位检测和查询
 *           急停按钮检测和查询 
 * 更新    ：补货位中间位置修改 左右两侧回中间触发传感器方式不同 上升沿和下降沿
 * 日期    ：2019 07 30
 * 版本    : 2.0.3 增加补货位异常处理
 
**********************************************************************************/

#include "GPIO.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"	
#include "includes.h"
#include "24cxx.h" 
#include "ads1115.h"	 
#include "adc.h"
//#include "flash.h"	
#include "HS_BOX_IOT.h"
#include "HS_BOX_CON.h"
#include "DEBUG.h"


#define Software_Version "2.0.4"

const u8 TEXT_Buffer[]={"WarShipSTM32 SPI TEST"};
#define SIZE sizeof(TEXT_Buffer)
//START 任务
#define START_TASK_PRIO      			20 	//开始任务的优先级设置为最低
#define START_STK_SIZE  					64	//设置任务堆栈大小
OS_STK START_TASK_STK[START_STK_SIZE];//创建任务堆栈空间	
void start_task(void *pdata);					//任务函数接口

//测试 任务
#define TEST_TASK_PRIO      			19 	//开始任务的优先级设置为最低
#define TEST_STK_SIZE  						64	//设置任务堆栈大小
OS_STK TEST_TASK_STK[TEST_STK_SIZE];//创建任务堆栈空间	
void TEST_task(void *pdata);					//任务函数接口

//测试 任务
#define RS485_TASK_PRIO      			18 	//开始任务的优先级设置为最低
#define RS485_STK_SIZE  						64	//设置任务堆栈大小
OS_STK RS485_TASK_STK[RS485_STK_SIZE];//创建任务堆栈空间	
void RS485_task(void *pdata);					//任务函数接口


//ADC任务 4-20mA电流采集
#define ADC_TASK_PRIO       			10 	//设置任务优先级
#define ADC_STK_SIZE  		    		128	//设置任务堆栈大小
OS_STK ADC_TASK_STK[ADC_STK_SIZE];		//创建任务堆栈空间	
void ADC_task(void *pdata);						//任务函数接口

//ADS任务 4-20mA电流采集
#define ADS_TASK_PRIO       			9 	//设置任务优先级
#define ADS_STK_SIZE  		    		128	//设置任务堆栈大小
OS_STK ADS_TASK_STK[ADS_STK_SIZE];		//创建任务堆栈空间	
void ADS_task(void *pdata);						//任务函数接口

//RTH任务 Modbus 读取温湿度
#define RTH_TASK_PRIO       			8 	//设置任务优先级
#define RTH_STK_SIZE  		    		128	//设置任务堆栈大小
OS_STK RTH_TASK_STK[RTH_STK_SIZE];		//创建任务堆栈空间	
void RTH_task(void *pdata);						//任务函数接口
 

//CON任务定时上报
#define CONT_TASK_PRIO       			11 	//设置任务优先级
#define CONT_STK_SIZE  		    		128	//设置任务堆栈大小
OS_STK CONT_TASK_STK[CONT_STK_SIZE];		//创建任务堆栈空间	
void CON_task(void *pdata);						//任务函数接口
//CON任务
#define CON_TASK_PRIO       			7 	//设置任务优先级
#define CON_STK_SIZE  		    		128	//设置任务堆栈大小
OS_STK CON_TASK_STK[CON_STK_SIZE];		//创建任务堆栈空间	
void CON_task(void *pdata);						//任务函数接口
//IOT任务
#define IOT_TASK_PRIO       			6 	//设置任务优先级
#define IOT_STK_SIZE  						128	//设置任务堆栈大小
OS_STK IOT_TASK_STK[IOT_STK_SIZE];		//创建任务堆栈空间	
void IOT_task(void *pdata);						//任务函数接口

//KEY任务
#define KEY_TASK_PRIO       			3 	//设置任务优先级
#define KEY_STK_SIZE  						128	//设置任务堆栈大小
OS_STK KEY_TASK_STK[KEY_STK_SIZE];		//创建任务堆栈空间	
void KEY_task(void *pdata);

//WDG任务 喂狗心跳指示LED
#define WDG_TASK_PRIO       			2 	//设置任务优先级
#define WDG_STK_SIZE  						64	//设置任务堆栈大小
OS_STK WDG_TASK_STK[WDG_STK_SIZE];	//创建任务堆栈空间	
void WDG_task(void *pdata);					//任务函数接口
	
 int main(void)
 {
	 
//	u8 datatemp[SIZE];
//	u32 FLASH_SIZE;
	delay_init();	    			 		//延时初始化	  
	NVIC_Configuration(); 	 		//设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(115200,115200,115200,115200,115200);	//串口初始化为115200
	CON_IOT_GPIO_Init();		  			//初始化与LED连接的硬件接口
	
	Feed_WDG();  								//喂狗
	Adc_Init();		  		   			//ADC初始化
	I2C_GPIO_Init();						//初始化IIC ADS1115
	AT24CXX_Init();							//初始化EEPROM
//	SPI_Flash_Init();  					//SPI FLASH 初始化 
	 
	Feed_WDG();
	 
	if(DEBUG) 
		printf("[DEBUG]->(M):Software Version:%s\r\n",Software_Version);

	OSInit();   
 	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//创建起始任务
	OSStart();
 }

 

//开始任务
void start_task(void *pdata)
{
  OS_CPU_SR cpu_sr=0;
	pdata = pdata; 
	OSStatInit();					//初始化统计任务.这里会延时1秒钟左右	
 	OS_ENTER_CRITICAL();			//进入临界区(无法被中断打断)    
	OSTaskCreate(WDG_task,(void *)0,(OS_STK*)&WDG_TASK_STK[WDG_STK_SIZE-1],WDG_TASK_PRIO);
 	OSTaskCreate(CON_task,(void *)0,(OS_STK*)&CON_TASK_STK[CON_STK_SIZE-1],CON_TASK_PRIO);	
//	OSTaskCreate(CONT_task,(void *)0,(OS_STK*)&CONT_TASK_STK[CONT_STK_SIZE-1],CONT_TASK_PRIO);			
 	OSTaskCreate(IOT_task,(void *)0,(OS_STK*)&IOT_TASK_STK[IOT_STK_SIZE-1],IOT_TASK_PRIO);	
	OSTaskCreate(RTH_task,(void *)0,(OS_STK*)&RTH_TASK_STK[RTH_STK_SIZE-1],RTH_TASK_PRIO);	
  OSTaskCreate(ADS_task,(void *)0,(OS_STK*)&ADS_TASK_STK[ADS_STK_SIZE-1],ADS_TASK_PRIO);
//  OSTaskCreate(ADC_task,(void *)0,(OS_STK*)&ADC_TASK_STK[ADC_STK_SIZE-1],ADC_TASK_PRIO);	
//	OSTaskCreate(KEY_task,(void *)0,(OS_STK*)&KEY_TASK_STK[KEY_STK_SIZE-1],KEY_TASK_PRIO);
//	OSTaskCreate(RS485_task,(void *)0,(OS_STK*)&RS485_TASK_STK[RS485_STK_SIZE-1],RS485_TASK_PRIO);

  OSTaskCreate(TEST_task,(void *)0,(OS_STK*)&TEST_TASK_STK[TEST_STK_SIZE-1],TEST_TASK_PRIO);	
	OSTaskSuspend(START_TASK_PRIO);	//挂起起始任务.
	OS_EXIT_CRITICAL();				//退出临界区(可以被中断打断)
}

/************************************喂狗任务****************************************/
void WDG_task(void *pdata)
{	 	
	while(1)
	{
		Feed_WDG();
		delay_ms(200);
		LED1 = !LED1;
	}
}




/***********************************集装箱协议解析*************************************/
void CON_task(void *pdata)
{	 	
	CON_Receive_Protocol();//读取上位机指令并解析
}




/*************************************IOT协议解析*************************************/
//协议解析 通信
void IOT_task(void *pdata)
{	 	
	IOT_Receive_Protocol();//读取上位机指令并解析
}
//通过Modbus读取温湿度
void RTH_task(void *pdata)  
{
	IOT_Modbus_Read_TH();	
}

//通过电流采集温度、亮度
void ADS_task(void *pdata)  
{
	IOT_Read_4_20mA();	
}
















//通过电流采集温度
void RS485_task(void *pdata)  
{

	u8 buf1[50] = {0x00,0x01,0x02,0x03};
	u8 buf2[50] = {0};
	u8 len1 =0;
	u8 len2 = 0;
	printf("WIFI_RS485_task:wifi 和 RS485 透传\r\n");
	RS485_1_Send_Data(buf1,4);
	while(1)
	{

//		RS485_1_Receive_Data(buf1,&len1);
//		if(len1)
//		{
//			printf("CON_Receive:%d\r\n",len1);
//			RS485_1_Send_Data(buf1,len1);
//			len1 = 0;
//		}
//			delay_ms(100);
		RS485_1_Receive_Data(buf2,&len2);
		if(len2)
		{
			RS485_1_Send_Data(buf2,len2);				
			len2 = 0;
		}
		delay_ms(100);
	}
}


//通过电流采集温度
void TEST_task(void *pdata)  
{	u8 k = 0;
	printf("TEST_task\r\n");
	
	while(1)
	{
		
//		DO7 = 1;
//		delay_ms(1000);
//		DO7 = 0;
//		delay_ms(1000);
		delay_ms(100);
//		k = A_DIO_SCAN();
		if(k)
			printf("k = %d\r\n",k);
		LED2 = !LED2;
	}
}


//通过电流采集温度
void KEY_task(void *pdata)  
{

	printf("KEY_task\r\n");
	while(1)
	{
//		K = CON_KEY_SCAN();
//		if(K)
//		{
//			KA = K&0x20;
//			printf("K = %d KA:%d\r\n",K,KA);
//			
//		}
		
		
	}
}

//通过电流采集温度
void ADC_task(void *pdata)  
{
	IOT_STM32_ADC();	
}





////集装箱定时上报任务
//void CONT_task(void *pdata)
//{	 	

//	CON_Device_State_Regular_Report();//定时上报设备状态
//}









