#include "GPIO.h"
#include "delay.h"
#include "usart.h"
#include "HS_BOX_CON.h"
#include "DEBUG.h"
/*
**********************************集装箱****************************************
***设备：1.补货机构    电机						动作：正转、反转、停止（1-3）
																			状态：左、中、右、未到位（1-4）
***设备：2.集装箱门		 1# 2# 3#  			状态：关闭、打开（0-1）
***设备：3.堆垛车      								状态：空闲、忙碌、报错（0-2）
***设备：4.安全门      1# 2#  				状态：关闭、打开（0-1）
***设备：5.buffer位    1# 2# 3#      	状态：在位、不在位（0-1）
***设备：6.急停按钮    1# 2#          状态：未按下、按下（0-1）
 */ 


typedef struct {
	u8 Replenish_Motor_Action;//补货电机动作状态 0停止 1 左转 2 右转  3到中点  4 到零点
	u8 Replenish_Motor_State; //补货电机位置  状态：左、中、右、未到位 故障 零点（1 2 4 0 5 3）
	u8 Replenish_Motor_Site; 	//补货电机位置  通过旋转判断电机位置 左、中、右、未到位 故障 零点（1 2 4 0 5 3）
	u8 Replenish_Motor_Last_State; //上一状态	
	u8 Replenish_S_Con_State; //存货柜状态  0  旋转机构上无存货柜
	u8 Replenish_Receive_Com; //补货装置接收到的命令 0：不旋转 1 转到右侧 2转到左侧 3转到中点 4转到零点
	u8 Replenish_Motor_Move;  //电机旋转方向 0：不旋转 1 中点转到右侧-向右转 2中点转到左侧-向左转 3右侧转到中点-向左转 4左侧转到中点-向右旋转 5；零点转到中点-向左转 6.中点、不在位转到零点-向右转
	u8 Replenish_Key_State;   //补货按钮&&门的状态 1触发 0未出
	
	u8 Con_Door_State[3];     //集装箱门状态
	u8 Con_Door_State_Code;   //集装箱门状态码
	u8 Piling_Car_State;			//堆垛车状态
	u8 Safety_Door_State[2];  //安全门状态
	u8 Safety_Door_State_Code;//安全门门状态码
	u8 buffer_State[3];       //buffer 位状态
	u8 buffer_State_Code;			//buffer 状态码
	u8 E_Stop_State;					//紧急停止按钮
	u32 Fault_Code;           //故障码
}HS_Box_CON;

#define CTR_Device_Type  0x00 //Box Contrl 地址
#define CON_Device_Type  0x20 //BOX CON 集装箱控制板地址

#define Head 0xAAAB
#define END  0xBABB


HS_Box_CON Box_CON;

void CON_Replenish_Motor_Action_Response(u8 Value);
//void CON_Start_SCAN_Report(void);
//void CON_SCAN_Report(void);
//void CON_Read_REP_Position(void);//读取一次补货机构位置
void CON_Reporte_Door_State(u8 Door);//触发上报门状态
void CON_SDR_OPEN_EN(u8 Enable);			//安全门是否可以打开

void CON_Safety_Door_State_Report(u8 Door);//上报安全门状态变化
void CON_Buffer_State_Report(u8 Buffer);//上报buffer状态变化
void CON_E_Stop_State_Report(void);     //上报急停按钮状态
/*********************************************底层串口驱动***********************************************/
/*****************************************************
 * 函数名: CON_Send_Data
 * 描述  ：集装箱设备上报
 * 输入  ：buf 数据组 len 数据长度
 * 输出  ：无
 ****************************************************/
void CON_Send_Data(u8* buf,u8 len)
{
	 USART2_Send_Data(buf,len);
}

/*****************************************************
 * 函数名: CON_Receive_Data
 * 描述  ：集装箱设备接收数据
 * 输入  ：buf 数据组 len 数据长度
 * 输出  ：无
 ****************************************************/
void CON_Receive_Data(u8* RSbuf,u8* rs_len)
{
	USART2_Receive_Data(RSbuf,rs_len);
}


/*********************************************电机旋转驱动***********************************************/
/*****************************************************
 * 函数名: Motot_Turn_Right
 * 描述  ：补货电机向右旋转
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void Motor_Turn_Right(void)
{
		M_REP_DIR = 1;         //方向
		M_REP_Start = 1;       //启动 取消刹车
}
/*****************************************************
 * 函数名: Motot_Turn_Left
 * 描述  ：补货电机向左旋转
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void Motor_Turn_Left(void)
{
		M_REP_DIR = 0;         //方向
		M_REP_Start = 1;       //启动 取消刹车
}
/*****************************************************
 * 函数名: Motot_Turn_Stop
 * 描述  ：补货电机停止
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void Motor_Turn_Stop(void)
{
		M_REP_DIR = 0;         //方向
		M_REP_Start = 0;       //启动 取消刹车
}
/*********************************************故障码**********************************************/
/*0x3C****************************************************
 * 函数名: CON_Fault_Code_Report
 * 描述  ：上报故障码        等待 0x3D的数据
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_Fault_Code_Report(u16 Fault_Code)
{
	u8 Send_Buf[15]  = {0xAA,0xAB,0x20,0x3C,0x03,0x02,0x00,0x00,0x00,0xBA,0xBB};

	Send_Buf[6] = Fault_Code/256;
	Send_Buf[7] = Fault_Code%256;	
	
	CON_Send_Data(Send_Buf,11);
}

/*????*******************************************设备信息**********************************************/
/*0x20****************************************************
 * 函数名: CON_Device_State
 * 描述  ：设备状态信息
 * 输入  ：Type ：类型
 * 输出  ：无
 ****************************************************/
