#include "GPIO.h"
#include "delay.h"
/*
 *  DI    ： ----------------------------------------------------------------------
 *         | IN1 -->PE2 IN2 -->PE3 IN3 -->PE4 IN4 -->PE6 IN5 -->PE6  IN6 -->PE7  IN7 -->PE8  |
 *          -----------------------------------------------------------------------
 *  D0    ： ----------------------------------------------------------------------
 *         | LED2 -->PD5  LED3 -->PD6 LED4 -->PD6                                  |
 *         | JQ1 --> PB5                                                           |
 *         | Q1  -->PE9       	                                                   |
 *         | WDI --> PE10                                                          |
 *         | RS485_RE1 -->PE15                                                     |
 
 *         | M1_1 --> PE1 M1_2 -->PE0   M2_1 -->PB9  M2_2 -->PB8  M3_1 -->PB5 M3_2 -->PB4  |
 *
 *  DI1    | IN3 --> PE11 IN7 --> PE12 IN11 -->PB14 IN4 -->PB15 IN8 -->PE13 IN12 -->PD8
						 IN1 -->PB0  IN5 -->PB12 IN9 -->PB13 IN2 -->PE14 IN6 -->PB1 IN10 -->PD9
	
						IN1 -->PB0 IN2 -->PE14 IN3 --> PE11 IN4 -->PB15 IN5 -->PB12 IN6 -->PB1
				&&&&IN10-->PB0 IN11-->PE14 IN12--> PE11 IN13-->PB15 IN14-->PB12 IN15-->PB1
				    IN7 --> PE12 IN8 -->PE13 IN9 -->PB13 IN10 -->PD9 IN11 -->PB14 IN12 -->PD8
				&&&&IN16--> PE12 IN17-->PE13 IN18-->PB13 IN19 -->PD9 IN20 -->PB14 IN21 -->PD8
						
		DO1     DQ8 -->PD10 DQ4 -->PD12 DQ7 -->PD14 DQ3 -->PC3 DQ9 -->PC1
		        DQ6 -->PD11 DQ2 -->PD13 DQ5 -->PD15 DQ1 -->PC2
						DQ1 -->PC2 DQ3 -->PC3 DQ5 -->PD15 DQ7 -->PD14 DQ2 -->PD13 DQ4 -->PD12 DQ6 -->PD11 DQ8 -->PD10  DQ9 -->PC1
				&&&&DO0-->PC2  DO1 -->PC3 DO2 -->PD15 DO3 -->PD14 DO4 -->PD13 DO5 -->PD12 DO6 -->PD11 DO7 -->PD10 DO8 -->PC1
						
 */
 
 u8 Min_POS_State = 1;
 
/**************************************************************
 * 函数名：LED_Init
 * 描述  ：LED初始化
 * 硬件  ：LED2 -->PD5  LED3 -->PD6 LED4 -->PD6  
 **************************************************************/ 
void LED_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);	 //使能PD端口时钟
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;				 	//LED端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 										//推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 										//IO口速度为50MHz
 GPIO_Init(GPIOD, &GPIO_InitStructure);					 													//根据设定参数初始化
 GPIO_SetBits(GPIOD,GPIO_Pin_5);						 															//输出高
}

/**************************************************************
 * 函数名：Q_Drive_Init
 * 描述  ：继电器驱动
 * 硬件  ：JQ1 --> PB5         低电平使能   
 **************************************************************/ 
void Q_Drive_Init(void)
{
 
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //使能PB端口时钟	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;				 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		  //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;		   //IO口速度为50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_3);
}

/**************************************************************
 * 函数名：M_Drive_Init
 * 描述  ：电机控制驱动初始化
 * 硬件  ：M1_1 --> PE1 M1_2 -->PE0   M2_1 -->PB9  M2_2 -->PB8  M3_1 -->PB5 M3_2 -->PB4 低电平使能   
 **************************************************************/ 
