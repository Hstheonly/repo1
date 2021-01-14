 /*********************************************************************************
 * �ļ���  ��main.c
 * ����    ��1.AD��ӡ����
 *           2.ÿ��Ҫת����ͨ��������ʱ�佨�龡����һ�㣬�Ի�ýϸߵ�׼ȷ�ȣ�
							 ���������ή�� ADC ��ת������
						 3.Tcovn=����ʱ��+12.5 ������   Tcovn Ϊ��ת��ʱ��
						   �� ADCCLK=14Mhz ��ʱ�򣬲����� 1.5 �����ڵĲ���ʱ�䣬
							 ��õ���Tcovn=1.5+12.5=14 ������=1us��
 * Ӳ��    ��CH11 --> PC1
 * ��ֲ˵����ֻҪ��adc.c �� adc.h��ӵ�����Ҫ�Ĺ����� NVIC_Configuration()�����ж���
 *           Adc_Init();��ʼ��AD ��������ͨ�� Get_Adc_Average(ADC_Channel_17,10)
 *           ����ȡAD������ ADC_Channel_17ѡ��ADͨ��
 * ����    ��2014 07 08 
 *       AD0-->PA7_ADC7 AD AD1-->PA6_ADC6 AD2-->PA5_ADC5 AD3-->PA4_ADC4 AD4-->PC4_ADC14 AD5-->PC5_ADC15
**********************************************************************************/
#ifndef __ADC_H
#define __ADC_H	
#include "sys.h"

#define AD0 ADC_Channel_7
#define AD1 ADC_Channel_6
#define AD2 ADC_Channel_5
#define AD3 ADC_Channel_4
#define AD4 ADC_Channel_14
#define AD5 ADC_Channel_15
#define MCP_AD ADC_Channel_10


void Adc_Init(void);
u16  Get_Adc(u8 ch); 
u16 Get_Adc_Average(u8 ch,u8 times); 
 
u8 Motor_I_Monitor(void); //������
u16 Draw_Calculate(u16 *Front_Data,u8 Fi, u16 adcx);//���������10������ƽ��ֵ 
#endif 