void CON_Device_State(u8 Type)
{
	u8 i = 0;
	
	u8 Send_Buf[30] = {0xAA,0xAB,0x20};	
	
	CON_SCAN_Device_State();//扫描设备状态	
	
	Send_Buf[3] = Type;
	
	i = 5;
	Send_Buf[i] = Box_CON.Replenish_Motor_State;//补货机构位置
	i++;
	Send_Buf[i] = 3;//集装箱门数量
	i++;
	Send_Buf[i] = Box_CON.Con_Door_State[0];//集装箱门状态
	i++;
	Send_Buf[i] = Box_CON.Con_Door_State[1];
	i++;
	Send_Buf[i] = Box_CON.Con_Door_State[2];
	
	i++;
	Send_Buf[i] = 2;//安全门数量
	i++;
	Send_Buf[i] = Box_CON.Safety_Door_State[0];//安全门状态
	i++;
	Send_Buf[i] = Box_CON.Safety_Door_State[1];
	
	i++;
	Send_Buf[i] = 3;//buffer 数量
	i++;
	Send_Buf[i] = Box_CON.buffer_State[0];//buffer 状态
	i++;
	Send_Buf[i] = Box_CON.buffer_State[1];
	i++;
	Send_Buf[i] = Box_CON.buffer_State[2];
	
	i++;
	Send_Buf[i] = Box_CON.E_Stop_State;//急停状态
	
	i++;
	Send_Buf[i] = 0;//校验
	
	i++;
	Send_Buf[i] = 0xBA;//校验
	i++;
	Send_Buf[i] = 0xBB;//校验
	
	i++;//数量
	Send_Buf[4] = i-8;//数据长度
	
	CON_Send_Data(Send_Buf,i);
}

/*0x20****************************************************
 * 函数名: CON_Start_Report_Check
 * 描述  ：开机上报设备信息   等待0x21 
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_Start_Report_Check()
{
	CON_Device_State(0x20);
}
/*0x23****************************************************
 * 函数名: CON_Device_Info_Response
 * 描述  ：设备信息查询   回复0x22
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_Device_Info_Response(void)
{
	CON_Device_State(0x23);
}


/*********************************************设备类型***********************************************/
/*0x24****************************************************
 * 函数名: Start_Report_Device_Type
 * 描述  ：开机上报设备类型 等待 0x25
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_Start_Report_Device_Type(void)
{
	u8 Send_Buf[9] = {0xAA,0xAB,0x20,0x24,0x01,0x20,0x00,0xBA,0xBB};
	CON_Send_Data(Send_Buf,9);
}

/*0x27****************************************************
 * 函数名: CON_Device_Type_Response
 * 描述  ：设备类型应答  回复 0x26
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_Device_Type_Response(void)
{
	u8 Send_Buf[9] = {0xAA,0xAB,0x20,0x27,0x01,0x20,0x00,0xBA,0xBB};
	CON_Send_Data(Send_Buf,9);
}



/*********************************************补货位***********************************************/
/*****************************************************
 * 函数名: CON_Replenish_Motor_Stop
 * 描述  ：补货位电机停止
  #define RM0          //刹车 0启动刹车 1取消刹车
  #define RM1          //启动停止 0停止 1启动
  #define RM2          //方向控制 0正转 1反转
 ****************************************************/
void CON_Replenish_Motor_Stop(void)
{
		M_REP_Start = 0;//停止 刹车
	  M_REP_DIR = 0;  //关闭方向继电器
	  Box_CON.Replenish_Motor_Action = 0;//状态设置为停止       
}

#define TIME_CON_REP  2500  //ms 超时时间单位ms


/*****************************************************
 * 函数名: Read_REP_Sit_State
 * 描述  ：读取补货位位置状态
 ****************************************************/
u8 Read_REP_Sit_State(void)
{
	u8 REP_TEMP = 0;	
	REP_TEMP = REP_KEY_SCAN(); 
	
	switch(REP_TEMP)
	{
		case 0:
			Box_CON.Replenish_Motor_Site = 0;//补货电机位置   左、中、右、未到位 故障 零点（1 2 4 0 5 3）  不在位
			break;
		case 1:
			Box_CON.Replenish_Motor_Site = 1;//补货电机位置   左、中、右、未到位 故障 零点（1 2 4 0 5 3）  左
			break;
		case 2:
			Box_CON.Replenish_Motor_Site = 2;//补货电机位置   左、中、右、未到位 故障 零点（1 2 4 0 5 3）  中点
			break;
		case 4:
			Box_CON.Replenish_Motor_Site = 4;//补货电机位置   左、中、右、未到位 故障 零点（1 2 4 0 5 3）  右
			break;
		case 8:
			Box_CON.Replenish_Motor_Site = 3;//补货电机位置   左、中、右、未到位 故障 零点（1 2 4 0 5 3）   零点
			break;
		default:
			Box_CON.Replenish_Motor_Site = 5;//补货电机位置   左、中、右、未到位 故障 零点（1 2 4 0 5 3）   故障
			break;
	}	
}