void M_Drive_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOE, ENABLE);	 //使能PD端口时钟
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
 GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); 
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;				 							//端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 										//推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 										//IO口速度为50MHz
 GPIO_Init(GPIOE, &GPIO_InitStructure);					 													//根据设定参数初始化
 GPIO_SetBits(GPIOE,GPIO_Pin_0|GPIO_Pin_1);						 										//输出高
// GPIO_ResetBits(GPIOE,GPIO_Pin_0|GPIO_Pin_1);						 										//输出高
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_8|GPIO_Pin_9;				 							//端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 										//推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 										//IO口速度为50MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);					 													//根据设定参数初始化
 GPIO_SetBits(GPIOB,GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_8|GPIO_Pin_9);						 										//输出高
//	GPIO_ResetBits(GPIOB,GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_8|GPIO_Pin_9);				
}
/**************************************************************
 * 函数名：B_JD_Drive_Init
 * 描述  ：扩展板继电器驱动
 * 硬件  ：DO0-->PC2  DO1-->PD13  DO2-->PC3   DO3-->PD12  DO4-->PD15  DO5-->PD11  DO6-->PD14   DO7-->PD10   DO8-->PC1        高电平使能   
 **************************************************************/ 
void B_JD_Drive_Init(void)
{
 
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD, ENABLE);	 //使能PB端口时钟	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;				 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		  //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;		   //IO口速度为50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOC,GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;				 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		  //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;		   //IO口速度为50MHz
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOD,GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
}



/**************************************************************
 * 函数名：KEY_Init
 * 描述  ：IO输入初始化
 * 硬件  ：IN1 -->PE2 IN2 -->PE3 IN3 -->PE4 IN4 -->PE6 IN5 -->PE6  IN6 -->PE7 
 **************************************************************/ 
void KEY_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);	 //使能PD端口时钟
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成下拉输
// 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 	GPIO_Init(GPIOE, &GPIO_InitStructure);
}
 
/**************************************************************
 * 函数名：B_KEY_Init
 * 描述  ：IO输入初始化
 * 硬件  ：          
 *	IN10-->PB0 IN15-->PB1 IN14-->PB12 IN18-->PB13 IN20 -->PB14 IN13-->PB15
 *	IN21 -->PD8 IN19 -->PD9 
 *	IN12--> PE11 IN16--> PE12 IN17-->PE13 IN11-->PE14
 **************************************************************/ 
void B_KEY_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOE, ENABLE);	 //使能PD端口时钟
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成下拉输
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_8|GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成下拉输
 	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成下拉输
 	GPIO_Init(GPIOE, &GPIO_InitStructure);
}

/**************************************************************
 * 函数名：WDI_GPIO_Init
 * 描述  ：看门狗IO初始化
 * 硬件  ：WDI --> PE10          
 **************************************************************/ 
void WDI_GPIO_Init(void)
{
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);	 //使能PD端口时钟
	
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;				 
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		  //推挽输出
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;		   //IO口速度为50MHz
GPIO_Init(GPIOE, &GPIO_InitStructure);
GPIO_SetBits(GPIOE,GPIO_Pin_10);
	
}
/**************************************************************
 * 函数名：Feed_WDG
 * 描述  ：喂狗
 **************************************************************/ 
void Feed_WDG(void)
{
	WDI = !WDI;
	delay_ms(10);
	WDI = !WDI;
}
/**************************************************************
 * 函数名：RS485_EN_Init
 * 描述  ：485使能IO口初始化
 * 硬件  ：WDI --> PE10          
 **************************************************************/ 
void RS485_EN_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);	 //使能PE端口时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;				 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		  	//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;		   	//IO口速度为50MHz
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOE,GPIO_Pin_15);
}

/**************************************************************
 * 函数名：CON_IOT_GPIO_Init
 * 描述  ：所有IO口初始化       
 **************************************************************/ 
void CON_IOT_GPIO_Init(void)
{
	LED_Init();
	Q_Drive_Init();
	M_Drive_Init();
	KEY_Init();
	WDI_GPIO_Init();
	RS485_EN_Init();
	
	B_JD_Drive_Init();
	B_KEY_Init();
}























