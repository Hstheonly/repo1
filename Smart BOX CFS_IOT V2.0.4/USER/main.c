/*********************************************************************************
 * �ļ���  ��main.c
 * ����    ��1. 
 * Ӳ�����ӣ�---------------------------------------------
 *  UART    | PA9  - USART1(Tx) PA10 - USART1(Rx)         |
 *          | PA2  - USART2(Tx) PA3  - USART2(Rx)         |
 *          | PB10 - USART3(Tx) PB11 - USART3(Rx)         |
 *          | PC10 - UART4(Tx)  PC11 - UART4(Rx)          |
 *          | PC12 - UART5(Tx)  PD2  - UART5(Rx)          |
 *           --------------------------------------------- 
 *  485   �� ----------------------------------------------------------------------
 *         | RS485-1 USART3  RS485_RE1 -->PE15     |
 *          -----------------------------------------------------------------------
 *  DI    �� ----------------------------------------------------------------------
 *         | IN1 -->PE2 IN2 -->PE3 IN3 -->PE4 IN4 -->PE6 IN5 -->PE6  IN6 -->PE7    |
 *          -----------------------------------------------------------------------
 *  D0    �� ----------------------------------------------------------------------
 *         | LED2 -->PD5  LED3 -->PD6 LED4 -->PD6                                  |
 *         | Q1 --> PE8  Q2 -->PE9       	                                         |
 *         | WDI --> PE10                                                          |
 *         | M1 --> PE1 M2 -->PE0   M3 -->PB9  M4 -->PB8                           |
 *          -----------------------------------------------------------------------
 * IIC   �� ----------------------------------------------------------------------
 *         | ADS_SDA -->PC7  ADS_SCL -->PC8                                        |
 *         | SDA -->PB7      SCL -->PB6                                            |
 *          -----------------------------------------------------------------------
 * SPI   �� -----------------------------------------------------------------------
 *         | Flash WQ_CS-->PB12 WQ_CLK-->PB13 WQ_MISO-->PB14 WQ_MOSI-->PB15        |
 *          -----------------------------------------------------------------------
 * �������ã�CON:USART2--> TTL1 Client 9002           IOT:UART4--> TTL2 Client 9003
 * 8.7
 * �������񣺲���λ�������תλ�ü���߼�
 *           �Ѷ⳵״̬���
 *           ��϶Ѷ⳵��ȫ�Ŷ����Ϳ��ؼ��
 *           ��װ���ſ��ؼ��Ͳ�ѯ
 *           bufferλ���Ͳ�ѯ
 *           ��ͣ��ť���Ͳ�ѯ 
 * ����    ������λ�м�λ���޸� ����������м䴥����������ʽ��ͬ �����غ��½���
 * ����    ��2019 07 30
 * �汾    : 2.0.3 ���Ӳ���λ�쳣����
 
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
//START ����
#define START_TASK_PRIO      			20 	//��ʼ��������ȼ�����Ϊ���
#define START_STK_SIZE  					64	//���������ջ��С
OS_STK START_TASK_STK[START_STK_SIZE];//���������ջ�ռ�	
void start_task(void *pdata);					//�������ӿ�

//���� ����
#define TEST_TASK_PRIO      			19 	//��ʼ��������ȼ�����Ϊ���
#define TEST_STK_SIZE  						64	//���������ջ��С
OS_STK TEST_TASK_STK[TEST_STK_SIZE];//���������ջ�ռ�	
void TEST_task(void *pdata);					//�������ӿ�

//���� ����
#define RS485_TASK_PRIO      			18 	//��ʼ��������ȼ�����Ϊ���
#define RS485_STK_SIZE  						64	//���������ջ��С
OS_STK RS485_TASK_STK[RS485_STK_SIZE];//���������ջ�ռ�	
void RS485_task(void *pdata);					//�������ӿ�


//ADC���� 4-20mA�����ɼ�
#define ADC_TASK_PRIO       			10 	//�����������ȼ�
#define ADC_STK_SIZE  		    		128	//���������ջ��С
OS_STK ADC_TASK_STK[ADC_STK_SIZE];		//���������ջ�ռ�	
void ADC_task(void *pdata);						//�������ӿ�

//ADS���� 4-20mA�����ɼ�
#define ADS_TASK_PRIO       			9 	//�����������ȼ�
#define ADS_STK_SIZE  		    		128	//���������ջ��С
OS_STK ADS_TASK_STK[ADS_STK_SIZE];		//���������ջ�ռ�	
void ADS_task(void *pdata);						//�������ӿ�

//RTH���� Modbus ��ȡ��ʪ��
#define RTH_TASK_PRIO       			8 	//�����������ȼ�
#define RTH_STK_SIZE  		    		128	//���������ջ��С
OS_STK RTH_TASK_STK[RTH_STK_SIZE];		//���������ջ�ռ�	
void RTH_task(void *pdata);						//�������ӿ�
 

//CON����ʱ�ϱ�
#define CONT_TASK_PRIO       			11 	//�����������ȼ�
#define CONT_STK_SIZE  		    		128	//���������ջ��С
OS_STK CONT_TASK_STK[CONT_STK_SIZE];		//���������ջ�ռ�	
void CON_task(void *pdata);						//�������ӿ�
//CON����
#define CON_TASK_PRIO       			7 	//�����������ȼ�
#define CON_STK_SIZE  		    		128	//���������ջ��С
OS_STK CON_TASK_STK[CON_STK_SIZE];		//���������ջ�ռ�	
void CON_task(void *pdata);						//�������ӿ�
//IOT����
#define IOT_TASK_PRIO       			6 	//�����������ȼ�
#define IOT_STK_SIZE  						128	//���������ջ��С
OS_STK IOT_TASK_STK[IOT_STK_SIZE];		//���������ջ�ռ�	
void IOT_task(void *pdata);						//�������ӿ�

//KEY����
#define KEY_TASK_PRIO       			3 	//�����������ȼ�
#define KEY_STK_SIZE  						128	//���������ջ��С
OS_STK KEY_TASK_STK[KEY_STK_SIZE];		//���������ջ�ռ�	
void KEY_task(void *pdata);

//WDG���� ι������ָʾLED
#define WDG_TASK_PRIO       			2 	//�����������ȼ�
#define WDG_STK_SIZE  						64	//���������ջ��С
OS_STK WDG_TASK_STK[WDG_STK_SIZE];	//���������ջ�ռ�	
void WDG_task(void *pdata);					//�������ӿ�
	
 int main(void)
 {
	 
//	u8 datatemp[SIZE];
//	u32 FLASH_SIZE;
	delay_init();	    			 		//��ʱ��ʼ��	  
	NVIC_Configuration(); 	 		//����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200,115200,115200,115200,115200);	//���ڳ�ʼ��Ϊ115200
	CON_IOT_GPIO_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�
	
	Feed_WDG();  								//ι��
	Adc_Init();		  		   			//ADC��ʼ��
	I2C_GPIO_Init();						//��ʼ��IIC ADS1115
	AT24CXX_Init();							//��ʼ��EEPROM
//	SPI_Flash_Init();  					//SPI FLASH ��ʼ�� 
	 
	Feed_WDG();
	 
	if(DEBUG) 
		printf("[DEBUG]->(M):Software Version:%s\r\n",Software_Version);

	OSInit();   
 	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//������ʼ����
	OSStart();
 }

 

//��ʼ����
void start_task(void *pdata)
{
  OS_CPU_SR cpu_sr=0;
	pdata = pdata; 
	OSStatInit();					//��ʼ��ͳ������.�������ʱ1��������	
 	OS_ENTER_CRITICAL();			//�����ٽ���(�޷����жϴ��)    
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
	OSTaskSuspend(START_TASK_PRIO);	//������ʼ����.
	OS_EXIT_CRITICAL();				//�˳��ٽ���(���Ա��жϴ��)
}

/************************************ι������****************************************/
void WDG_task(void *pdata)
{	 	
	while(1)
	{
		Feed_WDG();
		delay_ms(200);
		LED1 = !LED1;
	}
}