/*****************************************************
 * 函数名: CON_SCAN_Report
 * 描述  ：补货机构触发停止上报
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_REP_SCAN_Report(void)
{	
	u8 REP_TEMP = 0;
	u8 KEY = 0;
	u8 Send_Buf[9] = {0xAA,0xAB,0x20,0x2B,0x01,0x01,0x00,0xBA,0xBB};//到位
	u8 Send_Timeout_Buf[9] = {0xAA,0xAB,0x20,0x2B,0x01,0x00,0x00,0xBA,0xBB};//超时未到位
	u8 Send_Suspend_Buf[9] = {0xAA,0xAB,0x20,0x2B,0x01,0x03,0x00,0xBA,0xBB};//旋转中止
	static u16 T = 0;
	
	REP_TEMP = Read_REP_Sit_State();//读取位置
	KEY = REP_BUT_SCAN();//按键、门磁状态
//	Box_CON.Replenish_Motor_Move =6;  //电机旋转方向 0：不旋转 1 中点转到右侧-向右转 2中点转到左侧-向左转 3右侧转到中点-向左转 4左侧转到中点-向右旋转 5；零点转到中点-向左转 6.中点、不在位转到零点-向右转
	
/*************按键*************/	
	if(KEY == 1)
	{
		switch(Box_CON.Replenish_Motor_Move)//电机旋转方向 0：不旋转 1 中点转到右侧-向右转 2中点转到左侧-向左转 3右侧转到中点-向左转 4左侧转到中点-向右旋转 5；零点转到中点-向左转 6.中点、不在位转到零点-向右转
		{
			case 1://中点转到右侧-向右转
				Motor_Turn_Right();
				T++;	//超时记录 一次累积时间10ms
			break;
			case 2://中点转到左侧-向左转
				Motor_Turn_Left();
				T++;	//超时记录 一次累积时间10ms
			break;
			case 3://右侧转到中点-向左转 
				Motor_Turn_Left();
				T++;	//超时记录 一次累积时间10ms
			break;
			case 4://左侧转到中点-向右旋转 
				Motor_Turn_Right();
				T++;	//超时记录 一次累积时间10ms
			break;
			case 5://零点转到中点-向左转
				Motor_Turn_Left();
				T++;	//超时记录 一次累积时间10ms
			break;
			case 6://中点、不在位转到零点-向右转
				Motor_Turn_Right();
				T++;	//超时记录 一次累积时间10ms
			break;
			default:
				break;		
		}		
		Box_CON.Replenish_Key_State = KEY;
	}
	else
	{
		Motor_Turn_Stop();//停止
		if(Box_CON.Replenish_Key_State == 1)
		{
			CON_Send_Data(Send_Suspend_Buf,9);//上报中止
		}
		Box_CON.Replenish_Key_State = 0;		
	}
	
/************************超时**********************/	
	if((Box_CON.Replenish_Motor_Move>0)&&(Box_CON.Replenish_Motor_Move<5))
	{
		if(T == TIME_CON_REP/10)	
		{
			Motor_Turn_Stop();//停止
			Box_CON.Replenish_Motor_Move =0;  //电机旋转方向 0：不旋转 1 中点转到右侧-向右转 2中点转到左侧-向左转 3右侧转到中点-向左转 4左侧转到中点-向右旋转 5；零点转到中点-向左转 6.中点、不在位转到零点-向右转
			CON_Send_Data(Send_Timeout_Buf,9);//上报超时停止
			T = 0;	//超时记录 清空
		}			
	}
	if((Box_CON.Replenish_Motor_Move==5)||(Box_CON.Replenish_Motor_Move==6))
	{
		if(T == TIME_CON_REP/5)	
		{
			Motor_Turn_Stop();//停止
			Box_CON.Replenish_Motor_Move =0;  //电机旋转方向 0：不旋转 1 中点转到右侧-向右转 2中点转到左侧-向左转 3右侧转到中点-向左转 4左侧转到中点-向右旋转 5；零点转到中点-向左转 6.中点、不在位转到零点-向右转
			CON_Send_Data(Send_Timeout_Buf,9);//上报超时停止
			T = 0;	//超时记录 清空
		}			
	}	
			
/************************限位检测**********************/	
	switch(Box_CON.Replenish_Motor_Move)//电机旋转方向 0：不旋转 1 中点转到右侧-向右转 2中点转到左侧-向左转 3右侧转到中点-向左转 4左侧转到中点-向右旋转 5；零点转到中点-向左转 6.中点、不在位转到零点-向右转
	{
		case 1://中点转到右侧-向右转
			if(Box_CON.Replenish_Motor_Site == 4)//补货电机位置   左、中、右、未到位 故障 零点（1 2 4 0 5 3）   
			{
				Motor_Turn_Stop();//停止
				Box_CON.Replenish_Motor_Move =0;  //电机旋转方向 0：不旋转 1 中点转到右侧-向右转 2中点转到左侧-向左转 3右侧转到中点-向左转 4左侧转到中点-向右旋转 5；零点转到中点-向左转 6.中点、不在位转到零点-向右转
				CON_Send_Data(Send_Buf,9);//上报到位
				T = 0;	//超时记录 清空
			}
			break;
		case 2://中点转到左侧-向左转
			if(Box_CON.Replenish_Motor_Site == 1)//补货电机位置   左、中、右、未到位 故障 零点（1 2 4 0 5 3）   
			{
				Motor_Turn_Stop();//停止
				Box_CON.Replenish_Motor_Move =0;  //电机旋转方向 0：不旋转 1 中点转到右侧-向右转 2中点转到左侧-向左转 3右侧转到中点-向左转 4左侧转到中点-向右旋转 5；零点转到中点-向左转 6.中点、不在位转到零点-向右转
				CON_Send_Data(Send_Buf,9);//上报到位
				T = 0;	//超时记录 清空
			}
			break;
		case 3://右侧转到中点-向左转 
			if(Box_CON.Replenish_Motor_Site == 2)//补货电机位置   左、中、右、未到位 故障 零点（1 2 4 0 5 3）   
			{
				Motor_Turn_Stop();//停止
				Box_CON.Replenish_Motor_Move =0;  //电机旋转方向 0：不旋转 1 中点转到右侧-向右转 2中点转到左侧-向左转 3右侧转到中点-向左转 4左侧转到中点-向右旋转 5；零点转到中点-向左转 6.中点、不在位转到零点-向右转
				CON_Send_Data(Send_Buf,9);//上报到位
				T = 0;	//超时记录 清空
			}
			break;
		case 4://左侧转到中点-向右旋转 
			if(Box_CON.Replenish_Motor_Site == 2)//补货电机位置   左、中、右、未到位 故障 零点（1 2 4 0 5 3）   
			{
				Motor_Turn_Stop();//停止
				Box_CON.Replenish_Motor_Move =0;  //电机旋转方向 0：不旋转 1 中点转到右侧-向右转 2中点转到左侧-向左转 3右侧转到中点-向左转 4左侧转到中点-向右旋转 5；零点转到中点-向左转 6.中点、不在位转到零点-向右转
				CON_Send_Data(Send_Buf,9);//上报到位
				T = 0;	//超时记录 清空
			}
			break;
		case 5://零点转到中点-向左转
			if(Box_CON.Replenish_Motor_Site == 2)//补货电机位置   左、中、右、未到位 故障 零点（1 2 4 0 5 3）   
			{
				Motor_Turn_Stop();//停止
				Box_CON.Replenish_Motor_Move =0;  //电机旋转方向 0：不旋转 1 中点转到右侧-向右转 2中点转到左侧-向左转 3右侧转到中点-向左转 4左侧转到中点-向右旋转 5；零点转到中点-向左转 6.中点、不在位转到零点-向右转
				CON_Send_Data(Send_Buf,9);//上报到位
				T = 0;	//超时记录 清空
			}
			break;
		case 6://中点、不在位转到零点-向右转
			if(Box_CON.Replenish_Motor_Site == 3)//补货电机位置   左、中、右、未到位 故障 零点（1 2 4 0 5 3）   
			{
				Motor_Turn_Stop();//停止
				Box_CON.Replenish_Motor_Move =0;  //电机旋转方向 0：不旋转 1 中点转到右侧-向右转 2中点转到左侧-向左转 3右侧转到中点-向左转 4左侧转到中点-向右旋转 5；零点转到中点-向左转 6.中点、不在位转到零点-向右转
				CON_Send_Data(Send_Buf,9);//上报到位
				T = 0;	//超时记录 清空
			}
			break;
		default://其他
			break;
	}	
}

