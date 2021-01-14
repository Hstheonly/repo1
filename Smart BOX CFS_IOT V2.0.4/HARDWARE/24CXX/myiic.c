/*********************************************************************************
 * �ļ���  ��myiic.c
 * ����    ��IIC�ĳ�ʼ�� ��ʼֹͣ ���� ����1�ֽڵȺ�������׼��IICЭ��ɹ��ⲿ����
 * ��ֲ˵����ֻҪ��myiic.c �� myiic.h��ӵ�����Ҫ�Ĺ����� ��IIC_Init()��ʼ�������Ӧ
 *           �ܽ��޸�һ�£��ر���.h��Ĳ����Ĵ����ģ����ÿ⺯��Ҳ���ԣ��Ĵ����ٶȿ�
 *           ��ֲʱע����ʱ����
 * Ӳ�����ӣ�SCL-->PB6  SDA-->PB7
 * ����    ��2014 07 09 
**********************************************************************************/
#include "myiic.h"
#include "delay.h"
#include "sys.h"


//SCL ---> PC8
//SDA ---> PC7
void I2C_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC ,ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8;   //A SCL SDA
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	SDA_A1;
	SCL_A1;
	
}

static void I2C_delay(u16 D)
{
	while(--D);
}

void delay_nms(u16 ms)
{
	u16 i;
	u32 M = 0;//720W
	for(i = 0;i < ms; i++)
	for(M=12000;M > 0;M--);
}

void delay_nus(u16 us)
{
	u16 i;
	u16 M = 0;//720W
	for(i = 0;i < us; i++)
	for(M=72;M > 0;M--);
}

//I2C��������
void I2CStart_A(void)
{
   SDA_A1;
   I2C_delay(50);
   SCL_A1;
   I2C_delay(50);
   SDA_A0;
   I2C_delay(50);//MIN 160ns
   SCL_A0;
   I2C_delay(50);
}


//I2Cֹͣ����
void I2CStop_A(void)
{
   SDA_A0;
   I2C_delay(50);
   SCL_A1;
   I2C_delay(50);
   SDA_A1;
   I2C_delay(50);//MIN 160ns
}

//I2C дһ�ֽ�
void I2CWriteByte_A(u8 DATA)
{
      u8 i;
//      SDA_OUT;
      SCL_A0;
      for(i = 0;i < 8; i++)
      {
         if(DATA&0x80)
         {
              SDA_A1;
         }
         else
         {
              SDA_A0;
         }
         SCL_A1;//�����ֲ᲻����ʱ
         I2C_delay(50);
         SCL_A0;
         I2C_delay(50);
         DATA	= DATA << 1;  
      }
      SDA_A1;
      SCL_A1;
      I2C_delay(50);
      SCL_A0;
}


//I2C ��һ�ֽ�
u8 I2CReadByte_A(void)
{
    u8 TData=0,i;
    for(i=0;i<8;i++)
    {
		SCL_A1;
		I2C_delay(50);
		TData=TData<<1;
		if(SDA_AI)
		{
			TData|=0x01;
		}
		SCL_A0;
		I2C_delay(50);
    }
    SCL_A0;
    I2C_delay(50);
    SDA_A0;
    I2C_delay(50);
    SCL_A1;
    I2C_delay(50);
    SCL_A0;
    I2C_delay(50);
    SDA_A1;
    return TData;
}

/*********************************************************************************************************
** Function name:       I2C_WriteNByte
** Descriptions:        �����ӵ�ַ����д��N�ֽ�����
** input parameters:    sla         ������ַ
**                      suba_type   �ӵ�ַ�ṹ    1�����ֽڵ�ַ    2��8+X�ṹ    2��˫�ֽڵ�ַ
**                      suba        �����ӵ�ַ
**                      s           ��Ҫд������ݵ�ָ��
**                      num         ��Ҫд������ݵĸ���
** Output parameters:   ����ֵΪ0ʱ��ʾ����Ϊ1ʱ��ʾ������ȷ��
** Returned value:      NONE
*********************************************************************************************************/
u8 I2C_WriteNByte(u8 sla, u8 suba_type, u32 suba, u8 *s, u8 num)
{
	u8 i = 0;

		SCL_A1;
		I2CStart_A();
		
		if(suba_type == 0x01)
		{
			I2CWriteByte_A(sla);
			I2C_delay(10);
			I2CWriteByte_A(suba);
			I2C_delay(10);
			for(i = 0;i < num ;i++)
			{
				I2CWriteByte_A(s[i]);
				I2C_delay(10);
			}
		}
		else
		{
			I2CWriteByte_A(sla);
			I2C_delay(10);
			I2CWriteByte_A(suba/256);
			I2C_delay(10);
			I2CWriteByte_A(suba%256);
			I2C_delay(10);
			for(i = 0;i < num ;i++)
			{
				I2CWriteByte_A(s[i]);
				I2C_delay(10);
			}
		}
		
		I2CStop_A();
		
		return 1;
}