/**************************************************************
 * 函数名：CON_KEY_SCAN
 * 描述  ：CON键盘扫描
 * 硬件  ：IN1 -->PE2 IN2 -->PE3 IN3 -->PE4 IN4 -->PE6 IN5 -->PE6  IN6 -->PE7 
 * 输出  : 按位  X X K6 K5    K4 K3 K2 K1
 **************************************************************/ 
u16 CON_KEY_SCAN(void)
{
	u16 Value = 0;
	if(K1 ==0)
	{
		delay_ms(2);
		if(K1 ==0)
		{
			Value+=1;
		}
	}
	
	
	if(K2 ==0)
	{
		delay_ms(2);
		if(K2 ==0)
		{
			Value+=2;
		}
	}
	
	if(K3 ==0)
	{
		delay_ms(2);
		if(K3 ==0)
		{
			Value+=4;
		}
	}
	
	if(K4 ==0)
	{
		delay_ms(2);
		if(K4 ==0)
		{
			Value+=8;
		}
	}
	
	if(K5 ==0)
	{
		delay_ms(2);
		if(K5 ==0)
		{
			Value+=16;
		}
	}
	
	if(K6 ==0)
	{
		delay_ms(2);
		if(K6 ==0)
		{
			Value+=32;
		}
	}
	
	return Value;
}

/**************************************************************
 * 函数名：REP_KEY_SCAN
 * 描述  ：补货位 限位开关扫描
 * 硬件  ：IN_REP_L -->PE2 IN_REP_M -->PE3 IN_REP_R -->PE4 
 * 输出  : 按位  0 IN_REP_R IN_REP_M IN_REP_L
 **************************************************************/ 
u8 REP_KEY_SCAN(void)
{
		u16 Value = 0;
		if(IN_REP_L ==0)
		{
			delay_ms(0);
			if(IN_REP_L ==0)
			{
				Value+=1;
			}
		}
		
		
		if(IN_REP_M ==0)
		{
			delay_ms(2);
			if(IN_REP_M ==0)
			{
				Value+=2;
			}
		}
		
		if(IN_REP_R ==0)
		{
			delay_ms(2);
			if(IN_REP_R ==0)
			{
				Value+=4;
			}
		}
		
		if(IN_REP_Z ==0)
		{
			delay_ms(2);
			if(IN_REP_Z ==0)
			{
				Value+=8;
			}
		}
		return Value;
}

/**************************************************************
 * 函数名：REP_BUT_SCAN
 * 描述  ：按钮扫描
 * 硬件  ：IN_REP_K
 * 输出  : 触发输出1 否则0
 **************************************************************/ 
u8 REP_BUT_SCAN(void)
{
		u16 Value = 0;
		if(IN_REP_K ==0)
		{
			delay_ms(0);
			if(IN_REP_K ==0)
			{
				Value+=1;
			}
		}
		
		return Value;
}

/**************************************************************
 * 函数名：REP_KEY_Read_Min_POS
 * 描述  ：中间为传感器检测
 * 硬件  ：IN_REP_M -->PE3 
 * 输出  : 上升沿 返回1 下降沿返回2 其他返回0
 **************************************************************/ 
u8 REP_KEY_Read_Min_POS(void)
{
	u8 Value = 1;
	
	if(Min_POS_State == 0)
	{
			if(IN_REP_M ==0)
			{
//				delay_ms(2);
				if(IN_REP_M ==0)
				{
					Value = 0;
					Min_POS_State = 0;
				}
			}
			else
			{
				if(IN_REP_M ==1)
				{
//					delay_ms(2);
					if(IN_REP_M ==1)
					{
						Value = 1;
						Min_POS_State = 1;
					}
				}
			}
	  
	}
	else
	{
		if(Min_POS_State == 1)
		{	
			if(IN_REP_M ==1)
			{
//				delay_ms(2);
				if(IN_REP_M ==1)
				{
					Value = 0;
					Min_POS_State = 1;
				}
			}
			else
			{
				if(IN_REP_M ==0)
				{
//					delay_ms(2);
					if(IN_REP_M ==0)
					{
						Value = 2;
						Min_POS_State = 0;
					}
				}
			}		
			
		}
	
	}
	
	return Value;	
}	

