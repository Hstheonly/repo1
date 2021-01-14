#ifndef __LED_H
#define __LED_H	 
#include "sys.h"
/*
 *  DI    ： ----------------------------------------------------------------------
 *         | IN1 -->PE2 IN2 -->PE3 IN3 -->PE4 IN4 -->PE6 IN5 -->PE6  IN6 -->PE7    |
 *          -----------------------------------------------------------------------
 *  D0    ： ----------------------------------------------------------------------
 *         | LED2 -->PD5  LED3 -->PD6 LED4 -->PD6                                  |
 *         | Q1 --> PE8  Q2 -->PE9       	                                         |
 *         | WDI --> PE10                                                          |
 *         | RS485_RE1 -->PE15                                                     |
 *          -----------------------------------------------------------------------
*/

/**************************************************************************************/
/*************************************底层驱动*****************************************/
/**************************************************************************************/
//M1_1 --> PE1 M1_2 -->PE0   M2_1 -->PB9  M2_2 -->PB8  M3_1 -->PB5 M3_2 -->PB4 低电平使能
#define M1_1 PEout(1) 
#define M1_2 PEout(0) 
#define M2_1 PBout(9) 
#define M2_2 PBout(8) 
#define M3_1 PBout(5) 
#define M3_2 PBout(4)

//继电器控制
#define JDQ1  			PBout(3)//
#define Q0          PEout(9)

//LED2 -->PD5  LED3 -->PD6 LED4 -->PD6  
#define LED1 PDout(5)// PD5	
#define LED2 PDout(6)// PD6	
#define LED3 PDout(7)// PD7	

//IO输入端口
#define DIO0  				PEin(2) //
#define DIO1  				PEin(3) //
#define DIO2  				PEin(4) //
#define DIO3 					PEin(5) //
#define DIO4  				PEin(6) //
#define DIO5  				PEin(7) //
#define DIO6  				PEin(8) //

//扩展板继电器驱动
//DO0-->PC2  DO1 -->PC3 DO2 -->PD15 DO3 -->PD14 DO4 -->PD13 DO5 -->PD12 DO6 -->PD11 DO7 -->PD10 DO8 -->PC1 高电平触发
#define DO0  PCout(2) 
#define DO1  PCout(3) 
#define DO2  PDout(15) 
#define DO3  PDout(14) 
#define DO4  PDout(13) 
#define DO5  PDout(12) 
#define DO6  PDout(11) 
#define DO7  PDout(10) 
#define DO8  PCout(1) 

#define DIO10  				PBin(0)  //IN1
#define DIO11  				PEin(11) //IN3
#define DIO12  				PBin(12) //IN5
#define DIO13 				PEin(12) //IN7
#define DIO14 				PBin(13) //IN9
#define DIO15 				PBin(14) //IN11
#define DIO16 				PEin(14) //IN2
#define DIO17 				PBin(15) //IN4
#define DIO18 				PBin(1)  //IN6
#define DIO19 				PEin(13) //IN8
#define DIO20 				PDin(9)  //IN10
#define DIO21 				PDin(8)  //IN12

#define WDI PEout(10)//WDI

#define RS485_1_RE PEout(15)
#define RS485_1_RX_EN PEout(15)=0
#define RS485_1_TX_EN PEout(15)=1









//IO输入端口
#define K1  				PEin(2) //
#define K2  				PEin(3) //
#define K3  				PEin(4) //
#define K4  				PEin(5) //
#define K5  				PEin(6) //
#define K6  				PEin(7) //






/**************************************************************************************/
/**************************************CON*********************************************/
/**************************************************************************************/

/**********补货REP**************/

//补货电机控制
//#define M_REP_0   PDout(5)        //刹车 0启动刹车 1取消刹车
//#define M_REP_1   PDout(5)        //启动停止 0停止 1启动
//#define M_REP_2   PDout(5)        //方向控制 0正转 1反转

#define M_REP_Start   PCout(2)        //启动停止 0停止 1启动   刹车 1启动刹车 0取消刹车    
#define M_REP_DIR     PCout(3)        //方向控制 1正转 0反转