/*****************************************************
 * 函数名: CON_Read_REP_Position
 * 描述  ：读取一次补货机构位置
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_Read_REP_Position(void)
{
	Box_CON.Replenish_Motor_State = REP_KEY_SCAN();  
	Box_CON.Replenish_Motor_Site = Box_CON.Replenish_Motor_State;
}


/*0x29****************************************************
 * 函数名: CON_Replenish_State_Response
 * 描述  ：补货位电机位置应答    回应 0x28
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_Replenish_State_Response(void)
{
	u8 Send_Buf[9] = {0xAA,0xAB,0x20,0x29,0x01,0x01,0x00,0xBA,0xBB};
	
	Read_REP_Sit_State();//读取位置
	Send_Buf[5] = Box_CON.Replenish_Motor_Site;//补货电机位置   左、中、右、未到位 故障 零点（1 2 4 0 5 3）  不在位
	CON_Send_Data(Send_Buf,9);
}


/*****************************************************
 * 函数名: CON_Replenish_Motor_A_Response
 * 描述  ：补货位电机运动方向
 * 输入  ：Dir 0:停止 1:转到右侧 2转到左侧 3：转到中点 4转到零点
 * 输出  ：无
 ****************************************************/
u8  CON_Replenish_Motor_Action(u8 Dir)
{	
	u8 Rep_ERROR = 0;
	u8 Send_Buf[9] = {0xAA,0xAB,0x20,0x2B,0x01,0x01,0x00,0xBA,0xBB};
	
	Read_REP_Sit_State();//确定补货机构位置

	if(DEBUG)
		printf("Receive rotation command  State:%d",Box_CON.Replenish_Motor_Site);//补货电机位置   左、中、右、未到位 故障 零点（1 2 4 0 5 3）

	switch(Dir)//Dir 0:停止 1:转到右侧 2转到左侧 3：转到中点 4转到零点
	{
		case 0://停止
			Motor_Turn_Stop();//停止
		  Box_CON.Replenish_Motor_Move = 0;  //电机旋转方向 0：不旋转 1 中点转到右侧-向右转 2中点转到左侧-向左转 3右侧转到中点-向左转 4左侧转到中点-向右旋转 5；零点转到中点-向左转 6.中点、不在位转到零点-向右转
			break;
	case 1://中点转到右侧
			if(Box_CON.Replenish_Motor_Site == 2)
			{
				Motor_Turn_Right();//向右转
				Box_CON.Replenish_Motor_Move = 1;  //电机旋转方向 0：不旋转 1 中点转到右侧-向右转 2中点转到左侧-向左转 3右侧转到中点-向左转 4左侧转到中点-向右旋转 5；零点转到中点-向左转 6.中点、不在位转到零点-向右转
//				Box_CON.Replenish_Receive_Com = Dir; //补货装置接收到的命令 0：不旋转 1 转到右侧 2转到左侧 3转到中点 4转到零点
			}				
			else
				Rep_ERROR = 1;
			break;
	case 2://中点转到左侧
			if(Box_CON.Replenish_Motor_Site == 2)
			{
				Motor_Turn_Left();//向左转
				Box_CON.Replenish_Motor_Move =2;  //电机旋转方向 0：不旋转 1 中点转到右侧-向右转 2中点转到左侧-向左转 3右侧转到中点-向左转 4左侧转到中点-向右旋转 5；零点转到中点-向左转 6.中点、不在位转到零点-向右转
//				Box_CON.Replenish_Receive_Com = Dir; //补货装置接收到的命令 0：不旋转 1 转到右侧 2转到左侧 3转到中点 4转到零点
			}				
			else
				Rep_ERROR = 1;
			break;
	case 3://转到中点
			switch(Box_CON.Replenish_Motor_Site)
			{
				case 1://在左侧向右旋转
					Motor_Turn_Right();//向右转
				  Box_CON.Replenish_Motor_Move =4;  //电机旋转方向 0：不旋转 1 中点转到右侧-向右转 2中点转到左侧-向左转 3右侧转到中点-向左转 4左侧转到中点-向右旋转 5；零点转到中点-向左转 6.中点、不在位转到零点-向右转
					break;
				case 4://在右侧向左旋转
					Motor_Turn_Left();//向左转
					Box_CON.Replenish_Motor_Move =3;  //电机旋转方向 0：不旋转 1 中点转到右侧-向右转 2中点转到左侧-向左转 3右侧转到中点-向左转 4左侧转到中点-向右旋转 5；零点转到中点-向左转 6.中点、不在位转到零点-向右转
					break;
				case 3://在零点向右旋转180
					Motor_Turn_Right();//向右转
				  Box_CON.Replenish_Motor_Move =5;  //电机旋转方向 0：不旋转 1 中点转到右侧-向右转 2中点转到左侧-向左转 3右侧转到中点-向左转 4左侧转到中点-向右旋转 5；零点转到中点-向左转 6.中点、不在位转到零点-向右转
					break;
				default:
					Rep_ERROR = 0;
					break;
			}
			break;
	case 4://转到零点  中点和不在位
			switch(Box_CON.Replenish_Motor_Site)//补货电机位置   左、中、右、未到位 故障 零点（1 2 4 0 5 3）
			{
				case 0://不在位转到零点 向右旋转
					Motor_Turn_Right();//向右转
				  Box_CON.Replenish_Motor_Move =6;  //电机旋转方向 0：不旋转 1 中点转到右侧-向右转 2中点转到左侧-向左转 3右侧转到中点-向左转 4左侧转到中点-向右旋转 5；零点转到中点-向左转 6.中点、不在位转到零点-向右转
					break;
				case 2://中点转到零点 向右旋转
					Motor_Turn_Right();//向右转
				  Box_CON.Replenish_Motor_Move =6;  //电机旋转方向 0：不旋转 1 中点转到右侧-向右转 2中点转到左侧-向左转 3右侧转到中点-向左转 4左侧转到中点-向右旋转 5；零点转到中点-向左转 6.中点、不在位转到零点-向右转
					break;
				default:
					Rep_ERROR = 0;
					break;
			}		
			break;
		default:
			Rep_ERROR = 0;
		break;
	}
		
	if(Rep_ERROR == 1)
	{
		Motor_Turn_Stop();//停止
		Box_CON.Replenish_Motor_Move =0;  //电机旋转方向 0：不旋转 1 中点转到右侧-向右转 2中点转到左侧-向左转 3右侧转到中点-向左转 4左侧转到中点-向右旋转 5；零点转到中点-向左转 6.中点、不在位转到零点-向右转
		
		Send_Buf[5] = 0x04;//旋转错误
		CON_Send_Data(Send_Buf,9);
		INFO_Print_M("Receive rotation command Atction ERROR!");
	}
		
	return ERROR;
		
	
}
/*0x2B****************************************************
 * 函数名: CON_Replenish_Motor_A_Response
 * 描述  ：补货位电机动作应答   回应 0x2A
 * 输入  ：Value 已经在左 到左 到中间 已经在有 到右 故障（超时）
 * 输出  ：无
 ****************************************************/
