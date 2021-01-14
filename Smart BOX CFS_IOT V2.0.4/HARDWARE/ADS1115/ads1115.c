#include "ads1115.h"
#include "sys.h"
#include "myiic.h"
#include "delay.h"

#define  ADS1115_ADD  0x90
#define ONE_BYTE_SUBA 0x01

/*****************************************************
 * 函数名: InitADS1115
 * 描述  ：配置ADS1115 
 * 输入  ：MUX ：0 采集压力传感器输出电压
 *         MUX : 1 采集温度传感器电压
 *         MUX ：2 采集电源电压
 * 输出  ：1:成功 0 ：失败
 ****************************************************/
 /*****************************************************
 *          	  AIN0-GND 恒电位仪电压		128SPS 
 *   0x00:0.256 0x01:0.512 0x02:1.024 0x03:2.048 0x04:4.096 0x05:6.144 *
 *           		AIN1-GND 阴极 零位电压差	
 *              0x10: 0.256 
 *          	  AIN2-GND 阴极 零位电压差	放大20倍	 
 *              0x20: 4.096
 *           		AIN3-GND 参比电位
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
	return I2C_WriteNByte(ADS1115_ADD, ONE_BYTE_SUBA, Pointer_1, Writebuf, 2);//写入配置信息         
}

/*****************************************************
 * 函数名: Read_AD_Average
 * 描述  ：选择模式读取AD数据
 * 模式  :   	  AIN0-GND 交流电压 		1.024  128SPS
 *           		AIN1-GND 通电电位 		4.096  128SPS
 *              AIN1-GND 断电电位 		4.096  860SPS
 *          	  AIN2-GND 自然电位 		4.096  128SPS
 *           		AIN3-GND 极化探头电流 0.256  128SPS 
 * 输入  ：Mode 模式选择 times 取平均值次数
 * 输出  ：电压值
 ****************************************************/
u16 Read_AD_Average(u8 Mode,u8 times)
{
	u8 Read_Data[2] = {0x00};
	u32 Sum = 0;
	u8 i = 0;
	
	InitADS1115(Mode);  //选择工作模式 通道 AD电压范围
	//去掉两次去平均值
	I2C_ReadNByte (ADS1115_ADD, ONE_BYTE_SUBA, 0x00, Read_Data, 2);
	delay_ms(10);

	I2C_ReadNByte (ADS1115_ADD, ONE_BYTE_SUBA, 0x00, Read_Data, 2);
	delay_ms(10);

	
	//采集数据取平均值
	for(i = 0;i < times;i++)
	{
		delay_ms(10);       //速度 128SPS 应该延迟7.8mS
		I2C_ReadNByte (ADS1115_ADD, ONE_BYTE_SUBA, 0x00, Read_Data, 2);
		if((Read_Data[0]*256+Read_Data[1]) < 0x7FFF)
			Sum += Read_Data[0]*256+Read_Data[1];	
	

	}	

	return Sum/times;
}

/*****************************************************
 * 函数名: Read_AD_Average
 * 描述  ：选择模式读取AD数据
 * 模式  : AIN0-AIN1 0.512V  8SPS 连续模式  
 * 输入  ：Mode 模式选择 times 取平均值次数
 * 输出  ：电压值
 ****************************************************/
u16 Read_FAST_AD_Average(u8 Mode,u8 times)
{
	u8 Read_Data[2] = {0x00};
	u32 Sum = 0;
	u8 i = 0;
	
	InitADS1115(Mode);  //选择工作模式 通道 AD电压范围
	//去掉两次去平均值
	I2C_ReadNByte (ADS1115_ADD, ONE_BYTE_SUBA, 0x00, Read_Data, 2);
	delay_ms(1);

	I2C_ReadNByte (ADS1115_ADD, ONE_BYTE_SUBA, 0x00, Read_Data, 2);
	delay_ms(1);
//	WDTFeed();
	
	//采集数据取平均值
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



