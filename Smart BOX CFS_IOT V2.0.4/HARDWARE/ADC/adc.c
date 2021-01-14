 /*********************************************************************************
 * �ļ���  ��main.c
 * ����    ��1.AD��ӡ����
 *           2.ÿ��Ҫת����ͨ��������ʱ�佨�龡����һ�㣬�Ի�ýϸߵ�׼ȷ�ȣ�
 *   					 ���������ή�� ADC ��ת������
 *					 3.Tcovn=����ʱ��+12.5 ������   Tcovn Ϊ��ת��ʱ��
 *					   �� ADCCLK=14Mhz ��ʱ�򣬲����� 1.5 �����ڵĲ���ʱ�䣬
 *						 ��õ���Tcovn=1.5+12.5=14 ������=1us��
 * Ӳ��    ��CH11 --> PC1
 * ע��    ���ڵ�����ֲ�����а���������� float tmep ��ֵ u32 temp ���ȫ����0 
 *           �����˺�ʱ�����ڷ��������� ���ڸ�ֵ���͵Ĳ�ͬ�����ͬ
 * ��ֲ˵����ֻҪ��adc.c �� adc.h��ӵ�����Ҫ�Ĺ����� NVIC_Configuration()�����ж���
 *           Adc_Init();��ʼ��AD ��������ͨ�� Get_Adc_Average(ADC_Channel_17,10)
 *           ����ȡAD������ ADC_Channel_17ѡ��ADͨ��
 * ����    ��2014 07 08 
**********************************************************************************/
#include "adc.h"
#include "delay.h"
#include "stm32f10x.h"
#include "usart.h"
/*****************************************************
 * ��������Adc_Init
 * ����  ����ʼ��ADC  
 *				 �Թ���ͨ��Ϊ�� 
 *         Ĭ�Ͻ�����ͨ��0~3	
 * ����  ����
 * ���  ����
 *       AD0-->PA7_ADC7 AD AD1-->PA6_ADC6 AD2-->PA5_ADC5 AD3-->PA4_ADC4 AD4-->PC4_ADC14 AD5-->PC5_ADC15
 ****************************************************/
 
void  Adc_Init(void)
{ 	
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_ADC1,ENABLE );	  //ʹ��ADC1ͨ��ʱ��
 	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //����ADC��Ƶ����6 72M/6=12,ADC���ʱ�䲻�ܳ���14M
		
  //PA4-7��Ϊģ��ͨ���������� 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		       //ģ����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);		
	//PA1  ��Ϊģ��ͨ����������                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		       //ģ����������
	GPIO_Init(GPIOC, &GPIO_InitStructure);	

	ADC_DeInit(ADC1);                                      //��λADC1,������ ADC1 ��ȫ���Ĵ�������Ϊȱʡֵ

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	   //ADC����ģʽ:ADC1��ADC2�����ڶ���ģʽ
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	         //ģ��ת�������ڵ�ͨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	   //ģ��ת�������ڵ���ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//ת��������������ⲿ��������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; //ADC�����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = 1;	               //˳����й���ת����ADCͨ������Ŀ
	ADC_Init(ADC1, &ADC_InitStructure);	                   //����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ���   

  ADC_Cmd(ADC1, ENABLE);	                    //ʹ��ָ����ADC1
	
	ADC_ResetCalibration(ADC1);	                //ʹ�ܸ�λУ׼  
	 
	while(ADC_GetResetCalibrationStatus(ADC1)); //�ȴ���λУ׼����
	
	ADC_StartCalibration(ADC1);	                //����ADУ׼
 
	while(ADC_GetCalibrationStatus(ADC1));	    //�ȴ�У׼����
}				  

/*****************************************************
 * ��������Get_Adc
 * ����  �����ADCֵ
 * ����  ����
 * ���  ����
 ****************************************************/
u16 Get_Adc(u8 ch)   
{
  //����ָ��ADC�Ĺ�����ͨ����һ�����У�����ʱ��
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADCͨ��,����ʱ��Ϊ239.5����	  			    
  
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		     //ʹ��ָ����ADC1�����ת����������	
	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//�ȴ�ת������

	return ADC_GetConversionValue(ADC1);	        //�������һ��ADC1�������ת�����
}

