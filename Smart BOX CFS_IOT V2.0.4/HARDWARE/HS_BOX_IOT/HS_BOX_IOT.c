#include "GPIO.h"
#include "delay.h"
#include "usart.h"
#include "HS_BOX_IOT.h"
#include "MB_CRC.h"
#include "myiic.h"
#include "ads1115.h"
#include "adc.h"
#include "24cxx.h" 
#include "DEBUG.h"

/*
**********************************IOT****************************************
***设备：1.温度        1#-5#          数值：u16
***设备：2.电表                       数值：电压、电流、功率、用电量（u16）
***设备：3.亮度    		 1#-4#					数值：u8
***设备：4.设置亮度            				数值：亮度1上限、亮度1下限 亮度2上限 亮度2下限
***设备：5.照明灯      1#-5#          状态：关闭、打开（0-1）
***设备：6.空调   开关、模式、温度    状态：
*/ 
#define Tem_Num  		6
#define Hum_Num  		2
#define Bright_Num 	1
#define Light_Num 	5

#define IOT_IIC_Offset_ADD 128						//偏移地址

typedef struct {
	u16 Tem[Tem_Num];										//采集的5个温度1-3对应温湿度设备  AIN0采集4号设备 AIN1 5号设备
	
	u8  Tem_Fault_Code[Tem_Num];				//温度故障码
	u16 Hum[Hum_Num];               		//采集湿度
	u8 Hum_Fault_Code[Hum_Num];  				//湿度故障码
	u16 Watt_Meter[4];									//电压、电流、功率、用电量	
	u8  Watt_Fault_Code;								//电表故障码
  u8  Bright[Bright_Num];	          	//采集5个亮度
	u8 	Bright_Fault_Code[Bright_Num];	//亮度故障码
	u8  Set_Bright[4];       						//亮度1上限、亮度1下限 亮度2上限 亮度2下限
	u8  Light_State[Light_Num];					//5个照明灯状态
	u8  Light_Fault_Code[Light_Num];		//照明灯故障码
	u8  LGT_KEY_State[5];     //5个按键状态
	u16 LGT_KEY_Code;     		//5个按键组合码
	u16 Interval;             //自动上报时间间隔
	u8  Air_Con_Mode;         //空调模式 00 制冷 01 制热 
	u8 	Air_Con_State;				//空调状态 00：关闭 0x01:打开
	u16 Air_Con_Tem;					//空调温度
	u8  Air_Con_Fault_Code;		//空调故障码
	
	u16 Stack_Cahe[10][10];					//堆栈缓存 10组  温湿度温度1-2 PT100 3-5 湿度6-7
}HS_Box_IOT;

#define IOT_Device_Type  0x30
#define CTR_Device_Type  0x00
#define Head 0xAAAB
#define END  0xBABB

#define EEPROM_IOT_ADD   0x30  //eeprom存储地址
//传感器设备数量

HS_Box_IOT Box_IOT;

/*****************************************************
 * 函数名: Print_Send_data
 * 描述  ：打印485发送的数据
 * 输入  ：buf 数据 len 数据的长度
 * 输出  ：无
 ****************************************************/
void Print_TST_Send_data(u8* buf,u8 len)
{
	#ifdef DEBUG_Print
	u8 i = 0;

	Asterisk_Print(60);CR_Print();
	
	printf("[DEBUG]->(TST):<Send> Data:");
	for(i = 0;i < len;i++)
		ONE_HEX_Print(buf[i]);
	CR_Print();
	
	Asterisk_Print(60);CR_Print();	
	#endif
}



/*****************************************************
 * 函数名: Print_RS485_Receive_data
 * 描述  ：打印485发送的数据
 * 输入  ：buf 数据 len 数据的长度
 * 输出  ：无
 ****************************************************/
void Print_RS485_Receive_data(u8* buf,u8 len)
{
#ifdef DEBUG_Print
	u8  i = 0;
	
	Asterisk_Print(60);CR_Print();
	
	printf("[DEBUG]->(TST_RS485):<Receive> Data:");
	for(i = 0;i < len;i++)
		ONE_HEX_Print(buf[i]);
	CR_Print();
	
	Asterisk_Print(60);CR_Print();	
	#endif
}
/*****************************************************
 * 函数名: Print_RS485_Send_data
 * 描述  ：打印485发送的数据
 * 输入  ：buf 数据 len 数据的长度
 * 输出  ：无
 ****************************************************/
void Print_RS485_Send_data(u8* buf,u8 len)
{
	#ifdef DEBUG_Print
	u8 i = 0;

	Asterisk_Print(60);CR_Print();
	
	printf("[DEBUG]->(TST_RS485):<Send> Data:");
	for(i = 0;i < len;i++)
		ONE_HEX_Print(buf[i]);
	CR_Print();
	
	Asterisk_Print(60);CR_Print();	
	#endif
}

/*********************************************底层串口驱动***********************************************/
/*****************************************************
 * 函数名: IOT_Send_Data
 * 描述  ：IOT设备上报
 * 输入  ：buf 数据组 len 数据长度
 * 输出  ：无
 ****************************************************/
void IOT_Send_Data(u8* buf,u8 len)
{
	
	UART4_Send_Data(buf,len);
	
}

/*****************************************************
 * 函数名: IOT_Receive_Data
 * 描述  ：IOT设备接收数据
 * 输入  ：buf 数据组 len 数据长度
 * 输出  ：无
 ****************************************************/
void IOT_Receive_Data(u8* RSbuf,u8* rs_len)
{
	UART4_Receive_Data(RSbuf,rs_len);
}



/*****************************************************
 * 函数名: IOT_Send_Data
 * 描述  ：IOT设备上报
 * 输入  ：buf 数据组 len 数据长度
 * 输出  ：无
 ****************************************************/
void RS485_1_Send_Data(u8* buf,u8 len)
{

	RS485_1_TX_EN;
	USART3_Send_Data(buf,len);
	RS485_1_RX_EN;
	Print_RS485_Send_data(buf,len);

}

/*****************************************************
 * 函数名: IOT_Receive_Data
 * 描述  ：IOT设备接收数据
 * 输入  ：buf 数据组 len 数据长度
 * 输出  ：无
 ****************************************************/
void RS485_1_Receive_Data(u8* RSbuf,u8* rs_len)
{
	USART3_Receive_Data(RSbuf,rs_len);
//	Print_RS485_Receive_data（RSbuf,rs_len);
}


/*????*******************************************设备信息**********************************************/

