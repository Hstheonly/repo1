 /*********************************************************************************
 * 文件名  ：main.c
 * 描述    ：1.AD打印程序
 *           2.每个要转换的通道，采样时间建议尽量长一点，以获得较高的准确度，
 *   					 但是这样会降低 ADC 的转换速率
 *					 3.Tcovn=采样时间+12.5 个周期   Tcovn 为总转换时间
 *					   当 ADCCLK=14Mhz 的时候，并设置 1.5 个周期的采样时间，
 *						 则得到：Tcovn=1.5+12.5=14 个周期=1us。
 * 硬件    ：CH11 --> PC1
 * 注意    ：在调试移植过程中把主函数里的 float tmep 赋值 u32 temp 结果全都是0 
 *           调试了很时间终于发现了问题 由于赋值类型的不同结果不同
 * 移植说明：只要把adc.c 和 adc.h添加到所需要的工程里 NVIC_Configuration()配置中断组
 *           Adc_Init();初始化AD 在主函数通过 Get_Adc_Average(ADC_Channel_17,10)
 *           来读取AD的数据 ADC_Channel_17选择AD通道
 * 日期    ：2014 07 08 
**********************************************************************************/
#include "adc.h"
#include "delay.h"
#include "stm32f10x.h"
#include "usart.h"
/*****************************************************
 * 函数名：Adc_Init
 * 描述  ：初始化ADC  
 *				 以规则通道为例 
 *         默认将开启通道0~3	
 * 输入  ：无
 * 输出  ：无
 *       AD0-->PA7_ADC7 AD AD1-->PA6_ADC6 AD2-->PA5_ADC5 AD3-->PA4_ADC4 AD4-->PC4_ADC14 AD5-->PC5_ADC15
 ****************************************************/
 
void  Adc_Init(void)
{ 	
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_ADC1,ENABLE );	  //使能ADC1通道时钟
 	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M
		
  //PA4-7作为模拟通道输入引脚 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		       //模拟输入引脚
	GPIO_Init(GPIOA, &GPIO_InitStructure);		
	//PA1  作为模拟通道输入引脚                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		       //模拟输入引脚
	GPIO_Init(GPIOC, &GPIO_InitStructure);	

	ADC_DeInit(ADC1);                                      //复位ADC1,将外设 ADC1 的全部寄存器重设为缺省值

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	   //ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	         //模数转换工作在单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	   //模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; //ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;	               //顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC1, &ADC_InitStructure);	                   //根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器   

  ADC_Cmd(ADC1, ENABLE);	                    //使能指定的ADC1
	
	ADC_ResetCalibration(ADC1);	                //使能复位校准  
	 
	while(ADC_GetResetCalibrationStatus(ADC1)); //等待复位校准结束
	
	ADC_StartCalibration(ADC1);	                //开启AD校准
 
	while(ADC_GetCalibrationStatus(ADC1));	    //等待校准结束
}				  

/*****************************************************
 * 函数名：Get_Adc
 * 描述  ：获得ADC值
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
u16 Get_Adc(u8 ch)   
{
  //设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期	  			    
  
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		     //使能指定的ADC1的软件转换启动功能	
	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束

	return ADC_GetConversionValue(ADC1);	        //返回最近一次ADC1规则组的转换结果
}

/*****************************************************
 * 函数名：Get_Adc_Average
 * 描述  ：获得ADC平均值
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
u16 Get_Adc_Average(u8 ch,u8 times)
{
	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		temp_val+=Get_Adc(ch);//239.5+12.5 = 252周期  12M  252/12us = 21uS
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




//#define Unload_I 2600  //mV空载时 AC采集电压值
//#define Hinder_I 4 		 //A 超过这个值 关闭电机
//u8 Motor_I_Monitor(void)
//{
////	static u32 Sum = 0;
//	static u8 T = 0;
//	u16 HEX;
//	float V,I;
//	
//	HEX = Get_Adc_Average(ADC_Channel_14,10);
//	
//	V = HEX*3300.0/4096*2;//AD采集到电压
//	I = (V - Unload_I)/64; //采集到的电压
//	printf("M_I:%f A  V:%f  %f  %d\r\n",I,V,V/64,T);
//	if(I > Hinder_I)
//		T++;
//	else
//		T = 0;
//	if(T >= 5)    //过载超过5次 关闭电机
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
////		OSMboxPost(msg_LoadI,(void*)State);//发送消息 开始监测电机电流
//			State=(u32)OSMboxPend(msg_LoadI,0,&err);   //去邮箱读取消息 后进入下一个状态
//		while(1)
//		{
//			delay_ms(3);
//			N++;
//			adcx=Get_Adc_Average(ADC_Channel_14,5);//1mS *5 = 5
//			V = adcx*3300.0/4096*2;//AD采集到电压
//			I = (V - 2600)/64; //采集到的电压
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
//				if(Flag)    //过载超过5次 关闭电机
//				{
//					printf("************Over_I******************\r\n");
//					
//	//				if(State == 6)
//	//						State = 1;							
//	//				if(State == 15)
//	//					State = 10;
//					OSMboxPost(msg_keyout,(void*)Flag_Over);//发送消息  出发后再将此 状态标志发送回去
//					OSMboxPost(msg_keyin,(void*)Key_Stop);			//发送邮件让限位开关停止扫描
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