/*****************************************************
 * ��������Get_Adc_Average
 * ����  �����ADCƽ��ֵ
 * ����  ����
 * ���  ����
 ****************************************************/
u16 Get_Adc_Average(u8 ch,u8 times)
{
	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		temp_val+=Get_Adc(ch);//239.5+12.5 = 252����  12M  252/12us = 21uS
		delay_us(50);
//		delay_ms(1);
	}
	return temp_val/times;
} 	

u16 Draw_Calculate(u16 *Front_Data,u8 Fi, u16 adcx)
{
	u8 i = 0;
	u32 Sum = 0;
	Front_Data[Fi] = adcx;
	
	for(i = 0;i < 5;i++)
	{
		Sum += Front_Data[i];
	}
	return Sum/5;
}




//#define Unload_I 2600  //mV����ʱ AC�ɼ���ѹֵ
//#define Hinder_I 4 		 //A �������ֵ �رյ��
//u8 Motor_I_Monitor(void)
//{
////	static u32 Sum = 0;
//	static u8 T = 0;
//	u16 HEX;
//	float V,I;
//	
//	HEX = Get_Adc_Average(ADC_Channel_14,10);
//	
//	V = HEX*3300.0/4096*2;//AD�ɼ�����ѹ
//	I = (V - Unload_I)/64; //�ɼ����ĵ�ѹ
//	printf("M_I:%f A  V:%f  %f  %d\r\n",I,V,V/64,T);
//	if(I > Hinder_I)
//		T++;
//	else
//		T = 0;
//	if(T >= 5)    //���س���5�� �رյ��
//	{
//		T = 0;
//		return 1;
//	}

//	return 0;
//	
//}


//void ADC_task(void *pdata)
//{
//	u16 adcx;	
//	float V,I;
//	u8 T = 0;
//	u8 err,State;
//  u8 T_LED = 0;
//	u8 Flag_Over = 99;
//	u8 Key_Stop = 100;
//	u8 Flag = 0;
//	u16 N = 0;
//	while(1)
//	{
////		OSMboxPost(msg_LoadI,(void*)State);//������Ϣ ��ʼ���������
//			State=(u32)OSMboxPend(msg_LoadI,0,&err);   //ȥ�����ȡ��Ϣ �������һ��״̬
//		while(1)
//		{
//			delay_ms(3);
//			N++;
//			adcx=Get_Adc_Average(ADC_Channel_14,5);//1mS *5 = 5
//			V = adcx*3300.0/4096*2;//AD�ɼ�����ѹ
//			I = (V - 2600)/64; //�ɼ����ĵ�ѹ
//				printf("M_I:%f A  V:%f   %d \r\n",I,V,N);
//			if(State == 6)
//			{
////				if((N < 150)&&(N > 40))
////				{
////					if(I > 5)
////						T++;
////					if(T > 5)
////						Flag = 1;
////				}
//				
////				if((N > 100)&&(N < 120))
////				{
////					
////				}
//				
//				
//			}
//			
//			if(State == 14)
//			{
//				if((N < 170)&&(N > 20))
//				{
//					if(I > 3.5)
//						Flag = 1;
////						T++;
////					if(T > 2)
////						Flag = 1;
//				}
//				
//					
//			}
//			
////			  T = 0;
////				if(I > 4)
////				T++;
////				else
////					T = 0;
//				if(Flag)    //���س���5�� �رյ��
//				{
//					printf("************Over_I******************\r\n");
//					
//	//				if(State == 6)
//	//						State = 1;							
//	//				if(State == 15)
//	//					State = 10;
//					OSMboxPost(msg_keyout,(void*)Flag_Over);//������Ϣ  �������ٽ��� ״̬��־���ͻ�ȥ
//					OSMboxPost(msg_keyin,(void*)Key_Stop);			//�����ʼ�����λ����ֹͣɨ��
//					T = 0;
//					Flag = 0;
//					break;
//				}
//				if(Over_I_Flag)
//				{
//					printf("*****N %d*   %d\r\n",N,State);
//					N = 0;
//					break;
//				}
//				T_LED++;
//				if(T_LED == 4)
//				{
//					T_LED = 0;
//					LED3 = !LED3;
//				}
//			
//		}
//	
//	}
//}