//typedef struct {
//	u16 Tem[6];								//采集的5个温度1-3对应温湿度设备  AIN0采集4号设备 AIN1 5号设备
//	u16 Tem_Fault_Code;				//温度故障码
//	u16 Hum[5];               //采集湿度
//	u16 Hum_Fault_Code;				//湿度故障码
//	u16 Watt_Meter[4];				//电压、电流、功率、用电量	
//	u8  Watt_Fault_Code;			//电表故障码
//  u8  Bright[5];	          //采集5个亮度
//	u16 Bright_Fault_Code;		//亮度故障码
//	u8  Set_Bright[4];        //亮度1上限、亮度1下限 亮度2上限 亮度2下限
//	u8  Light_State[5];				//5个照明灯状态
//	u16 Light_Fault_Code;			//照明灯故障码
//	u8  LGT_KEY_State[5];     //5个按键状态
//	u16 LGT_KEY_Code;     		//5个按键组合码
//	u16 Interval;             //自动上报时间间隔
//	u8 	Air_Con_State;				//空调状态 00：关闭 0x01:打开制冷 0x02:打开制热
//	u16 Air_Con_Tem;					//空调温度
//	u8  Air_Con_Fault_Code;		//空调故障码
//}HS_Box_IOT;
//#define Tem_Num  		4
//#define Hum_Num  		2
//#define Bright_Num 	1
//#define Light_Num 	5
/*****************************************************
 * 函数名: IOT_Device_Info
 * 描述  ：设备信息
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void IOT_Device_Info(u8 Type)
{
	u8 Data_Len = 0;//数据包长度
	u8 i = 0;
	u8 j = 0;
	
	u8 Send_Buf[30] = {0xAA,0xAB,0x30,0x50,0x01,0x30,0x00,0xBA,0xBB};
	
	Send_Buf[3] = Type;			//设备类型
	Send_Buf[4] = Data_Len;	//数据长度
	i = 5;
	
	//温度故障码
	Send_Buf[i] = Tem_Num;
	i++;
	for(j = 0;j < Tem_Num;j++)
	{
		Send_Buf[i] = Box_IOT.Tem_Fault_Code[j];
		i++;
	}
	//湿度故障码
	Send_Buf[i] = Hum_Num;
	i++;
	for(j = 0;j < Hum_Num;j++)
	{
		Send_Buf[i] = Box_IOT.Hum_Fault_Code[j];
		i++;
	}
	//亮度故障码
	Send_Buf[i] = Bright_Num;
	i++;
	for(j = 0;j < Bright_Num;j++)
	{
		Send_Buf[i] = Box_IOT.Bright_Fault_Code[j];
		i++;
	}
	//照明故障码
	Send_Buf[i] = Light_Num;
	i++;
	for(j = 0;j < Light_Num;j++)
	{
		Send_Buf[i] = Box_IOT.Light_Fault_Code[j];
		i++;
	}
	//空调故障码
	Send_Buf[i] = Box_IOT.Air_Con_Fault_Code;
	i++;
	//电表障码
	Send_Buf[i] = Box_IOT.Watt_Fault_Code;
	i++;
	//空调故障
	Send_Buf[i] = Box_IOT.Air_Con_State;
  i++;
	//数据长度
	Data_Len = i-4;
	Send_Buf[4] = Data_Len;//数据长度
	
	//和校验
	Send_Buf[i] = 0x00;
  i++;
	
	//包尾
	Send_Buf[i] = 0xBA;
  i++;
	Send_Buf[i] = 0xBB;
  i++;	
	
	IOT_Send_Data(Send_Buf,i);
}

/*0x50****************************************************
 * 函数名: IOT_Start_Report_Check
 * 描述  ：设备开机自检信息  等待0x51
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void IOT_Start_Report_Check(void)
{
	IOT_Device_Info(0x50);
}
/*0x53****************************************************
 * 函数名: IOT_Device_Info_Response
 * 描述  ：设备信息查询回复  回复0x52
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void IOT_Device_Info_Response(void)
{
	IOT_Device_Info(0x53);
}


/*********************************************设备类型***********************************************/
/*0x54****************************************************
 * 函数名: IOT_Start_Report_Device_Type
 * 描述  ：开机上报设备类型 等待 0x55
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void IOT_Start_Report_Device_Type(void)
{
	u8 Send_Buf[9] = {0xAA,0xAB,0x30,0x54,0x01,0x30,0x00,0xBA,0xBB};
//	Send_Buf[5] = CON_Device_Type;
	IOT_Send_Data(Send_Buf,9);
}

/*0x57****************************************************
 * 函数名: IOT_Device_Type_Response
 * 描述  ：设备类型应答  回复 0x56
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void IOT_Device_Type_Response(void)
{
	u8 Send_Buf[9] = {0xAA,0xAB,0x30,0x57,0x01,0x30,0x00,0xBA,0xBB};
	//	Send_Buf[5] = CON_Device_Type;
	IOT_Send_Data(Send_Buf,9);
}



/*********************************************温度***********************************************/
/*0x59****************************************************
 * 函数名: IOT_Temperature_Response
 * 描述  ：补货位电机位置应答    回应 0x58
 * 输入  ：Num: 上报温度个数 最大5
 * 输出  ：无
 ****************************************************/
void IOT_Temperature_Response(u8 Num)
{
	u8 i = 0;
	u8 j = 0;	
	u8 Send_Buf[30] = {0xAA,0xAB,0x30,0x59,0x0B,0x05,      0x00,0x00,  0x00,0x00,  0x00,0x00,  0x00,0x00,  0x00,0x00,    0x00,0xBA,0xBB};
		
	if(Num >  6)
		Num = 6;
		
	Send_Buf[3] = 0x59;
	Send_Buf[4] = Num*2+1;
	Send_Buf[5] = Num;
	j = 6;
	printf("Num:%d\r\n",Num);
	for(i = 0;i < Num;i++)
	{		
		Send_Buf[j] = Box_IOT.Tem[i]/256;
		j++;
		Send_Buf[j] = Box_IOT.Tem[i]%256;
		j++;
		
		printf("Tem[%d] = %d\r\n",i,Box_IOT.Tem[i]);
	}
	Send_Buf[j] = 0x00;//校验
	j++;
	Send_Buf[j] = 0xBA;//包尾
	j++;
	Send_Buf[j] = 0xBB;//包尾
	j++;
	IOT_Send_Data(Send_Buf,j);
}




/*********************************************湿度***********************************************/
/*0x5B****************************************************
 * 函数名: CON_Ask_Door_Response
 * 描述  ：查询湿度应答    回应 0x5A
 * 输入  ：Num: 上报湿度个数 最大5
 * 输出  ：无
 ****************************************************/
void IOT_Hum_Response(u8 Num)
{
	u8 i = 0;
	u8 j = 0;	
	u8 Send_Buf[30] = {0xAA,0xAB,0x30,0x5B,0x0B,0x05,      0x00,0x00,  0x00,0x00,  0x00,0x00,  0x00,0x00,  0x00,0x00,    0x00,0xBA,0xBB};
		
	if(Num >  5)
		Num = 5;
	Send_Buf[3] = 0x5B;
	Send_Buf[4] = Num*2+1;
	Send_Buf[5] = Num;
	j = 6;
	for(i = 0;i < Num;i++)
	{		
		Send_Buf[j] = Box_IOT.Hum[i]/256;
		j++;
		Send_Buf[j] = Box_IOT.Hum[i]%256;
		j++;
	}
	Send_Buf[j] = 0x00;//校验
	j++;
	Send_Buf[j] = 0xBA;//包尾
	j++;
	Send_Buf[j] = 0xBB;//包尾
	j++;
	IOT_Send_Data(Send_Buf,j);
}