/*********************************************************************************************************
** Function name:       I2C_ReadNByte
** Descriptions:        �����ӵ�ַ���������ַ��ʼ��ȡN�ֽ�����
** input parameters:    sla         ������ַ
**                      suba_type   �ӵ�ַ�ṹ    1�����ֽڵ�ַ    2��8+X�ṹ    2��˫�ֽڵ�ַ
**                      suba        �����ӵ�ַ
**                      s           ���ݽ��ջ�����ָ��
**                      num         ��ȡ�ĸ���
** Output parameters:   ����ֵΪ0ʱ��ʾ����Ϊ1ʱ��ʾ������ȷ��
** Returned value:      NONE
*********************************************************************************************************/
u8 I2C_ReadNByte (u8 sla, u32 suba_type, u32 suba, u8 *s, u32 num)
{
	u8 i = 0;
	
		SCL_A1;
		
		SCL_A1;
		I2CStart_A();
		if(suba_type == 0x01)
		{
			I2CWriteByte_A(sla);
			I2CWriteByte_A(suba);
		}
		else
		{
			I2CWriteByte_A(sla);
			I2CWriteByte_A(suba/256);
			I2CWriteByte_A(suba%256);
		}
		I2CStop_A();
		SCL_A0;
		
		
		SCL_A1;
		I2CStart_A();
		I2CWriteByte_A(sla+1);
		for(i = 0;i < num;i++)
		{
			s[i] = I2CReadByte_A();
		}
		I2CStop_A();
		
		return 1;
}





//I2C��������
void I2CStart_B(void)
{
   SDA_B1;
   I2C_delay(50);
   SCL_B1;
   I2C_delay(50);
   SDA_B0;
   I2C_delay(50);//MIN 160ns
   SCL_B0;
   I2C_delay(50);
}


//I2Cֹͣ����
void I2CStop_B(void)
{
   SDA_B0;
   I2C_delay(50);
   SCL_B1;
   I2C_delay(50);
   SDA_B1;
   I2C_delay(50);//MIN 160ns
}

//I2C дһ�ֽ�
void I2CWriteByte_B(u8 DATA)
{
      u8 i;
//      SDA_OUT;
      SCL_B0;
      for(i = 0;i < 8; i++)
      {
         if(DATA&0x80)
         {
              SDA_B1;
         }
         else
         {
              SDA_B0;
         }
         SCL_B1;//�����ֲ᲻����ʱ
         I2C_delay(50);
         SCL_B0;
         I2C_delay(50);
         DATA	= DATA << 1;  
      }
      SDA_B1;
      SCL_B1;
      I2C_delay(50);
      SCL_B0;
}


//I2C ��һ�ֽ�
u8 I2CReadByte_B(void)
{
    u8 TData=0,i;
    for(i=0;i<8;i++)
    {
		SCL_B1;
		I2C_delay(50);
		TData=TData<<1;
		if(SDA_BI)
		{
			TData|=0x01;
		}
		SCL_B0;
		I2C_delay(50);
    }
    SCL_B0;
    I2C_delay(50);
    SDA_B0;
    I2C_delay(50);
    SCL_B1;
    I2C_delay(50);
    SCL_B0;
    I2C_delay(50);
    SDA_B1;
    return TData;
}