void CON_Replenish_Motor_Action_Response(u8 Value)
{
	u8 Send_Buf[9] = {0xAA,0xAB,0x20,0x2B,0x01,0x01,0x00,0xBA,0xBB};
	
	Send_Buf[5] = Value;
	CON_Send_Data(Send_Buf,9);
}

/*****************************************************
 * 函数名: CON_Replenish_Motor_Start_Init
 * 描述  ：设备开机补货电机初始化 如果不在位回复到中间
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_Replenish_Motor_Start_Init(void)
{
	
	printf("Motor_Site:%d\r\n",Box_CON.Replenish_Motor_Site);
	if(Box_CON.Replenish_Motor_Site == 0)		//电机不再位
	{
		printf("Motor_Site:%d\r\n",Box_CON.Replenish_Motor_Site);
		CON_Replenish_Motor_Action(1);
		
		while(Box_CON.Replenish_Motor_Action)//直到停止
		{
			CON_REP_SCAN_Report();
			delay_ms(10);
		}
		delay_ms(1000);
		CON_Replenish_Motor_Action(1);		
		while(Box_CON.Replenish_Motor_Action)//直到停止
		{
			CON_REP_SCAN_Report();
			delay_ms(10);
		}
		delay_ms(1000);
		CON_Replenish_Motor_Action(2);		
		while(Box_CON.Replenish_Motor_Action)//直到停止
		{
			CON_REP_SCAN_Report();
			delay_ms(10);
		}
		
	}
	
}

/*********************************************集装箱门状态***********************************************/
/*****************************************************
 * 函数名: CON_Read_CDR_Position
 * 描述  ：开机读取一次集装箱门构位置
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_Read_CDR_Position(void)
{
	u8 CDR_TEMP = 0;
	CDR_TEMP = CDR_KEY_SCAN();                    //扫描值

  Box_CON.Con_Door_State_Code	= CDR_TEMP;       //赋值
			
	Box_CON.Con_Door_State[0] = CDR_TEMP%2;				//等到1#门现在状态
	CDR_TEMP = CDR_TEMP/2;
	Box_CON.Con_Door_State[1] = CDR_TEMP%2;				//等到2#门现在状态
	CDR_TEMP = CDR_TEMP/2;
	Box_CON.Con_Door_State[2] = CDR_TEMP%2;  			//等到3#门现在状态	
}
/*****************************************************
 * 函数名: CON_CDR_SCAN_Report
 * 描述  ：集装箱状态变化上报
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_CDR_SCAN_Report(void)
{
	u8 CDR_TEMP = 0;
	u8 State[3] = {0};
	u8 i = 0;
	
	
		CDR_TEMP = CDR_KEY_SCAN();                  		//按位   IN_CDR_1 IN_CDR_2 IN_CDR_3     0-7
		if(CDR_TEMP !=Box_CON.Con_Door_State_Code)			//位置有变化记录
		{
			Box_CON.Con_Door_State_Code	= CDR_TEMP;       //赋值
			
			State[0] = CDR_TEMP%2;																			//等到1#门现在状态
			CDR_TEMP = CDR_TEMP/2;
			State[1] = CDR_TEMP%2;																			//等到2#门现在状态
			CDR_TEMP = CDR_TEMP/2;
			State[2] = CDR_TEMP%2;                                      //等到3#门现在状态
			
			for(i = 0;i < 3;i++)
			{
				if(State[i] != Box_CON.Con_Door_State[i])                 //门状态有变化
				{
					Box_CON.Con_Door_State[i] = State[i];                   //赋值
					CON_Reporte_Door_State(i);															//上报门状态
					delay_ms(50);
				}
			}			
		}		
}

/*0x2D****************************************************
 * 函数名: CON_Ask_Door_Response
 * 描述  ：查询集装箱门应答   回应 0x2C
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_Ask_Door_Response(u8 Door)
{
	u8 Send_Buf[11] = {0xAA,0xAB,0x20,0x2D,0x03,0x00,0x00,0x00,0x00,0xBA,0xBB};
	Send_Buf[6] = Door;																		//门编号
	Send_Buf[7] = Box_CON.Con_Door_State[Door-1];					//门状态
	CON_Send_Data(Send_Buf,11);
}

/*0x2E****************************************************
 * 函数名: CON_Reporte_Door_State
 * 描述  ：触发上报门状态   等待0x2F
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_Reporte_Door_State(u8 Door)
{
	u8 Send_Buf[11] = {0xAA,0xAB,0x20,0x2E,0x03,0x00,0x00,0x00,0x00,0xBA,0xBB};
	Send_Buf[6] = Door+1;					     												//门标号
	Send_Buf[7] = Box_CON.Con_Door_State[Door];					      //门状态
	CON_Send_Data(Send_Buf,11);
}


/*********************************************堆垛车状态***********************************************/
/*0x30****************************************************
 * 函数名: CON_Ask_Piling_Car_State
 * 描述  ：主动查询堆垛车状态   等待0x31
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_Ask_Piling_Car_State(void)
{
	u8 Send_Buf[8] = {0xAA,0xAB,0x20,0x30,0x00,0x00,0xBA,0xBB};
	CON_Send_Data(Send_Buf,8);
}

/*0x31****************************************************
 * 函数名: CON_Ask_Piling_Car_State_Response
 * 描述  ：主动查询堆垛车状态  等到回应
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_Ask_Piling_Car_State_Response(u8 State)
{
	Box_CON.Piling_Car_State = State;
}



/*0x33****************************************************
 * 函数名: CON_Piling_Car_State_Response
 * 描述  ：堆垛车状态变化应答  回应 0x32的数据
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_Piling_Car_State_Response(u8 State)
{
	u8 Send_Buf[8] = {0xAA,0xAB,0x20,0x33,0x00,0x00,0xBA,0xBB};
	Box_CON.Piling_Car_State = State;
	if(State <= 1)									//忙碌状态
		CON_SDR_OPEN_EN(1);
	if(State == 2) //空闲 报错状态
		CON_SDR_OPEN_EN(0);
	CON_Send_Data(Send_Buf,8);
}


/*********************************************安全门***********************************************/
/*****************************************************
 * 函数名: CON_Read_SDR_Position
 * 描述  ：开机读取一次安全门状态
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_Read_SDR_Position(void)
{
	u8 SDR_TEMP = 0;
	SDR_TEMP = SDR_KEY_SCAN();                    //扫描值

  Box_CON.Safety_Door_State_Code	= SDR_TEMP;   //赋值
			
	Box_CON.Safety_Door_State[0] = SDR_TEMP%2;		//等到1#门现在状态
	SDR_TEMP = SDR_TEMP/2;
	Box_CON.Safety_Door_State[1] = SDR_TEMP%2;		//等到2#门现在状态

}
/*****************************************************
 * 函数名: CON_SDR_SCAN_Report
 * 描述  ：安全门状态变化上报
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_SDR_SCAN_Report(void)
{
	u8 SDR_TEMP = 0;
	u8 State[3] = {0};
	u8 i = 0;
	
	
		SDR_TEMP = SDR_KEY_SCAN();                  		//按位   0 IN_CDR_1 IN_CDR_2 IN_CDR_3 0-3
		if(SDR_TEMP !=Box_CON.Safety_Door_State_Code)		//位置有变化记录
		{
			Box_CON.Safety_Door_State_Code	= SDR_TEMP;  //赋值
			
			State[0] = SDR_TEMP%2;																			//等到1#门现在状态
			SDR_TEMP = SDR_TEMP/2;
			State[1] = SDR_TEMP%2;																			//等到2#门现在状态
			
			for(i = 0;i < 2;i++)
			{
				if(State[i] != Box_CON.Safety_Door_State[i])              //门状态有变化
				{
					Box_CON.Safety_Door_State[i] = State[i];                //赋值
					CON_Safety_Door_State_Report(i);											  //上报门状态
					delay_ms(50);
				}
			}			
		}		
}
/*****************************************************
 * 函数名: CON_SDR_OPEN_EN
 * 描述  ：安全门是否可以打开
 * 输入  ：1 可以 0 不可以
 * 输出  ：无
 ****************************************************/
