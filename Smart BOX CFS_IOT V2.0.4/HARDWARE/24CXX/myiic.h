/*********************************************************************************
 * �ļ���  ��myiic.h
 * ����    ��IIC�ĳ�ʼ�� ��ʼֹͣ ���� ����1�ֽڵȺ�������׼��IICЭ��ɹ��ⲿ����
 * ��ֲ˵����ֻҪ��myiic.c �� myiic.h��ӵ�����Ҫ�Ĺ����� ��IIC_Init()��ʼ�������Ӧ
 *           �ܽ��޸�һ�£��ر���.h��Ĳ����Ĵ����ģ����ÿ⺯��Ҳ���ԣ��Ĵ����ٶȿ�
 * Ӳ�����ӣ�SCL-->PB6  SDA-->PB7
 * ����    ��2014 07 09 
**********************************************************************************/
#ifndef __MYIIC_H
#define __MYIIC_H
#include "sys.h"


//SCL ---> PC8
//SDA ---> PC7
#define	 SDA_A1     PCout(7)=1   //SDA���
#define	 SDA_A0     PCout(7)=0
#define	 SCL_A1	    PCout(8)=1    //SCL
#define	 SCL_A0	    PCout(8)=0
#define	 SDA_AI	    PCin(7)    //SDA����

//AT24C02 SDA--->PB7  SCL--->PB6
#define	 SDA_B1     PBout(7)=1   //SDA���
#define	 SDA_B0     PBout(7)=0
#define	 SCL_B1	    PBout(6)=1    //SCL
#define	 SCL_B0	    PBout(6)=0
#define	 SDA_BI	    PBin(7)    //SDA����


//#define	 SDA_A1     PC7=1   //SDA���
//#define	 SDA_A0     PC7=0
//#define	 SCL_A1	    PC8=1    //SCL
//#define	 SCL_A0	    PC8=0
//#define	 SDA_AI	    PC7in   //SDA����


//AT24C02 SDA--->PB7  SCL--->PB6
//IO��������
#define SDA_IN()  {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=8<<28;}
#define SDA_OUT() {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=3<<28;}

//IO��������	 
#define IIC_SCL    PBout(6) //SCL
#define IIC_SDA    PBout(7) //SDA	 
#define READ_SDA   PBin(7)  //����SDA 




//IIC���в�������
void IIC_Init(void);                //��ʼ��IIC��IO��				 
void IIC_Start(void);				        //����IIC��ʼ�ź�
void IIC_Stop(void);	  			      //����IICֹͣ�ź�
void IIC_Send_Byte(u8 txd);			    //IIC����һ���ֽ�
u8 IIC_Read_Byte(unsigned char ack);//IIC��ȡһ���ֽ�
u8 IIC_Wait_Ack(void); 				      //IIC�ȴ�ACK�ź�
void IIC_Ack(void);					        //IIC����ACK�ź�
void IIC_NAck(void);				        //IIC������ACK�ź�

void IIC_Write_One_Byte(u8 daddr,u8 addr,u8 data);
u8 IIC_Read_One_Byte(u8 daddr,u8 addr);	  

void I2C_GPIO_Init(void);
u8 I2C_WriteNByte(u8 sla, u8 suba_type, u32 suba, u8 *s, u8 num);
u8 I2C_ReadNByte (u8 sla, u32 suba_type, u32 suba, u8 *s, u32 num);

u8 I2CB_WriteNByte(u8 sla, u8 suba_type, u32 suba, u8 *s, u8 num);
u8 I2CB_ReadNByte (u8 sla, u32 suba_type, u32 suba, u8 *s, u32 num);
#endif
