/*********************************************电表***********************************************/
/*0x5D****************************************************
 * 函数名: IOT_Watt_Meter_Response
 * 描述  ：查询电表      回应 0x5C
 * 输入  ：Num 上报个数  电压 电流 功率 用电量 4个数据
 * 输出  ：
 ****************************************************/
void IOT_Watt_Meter_Response(u8 Num)
{
	u8 i = 0;
	u8 j = 0;	
	u8 Send_Buf[30] = {0xAA,0xAB,0x30,0x5D,0x0B,0x05,      0x00,0x00,  0x00,0x00,  0x00,0x00,  0x00,0x00,  0x00,0x00,    0x00,0xBA,0xBB};
		
	if(Num >  5)
		Num = 5;
	Send_Buf[3] = 0x5D;
	Send_Buf[4] = Num*2;

	j = 5;
	for(i = 0;i < Num;i++)
	{		
		Send_Buf[j] = Box_IOT.Watt_Meter[i]/256;
		j++;
		Send_Buf[j] = Box_IOT.Watt_Meter[i]%256;
		j++;
	}
	Send_Buf[j] = 0x00;//校验
	j++;
	Send_Buf[j] = 0xBA;//包尾
	j++;
	Send_Buf[j] = 0xBB;//包尾
	j++;
	IOT_Send_Data(Send_Buf,j);
}



/*********************************************亮度***********************************************/
/*0x5F****************************************************
 * 函数名: IOT_Bright_Response
 * 描述  ：查询亮度   回应 0x5E的数据
 * 输入  ：Num 上报个数
 * 输出  ：无
 ****************************************************/
void IOT_Bright_Response(u8 Num)
{
	u8 i = 0;
	u8 j = 0;	
	u8 Send_Buf[30] = {0xAA,0xAB,0x30,0x5F,0x0B,0x05,      0x00,0x00,  0x00,0x00,  0x00,0x00,  0x00,0x00,  0x00,0x00,    0x00,0xBA,0xBB};
		
	if(Num >  5)
		Num = 5;
	Send_Buf[3] = 0x5F;
	Send_Buf[4] = Num*2+1;
	Send_Buf[5] = Num;
	j = 6;
	for(i = 0;i < Num;i++)
	{		
		Send_Buf[j] = Box_IOT.Bright[i];
		j++;	
	}
	Send_Buf[j] = 0x00;//校验
	j++;
	Send_Buf[j] = 0xBA;//包尾
	j++;
	Send_Buf[j] = 0xBB;//包尾
	j++;
	IOT_Send_Data(Send_Buf,j);
}

/*0x61****************************************************
 * 函数名: IOT_Bright_Response
 * 描述  ：查询亮度设置  回应 0x60的数据
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void IOT_Set_Bright_Response(void)
{
	u8 i = 0;
	u8 j = 0;	
	u8 Num = 4;
	u8 Send_Buf[30] = {0xAA,0xAB,0x30,0x61};
		
	
	Send_Buf[3] = 0x61;
	Send_Buf[4] = Num;

	j = 5;
	for(i = 0;i < Num;i++)
	{		
		Send_Buf[j] = Box_IOT.Set_Bright[i]/256;
		j++;
	}
	Send_Buf[j] = 0x00;//校验
	j++;
	Send_Buf[j] = 0xBA;//包尾
	j++;
	Send_Buf[j] = 0xBB;//包尾
	j++;
	IOT_Send_Data(Send_Buf,j);
}


/*0x63****************************************************
 * 函数名: IOT_Set_Bright_Request
 * 描述  ：设置亮度设置  回应 0x62的数据
 * 输入  ：Serial 编号 Value 数值
 * 输出  ：无
 ****************************************************/
void IOT_Set_Bright_Request(u8 Serial, u8 Value)
{
	
	u8 Send_Buf[30] = {0xAA,0xAB,0x30,0x63,0x00,0xBA,0xBB};
	if(Serial<5)
	{
		Box_IOT.Set_Bright[Serial-1] = Value;
	
		IOT_Send_Data(Send_Buf,6);
	}
}

/*********************************************照明灯***********************************************/
/*****************************************************
 * 函数名: Light_State_Save
 * 描述  ：保存照明灯数据到EEPROM
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void Light_State_Save(void)
{
	u8 IIC_buf[6] = {0};     //EEPROM存储
	u8 IIC_ADD = 0;
	u8 i = 0;
	
	IIC_ADD = IOT_IIC_Offset_ADD + 0;
	
	for(i = 0;i < 5;i++)
		IIC_buf[i] = Box_IOT.Light_State[i];
		
	AT24CXX_Write(IIC_ADD,IIC_buf,5);
}
/*****************************************************
 * 函数名: Read_EEPROM_Light_State
 * 描述  ：读取EEPROM中照明灯状态
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void Read_EEPROM_Light_State(void)
{
	u8 IIC_buf[6] = {0};     //EEPROM存储
	u8 IIC_ADD = 0;
	u8 i = 0;
	
	IIC_ADD = IOT_IIC_Offset_ADD + 0;
	
	AT24CXX_Read(IIC_ADD,IIC_buf,5);
	
	for(i = 0;i < 5;i++)
		 Box_IOT.Light_State[i] = IIC_buf[i] ;
	
	Q_LGT_1 =  Box_IOT.Light_State[0];
	Q_LGT_2 =  Box_IOT.Light_State[1];
	Q_LGT_3 =  Box_IOT.Light_State[2];
	Q_LGT_4 =  Box_IOT.Light_State[3];
	Q_LGT_5 =  Box_IOT.Light_State[4];
}

/*****************************************************
 * 函数名: IOT_Light_Handle_Not
 * 描述  ：照明灯取反操作
 * 输入  ：Serial 照明灯编号 0-4  对应 1#-5#
 * 输出  ：无
 ****************************************************/
void IOT_Light_Handle_Not(u8 Serial,u8 Light_State)
{
	  Box_IOT.Light_State[Serial] = !Box_IOT.Light_State[Serial];	//照明灯状态取反
		printf("Serial:%d Light_State:%d\r\n",Serial,Light_State);
		switch(Serial)															//照明灯状态具体控制
		{
			case 0:
				Q_LGT_1 = !Box_IOT.Light_State[0];
				break;
			case 1:
				Q_LGT_2 = !Box_IOT.Light_State[1];
				break;
			case 2:
				Q_LGT_3 = !Box_IOT.Light_State[2];
				break;
			case 3:
				Q_LGT_4 = !Box_IOT.Light_State[3];
				break;
			case 4:
				Q_LGT_5 = !Box_IOT.Light_State[4];
				break;
			default:
				break;
		}
		
		Light_State_Save();//将照明灯数据保存到EEPROM里
}

/*0x67****************************************************
 * 函数名: Light_ON_OFF
 * 描述  ：开关灯控制
 * 输入  ：Serial 编号 ON_OFF 1:打开 0:关闭
 * 输出  ：无
 ****************************************************/
