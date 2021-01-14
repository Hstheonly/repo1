 /*********************************************************************************
 * 文件名  ：main.c
 * 描述    ：1.AD打印程序
 *           2.每个要转换的通道，采样时间建议尽量长一点，以获得较高的准确度，
							 但是这样会降低 ADC 的转换速率
						 3.Tcovn=采样时间+12.5 个周期   Tcovn 为总转换时间
						   当 ADCCLK=14Mhz 的时候，并设置 1.5 个周期的采样时间，
							 则得到：Tcovn=1.5+12.5=14 个周期=1us。
 * 硬件    ：CH11 --> PC1
 * 移植说明：只要把adc.c 和 adc.h添加到所需要的工程里 NVIC_Configuration()配置中断组
 *           Adc_Init();初始化AD 在主函数通过 Get_Adc_Average(ADC_Channel_17,10)
 *           来读取AD的数据 ADC_Channel_17选择AD通道
 * 日期    ：2014 07 08 
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
 
u8 Motor_I_Monitor(void); //监测电流
u16 Draw_Calculate(u16 *Front_Data,u8 Fi, u16 adcx);//计算出最新10个数据平局值 
#endif 
