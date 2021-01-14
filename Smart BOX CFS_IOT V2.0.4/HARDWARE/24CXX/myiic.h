/*********************************************************************************
 * 文件名  ：myiic.h
 * 描述    ：IIC的初始化 开始停止 发送 接受1字节等函数，标准的IIC协议可供外部调用
 * 移植说明：只要把myiic.c 和 myiic.h添加到所需要的工程里 把IIC_Init()初始化里面对应
 *           管脚修改一下，特别是.h里的操作寄存器的，调用库函数也可以，寄存器速度快
 * 硬件连接：SCL-->PB6  SDA-->PB7
 * 日期    ：2014 07 09 
**********************************************************************************/
#ifndef __MYIIC_H
#define __MYIIC_H
#include "sys.h"


//SCL ---> PC8
//SDA ---> PC7
#define	 SDA_A1     PCout(7)=1   //SDA输出
#define	 SDA_A0     PCout(7)=0
#define	 SCL_A1	    PCout(8)=1    //SCL
#define	 SCL_A0	    PCout(8)=0
#define	 SDA_AI	    PCin(7)    //SDA读入

//AT24C02 SDA--->PB7  SCL--->PB6
#define	 SDA_B1     PBout(7)=1   //SDA输出
#define	 SDA_B0     PBout(7)=0
#define	 SCL_B1	    PBout(6)=1    //SCL
#define	 SCL_B0	    PBout(6)=0
#define	 SDA_BI	    PBin(7)    //SDA读入


//#define	 SDA_A1     PC7=1   //SDA输出
//#define	 SDA_A0     PC7=0
//#define	 SCL_A1	    PC8=1    //SCL
//#define	 SCL_A0	    PC8=0
//#define	 SDA_AI	    PC7in   //SDA读入


//AT24C02 SDA--->PB7  SCL--->PB6
//IO方向设置
#define SDA_IN()  {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=8<<28;}
#define SDA_OUT() {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=3<<28;}

//IO操作函数	 
#define IIC_SCL    PBout(6) //SCL
#define IIC_SDA    PBout(7) //SDA	 
#define READ_SDA   PBin(7)  //输入SDA 




//IIC所有操作函数
void IIC_Init(void);                //初始化IIC的IO口				 
void IIC_Start(void);				        //发送IIC开始信号
void IIC_Stop(void);	  			      //发送IIC停止信号
void IIC_Send_Byte(u8 txd);			    //IIC发送一个字节
u8 IIC_Read_Byte(unsigned char ack);//IIC读取一个字节
u8 IIC_Wait_Ack(void); 				      //IIC等待ACK信号
void IIC_Ack(void);					        //IIC发送ACK信号
void IIC_NAck(void);				        //IIC不发送ACK信号

void IIC_Write_One_Byte(u8 daddr,u8 addr,u8 data);
u8 IIC_Read_One_Byte(u8 daddr,u8 addr);	  

void I2C_GPIO_Init(void);
u8 I2C_WriteNByte(u8 sla, u8 suba_type, u32 suba, u8 *s, u8 num);
u8 I2C_ReadNByte (u8 sla, u32 suba_type, u32 suba, u8 *s, u32 num);

u8 I2CB_WriteNByte(u8 sla, u8 suba_type, u32 suba, u8 *s, u8 num);
u8 I2CB_ReadNByte (u8 sla, u32 suba_type, u32 suba, u8 *s, u32 num);
#endif
















