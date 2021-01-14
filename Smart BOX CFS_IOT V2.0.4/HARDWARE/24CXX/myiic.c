/*********************************************************************************
 * 文件名  ：myiic.c
 * 描述    ：IIC的初始化 开始停止 发送 接受1字节等函数，标准的IIC协议可供外部调用
 * 移植说明：只要把myiic.c 和 myiic.h添加到所需要的工程里 把IIC_Init()初始化里面对应
 *           管脚修改一下，特别是.h里的操作寄存器的，调用库函数也可以，寄存器速度快
 *           移植时注意延时函数
 * 硬件连接：SCL-->PB6  SDA-->PB7
 * 日期    ：2014 07 09 
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

//I2C总线启动
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


//I2C停止总线
void I2CStop_A(void)
{
   SDA_A0;
   I2C_delay(50);
   SCL_A1;
   I2C_delay(50);
   SDA_A1;
   I2C_delay(50);//MIN 160ns
}

//I2C 写一字节
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
         SCL_A1;//按照手册不需延时
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


//I2C 读一字节
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
** Descriptions:        向有子地址器件写入N字节数据
** input parameters:    sla         器件地址
**                      suba_type   子地址结构    1－单字节地址    2－8+X结构    2－双字节地址
**                      suba        器件子地址
**                      s           将要写入的数据的指针
**                      num         将要写入的数据的个数
** Output parameters:   返回值为0时表示出错，为1时表示操作正确。
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
** Descriptions:        从有子地址器件任意地址开始读取N字节数据
** input parameters:    sla         器件地址
**                      suba_type   子地址结构    1－单字节地址    2－8+X结构    2－双字节地址
**                      suba        器件子地址
**                      s           数据接收缓冲区指针
**                      num         读取的个数
** Output parameters:   返回值为0时表示出错，为1时表示操作正确。
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





//I2C总线启动
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


//I2C停止总线
void I2CStop_B(void)
{
   SDA_B0;
   I2C_delay(50);
   SCL_B1;
   I2C_delay(50);
   SDA_B1;
   I2C_delay(50);//MIN 160ns
}

//I2C 写一字节
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
         SCL_B1;//按照手册不需延时
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


//I2C 读一字节
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
** Descriptions:        向有子地址器件写入N字节数据
** input parameters:    sla         器件地址
**                      suba_type   子地址结构    1－单字节地址    2－8+X结构    2－双字节地址
**                      suba        器件子地址
**                      s           将要写入的数据的指针
**                      num         将要写入的数据的个数
** Output parameters:   返回值为0时表示出错，为1时表示操作正确。
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
** Descriptions:        从有子地址器件任意地址开始读取N字节数据
** input parameters:    sla         器件地址
**                      suba_type   子地址结构    1－单字节地址    2－8+X结构    2－双字节地址
**                      suba        器件子地址
**                      s           数据接收缓冲区指针
**                      num         读取的个数
** Output parameters:   返回值为0时表示出错，为1时表示操作正确。
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
 * 函数名：IIC_Init
 * 描述  ：IIC初始化
 * 输入  ：无
 * 输出  : 无
 * AT24C02 SDA--->PB6  SCL--->PB7
 **************************************************************/
void IIC_Init(void)
{					     
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE );	
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_6|GPIO_Pin_7); 	        //PB6,PB7 输出高
}



/**************************************************************
 * 函数名  ：IIC_Start
 * 描述    ：产生IIC起始信号
 * 开始信号：SCL 为高电平时，SDA 由高电平向低电平跳变，开始传送数据
 * 输入    ：无
 * 输出    : 无
 **************************************************************/
void IIC_Start(void)
{
//	I2CStart_B();
	SDA_OUT();     //sda线输出
	IIC_SDA=1;	  	  
	IIC_SCL=1;
	delay_us(4);
 	IIC_SDA=0;//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	IIC_SCL=0;//钳住I2C总线，准备发送或接收数据 
}	  

/**************************************************************
 * 函数名  ：IIC_Stop
 * 描述    ：产生IIC停止信号
 * 结束信号：SCL 为高电平时，SDA 由低电平向高电平跳变，结束传送数据。
 * 输入    ：无
 * 输出    : 无
 **************************************************************/
void IIC_Stop(void)
{
//	I2CStop_B();
	SDA_OUT();//sda线输出
	IIC_SCL=0;
	IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	IIC_SCL=1; 
	IIC_SDA=1;//发送I2C总线结束信号
	delay_us(4);							   	
}

/**************************************************************
 * 函数名  ：IIC_Wait_Ack
 * 描述    ：等待应答信号到来
 * 应答信号：接收数据的 IC 在接收到 8bit 数据后，向发送数据的 IC 
 *           发出特定的低电平脉冲,表示已收到数据
 * 输入    ：无
 * 输出    : 1，接收应答失败 0，接收应答成功
 **************************************************************/
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();      //SDA设置为输入  
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
	IIC_SCL=0;//时钟输出0 	   
	return 0;  
} 

/**************************************************************
 * 函数名：IIC_Ack
 * 描述  ：产生ACK应答  
 *         在数据接受完以后一个周期内产生低电平
 * 输入  ：无
 * 输出  : 无
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
 * 函数名：IIC_NAck
 * 描述  ：不产生ACK应答  
 *         在数据接受完以后一个周期内产生高电平
 * 输入  ：无
 * 输出  : 无
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
 * 函数名：IIC_Send_Byte
 * 描述  ：返回从机有无应答
 * 规则  ：SDA线上的数据在时钟“高”期间必须是稳定的，
 *				 只有当SCL线上的时钟信号为低时，
 *				 数据线上的“高”或“低”状态才可以改变
 * 输入  ：无
 * 输出  : 1，有应答 0，无应答	
 **************************************************************/
void IIC_Send_Byte(u8 txd)
{          
//		I2CWriteByte_B(txd);
    u8 t;   
	  SDA_OUT(); 	    
    IIC_SCL=0;//拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {              
      //IIC_SDA=(txd&0x80)>>7;
			if((txd&0x80)>>7)  //发送的数据不是0就是1
				IIC_SDA=1;
			else
				IIC_SDA=0;
			txd<<=1; 	  
			delay_us(2);   //对TEA5767这三个延时都是必须的
			IIC_SCL=1;
			delay_us(2); 
			IIC_SCL=0;	
			delay_us(2);
    }	 
} 	    

/**************************************************************
 * 函数名：IIC_Read_Byte
 * 描述  ：读1个字节，ack=1时，发送ACK，ack=0，发送nACK
 *         在低电平时数据变化，在高电平时数据稳定 进行读取
 * 输入  ：无
 * 输出  : 接收到的数据
 **************************************************************/
u8 IIC_Read_Byte(unsigned char ack)
{
	
	
//	return I2CReadByte_B();
	unsigned char i,receive=0;
	
	SDA_IN();//SDA设置为输入
  for(i=0;i<8;i++ )
	{
      IIC_SCL=0;    //数据变化
      delay_us(2);
			IIC_SCL=1;    //数据问题可以读取
			receive<<=1;
			if(READ_SDA)  //如果数据位1就加1 否则加0
				 receive++;   
			delay_us(1); 
	}					 
	if (!ack)
			IIC_NAck();//发送nACK
	else
			IIC_Ack(); //发送ACK   
	return receive;
}