void CON_SDR_OPEN_EN(u8 Enable)
{
	Q_SDR = Enable;
	

}

/*0x35****************************************************
 * 函数名: CON_Open_Safety_Door_Request
 * 描述  ：查询安全门状态   回应 0x34的数据
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_Safety_Door_State_Response(u8 Door)
{
	u8 Send_Buf[10] = {0xAA,0xAB,0x20,0x35,0x02,0x00,0x00,0x00,0xBA,0xBB};
	Send_Buf[5] = Door;
	Send_Buf[6] = Box_CON.Safety_Door_State[Door-1] ;
	CON_Send_Data(Send_Buf,10);
}

/*0x36****************************************************
 * 函数名: CON__Safety_Door_State_Report
 * 描述  ：主动上报安全门状态变化  等待 0x37的数据
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_Safety_Door_State_Report(u8 Door)
{
	u8 Send_Buf[11] = {0xAA,0xAB,0x20,0x36,0x03,0x00,0x00,0x01,0x00,0xBA,0xBB};
	Send_Buf[6] = Door+1;
	Send_Buf[7] = Box_CON.Safety_Door_State[Door] ;
	CON_Send_Data(Send_Buf,11);
}


/*********************************************Buffer***********************************************/
/*****************************************************
 * 函数名: CON_Read_BUF_Position
 * 描述  ：开机读取一次Buffer 状态
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_Read_BUF_Position(void)
{
	u8 BUF_TEMP = 0;
	BUF_TEMP = CDR_KEY_SCAN();                    //扫描值

  Box_CON.buffer_State_Code	= BUF_TEMP;      		//赋值 状态码
			
	Box_CON.buffer_State[0] = BUF_TEMP%2;					//等到1#现在状态
	BUF_TEMP = BUF_TEMP/2;
	Box_CON.buffer_State[1] = BUF_TEMP%2;					//等到2#现在状态
	BUF_TEMP = BUF_TEMP/2;
	Box_CON.buffer_State[2] = BUF_TEMP%2;  				//等到3#现在状态	
}
/*****************************************************
 * 函数名: CON_BUF_SCAN_Report
 * 描述  ：Buffer状态变化上报
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_BUF_SCAN_Report(void)
{
	u8 BUF_TEMP = 0;
	u8 State[3] = {0};
	u8 i = 0;	
	
	BUF_TEMP = BUF_KEY_SCAN();                  		//按位   IN_BUF_1 IN_BUF_2 IN_BUF_3     0-7
	if(BUF_TEMP !=Box_CON.buffer_State_Code)				//位置有变化记录
	{
		Box_CON.buffer_State_Code	= BUF_TEMP;       //赋值
		
		State[0] = BUF_TEMP%2;																			//等到1#现在状态
		BUF_TEMP = BUF_TEMP/2;
		State[1] = BUF_TEMP%2;																			//等到2#现在状态
		BUF_TEMP = BUF_TEMP/2;
		State[2] = BUF_TEMP%2;                                      //等到3#现在状态
		
		for(i = 0;i < 3;i++)
		{
			if(State[i] != Box_CON.buffer_State[i])                 	//门状态有变化
			{
				Box_CON.buffer_State[i] = State[i];                   	//赋值 状态值
				CON_Buffer_State_Report(i);															//上报门状态
				delay_ms(50);
			}
		}			
	}		
}
/*0x38****************************************************
 * 函数名: CON_Buffer_State_Report
 * 描述  ：buffer位状态变化上报  等待 0x39的数据
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_Buffer_State_Report(u8 Buffer)
{
	u8 Send_Buf[11] = {0xAA,0xAB,0x20,0x38,0x03,0x00,0x01,0x01,0x00,0xBA,0xBB};
	Send_Buf[6] = Buffer+1;
	Send_Buf[7] = Box_CON.buffer_State[Buffer];	
	CON_Send_Data(Send_Buf,11);
}

/*0x3B****************************************************
 * 函数名: CON_Buffer_State_Response
 * 描述  ：查询buffer位状态变化  回应 0x3A的数据
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_Buffer_State_Response(u8 Buffer)
{
	u8 Send_Buf[11] = {0xAA,0xAB,0x20,0x3B,0x03,0x00,0x00,0x00,0x00,0xBA,0xBB};
	Send_Buf[6] = Buffer;
	Send_Buf[7] = Box_CON.buffer_State[Buffer-1];
	CON_Send_Data(Send_Buf,11);
}





/*********************************************急停按钮***********************************************/
/*****************************************************
 * 函数名: CON_Read_STP_Position
 * 描述  ：开机读取一次STP 状态
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_Read_STP_Position(void)
{
	
	Box_CON.E_Stop_State = STP_KEY_SCAN();        //扫描值  状态码

}
/*****************************************************
 * 函数名: CON_STP_SCAN_Report
 * 描述  ：急停状态变化上报
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_STP_SCAN_Report(void)
{
	u8 STP_TEMP = 0;
	
	STP_TEMP = STP_KEY_SCAN();                  		//按位   IN_STP 0-1
	if(STP_TEMP !=Box_CON.E_Stop_State)							//位置有变化记录
	{
		Box_CON.E_Stop_State	= STP_TEMP;       			//赋值
		CON_E_Stop_State_Report();										//上报状态
	}		
}
/*0x3E****************************************************
 * 函数名: CON_E_Stop_State_Report
 * 描述  ：上报急停按钮状态       等待 0x3F的数据
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_E_Stop_State_Report(void)
{
	u8 Send_Buf[9] = {0xAA,0xAB,0x20,0x3E,0x01,0x00,0x00,0xBA,0xBB};
	Send_Buf[5] = Box_CON.E_Stop_State ;
	CON_Send_Data(Send_Buf,9);
}

/*0x41****************************************************
 * 函数名: CON_E_Stop_State_Response
 * 描述  ：查询急停按钮状态  回应 0x40的数据
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_E_Stop_State_Response(void)
{
	u8 Send_Buf[9] = {0xAA,0xAB,0x20,0x41,0x01,0x00,0x00,0xBA,0xBB};
	Send_Buf[5] = Box_CON.E_Stop_State;
	CON_Send_Data(Send_Buf,9);
}

/*0x41****************************************************
 * 函数名: CON_Device_State_Regular_Report
 * 描述  ：设备状态定时上报   等待 0x42
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_Device_State_Regular_Report(void)
{
	CON_Device_State(0x41);
}


/*********************************************包头包尾***********************************************/
/*****************************************************
 * 函数名: CON_Head_END_Type_LEN_Check
 * 描述  ：检测 包头 包尾 设备类型 数据长度
 * 输入  ：buf 			接收数据
 *         len   		接收数据长度
 * 输出  ：验证通过 返回1 否则返回 0
 ****************************************************/