/**************************************************************
 * 函数名：CDR_KEY_SCAN
 * 描述  ：集装箱门
 * 硬件  ：IN_CDR_1 -->PE2 IN_CDR_2 -->PE3 IN_CDR_3 -->PE4 
 * 输出  : 按位  0 IN_CDR_1 IN_CDR_2 IN_CDR_3
 **************************************************************/ 
u8 CDR_KEY_SCAN(void)
{
		u16 Value = 0;
		if(IN_CDR_1 ==0)
		{
			delay_ms(2);
			if(IN_CDR_1 ==0)
			{
				Value+=1;
			}
		}
		
		
		if(IN_CDR_2 ==0)
		{
			delay_ms(2);
			if(IN_CDR_2 ==0)
			{
				Value+=2;
			}
		}
		
		if(IN_CDR_3 ==0)
		{
			delay_ms(2);
			if(IN_CDR_3 ==0)
			{
				Value+=4;
			}
		}
		return Value;
}

/**************************************************************
 * 函数名：SDR_KEY_SCAN
 * 描述  ：安全门
 * 硬件  ：IN_SDR_1 -->PE2 IN_SDR_2 -->PE3 
 * 输出  : 按位  0 IN_CDR_1 IN_CDR_2 IN_CDR_3
 **************************************************************/ 
u8 SDR_KEY_SCAN(void)
{
		u16 Value = 0;
		if(IN_SDR_1 ==0)
		{
			delay_ms(2);
			if(IN_SDR_1 ==0)
			{
				Value+=1;
			}
		}
		
		
		if(IN_SDR_2 ==0)
		{
			delay_ms(2);
			if(IN_SDR_2 ==0)
			{
				Value+=2;
			}
		}
		
		return Value;
}

/**************************************************************
 * 函数名：BUF_KEY_SCAN
 * 描述  ：Buffer 状态
 * 硬件  ：IN_BUF_1 -->PE2 IN_BUF_2 -->PE3 IN_BUF_3 -->PE4 
 * 输出  : 按位  0 IN_BUF_1 IN_BUF_2 IN_BUF_3
 **************************************************************/ 
u8 BUF_KEY_SCAN(void)
{
		u16 Value = 0;
		if(IN_BUF_1 ==0)
		{
			delay_ms(2);
			if(IN_BUF_1 ==0)
			{
				Value+=1;
			}
		}		
		
		if(IN_BUF_2 ==0)
		{
			delay_ms(2);
			if(IN_BUF_2 ==0)
			{
				Value+=2;
			}
		}
		
		if(IN_BUF_3 ==0)
		{
			delay_ms(2);
			if(IN_BUF_3 ==0)
			{
				Value+=4;
			}
		}
		return Value;
}

/**************************************************************
 * 函数名：STP_KEY_SCAN
 * 描述  ：急停按钮
 * 硬件  ：IN_STP -->PE2 
 * 输出  : 按位  0 IN_STP
 **************************************************************/ 
u8 STP_KEY_SCAN(void)
{
		u16 Value = 0;
		if(IN_STP ==0)
		{
			delay_ms(2);
			if(IN_STP ==0)
			{
				Value+=1;
			}
		}
		
		return Value;
}




/**************************************************************
 * 函数名：LGT_KEY_SCAN
 * 描述  ：集装箱门
 * 硬件  ：IN_LGT_1 -->PE2 IN_LGT_2 -->PE3 IN_LGT_3 -->PE4  IN_LGT_4 -->PE5 IN_LGT_5 -->PE6
 * 输出  : 按位  0 IN_LGT_1 IN_LGT_2 IN_LGT_3 IN_LGT_4 IN_LGT_5
 **************************************************************/ 