void Light_ON_OFF(u8 Serial,u8 ON_OFF)
{
	
	printf("Light_ON_OFF:%d  %d\r\n",Serial,ON_OFF);
		switch(Serial)												//照明灯状态具体控制
		{
			case 0:
				Q_LGT_1 = ON_OFF;
				break;
			case 1:
				Q_LGT_2 = ON_OFF;
				break;
			case 2:
				Q_LGT_3 = ON_OFF;
				break;
			case 3:
				Q_LGT_4 = ON_OFF;
				break;
			case 4:
				Q_LGT_5 = ON_OFF;
				break;
			default:
				break;
		}
		Light_State_Save();//将照明灯数据保存到EEPROM里
}
/*****************************************************
 * 函数名: IOT_LGT_State_Report
 * 描述  ：照明灯状态变化
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void IOT_LGT_State_Report(void)
{
	
}
/*****************************************************
 * 函数名: IOT_Read_LGT_KEY
 * 描述  ：读取照明灯开关按键
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void IOT_Read_LGT_KEY(void)
{
	u8 	i = 0;
	u16 LGT_TEMP = 0; //读取照明灯按键实时状态
	u8  KEY_State[5] = {0};
	

		LGT_TEMP = LGT_KEY_SCAN();                  		//按位   0 IN_LGT_1 IN_LGT_2 IN_LGT_3 IN_LGT_4 IN_LGT_5 0-31
		if(LGT_TEMP !=Box_IOT.LGT_KEY_Code)							//按键组合码
		{
			Box_IOT.LGT_KEY_Code	= LGT_TEMP;       			//赋值 按键组合码
			
			KEY_State[0] = LGT_TEMP%2;																			//等到1#按键状态
			LGT_TEMP = LGT_TEMP/2;
			KEY_State[1] = LGT_TEMP%2;																			//等到2#按键状态
			LGT_TEMP = LGT_TEMP/2;
			KEY_State[2] = LGT_TEMP%2;                                      //等到3#按键状态
			LGT_TEMP = LGT_TEMP/2;
			KEY_State[3] = LGT_TEMP%2;                                      //等到4#按键状态
			LGT_TEMP = LGT_TEMP/2;
			KEY_State[4] = LGT_TEMP%2;                                      //等到5#按键状态
			printf("IOT_Read_LGT_KEY:%d\r\n",LGT_TEMP);
			for(i = 0;i < 5;i++)
			{
				if(KEY_State[i] != Box_IOT.LGT_KEY_State[i])              //按键有变化
				{
					Box_IOT.LGT_KEY_State[i] = KEY_State[i];                //赋值
					IOT_Light_Handle_Not(i,KEY_State[i]);                   //照明灯编号 值编号
//					IOT_LGT_State_Report();																	//上报变化
					delay_ms(50);
				}
			}	
		}		  
}


/*0x65****************************************************
 * 函数名: IOT_Light_Response
 * 描述  ：查询照明灯状态     应答 0x64的数据
 * 输入  ：Serial 编号
 * 输出  ：无
 ****************************************************/
void IOT_Light_Response(u8 Serial)
{
	u8 Send_Buf[11] = {0xAA,0xAB,0x30,0x65,0x03,0x00,0x01,0x01,0x00,0xBA,0xBB};
	 
	Send_Buf[6] = Serial;
	Send_Buf[7] = Box_IOT.Light_State[Serial-1];
	IOT_Send_Data(Send_Buf,11);
}


/****************************************************
 * 函数名: Update_LGT_KEY_Code
 * 描述  ：通过开关状态修改锁控板对应的Code码
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void Update_LGT_KEY_Code(void)
{
	Box_IOT.LGT_KEY_Code =Box_IOT.LGT_KEY_State[4]*16 + Box_IOT.LGT_KEY_State[3]*8 + Box_IOT.LGT_KEY_State[2]*4 + Box_IOT.LGT_KEY_State[1]*2 + Box_IOT.LGT_KEY_State[0];
}

/*0x67****************************************************
 * 函数名: IOT_Set_Light_Request
 * 描述  ：设置照明灯  回应 0x66的数据
 * 输入  ：Serial 编号
 * 输出  ：无
 ****************************************************/
void IOT_Set_Light_Request(u8 Serial,u8 Value)
{
	u8 Send_Buf[9] = {0xAA,0xAB,0x30,0x67,0x01,0x00,0x00,0xBA,0xBB};
	if(Serial < 6)
	{
		if(Box_IOT.Light_State[Serial-1] == Value) //灯的状态和要达到状态一致
		{
			if(Value == 1)       //灯已打开
				Send_Buf[5] = 0x03;
			else                 //灯已关闭
				Send_Buf[5] = 0x01;
		}
		else																//灯的状态和要达到状态不一致
		{
			Box_IOT.Light_State[Serial-1] = Value;//照明灯状态赋值
			if(Value == 1)										//开灯
			{
				Light_ON_OFF(Serial-1,1);
				Send_Buf[5] = 0x02;             //灯打开			
			}
			else															//关灯
			{	
				Light_ON_OFF(Serial-1,0);
				Send_Buf[5] = 0x00;							//灯关闭
			}
		}
		Update_LGT_KEY_Code();
		
		IOT_Send_Data(Send_Buf,9);
	}
}


/*********************************************自动上报**********************************************/
/*****************************************************
 * 函数名: Read_EEPROM_Light_State
 * 描述  ：读取EEPROM中照明灯状态
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void Read_EEPROM_Interval_Time(void)
{
	u8 IIC_buf[2] = {0};     //EEPROM存储
	u8 IIC_ADD = 0;
	u8 i = 0;
	
	IIC_ADD = IOT_IIC_Offset_ADD + 5;
	
	AT24CXX_Read(IIC_ADD,IIC_buf,2);
	Box_IOT.Interval = IIC_buf[0]*256+IIC_buf[1];
	
}
/*0x81****************************************************
 * 函数名: IOT_Interval_Request
 * 描述  ：查询自动上报间隔        回应0x80的数据
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void IOT_Interval_Request(void)
{
	u8 Send_Buf[10] = {0xAA,0xAB,0x30,0x81,0x02,0x00,0x00,0x00,0xBA,0xBB};
	Send_Buf[5] = Box_IOT.Interval/256;
	Send_Buf[6] = Box_IOT.Interval%256;
	IOT_Send_Data(Send_Buf,10);
}

/*0x83****************************************************
 * 函数名: IOT_Set_Interval_Request
 * 描述  ：设置自动上报间隔        回应0x82的数据
 * 输入  ：Time 间隔时间
 * 输出  ：无
 ****************************************************/
void IOT_Set_Interval_Request(u16 Time)
{
	u8 IIC_buf[2] = {0};     //EEPROM存储
	u8 IIC_ADD = 0;
	u8 Send_Buf[10] = {0xAA,0xAB,0x30,0x83,0x00,0x00,0xBA,0xBB};
	
	Box_IOT.Interval = Time;
	
	//IIC存储************************************************
	IIC_ADD = IOT_IIC_Offset_ADD + 5;
	IIC_buf[0] = Time/256;
	IIC_buf[1] = Time%256;
	AT24CXX_Write(IIC_ADD,IIC_buf,2);
	
	
	
	IOT_Send_Data(Send_Buf,8);
	
}
/*0x84****************************************************
 * 函数名: IOT_Auto_Temperature_Report
 * 描述  ：自动上报    等待 0x85 回应
 * 输入  ：Num: 上报个数
 * 输出  ：无
 ****************************************************/