u8 CON_Head_END_Type_LEN_Check(u8* buf,u8 len)
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
		if(buf[2] != CON_Device_Type)//设备类型
			return 0;
	}
	if(buf[4] != len - 8)				 //数据长度
		return 0;
	
	return 1;	
}

/*********************************************解析***********************************************/
/**************************************************************
 * 函数名：Receive_CON_Protocol_Analysis
 * 描述  ：接收数据解析初始化 
 * 输入  ：buf 			接收数据
 *         len   		接收数据长度
 * 输出  ：无
 ****************************************************/
void Receive_CON_Protocol_Analysis(u8* buf,u8 len)
{	
	//AA AB 02 09 02 10 02 00 BA BB
	if(CON_Head_END_Type_LEN_Check(buf,len))
	{
		
			switch(buf[3])
			{
				case  0x22:													//0x22下发设备信息查询	
					CON_Device_Info_Response();				//0x23上报设备信息查询应答	？
					break;
				case  0x0F:                         //0x26设备类型信息查询
				  CON_Device_Type_Response();				//0x27设备类型信息查询应答
					break;
				
				case  0x28:													//0x28查询补货机构位置 	
					CON_Replenish_State_Response();   //0x29应答
					break;
				case 0x2A:																		//0x2A补货电机旋转
					CON_Replenish_Motor_Action(buf[5]);         //旋转到位后发送应答
//					CON_Replenish_Motor_Action_Response();//0x2B应答
					break;
				case 0x2C:																		//查询集装箱门状态
					if(buf[5 == 0])
						CON_Ask_Door_Response(buf[6]);							//应答
					break;
				case 0x31:																		//0x31主动查询堆垛车状态收到回应
					CON_Ask_Piling_Car_State_Response(buf[5]);	
					break;
				case 0x32:																		//0x32堆垛车变化状态
					CON_Piling_Car_State_Response(buf[5]);			//0x33应答 打开安全门防止打开
					break;
				case 0x34:																		//0x34查询安全门状态 
					CON_Safety_Door_State_Response(buf[5]);			//0x35应答
					break;
				case 0x3A:																		//0x3A查询Buffer状态 
					if(buf[5 == 0])
					CON_Buffer_State_Response(buf[6]);					//0x3B应答
					break;
				case 0x40:																		//0x40查询急停按钮状态 
					CON_E_Stop_State_Response();								//0x41应答
					break;				
				default:
					break;
			}
		
	}
}
 