/***********************************��װ��Э�����*************************************/
void CON_task(void *pdata)
{	 	
	CON_Receive_Protocol();//��ȡ��λ��ָ�����
}




/*************************************IOTЭ�����*************************************/
//Э����� ͨ��
void IOT_task(void *pdata)
{	 	
	IOT_Receive_Protocol();//��ȡ��λ��ָ�����
}
//ͨ��Modbus��ȡ��ʪ��
void RTH_task(void *pdata)  
{
	IOT_Modbus_Read_TH();	
}

//ͨ�������ɼ��¶ȡ�����
void ADS_task(void *pdata)  
{
	IOT_Read_4_20mA();	
}
















//ͨ�������ɼ��¶�
void RS485_task(void *pdata)  
{

	u8 buf1[50] = {0x00,0x01,0x02,0x03};
	u8 buf2[50] = {0};
	u8 len1 =0;
	u8 len2 = 0;
	printf("WIFI_RS485_task:wifi �� RS485 ͸��\r\n");
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


//ͨ�������ɼ��¶�
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


//ͨ�������ɼ��¶�
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

//ͨ�������ɼ��¶�
void ADC_task(void *pdata)  
{
	IOT_STM32_ADC();	
}





////��װ�䶨ʱ�ϱ�����
//void CONT_task(void *pdata)
//{	 	

//	CON_Device_State_Regular_Report();//��ʱ�ϱ��豸״̬
//}