void IOT_Auto_Temperature_Report(u8 Num)
{
	u8 i = 0;
	u8 j = 0;	
	u8 Send_Buf[30] = {0xAA,0xAB,0x30,0x84,0x0B,0x05,      0x00,0x00,  0x00,0x00,  0x00,0x00,  0x00,0x00,  0x00,0x00,    0x00,0xBA,0xBB};
		
	if(Num >  6)
		Num = 6;
		
	Send_Buf[3] = 0x84;    //数据类型
	Send_Buf[4] = Num*3+1; //数据长度
	Send_Buf[5] = Num;     //传感器个数
	j = 6;
	for(i = 0;i < Num;i++) //编号 数据
	{	
		Send_Buf[j] = i+1;
		j++;
		Send_Buf[j] = Box_IOT.Tem[i]/256;
		j++;
		Send_Buf[j] = Box_IOT.Tem[i]%256;
		j++;
	}
	Send_Buf[j] = 0x00;//校验
	j++;
	Send_Buf[j] = 0xBA;//包尾
	j++;
	Send_Buf[j] = 0xBB;//包尾
	j++;
	IOT_Send_Data(Send_Buf,j);
}


/*0x86****************************************************
 * 函数名: IOT_Auto_Hum_Report
 * 描述  ：自动上报    等待 0x87 回应
 * 输入  ：Num: 上报个数
 * 输出  ：无
 ****************************************************/
void IOT_Auto_Hum_Report(u8 Num)
{
	u8 i = 0;
	u8 j = 0;	
	u8 Send_Buf[30] = {0xAA,0xAB,0x30,0x84,0x0B,0x05,      0x00,0x00,  0x00,0x00,  0x00,0x00,  0x00,0x00,  0x00,0x00,    0x00,0xBA,0xBB};
		
	if(Num >  5)
		Num = 5;
		
	Send_Buf[3] = 0x86;    //数据类型
	Send_Buf[4] = Num*3+1; //数据长度
	Send_Buf[5] = Num;     //传感器个数
	j = 6;
	for(i = 0;i < Num;i++) //编号 数据
	{	
		Send_Buf[j] = i+1;
		j++;
		Send_Buf[j] = Box_IOT.Hum[i]/256;
		j++;
		Send_Buf[j] = Box_IOT.Hum[i]%256;
		j++;
	}
	Send_Buf[j] = 0x00;//校验
	j++;
	Send_Buf[j] = 0xBA;//包尾
	j++;
	Send_Buf[j] = 0xBB;//包尾
	j++;
	IOT_Send_Data(Send_Buf,j);
}

/*0x88****************************************************
 * 函数名: IOT_Auto_Bright_Report
 * 描述  ：自动上报    等待 0x89 回应
 * 输入  ：Num: 上报个数
 * 输出  ：无
 ****************************************************/
void IOT_Auto_Bright_Report(u8 Num)
{
	u8 i = 0;
	u8 j = 0;	
	u8 Send_Buf[30] = {0xAA,0xAB,0x30,0x88,0x0B,0x05,      0x00,0x00,  0x00,0x00,  0x00,0x00,  0x00,0x00,  0x00,0x00,    0x00,0xBA,0xBB};
		
	if(Num >  5)
		Num = 5;
		
	Send_Buf[3] = 0x88;    //数据类型
	Send_Buf[4] = Num*3+1; //数据长度
	Send_Buf[5] = Num;     //传感器个数
	j = 6;
	for(i = 0;i < Num;i++) //编号 数据
	{	
		Send_Buf[j] = i+1;
		j++;
		Send_Buf[j] = Box_IOT.Bright[i];
		j++;
	}
	Send_Buf[j] = 0x00;//校验
	j++;
	Send_Buf[j] = 0xBA;//包尾
	j++;
	Send_Buf[j] = 0xBB;//包尾
	j++;
	IOT_Send_Data(Send_Buf,j);
}

/*0x8A****************************************************
 * 函数名: IOT_Auto_Watt_Meter_Report
 * 描述  ：自动上报    等待 0x8B 回应
 * 输入  ：Num: 上报个数
 * 输出  ：无
 ****************************************************/
void IOT_Auto_Watt_Meter_Report(u8 Num)
{
	u8 i = 0;
	u8 j = 0;	
	u8 Send_Buf[30] = {0xAA,0xAB,0x30,0x88,0x0B,0x05,      0x00,0x00,  0x00,0x00,  0x00,0x00,  0x00,0x00,  0x00,0x00,    0x00,0xBA,0xBB};
		
	if(Num >  5)
		Num = 5;
		
	Send_Buf[3] = 0x88;    //数据类型
	Send_Buf[4] = Num*3+1; //数据长度

	j = 5;
	for(i = 0;i < Num;i++) //编号 数据
	{	
		Send_Buf[j] = Box_IOT.Watt_Meter[i]/256;
		j++;
		Send_Buf[j] = Box_IOT.Watt_Meter[i]%256;
		j++;
	}
	Send_Buf[j] = 0x00;//校验
	j++;
	Send_Buf[j] = 0xBA;//包尾
	j++;
	Send_Buf[j] = 0xBB;//包尾
	j++;
	IOT_Send_Data(Send_Buf,j);
}

/*0x8C****************************************************
 * 函数名: IOT_Auto_Watt_Meter_Report
 * 描述  ：自动上报    等待 0x8D 回应
 * 输入  ：Num: 上报个数
 * 输出  ：无
 ****************************************************/
void IOT_Auto_Light_Report(u8 Num)
{
	u8 i = 0;
	u8 j = 0;	
	u8 Send_Buf[30] = {0xAA,0xAB,0x30,0x88,0x0B,0x05,      0x00,0x00,  0x00,0x00,  0x00,0x00,  0x00,0x00,  0x00,0x00,    0x00,0xBA,0xBB};
		
	if(Num >  5)
		Num = 5;
		
	Send_Buf[3] = 0x88;    //数据类型
	Send_Buf[4] = Num*3+1; //数据长度
  Send_Buf[5] = Num;
	j = 6;
	for(i = 0;i < Num;i++) //编号 数据
	{	
		Send_Buf[j] = Box_IOT.Light_State[i]/256;
		j++;
	}
	Send_Buf[j] = 0x00;//校验
	j++;
	Send_Buf[j] = 0xBA;//包尾
	j++;
	Send_Buf[j] = 0xBB;//包尾
	j++;
	IOT_Send_Data(Send_Buf,j);
}