/*****************************************************
 * 函数名: CON_Start_SCAN_Report
 * 描述  ：开机扫描一下所有设备状态
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_SCAN_Device_State(void)
{
	Read_REP_Sit_State();		//补货位状态
	CON_Read_CDR_Position();//集装箱门状态
	CON_Read_SDR_Position();//安全门状态
	CON_Read_BUF_Position();//buffer状态
	CON_Read_STP_Position();//急停按钮状态
}
#define Regular_Report_Time  30000
/*****************************************************
 * 函数名: CON_Receive_Protocol
 * 描述  ：读取上位机指令并解析
 * 格式  ：0xAAAB	N	0x03	 len(1Byte)		data (n Bytes)	和校验	0xBABB
 * 输入  ：无
 * 输出  ：无
 ****************************************************/
void CON_Receive_Protocol(void)
{
	u8 RE_BUF[128] = {0};
	u8 len = 0;
	u32 T = 0;
  CON_Start_Report_Device_Type();	//开机发送设备类型
	delay_ms(200);
	CON_Start_Report_Check();			 	//开机发送设备信息

	while(1)
	{
		
		CON_Receive_Data(RE_BUF,&len);  								//10ms延迟
		if(len)                													//收到数据                			 
		{
			Receive_CON_Protocol_Analysis(RE_BUF,len);		//解析数据
			len = 0;  																		//数据长度清空
		}
		
		CON_REP_SCAN_Report();													//扫描补货机构设备状态
		CON_CDR_SCAN_Report();                          //扫描集装箱门状态
		CON_SDR_SCAN_Report();                          //扫描安全门状态
		CON_BUF_SCAN_Report();                          //扫描Buffer状态
		CON_STP_SCAN_Report();													//扫描急停按钮状态
		
		T++;
		if(T == Regular_Report_Time/15)
		{
			T = 0;
//			CON_Device_State_Regular_Report();
		}
	}
}
