u16 LGT_KEY_SCAN(void)
{
		u16 Value = 0;
		if(IN_LGT_1 ==0)
		{
			delay_ms(20);
			if(IN_LGT_1 ==0)
			{
				Value+=1;
			}
		}
		
		
		if(IN_LGT_2 ==0)
		{
			delay_ms(20);
			if(IN_LGT_2 ==0)
			{
				Value+=2;
			}
		}
		
		if(IN_LGT_3 ==0)
		{
			delay_ms(20);
			if(IN_LGT_3 ==0)
			{
				Value+=4;
			}
		}
		
		if(IN_LGT_4 ==0)
		{
			delay_ms(20);
			if(IN_LGT_4 ==0)
			{
				Value+=8;
			}
		}
		
		if(IN_LGT_5 ==0)
		{
			delay_ms(20);
			if(IN_LGT_5 ==0)
			{
				Value+=16;
			}
		}
		return Value;
}


/**************************************************************
 * 函数名：STP_KEY_SCAN
 * 描述  ：急停按钮
 * 硬件  ：IN_STP -->PE2 
 * 输出  : 按位  0 IN_STP
 **************************************************************/ 
u8 B_DIO_SCAN(void)
{
		u16 Value = 0;
		if(DIO10 ==0)
		{
			delay_ms(2);
			if(DIO10 ==0)
			{
				Value=10;
			}
		}
		if(DIO11 ==0)
		{
			delay_ms(2);
			if(DIO11 ==0)
			{
				Value=11;
			}
		}
		if(DIO12 ==0)
		{
			delay_ms(2);
			if(DIO12 ==0)
			{
				Value=12;
			}
		}
		if(DIO13 ==0)
		{
			delay_ms(2);
			if(DIO13 ==0)
			{
				Value=13;
			}
		}
		if(DIO14 ==0)
		{
			delay_ms(2);
			if(DIO14 ==0)
			{
				Value=14;
			}
		}
		if(DIO15 ==0)
		{
			delay_ms(2);
			if(DIO15 ==0)
			{
				Value=15;
			}
		}
		if(DIO16 ==0)
		{
			delay_ms(2);
			if(DIO16 ==0)
			{
				Value=16;
			}
		}
		if(DIO17 ==0)
		{
			delay_ms(2);
			if(DIO17 ==0)
			{
				Value=17;
			}
		}
		if(DIO18 ==0)
		{
			delay_ms(2);
			if(DIO18 ==0)
			{
				Value=18;
			}
		}
		if(DIO19 ==0)
		{
			delay_ms(2);
			if(DIO19 ==0)
			{
				Value=19;
			}
		}
		if(DIO20 ==0)
		{
			delay_ms(2);
			if(DIO20 ==0)
			{
				Value=20;
			}
		}
		if(DIO21 ==0)
		{
			delay_ms(2);
			if(DIO21 ==0)
			{
				Value=21;
			}
		}
		
		
		
		return Value;
}

/**************************************************************
 * 函数名：STP_KEY_SCAN
 * 描述  ：急停按钮
 * 硬件  ：IN_STP -->PE2 
 * 输出  : 按位  0 IN_STP
 **************************************************************/ 
u8 A_DIO_SCAN(void)
{
		u16 Value = 0;
		if(DIO0 ==0)
		{
			delay_ms(2);
			if(DIO0 ==0)
			{
				Value=10;
			}
		}
		if(DIO1 ==0)
		{
			delay_ms(2);
			if(DIO1 ==0)
			{
				Value=1;
			}
		}
		if(DIO2 ==0)
		{
			delay_ms(2);
			if(DIO2 ==0)
			{
				Value=2;
			}
		}
		if(DIO3 ==0)
		{
			delay_ms(2);
			if(DIO3 ==0)
			{
				Value=3;
			}
		}
		if(DIO4 ==0)
		{
			delay_ms(2);
			if(DIO4 ==0)
			{
				Value=4;
			}
		}
		if(DIO5 ==0)
		{
			delay_ms(2);
			if(DIO5 ==0)
			{
				Value=5;
			}
		}
		
			if(DIO6 ==0)
		{
			delay_ms(2);
			if(DIO6 ==0)
			{
				Value=6;
			}
		}
		
		
		
		
		return Value;
}