/*****************************************************
 * 函数名: IOT_Auto_Report_Fun
 * 描述  ：自动上报函数    
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void IOT_Auto_Report_Fun(u8 N)
{
	switch(N)
	{
		case 0:IOT_Auto_Temperature_Report(6);
			break;
		case 1:IOT_Auto_Hum_Report(2);
			break;
		case 2:IOT_Auto_Bright_Report(2);
			break;
		case 3:IOT_Auto_Watt_Meter_Report(5);
			break;
		case 4:IOT_Auto_Light_Report(5);
			break;
		default :
			break;
	}	
}


/*********************************************故障码***********************************************/
/*0x8E****************************************************
 * 函数名: CON_Fault_Code_Report
 * 描述  ：上报故障码        等待 0x8F的数据
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void IOT_Fault_Code_Report(u8 State)
{
	u8 Send_Buf[9] = {0xAA,0xAB,0x20,0x3E,0x01,0x00,0x00,0xBA,0xBB};

	IOT_Send_Data(Send_Buf,9);
}

/*********************************************包头包尾***********************************************/
/*****************************************************
 * 函数名: IOT_Head_END_Type_LEN_Check
 * 描述  ：检测 包头 包尾 设备类型 数据长度
 * 输入  ：buf 			接收数据
 *         len   		接收数据长度
 * 输出  ：验证通过 返回1 否则返回 0
 ****************************************************/
u8 IOT_Head_END_Type_LEN_Check(u8* buf,u8 len)
{
	if((buf[0]*256+buf[1] != Head)||(buf[len-2]*256+buf[len-1] != END))//包头包尾
		return 0;
	if(buf[3] == 0x0F)
	{
		if(buf[2] != CTR_Device_Type)//设备类型
			return 0;
	}
	else
	{	
		if(buf[2] != IOT_Device_Type)//设备类型
			return 0;
	}
	if(buf[4] != len - 8)				 //数据长度
		return 0;
	
	return 1;	
}

/*********************************************空调控制程序***********************************************/
/****************************************************
 * 函数名：Air_Switch_Send_Data
 * 描述  ：空调开关指令
 * 输入  ：Value 数据值
 * 输出  ：无
 ****************************************************/
void Air_Switch_Send_Data(u8 Value)
{
	u8 Txbuf[8] = {0x03,0x06,0x00,0x01,0x00,0x01,0x00,0x00};
	if(Value < 2)
	{
		Txbuf[5] = Value +1;
	  ADD_CRC(Txbuf,6);            //加上CRC
		RS485_1_Send_Data(Txbuf,8);   //通过485发送数据
		Box_IOT.Air_Con_State = Value;
	}
}

/****************************************************
 * 函数名：Air_Mode_Send_Data
 * 描述  ：空调模式切换
 * 输入  ：Value 数据值
 * 输出  ：无
 ****************************************************/
void Air_Mode_Send_Data(u8 Value)
{
	u8 Txbuf[8] = {0x03,0x06,0x00,0x02,0x00,0x01,0x00,0x00};
	if(Value < 5)
	{
		Txbuf[5] = Value;
	  ADD_CRC(Txbuf,6);            //加上CRC
		RS485_1_Send_Data(Txbuf,8);   //通过485发送数据
		Box_IOT.Air_Con_Mode = Value;
	}
}
/****************************************************
 * 函数名：Air_Temp_Send_Data
 * 描述  ：空调温度设置
 * 输入  ：Value 数据值
 * 输出  ：无
 ****************************************************/
void Air_Temp_Send_Data(u8 Value)
{
	u8 Txbuf[8] = {0x03,0x06,0x00,0x03,0x00,0x01,0x00,0x00};
	if((Value < 31)&&(Value > 16))
	{
		Txbuf[5] = Value;
	  ADD_CRC(Txbuf,6);            //加上CRC
		RS485_1_Send_Data(Txbuf,8);   //通过485发送数据
		Box_IOT.Air_Con_Tem = Value;
	}
}

/****************************************************
 * 函数名：IOT_Set_AIR_Request
 * 描述  ：空调控制

 * 输入  ：Value 数据值
 * 输出  ：无
 ****************************************************/
void IOT_Set_AIR_Request(u8 Mode,u8 Value )
{
	switch(Mode)
	{
		case 1://开关模式
			Air_Switch_Send_Data(Value);
			break;
		case 2://模式设置
			Air_Mode_Send_Data(Value);
			break;
		case 3://温度设置
			Air_Temp_Send_Data(Value);
			break;
		default:
			break;		
	}
	
}

/****************************************************
 * 函数名：IOT_AIR_Request
 * 描述  ：空调控制
 * 输入  ：Value 数据值
 * 输出  ：无
 ****************************************************/
void IOT_AIR_Request(u8 Len)
{
	u8 Send_Buf[11] = {0xAA,0xAB,0x30,0x69,0x03,  0x00,0x00,0x00,    0x00,0xBA,0xBB};
	if(Len == 0)
	{
		Send_Buf[5] = Box_IOT.Air_Con_State;
		Send_Buf[6] = Box_IOT.Air_Con_Mode;
		Send_Buf[7] = Box_IOT.Air_Con_Tem;
		
		IOT_Send_Data(Send_Buf,11);
	}
}

/*********************************************协议解析***********************************************/
/****************************************************
 * 函数名：Receive_CON_Protocol_Analysis
 * 描述  ：接收数据解析初始化 
 * 输入  ：buf 			接收数据
 *         len   		接收数据长度
 * 输出  ：无
 ****************************************************/
void Receive_IOT_Protocol_Analysis(u8* buf,u8 len)
{	
	//AA AB 02 09 02 10 02 00 BA BB
	if(IOT_Head_END_Type_LEN_Check(buf,len))
	{		
			switch(buf[3])
			{
				case  0x52:													//0x52下发设备信息查询	
					IOT_Device_Info_Response();				//0x53上报设备信息查询应答	？
					break;
				case  0x0F:                         //0x56设备类型信息查询
				  IOT_Device_Type_Response();				//0x57设备类型信息查询应答
					break;
				case  0x58:													//0x58查询温度
					IOT_Temperature_Response(6);  		//0x59应答 5个温度
					break;
				case 0x5A:													//0x5A查询湿度
					IOT_Hum_Response(2);         			//0x5B应答 2个湿度
					break;
				case 0x5C:			  									//0x5C查询电表数据?
					IOT_Watt_Meter_Response(4);		    //0x5D应答
					break;
				case 0x5E:													//0x5E查询亮度?
					IOT_Bright_Response(2);	    			//0x5F应答
					break;
				case 0x60:													//0x60查询亮度设置?
					IOT_Set_Bright_Response();;				//0x61应答
					break;
				case 0x62:																	//0x62设置亮度?
					IOT_Set_Bright_Request(buf[5],buf[6]);		//0x635应答
					break;
				case 0x64:													//0x64查询照明灯状态 
					IOT_Light_Response(buf[5]);				//0x65应答
					break;
				case 0x66:															//0x66设置照明灯
					IOT_Set_Light_Request(buf[5]*256+buf[6],buf[7]);	//0x67应答
					break;	
				case 0x68:															//0x68询问空调
					IOT_AIR_Request(buf[4]);							//0x69
					break;
				case 0x6A:															//0x6A设置空调
					IOT_Set_AIR_Request(buf[5],buf[6]);	//0x6B
					break;
				
				
				
				
	
				case 0x80:													//0x80查询自动上报间隔
					IOT_Interval_Request();						//0x81应答
					break;	
				case 0x82:													//0x82设置自动上报间隔
					IOT_Set_Interval_Request(buf[5]*256+buf[6]);//0x81应答
					break;
				
				default:
					break;
			}
	}
	
}
 