/*********************************************************************************************************
** Function name:       I2C_WriteNByte
** Descriptions:        �����ӵ�ַ����д��N�ֽ�����
** input parameters:    sla         ������ַ
**                      suba_type   �ӵ�ַ�ṹ    1�����ֽڵ�ַ    2��8+X�ṹ    2��˫�ֽڵ�ַ
**                      suba        �����ӵ�ַ
**                      s           ��Ҫд������ݵ�ָ��
**                      num         ��Ҫд������ݵĸ���
** Output parameters:   ����ֵΪ0ʱ��ʾ����Ϊ1ʱ��ʾ������ȷ��
** Returned value:      NONE
*********************************************************************************************************/
u8 I2CB_WriteNByte(u8 sla, u8 suba_type, u32 suba, u8 *s, u8 num)
{
	u8 i = 0;

		SCL_B1;
		I2CStart_B();
		
		if(suba_type == 0x01)
		{
			I2CWriteByte_B(sla);
			I2C_delay(10);
			I2CWriteByte_B(suba);
			I2C_delay(10);
			for(i = 0;i < num ;i++)
			{
				I2CWriteByte_B(s[i]);
				I2C_delay(10);
			}
		}
		else
		{
			I2CWriteByte_B(sla);
			I2C_delay(10);
			I2CWriteByte_B(suba/256);
			I2C_delay(10);
			I2CWriteByte_B(suba%256);
			I2C_delay(10);
			for(i = 0;i < num ;i++)
			{
				I2CWriteByte_B(s[i]);
				I2C_delay(10);
			}
		}
		
		I2CStop_B();
		
		return 1;
}

/*********************************************************************************************************
** Function name:       I2C_ReadNByte
** Descriptions:        �����ӵ�ַ���������ַ��ʼ��ȡN�ֽ�����
** input parameters:    sla         ������ַ
**                      suba_type   �ӵ�ַ�ṹ    1�����ֽڵ�ַ    2��8+X�ṹ    2��˫�ֽڵ�ַ
**                      suba        �����ӵ�ַ
**                      s           ���ݽ��ջ�����ָ��
**                      num         ��ȡ�ĸ���
** Output parameters:   ����ֵΪ0ʱ��ʾ����Ϊ1ʱ��ʾ������ȷ��
** Returned value:      NONE
*********************************************************************************************************/
u8 I2CB_ReadNByte (u8 sla, u32 suba_type, u32 suba, u8 *s, u32 num)
{
	u8 i = 0;
	
//		SCL_B1;
		
		SCL_B1;
		I2CStart_B();
		if(suba_type == 0x01)
		{
			I2CWriteByte_B(sla);
			I2CWriteByte_B(suba);
		}
		else
		{
			I2CWriteByte_B(sla);
			I2CWriteByte_B(suba/256);
			I2CWriteByte_B(suba%256);
		}
		I2CStop_B();
		SCL_B0;
		
		
		SCL_B1;
		I2CStart_B();
		I2CWriteByte_B(sla+1);
		for(i = 0;i < num;i++)
		{
			s[i] = I2CReadByte_B();
		}
		I2CStop_B();
		
		return 1;
}





/**************************************************************
 * ��������IIC_Init
 * ����  ��IIC��ʼ��
 * ����  ����
 * ���  : ��
 * AT24C02 SDA--->PB6  SCL--->PB7
 **************************************************************/
void IIC_Init(void)
{					     
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE );	
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_6|GPIO_Pin_7); 	        //PB6,PB7 �����
}



/**************************************************************
 * ������  ��IIC_Start
 * ����    ������IIC��ʼ�ź�
 * ��ʼ�źţ�SCL Ϊ�ߵ�ƽʱ��SDA �ɸߵ�ƽ��͵�ƽ���䣬��ʼ��������
 * ����    ����
 * ���    : ��
 **************************************************************/
void IIC_Start(void)
{
//	I2CStart_B();
	SDA_OUT();     //sda�����
	IIC_SDA=1;	  	  
	IIC_SCL=1;
	delay_us(4);
 	IIC_SDA=0;//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	IIC_SCL=0;//ǯסI2C���ߣ�׼�����ͻ�������� 
}	  

