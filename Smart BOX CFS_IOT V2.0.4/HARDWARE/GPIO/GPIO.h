#ifndef __LED_H
#define __LED_H	 
#include "sys.h"
/*
 *  DI    �� ----------------------------------------------------------------------
 *         | IN1 -->PE2 IN2 -->PE3 IN3 -->PE4 IN4 -->PE6 IN5 -->PE6  IN6 -->PE7    |
 *          -----------------------------------------------------------------------
 *  D0    �� ----------------------------------------------------------------------
 *         | LED2 -->PD5  LED3 -->PD6 LED4 -->PD6                                  |
 *         | Q1 --> PE8  Q2 -->PE9       	                                         |
 *         | WDI --> PE10                                                          |
 *         | RS485_RE1 -->PE15                                                     |
 *          -----------------------------------------------------------------------
*/

/**************************************************************************************/
/*************************************�ײ�����*****************************************/
/**************************************************************************************/
//M1_1 --> PE1 M1_2 -->PE0   M2_1 -->PB9  M2_2 -->PB8  M3_1 -->PB5 M3_2 -->PB4 �͵�ƽʹ��
#define M1_1 PEout(1) 
#define M1_2 PEout(0) 
#define M2_1 PBout(9) 
#define M2_2 PBout(8) 
#define M3_1 PBout(5) 
#define M3_2 PBout(4)

//�̵�������
#define JDQ1  			PBout(3)//
#define Q0          PEout(9)

//LED2 -->PD5  LED3 -->PD6 LED4 -->PD6  
#define LED1 PDout(5)// PD5	
#define LED2 PDout(6)// PD6	
#define LED3 PDout(7)// PD7	

//IO����˿�
#define DIO0  				PEin(2) //
#define DIO1  				PEin(3) //
#define DIO2  				PEin(4) //
#define DIO3 					PEin(5) //
#define DIO4  				PEin(6) //
#define DIO5  				PEin(7) //
#define DIO6  				PEin(8) //

//��չ��̵�������
//DO0-->PC2  DO1 -->PC3 DO2 -->PD15 DO3 -->PD14 DO4 -->PD13 DO5 -->PD12 DO6 -->PD11 DO7 -->PD10 DO8 -->PC1 �ߵ�ƽ����
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









//IO����˿�
#define K1  				PEin(2) //
#define K2  				PEin(3) //
#define K3  				PEin(4) //
#define K4  				PEin(5) //
#define K5  				PEin(6) //
#define K6  				PEin(7) //






/**************************************************************************************/
/**************************************CON*********************************************/
/**************************************************************************************/

/**********����REP**************/

//�����������
//#define M_REP_0   PDout(5)        //ɲ�� 0����ɲ�� 1ȡ��ɲ��
//#define M_REP_1   PDout(5)        //����ֹͣ 0ֹͣ 1����
//#define M_REP_2   PDout(5)        //������� 0��ת 1��ת

#define M_REP_Start   PCout(2)        //����ֹͣ 0ֹͣ 1����   ɲ�� 1����ɲ�� 0ȡ��ɲ��    
#define M_REP_DIR     PCout(3)        //������� 1��ת 0��ת

//#define DO0  PCout(2) //ͨ��
//#define DO1  PCout(3) //����
//������λ
/*
					��L      ��R
							��M
*/
#define IN_REP_L  				PEin(2)  //�����λ
#define IN_REP_M  				PEin(3)  //�е���λ
#define IN_REP_R  				PEin(4)  //�Ҳ���λ
#define IN_REP_Z  				PEin(4)  //�����λ

#define IN_REP_K  				PEin(4)  //����+�Ŵ�

/**********��װ����CDR**************/
//��λ��
/*
					3#
				1#2#		
*/
#define IN_CDR_1  				PBin(0)  //1#����
#define IN_CDR_2  				PEin(11) //2#����
#define IN_CDR_3  				PBin(12) //3#����
//#define DIO10  				PBin(0)  //IN1
//#define DIO11  				PEin(11) //IN3
//#define DIO12  				PBin(12) //IN5


/**********��ȫ��SDR**************/
//˫������һ����1#�� ������һ����2#��
//#define Q_SDR  			PBout(3)//
#define Q_SDR  			PDout(15)//
//#define Q_SDR_1   PBout(5)        //��ȫ���Ƿ���Դ�ʹ��
//#define Q_SDR_2   PBout(4)        //��ȫ���Ƿ���Դ�ʹ��
//#define JDQ1  			PBout(3)//

#define IN_SDR_1  				PEin(5) //1#����
#define IN_SDR_2  				PEin(6) //2#����
//#define DI3 				PEin(5) //
//#define DI4  				PEin(6) //
/**********Buffer BUF**************/
//���ϵ���������1#-3#

#define IN_BUF_1  				PEin(12) //1#��
#define IN_BUF_2  				PBin(13) //2#��
#define IN_BUF_3  				PBin(14) //3#��
//#define DIO13 				PEin(12) //IN7
//#define DIO14 				PBin(13) //IN9
//#define DIO15 				PBin(14) //IN11
/**********��ͣSTP**************/
#define IN_STP  				  PEin(7) //1#����  ���� 0  ����1
//#define DI5  				PEin(7) //

/**********������LGT*************/
#define IN_LGT_1  				  PEin(14) //1# ���ڵ� 
#define IN_LGT_2  				  PBin(15) //2# ��������1
#define IN_LGT_3  				  PBin(1)  //3# ��������2
#define IN_LGT_4  				  PEin(13) //4# ������
#define IN_LGT_5  				  PDin(9)  //5# ����������
//#define DIO16 						PEin(14) //IN2
//#define DIO17 						PBin(15) //IN4
//#define DIO18 						PBin(1)  //IN6
//#define DIO19 						PEin(13) //IN8
//#define DIO20 						PDin(9)  //IN10

#define Q_LGT_1  				   	PDout(11) //1# ���ڵ� 
#define Q_LGT_2  				  	PDout(10)  //2# ��������1
#define Q_LGT_3  				  	PDout(12) //3# ��������2
#define Q_LGT_4  				   	PDout(14)  //4# ������
#define Q_LGT_5  				 		PDout(13)  //5# ����������

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
u8 REP_KEY_SCAN(void);//����λ״̬ɨ��
u8 REP_BUT_SCAN(void);//����λ����������
u8 REP_KEY_Read_Min_POS(void);//����λ�м�ɨ��
u8 CDR_KEY_SCAN(void);//��װ����״̬ɨ��
u8 SDR_KEY_SCAN(void);//��ȫ��״̬ɨ��
u8 BUF_KEY_SCAN(void);//Buffer״̬ɨ��
u8 STP_KEY_SCAN(void);//��ͣ��ť״̬ɨ��
u16 LGT_KEY_SCAN(void);//ɨ�谴��״̬

u8 B_DIO_SCAN(void);
u8 A_DIO_SCAN(void);
#endif