/*****************************************************
EEPROM  1.自动上报数据  2.故障码
 ****************************************************/
/*****************************************************
 * 函数名: IOT_Read_EEPROM_Data
 * 描述  ：读取EEPROM的配置信息
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void IOT_Read_EEPROM_Data(void)
{
	Read_EEPROM_Light_State();
	Read_EEPROM_Interval_Time();
	
	Box_IOT.Tem_Fault_Code[3] = 0x01;
	Box_IOT.Hum_Fault_Code[1] = 0x01;
	Box_IOT.Light_Fault_Code[4] = 1;
	Box_IOT.Bright_Fault_Code[0]= 1;
	Box_IOT.Watt_Fault_Code = 2;
	Box_IOT.Air_Con_State = 2;
	Box_IOT.Air_Con_Fault_Code = 2;
}

/*****************************************************
 * 函数名: IOT_Device_Init
 * 描述  ：开始设备初始化
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void IOT_Device_Init(void)
{
	u8 i = 0;
	for(i = 0;i < Tem_Num;i++)
		Box_IOT.Tem[i] = 0;
	
	for(i = 0;i < Hum_Num;i++)
		Box_IOT.Hum[i] = 0;
	
}
/*****************************************************
 * 函数名: IOT_Receive_Protocol
 * 描述  ：读取上位机指令并解析
 * 格式  ：0xAAAB	N	0x03	 len(1Byte)		data (n Bytes)	和校验	0xBABB
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void IOT_Receive_Protocol(void)
{
	u8 RE_BUF[128] = {0};
	u8 len = 0;
	u32 T = 0;
	u8 N = 0;
	
	IOT_Device_Init();
	IOT_Read_EEPROM_Data();					//读取EEPROM数据
  IOT_Start_Report_Device_Type();	//上报设备类型
	delay_ms(50);
	IOT_Start_Report_Check();			 	//上报开机自检信息
	
	while(1)
	{
		
		IOT_Receive_Data(RE_BUF,&len);  
		if(len)                											//收到数据                			 
		{
			Receive_IOT_Protocol_Analysis(RE_BUF,len);		//解析数据
			len = 0;  																	//数据长度清空
		}
		T++;
		if(T == Box_IOT.Interval*100)//10ms*100=1s
		{
			T = 0;
			N++;
			if(N == 5)
				N = 0;
//			IOT_Auto_Report_Fun(N);
		}
		
		IOT_Read_LGT_KEY();									//开门检测
	}
}















/*********************************************Modbus 温湿度解析***********************************************/
#define Modbus_ADD 0x01
/**************************************************************
 * 函数名：RS485_Add_TEST
 * 描述  ：判断数据地址
 * 实例  : 02 03 00 05 00 02 CRC
 * 输入  ：rsbuf：接受的数据 
 * 输出  : 1：本机 0：非本机
 **************************************************************/
u8 Modbus_Add_TEST(u8* buf)
{
	if(buf[0] == Modbus_ADD)       //验证数据是否是发给本机数据
		return 1;
	else
		return 0;
}

/**************************************************************
 * 函数名：Read_Modbus_Add
 * 描述  ：读取Modbus设备地址
 * 输入  ：buf：接受的数据 
 * 输出  : modbus地址
 **************************************************************/
u8 Read_Modbus_Add(u8* buf)
{
	return buf[0];
}

/**************************************************************
 * 函数名：Function_Code_Len_TEST
 * 描述  ：判断功能码和数据长度
 * 实例  : 01 04 04 01 31 02 22 2A CE
 * 输入  ：rsbuf：接受的数据  FUN_Code功能码 Data_LEN 数据长度
 * 输出  : 1：本机 0：非本机
 ***********************************************************/
u8 Function_Code_Len_TEST(u8* buf,u8 FUN_Code,u8 Data_LEN)
{
	if((buf[1]==FUN_Code)&&(buf[2]==Data_LEN))
		return 1;
	else
		return 0;
}

/**************************************************************
 * 函数名：Stack_Average_Value
 * 描述  ：堆栈缓存区返回平局值
 * 输入  ：serial 序号 data 数据 Depth 堆栈深度
 *         结构 0 位 堆栈指针位置
 * 输出  ：无
 ***********************************************************/
u16 Stack_Average_Value(u8 Serial,u16 Data,u8 Depth)
{
	u8 j  = 0;
	u32 Sum = 0;
	
	u8 i = Box_IOT.Stack_Cahe[Serial][0];
	i++;
	Box_IOT.Stack_Cahe[Serial][i] = Data;
	
	if(i == Depth)
		i = 0;
	Box_IOT.Stack_Cahe[Serial][0] = i;
	
//	printf("***Stack:%d ",Serial);
//	for(j = 0;j <= Depth;j++)
//		printf(" %d ",Box_IOT.Stack_Cahe[Serial][j]);
	
	
	for(j = 1;j <= Depth;j++)
		Sum += Box_IOT.Stack_Cahe[Serial][j];
//	printf("Ave:%d\r\n", Sum/Depth);
	return Sum/Depth;
	
}
/**************************************************************
 * 函数名：Receive_IOT_Modbus_TH_Analysis
 * 描述  ：接收温湿度数据解析
 * 输入  ：buf 			接收数据
 *         len   		接收数据长度
 *         01 04 04 01 31 02 22 2A CE
 * 输出  ：无
 ***********************************************************/
