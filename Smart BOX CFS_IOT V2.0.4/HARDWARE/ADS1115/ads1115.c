#include "ads1115.h"
#include "sys.h"
#include "myiic.h"
#include "delay.h"

#define  ADS1115_ADD  0x90
#define ONE_BYTE_SUBA 0x01

/*****************************************************
 * ������: InitADS1115
 * ����  ������ADS1115 
 * ����  ��MUX ��0 �ɼ�ѹ�������������ѹ
 *         MUX : 1 �ɼ��¶ȴ�������ѹ
 *         MUX ��2 �ɼ���Դ��ѹ
 * ���  ��1:�ɹ� 0 ��ʧ��
 ****************************************************/
 /*****************************************************
 *          	  AIN0-GND ���λ�ǵ�ѹ		128SPS 
 *   0x00:0.256 0x01:0.512 0x02:1.024 0x03:2.048 0x04:4.096 0x05:6.144 *
 *           		AIN1-GND ���� ��λ��ѹ��	
 *              0x10: 0.256 
 *          	  AIN2-GND ���� ��λ��ѹ��	�Ŵ�20��	 
 *              0x20: 4.096
 *           		AIN3-GND �αȵ�λ
 *              0x30: 4.096
 ****************************************************/
u8 InitADS1115(uint8_t MUX)
{	
	u8 Writebuf[2];
	u16 AD_Config = 0;
	
	switch(MUX){
		case 0x00:   AD_Config = MUX_4+  PGA_5   +DR_4+COMP_QUE_3;//AIN0-GND   		0.256  128SPS								   
							break;
		case 0x01:   AD_Config = MUX_4+  PGA_4   +DR_4+COMP_QUE_3;//AIN0-GND  		0.512  128SPS								   
							break;
		case 0x02:   AD_Config = MUX_4+  PGA_3   +DR_4+COMP_QUE_3;//AIN0-GND  		1.024  128SPS								   
							break;
		case 0x03:   AD_Config = MUX_4+  PGA_2   +DR_4+COMP_QUE_3;//AIN0-GND   		2.048  128SPS								   
							break;
		case 0x04:   AD_Config = MUX_4+  PGA_1   +DR_4+COMP_QUE_3;//AIN0-GND  		4.096  128SPS								   
							break;
		case 0x05:   AD_Config = MUX_4+  PGA_0   +DR_4+COMP_QUE_3;//AIN0-GND  		6.114  128SPS								   
							break;
		
		case 0x10:   AD_Config = MUX_5+PGA_5+DR_4+COMP_QUE_3; //AIN1-GND 	0.256  128SPS			
							break;
		case 0x11:   AD_Config = MUX_5+PGA_4+DR_4+COMP_QUE_3; //AIN1-GND 	0.512  128SPS			
							break;
		case 0x13:   AD_Config = MUX_5+PGA_2+DR_4+COMP_QUE_3; //AIN1-GND 	2.048  128SPS			
							break;
		
		case 0x20:   AD_Config = MUX_6+PGA_1+DR_4+COMP_QUE_3; //AIN2-GND 	4.096  128SPS			
							break;
		case 0x23:   AD_Config = MUX_6+PGA_2+DR_4+COMP_QUE_3; //AIN2-GND 	2.048  128SPS			
							break;
		
		case 0x30:   AD_Config = MUX_7+PGA_1+DR_4+COMP_QUE_3; //AIN3-GND 	4.096  128SPS			
							break;
		case 0x33:   AD_Config = MUX_7+PGA_2+DR_4+COMP_QUE_3; //AIN3-GND 	2.048  128SPS			
							break;

		default:	break;						
	  
					}
  Writebuf[0]= AD_Config/256;     
  Writebuf[1]= AD_Config%256;           
	return I2C_WriteNByte(ADS1115_ADD, ONE_BYTE_SUBA, Pointer_1, Writebuf, 2);//д��������Ϣ         
}

/*****************************************************
 * ������: Read_AD_Average
 * ����  ��ѡ��ģʽ��ȡAD����
 * ģʽ  :   	  AIN0-GND ������ѹ 		1.024  128SPS
 *           		AIN1-GND ͨ���λ 		4.096  128SPS
 *              AIN1-GND �ϵ��λ 		4.096  860SPS
 *          	  AIN2-GND ��Ȼ��λ 		4.096  128SPS
 *           		AIN3-GND ����̽ͷ���� 0.256  128SPS 
 * ����  ��Mode ģʽѡ�� times ȡƽ��ֵ����
 * ���  ����ѹֵ
 ****************************************************/
u16 Read_AD_Average(u8 Mode,u8 times)
{
	u8 Read_Data[2] = {0x00};
	u32 Sum = 0;
	u8 i = 0;
	
	InitADS1115(Mode);  //ѡ����ģʽ ͨ�� AD��ѹ��Χ
	//ȥ������ȥƽ��ֵ
	I2C_ReadNByte (ADS1115_ADD, ONE_BYTE_SUBA, 0x00, Read_Data, 2);
	delay_ms(10);

	I2C_ReadNByte (ADS1115_ADD, ONE_BYTE_SUBA, 0x00, Read_Data, 2);
	delay_ms(10);

	
	//�ɼ�����ȡƽ��ֵ
	for(i = 0;i < times;i++)
	{
		delay_ms(10);       //�ٶ� 128SPS Ӧ���ӳ�7.8mS
		I2C_ReadNByte (ADS1115_ADD, ONE_BYTE_SUBA, 0x00, Read_Data, 2);
		if((Read_Data[0]*256+Read_Data[1]) < 0x7FFF)
			Sum += Read_Data[0]*256+Read_Data[1];	
	

	}	

	return Sum/times;
}

/*****************************************************
 * ������: Read_AD_Average
 * ����  ��ѡ��ģʽ��ȡAD����
 * ģʽ  : AIN0-AIN1 0.512V  8SPS ����ģʽ  
 * ����  ��Mode ģʽѡ�� times ȡƽ��ֵ����
 * ���  ����ѹֵ
 ****************************************************/
u16 Read_FAST_AD_Average(u8 Mode,u8 times)
{
	u8 Read_Data[2] = {0x00};
	u32 Sum = 0;
	u8 i = 0;
	
	InitADS1115(Mode);  //ѡ����ģʽ ͨ�� AD��ѹ��Χ
	//ȥ������ȥƽ��ֵ
	I2C_ReadNByte (ADS1115_ADD, ONE_BYTE_SUBA, 0x00, Read_Data, 2);
	delay_ms(1);

	I2C_ReadNByte (ADS1115_ADD, ONE_BYTE_SUBA, 0x00, Read_Data, 2);
	delay_ms(1);
//	WDTFeed();
	
	//�ɼ�����ȡƽ��ֵ
	for(i = 0;i < times;i++)
	{
		delay_ms(1);       
		I2C_ReadNByte (ADS1115_ADD, ONE_BYTE_SUBA, 0x00, Read_Data, 2);
		if((Read_Data[0]*256+Read_Data[1]) < 0x7FFF)
			Sum += Read_Data[0]*256+Read_Data[1];	
	
//		WDTFeed();
	}	

	return Sum/times;
}