//#define DO0  PCout(2) //通断
//#define DO1  PCout(3) //方向
//补货限位
/*
					左L      右R
							中M
*/
#define IN_REP_L  				PEin(2)  //左侧限位
#define IN_REP_M  				PEin(3)  //中点限位
#define IN_REP_R  				PEin(4)  //右侧限位
#define IN_REP_Z  				PEin(4)  //零点限位

#define IN_REP_K  				PEin(4)  //按键+门磁

/**********集装箱门CDR**************/
//门位置
/*
					3#
				1#2#		
*/
#define IN_CDR_1  				PBin(0)  //1#号门
#define IN_CDR_2  				PEin(11) //2#号门
#define IN_CDR_3  				PBin(12) //3#号门
//#define DIO10  				PBin(0)  //IN1
//#define DIO11  				PEin(11) //IN3
//#define DIO12  				PBin(12) //IN5


/**********安全门SDR**************/
//双扇门那一侧是1#门 单扇门一侧是2#门
//#define Q_SDR  			PBout(3)//
#define Q_SDR  			PDout(15)//
//#define Q_SDR_1   PBout(5)        //安全门是否可以打开使能
//#define Q_SDR_2   PBout(4)        //安全门是否可以打开使能
//#define JDQ1  			PBout(3)//

#define IN_SDR_1  				PEin(5) //1#号门
#define IN_SDR_2  				PEin(6) //2#号门
//#define DI3 				PEin(5) //
//#define DI4  				PEin(6) //
/**********Buffer BUF**************/
//从上到下依次是1#-3#

#define IN_BUF_1  				PEin(12) //1#上
#define IN_BUF_2  				PBin(13) //2#中
#define IN_BUF_3  				PBin(14) //3#下
//#define DIO13 				PEin(12) //IN7
//#define DIO14 				PBin(13) //IN9
//#define DIO15 				PBin(14) //IN11
/**********急停STP**************/
#define IN_STP  				  PEin(7) //1#号门  拍下 0  不拍1
//#define DI5  				PEin(7) //

/**********照明灯LGT*************/
#define IN_LGT_1  				  PEin(14) //1# 室内灯 
#define IN_LGT_2  				  PBin(15) //2# 室外照明1
#define IN_LGT_3  				  PBin(1)  //3# 室外照明2
#define IN_LGT_4  				  PEin(13) //4# 广告灯箱
#define IN_LGT_5  				  PDin(9)  //5# 购物区照明
//#define DIO16 						PEin(14) //IN2
//#define DIO17 						PBin(15) //IN4
//#define DIO18 						PBin(1)  //IN6
//#define DIO19 						PEin(13) //IN8
//#define DIO20 						PDin(9)  //IN10

#define Q_LGT_1  				   	PDout(11) //1# 室内灯 
#define Q_LGT_2  				  	PDout(10)  //2# 室外照明1
#define Q_LGT_3  				  	PDout(12) //3# 室外照明2
#define Q_LGT_4  				   	PDout(14)  //4# 广告灯箱
#define Q_LGT_5  				 		PDout(13)  //5# 购物区照明

//#define DO6  PDout(11)
//#define DO7  PDout(10)
//#define DO5  PDout(12)
//#define DO3  PDout(14) 
//#define DO4  PDout(13) 


/**************************************************************************************/
/**************************************IOT*********************************************/
/**************************************************************************************/







void Feed_WDG(void);	
void CON_IOT_GPIO_Init(void);
u16 CON_KEY_SCAN(void);
u8 REP_KEY_SCAN(void);//补货位状态扫描
u8 REP_BUT_SCAN(void);//补货位按键、门吸
u8 REP_KEY_Read_Min_POS(void);//补货位中间扫描
u8 CDR_KEY_SCAN(void);//集装箱门状态扫描
u8 SDR_KEY_SCAN(void);//安全门状态扫描
u8 BUF_KEY_SCAN(void);//Buffer状态扫描
u8 STP_KEY_SCAN(void);//急停按钮状态扫描
u16 LGT_KEY_SCAN(void);//扫描按键状态

u8 B_DIO_SCAN(void);
u8 A_DIO_SCAN(void);
#endif