/**************************************************************
 * ������  ��IIC_Stop
 * ����    ������IICֹͣ�ź�
 * �����źţ�SCL Ϊ�ߵ�ƽʱ��SDA �ɵ͵�ƽ��ߵ�ƽ���䣬�����������ݡ�
 * ����    ����
 * ���    : ��
 **************************************************************/
void IIC_Stop(void)
{
//	I2CStop_B();
	SDA_OUT();//sda�����
	IIC_SCL=0;
	IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	IIC_SCL=1; 
	IIC_SDA=1;//����I2C���߽����ź�
	delay_us(4);							   	
}

/**************************************************************
 * ������  ��IIC_Wait_Ack
 * ����    ���ȴ�Ӧ���źŵ���
 * Ӧ���źţ��������ݵ� IC �ڽ��յ� 8bit ���ݺ��������ݵ� IC 
 *           �����ض��ĵ͵�ƽ����,��ʾ���յ�����
 * ����    ����
 * ���    : 1������Ӧ��ʧ�� 0������Ӧ��ɹ�
 **************************************************************/
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();      //SDA����Ϊ����  
	IIC_SDA=1;delay_us(1);	   
	IIC_SCL=1;delay_us(1);	 
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL=0;//ʱ�����0 	   
	return 0;  
} 

/**************************************************************
 * ��������IIC_Ack
 * ����  ������ACKӦ��  
 *         �����ݽ������Ժ�һ�������ڲ����͵�ƽ
 * ����  ����
 * ���  : ��
 **************************************************************/
void IIC_Ack(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=0;
	delay_us(2);
	IIC_SCL=1;
	delay_us(2);
	IIC_SCL=0;
}
/**************************************************************
 * ��������IIC_NAck
 * ����  ��������ACKӦ��  
 *         �����ݽ������Ժ�һ�������ڲ����ߵ�ƽ
 * ����  ����
 * ���  : ��
 **************************************************************/    
void IIC_NAck(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=1;
	delay_us(2);
	IIC_SCL=1;
	delay_us(2);
	IIC_SCL=0;
}					 				     

/**************************************************************
 * ��������IIC_Send_Byte
 * ����  �����شӻ�����Ӧ��
 * ����  ��SDA���ϵ�������ʱ�ӡ��ߡ��ڼ�������ȶ��ģ�
 *				 ֻ�е�SCL���ϵ�ʱ���ź�Ϊ��ʱ��
 *				 �������ϵġ��ߡ��򡰵͡�״̬�ſ��Ըı�
 * ����  ����
 * ���  : 1����Ӧ�� 0����Ӧ��	
 **************************************************************/
void IIC_Send_Byte(u8 txd)
{          
//		I2CWriteByte_B(txd);
    u8 t;   
	  SDA_OUT(); 	    
    IIC_SCL=0;//����ʱ�ӿ�ʼ���ݴ���
    for(t=0;t<8;t++)
    {              
      //IIC_SDA=(txd&0x80)>>7;
			if((txd&0x80)>>7)  //���͵����ݲ���0����1
				IIC_SDA=1;
			else
				IIC_SDA=0;
			txd<<=1; 	  
			delay_us(2);   //��TEA5767��������ʱ���Ǳ����
			IIC_SCL=1;
			delay_us(2); 
			IIC_SCL=0;	
			delay_us(2);
    }	 
} 	    

/**************************************************************
 * ��������IIC_Read_Byte
 * ����  ����1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK
 *         �ڵ͵�ƽʱ���ݱ仯���ڸߵ�ƽʱ�����ȶ� ���ж�ȡ
 * ����  ����
 * ���  : ���յ�������
 **************************************************************/
u8 IIC_Read_Byte(unsigned char ack)
{
	
	
//	return I2CReadByte_B();
	unsigned char i,receive=0;
	
	SDA_IN();//SDA����Ϊ����
  for(i=0;i<8;i++ )
	{
      IIC_SCL=0;    //���ݱ仯
      delay_us(2);
			IIC_SCL=1;    //����������Զ�ȡ
			receive<<=1;
			if(READ_SDA)  //�������λ1�ͼ�1 �����0
				 receive++;   
			delay_us(1); 
	}					 
	if (!ack)
			IIC_NAck();//����nACK
	else
			IIC_Ack(); //����ACK   
	return receive;
}