void Receive_IOT_Modbus_TH_Analysis(u8* buf,u8 len)
{
	u16 T,H;	
	u8 Serial = 0;
	u16 Data = 0;
	
	if(CRC_TEST(buf,len))              		     //判断CRC校验
	{ 
		if(Function_Code_Len_TEST(buf,0x03,0x04))//读取温湿度数据
		{
			if(buf[0]<4)
			{	
				//温度0-5 6个  湿度6-7 2个
				Serial = buf[0]-1;
				Data = buf[3]*256+buf[4]-2000;
				Box_IOT.Tem[buf[0]-1] = Stack_Average_Value(Serial,Data,5);  

				Serial = buf[0]-1 +6;
				Data = buf[5]*256+buf[6];
				Box_IOT.Hum[buf[0]-1] = Stack_Average_Value(Serial,Data,5); 
							
//				Box_IOT.Tem[buf[0]-1] =  buf[3]*256+buf[4]-2000;
//				Box_IOT.Hum[buf[0]-1] = buf[5]*256+buf[6];
				T = buf[3]*256+buf[4]-2000;
				H = buf[5]*256+buf[6];  
				printf("ADD:%d   T = %d  %d   H = %d  %d \r\n",buf[0],T,Box_IOT.Tem[buf[0]-1],H,Box_IOT.Hum[buf[0]-1]);
			}
		}
		else
		{
//			printf("Function ERROR\r\n");
		}
	}
	else
	{;
//		printf("CRC ERROR\r\n");
	}
		
}
#define TH_Time 100
/*****************************************************
 * 函数名: IOT_Modbus_Read_TH
 * 描述  ：通过Modbus读取温湿度
 * 格式  ：发送：01 03 00 01 00 02 95 CB
 *         返回：01 03 04 13 4D 0B 6E E8 7C 
 *         
 *         温度 地址01 (寄存机值-2000)/100   
 *         湿度 地址02 寄存机值/100
 *         H 0x222=546/10=54.6
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void IOT_Modbus_Read_TH(void)
{
	u8 RE_BUF[128] = {0};
	u8 Send_BUF[10]  = {0x01,0x03,0x00,0x01,0x00,0x02,0x95,0xcb};
	u8 Send_BUF2[10] = {0x02,0x03,0x00,0x01,0x00,0x02,0x95,0xf8};
	u8 len = 0;
	u16 T = 0;
	printf("IOT_Modbus_Read_TH\r\n");

	RS485_1_Send_Data(Send_BUF,8);
	while(1)
	{
		
		RS485_1_Receive_Data(RE_BUF,&len);
		
		if(len)                											//收到数据                			 
		{
			Print_RS485_Receive_data(RE_BUF,len);	
			Receive_IOT_Modbus_TH_Analysis(RE_BUF,len);		//解析数据
			len = 0;  																	  //数据长度清空
			delay_ms(100);
		}
		T++;																						//10mS
		if(T == TH_Time/2)
		{
					
			RS485_1_Send_Data(Send_BUF,8);			
		}
		if(T == TH_Time)
		{
			T = 0;			
			RS485_1_Send_Data(Send_BUF2,8);			
		}
		
		
	}
}





/*****************************************************
 * 函数名: Current_Converts_Temperature
 * 描述  ：4-20MA 转换为数字温度
 * 输入  ：Data AIN0-3转换的u16数据
 *  			 AIN0-1 对应 Temp4-5
* 计算  ：-50-100度 对应4-20mA   AD:100R电阻 量程2.048
 * 输出  ：无
 ****************************************************/
void Current_Converts_Temperature(const u16* Data)
{
	float I0,I1;
	int T0,T1;
	I0 = Data[0]*0.000625;     //I1 = Data[0]*2048.0/32768/100;
	I1 = Data[1]*0.000625;     //I1 = Data[0]*2048.0/32768/100;
	
	T0 = (int)(((I0-4)*9.375-50)*10);
	T1 = (int)(((I1-4)*9.375-50)*10);
//	printf("Data:%d   %d    %d    %d \r\n",Data[0]	,Data[1],Data[2],Data[3]);	
	
	if(T0>0)
	{
		Box_IOT.Tem[3] = T0;
		
	}
	else
	{
		Box_IOT.Tem[3] = T0+65536;
	}
//		printf("T0:%d   %d   %d\r\n",Data[0],T0,Box_IOT.Tem[3]);
	if(T1>0)
	{
		Box_IOT.Tem[4] = T1;
		
	}
	else
	{
		Box_IOT.Tem[4] = T1+65536;
		
	}
//		printf("T1:%d   %d   %d\r\n",Data[1],T1,Box_IOT.Tem[4]);
	
	
	
}


/*****************************************************
 * 函数名: IOT_Read_4_20mA
 * 描述  ：4-20MA 采集
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void IOT_Read_4_20mA(void)
{
	u8 DAQ_Num = 0;
	u8 i = 0;
	u8 Read_Data[2] = {0};
	u32 SUM[4] = {0};
	u8 V_Num = 0;

	u16 Msg_Data[6] = {0};
	u16 *p = Msg_Data;
	u8 err = 0;
	u16 T[4] = {0};
	float I = 0;
	
	printf("ADS1115\r\n");
	while(1)//2s更新一次数据
	{			
			I2C_ReadNByte (0x90, 0x01, 0x00, Read_Data, 2);
			switch(DAQ_Num/50)
			{
				case 3: //AIN0
						if(DAQ_Num%50 < 5)
							InitADS1115(0x03);													//2.048
						else																					//前5个数据不要 
						{
							if((Read_Data[0]*256+Read_Data[1])<32768)
								SUM[0] += Read_Data[0]*256+Read_Data[1];																	
						}							
						break;				
				case 1:		//AIN1																			
						if(DAQ_Num%50 < 5)
							InitADS1115(0x13);													//2.048								
						else						
							SUM[1] += Read_Data[0]*256+Read_Data[1];	
						break;				
				case 2:	//AIN2																	 
						if(DAQ_Num%50 < 5)
							InitADS1115(0x23);													//2.048
						else						
							SUM[2] += Read_Data[0]*256+Read_Data[1];	
						break;				
				case 0:	//AIN3	 																//2.048
						if(DAQ_Num%50 < 5)
							InitADS1115(0x33);
						else						
							SUM[3] += Read_Data[0]*256+Read_Data[1];	
						break;	
			}
			DAQ_Num++;
			if(DAQ_Num == 200)  //200*10 = 2s
			{
					//复制数据进入队列
					
					for(i = 0;i < 4;i++)
					{
						Msg_Data[i] = SUM[i]/45;
						T[i] = (int)(((Msg_Data[i]*20.48/32768-4)*9.375-50)*100);
						if(i <2)
						 	Box_IOT.Tem[i+3] = Stack_Average_Value(3+i,T[i],5);					
					}	
					
//					T[0] = (int)(((Msg_Data[0]*20.48/32768-4)*9.375-50)*100);
//					T[1] = (int)(((Msg_Data[1]*20.48/32768-4)*9.375-50)*100);
//					T[2] = (int)(((Msg_Data[2]*20.48/32768-4)*9.375-50)*100);
//					T[3] = (int)(((Msg_Data[3]*20.48/32768-4)*9.375-50)*100);
					printf("AD0:%d  %d AD1:%d %d  AD2:%d AD3:%d\r\n",Msg_Data[0],T[0],Msg_Data[1],T[1],Msg_Data[2],Msg_Data[3]);
//					printf("T :%d  %d     %d %d %d  AD:%d   %d    %d    %d \r\n",Box_IOT.Tem[3],T[0],Box_IOT.Tem[4],Box_IOT.Tem[5],T[3],Msg_Data[0]	,Msg_Data[1],Msg_Data[2],Msg_Data[3]);	
//					Current_Converts_Temperature(Msg_Data);			
					LED1=0;
					//清空
					DAQ_Num = 0;					
					for(i = 0;i < 4;i++)
						SUM[i] = 0;		
			}	
			delay_ms(10);
	}	
}





/*****************************************************
 * 函数名: IOT_STM32_ADC
 * 描述  ：STM32 内部AD数据采集
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void IOT_STM32_ADC(void)
{
	u16 Hex = 0;
	
	while(1)
	{
		Hex = Get_Adc_Average(ADC_Channel_14,10);
		printf("ADC_Channel_14:%d\r\n",Hex);
		delay_ms(500);
		
	}
}






