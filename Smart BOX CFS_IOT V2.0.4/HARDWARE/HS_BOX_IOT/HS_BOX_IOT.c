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
***�豸��1.�¶�        1#-5#          ��ֵ��u16
***�豸��2.���                       ��ֵ����ѹ�����������ʡ��õ�����u16��
***�豸��3.����    		 1#-4#					��ֵ��u8
***�豸��4.��������            				��ֵ������1���ޡ�����1���� ����2���� ����2����
***�豸��5.������      1#-5#          ״̬���رա��򿪣�0-1��
***�豸��6.�յ�   ���ء�ģʽ���¶�    ״̬��
*/ 
#define Tem_Num  		6
#define Hum_Num  		2
#define Bright_Num 	1
#define Light_Num 	5

#define IOT_IIC_Offset_ADD 128						//ƫ�Ƶ�ַ

typedef struct {
	u16 Tem[Tem_Num];										//�ɼ���5���¶�1-3��Ӧ��ʪ���豸  AIN0�ɼ�4���豸 AIN1 5���豸
	
	u8  Tem_Fault_Code[Tem_Num];				//�¶ȹ�����
	u16 Hum[Hum_Num];               		//�ɼ�ʪ��
	u8 Hum_Fault_Code[Hum_Num];  				//ʪ�ȹ�����
	u16 Watt_Meter[4];									//��ѹ�����������ʡ��õ���	
	u8  Watt_Fault_Code;								//��������
  u8  Bright[Bright_Num];	          	//�ɼ�5������
	u8 	Bright_Fault_Code[Bright_Num];	//���ȹ�����
	u8  Set_Bright[4];       						//����1���ޡ�����1���� ����2���� ����2����
	u8  Light_State[Light_Num];					//5��������״̬
	u8  Light_Fault_Code[Light_Num];		//�����ƹ�����
	u8  LGT_KEY_State[5];     //5������״̬
	u16 LGT_KEY_Code;     		//5�����������
	u16 Interval;             //�Զ��ϱ�ʱ����
	u8  Air_Con_Mode;         //�յ�ģʽ 00 ���� 01 ���� 
	u8 	Air_Con_State;				//�յ�״̬ 00���ر� 0x01:��
	u16 Air_Con_Tem;					//�յ��¶�
	u8  Air_Con_Fault_Code;		//�յ�������
	
	u16 Stack_Cahe[10][10];					//��ջ���� 10��  ��ʪ���¶�1-2 PT100 3-5 ʪ��6-7
}HS_Box_IOT;

#define IOT_Device_Type  0x30
#define CTR_Device_Type  0x00
#define Head 0xAAAB
#define END  0xBABB

#define EEPROM_IOT_ADD   0x30  //eeprom�洢��ַ
//�������豸����

HS_Box_IOT Box_IOT;

/*****************************************************
 * ������: Print_Send_data
 * ����  ����ӡ485���͵�����
 * ����  ��buf ���� len ���ݵĳ���
 * ���  ����
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
 * ������: Print_RS485_Receive_data
 * ����  ����ӡ485���͵�����
 * ����  ��buf ���� len ���ݵĳ���
 * ���  ����
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
 * ������: Print_RS485_Send_data
 * ����  ����ӡ485���͵�����
 * ����  ��buf ���� len ���ݵĳ���
 * ���  ����
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

/*********************************************�ײ㴮������***********************************************/
/*****************************************************
 * ������: IOT_Send_Data
 * ����  ��IOT�豸�ϱ�
 * ����  ��buf ������ len ���ݳ���
 * ���  ����
 ****************************************************/
void IOT_Send_Data(u8* buf,u8 len)
{
	
	UART4_Send_Data(buf,len);
	
}

/*****************************************************
 * ������: IOT_Receive_Data
 * ����  ��IOT�豸��������
 * ����  ��buf ������ len ���ݳ���
 * ���  ����
 ****************************************************/
void IOT_Receive_Data(u8* RSbuf,u8* rs_len)
{
	UART4_Receive_Data(RSbuf,rs_len);
}



/*****************************************************
 * ������: IOT_Send_Data
 * ����  ��IOT�豸�ϱ�
 * ����  ��buf ������ len ���ݳ���
 * ���  ����
 ****************************************************/
void RS485_1_Send_Data(u8* buf,u8 len)
{

	RS485_1_TX_EN;
	USART3_Send_Data(buf,len);
	RS485_1_RX_EN;
	Print_RS485_Send_data(buf,len);

}

/*****************************************************
 * ������: IOT_Receive_Data
 * ����  ��IOT�豸��������
 * ����  ��buf ������ len ���ݳ���
 * ���  ����
 ****************************************************/
void RS485_1_Receive_Data(u8* RSbuf,u8* rs_len)
{
	USART3_Receive_Data(RSbuf,rs_len);
//	Print_RS485_Receive_data��RSbuf,rs_len);
}


/*????*******************************************�豸��Ϣ**********************************************/

//typedef struct {
//	u16 Tem[6];								//�ɼ���5���¶�1-3��Ӧ��ʪ���豸  AIN0�ɼ�4���豸 AIN1 5���豸
//	u16 Tem_Fault_Code;				//�¶ȹ�����
//	u16 Hum[5];               //�ɼ�ʪ��
//	u16 Hum_Fault_Code;				//ʪ�ȹ�����
//	u16 Watt_Meter[4];				//��ѹ�����������ʡ��õ���	
//	u8  Watt_Fault_Code;			//��������
//  u8  Bright[5];	          //�ɼ�5������
//	u16 Bright_Fault_Code;		//���ȹ�����
//	u8  Set_Bright[4];        //����1���ޡ�����1���� ����2���� ����2����
//	u8  Light_State[5];				//5��������״̬
//	u16 Light_Fault_Code;			//�����ƹ�����
//	u8  LGT_KEY_State[5];     //5������״̬
//	u16 LGT_KEY_Code;     		//5�����������
//	u16 Interval;             //�Զ��ϱ�ʱ����
//	u8 	Air_Con_State;				//�յ�״̬ 00���ر� 0x01:������ 0x02:������
//	u16 Air_Con_Tem;					//�յ��¶�
//	u8  Air_Con_Fault_Code;		//�յ�������
//}HS_Box_IOT;
//#define Tem_Num  		4
//#define Hum_Num  		2
//#define Bright_Num 	1
//#define Light_Num 	5
/*****************************************************
 * ������: IOT_Device_Info
 * ����  ���豸��Ϣ
 * ����  ����
 * ���  ����
 ****************************************************/
void IOT_Device_Info(u8 Type)
{
	u8 Data_Len = 0;//���ݰ�����
	u8 i = 0;
	u8 j = 0;
	
	u8 Send_Buf[30] = {0xAA,0xAB,0x30,0x50,0x01,0x30,0x00,0xBA,0xBB};
	
	Send_Buf[3] = Type;			//�豸����
	Send_Buf[4] = Data_Len;	//���ݳ���
	i = 5;
	
	//�¶ȹ�����
	Send_Buf[i] = Tem_Num;
	i++;
	for(j = 0;j < Tem_Num;j++)
	{
		Send_Buf[i] = Box_IOT.Tem_Fault_Code[j];
		i++;
	}
	//ʪ�ȹ�����
	Send_Buf[i] = Hum_Num;
	i++;
	for(j = 0;j < Hum_Num;j++)
	{
		Send_Buf[i] = Box_IOT.Hum_Fault_Code[j];
		i++;
	}
	//���ȹ�����
	Send_Buf[i] = Bright_Num;
	i++;
	for(j = 0;j < Bright_Num;j++)
	{
		Send_Buf[i] = Box_IOT.Bright_Fault_Code[j];
		i++;
	}
	//����������
	Send_Buf[i] = Light_Num;
	i++;
	for(j = 0;j < Light_Num;j++)
	{
		Send_Buf[i] = Box_IOT.Light_Fault_Code[j];
		i++;
	}
	//�յ�������
	Send_Buf[i] = Box_IOT.Air_Con_Fault_Code;
	i++;
	//�������
	Send_Buf[i] = Box_IOT.Watt_Fault_Code;
	i++;
	//�յ�����
	Send_Buf[i] = Box_IOT.Air_Con_State;
  i++;
	//���ݳ���
	Data_Len = i-4;
	Send_Buf[4] = Data_Len;//���ݳ���
	
	//��У��
	Send_Buf[i] = 0x00;
  i++;
	
	//��β
	Send_Buf[i] = 0xBA;
  i++;
	Send_Buf[i] = 0xBB;
  i++;	
	
	IOT_Send_Data(Send_Buf,i);
}

/*0x50****************************************************
 * ������: IOT_Start_Report_Check
 * ����  ���豸�����Լ���Ϣ  �ȴ�0x51
 * ����  ����
 * ���  ����
 ****************************************************/
void IOT_Start_Report_Check(void)
{
	IOT_Device_Info(0x50);
}
/*0x53****************************************************
 * ������: IOT_Device_Info_Response
 * ����  ���豸��Ϣ��ѯ�ظ�  �ظ�0x52
 * ����  ����
 * ���  ����
 ****************************************************/
void IOT_Device_Info_Response(void)
{
	IOT_Device_Info(0x53);
}


/*********************************************�豸����***********************************************/
/*0x54****************************************************
 * ������: IOT_Start_Report_Device_Type
 * ����  �������ϱ��豸���� �ȴ� 0x55
 * ����  ����
 * ���  ����
 ****************************************************/
void IOT_Start_Report_Device_Type(void)
{
	u8 Send_Buf[9] = {0xAA,0xAB,0x30,0x54,0x01,0x30,0x00,0xBA,0xBB};
//	Send_Buf[5] = CON_Device_Type;
	IOT_Send_Data(Send_Buf,9);
}

/*0x57****************************************************
 * ������: IOT_Device_Type_Response
 * ����  ���豸����Ӧ��  �ظ� 0x56
 * ����  ����
 * ���  ����
 ****************************************************/
void IOT_Device_Type_Response(void)
{
	u8 Send_Buf[9] = {0xAA,0xAB,0x30,0x57,0x01,0x30,0x00,0xBA,0xBB};
	//	Send_Buf[5] = CON_Device_Type;
	IOT_Send_Data(Send_Buf,9);
}



/*********************************************�¶�***********************************************/
/*0x59****************************************************
 * ������: IOT_Temperature_Response
 * ����  ������λ���λ��Ӧ��    ��Ӧ 0x58
 * ����  ��Num: �ϱ��¶ȸ��� ���5
 * ���  ����
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
	Send_Buf[j] = 0x00;//У��
	j++;
	Send_Buf[j] = 0xBA;//��β
	j++;
	Send_Buf[j] = 0xBB;//��β
	j++;
	IOT_Send_Data(Send_Buf,j);
}




/*********************************************ʪ��***********************************************/
/*0x5B****************************************************
 * ������: CON_Ask_Door_Response
 * ����  ����ѯʪ��Ӧ��    ��Ӧ 0x5A
 * ����  ��Num: �ϱ�ʪ�ȸ��� ���5
 * ���  ����
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
	Send_Buf[j] = 0x00;//У��
	j++;
	Send_Buf[j] = 0xBA;//��β
	j++;
	Send_Buf[j] = 0xBB;//��β
	j++;
	IOT_Send_Data(Send_Buf,j);
}



/*********************************************���***********************************************/
/*0x5D****************************************************
 * ������: IOT_Watt_Meter_Response
 * ����  ����ѯ���      ��Ӧ 0x5C
 * ����  ��Num �ϱ�����  ��ѹ ���� ���� �õ��� 4������
 * ���  ��
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
	Send_Buf[j] = 0x00;//У��
	j++;
	Send_Buf[j] = 0xBA;//��β
	j++;
	Send_Buf[j] = 0xBB;//��β
	j++;
	IOT_Send_Data(Send_Buf,j);
}



/*********************************************����***********************************************/
/*0x5F****************************************************
 * ������: IOT_Bright_Response
 * ����  ����ѯ����   ��Ӧ 0x5E������
 * ����  ��Num �ϱ�����
 * ���  ����
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
	Send_Buf[j] = 0x00;//У��
	j++;
	Send_Buf[j] = 0xBA;//��β
	j++;
	Send_Buf[j] = 0xBB;//��β
	j++;
	IOT_Send_Data(Send_Buf,j);
}

/*0x61****************************************************
 * ������: IOT_Bright_Response
 * ����  ����ѯ��������  ��Ӧ 0x60������
 * ����  ����
 * ���  ����
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
	Send_Buf[j] = 0x00;//У��
	j++;
	Send_Buf[j] = 0xBA;//��β
	j++;
	Send_Buf[j] = 0xBB;//��β
	j++;
	IOT_Send_Data(Send_Buf,j);
}


/*0x63****************************************************
 * ������: IOT_Set_Bright_Request
 * ����  ��������������  ��Ӧ 0x62������
 * ����  ��Serial ��� Value ��ֵ
 * ���  ����
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

/*********************************************������***********************************************/
/*****************************************************
 * ������: Light_State_Save
 * ����  ���������������ݵ�EEPROM
 * ����  ����
 * ���  ����
 ****************************************************/
void Light_State_Save(void)
{
	u8 IIC_buf[6] = {0};     //EEPROM�洢
	u8 IIC_ADD = 0;
	u8 i = 0;
	
	IIC_ADD = IOT_IIC_Offset_ADD + 0;
	
	for(i = 0;i < 5;i++)
		IIC_buf[i] = Box_IOT.Light_State[i];
		
	AT24CXX_Write(IIC_ADD,IIC_buf,5);
}
/*****************************************************
 * ������: Read_EEPROM_Light_State
 * ����  ����ȡEEPROM��������״̬
 * ����  ����
 * ���  ����
 ****************************************************/
void Read_EEPROM_Light_State(void)
{
	u8 IIC_buf[6] = {0};     //EEPROM�洢
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
 * ������: IOT_Light_Handle_Not
 * ����  ��������ȡ������
 * ����  ��Serial �����Ʊ�� 0-4  ��Ӧ 1#-5#
 * ���  ����
 ****************************************************/
void IOT_Light_Handle_Not(u8 Serial,u8 Light_State)
{
	  Box_IOT.Light_State[Serial] = !Box_IOT.Light_State[Serial];	//������״̬ȡ��
		printf("Serial:%d Light_State:%d\r\n",Serial,Light_State);
		switch(Serial)															//������״̬�������
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
		
		Light_State_Save();//�����������ݱ��浽EEPROM��
}

/*0x67****************************************************
 * ������: Light_ON_OFF
 * ����  �����صƿ���
 * ����  ��Serial ��� ON_OFF 1:�� 0:�ر�
 * ���  ����
 ****************************************************/
void Light_ON_OFF(u8 Serial,u8 ON_OFF)
{
	
	printf("Light_ON_OFF:%d  %d\r\n",Serial,ON_OFF);
		switch(Serial)												//������״̬�������
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
		Light_State_Save();//�����������ݱ��浽EEPROM��
}
/*****************************************************
 * ������: IOT_LGT_State_Report
 * ����  ��������״̬�仯
 * ����  ����
 * ���  ����
 ****************************************************/
void IOT_LGT_State_Report(void)
{
	
}
/*****************************************************
 * ������: IOT_Read_LGT_KEY
 * ����  ����ȡ�����ƿ��ذ���
 * ����  ����
 * ���  ����
 ****************************************************/
void IOT_Read_LGT_KEY(void)
{
	u8 	i = 0;
	u16 LGT_TEMP = 0; //��ȡ�����ư���ʵʱ״̬
	u8  KEY_State[5] = {0};
	

		LGT_TEMP = LGT_KEY_SCAN();                  		//��λ   0 IN_LGT_1 IN_LGT_2 IN_LGT_3 IN_LGT_4 IN_LGT_5 0-31
		if(LGT_TEMP !=Box_IOT.LGT_KEY_Code)							//���������
		{
			Box_IOT.LGT_KEY_Code	= LGT_TEMP;       			//��ֵ ���������
			
			KEY_State[0] = LGT_TEMP%2;																			//�ȵ�1#����״̬
			LGT_TEMP = LGT_TEMP/2;
			KEY_State[1] = LGT_TEMP%2;																			//�ȵ�2#����״̬
			LGT_TEMP = LGT_TEMP/2;
			KEY_State[2] = LGT_TEMP%2;                                      //�ȵ�3#����״̬
			LGT_TEMP = LGT_TEMP/2;
			KEY_State[3] = LGT_TEMP%2;                                      //�ȵ�4#����״̬
			LGT_TEMP = LGT_TEMP/2;
			KEY_State[4] = LGT_TEMP%2;                                      //�ȵ�5#����״̬
			printf("IOT_Read_LGT_KEY:%d\r\n",LGT_TEMP);
			for(i = 0;i < 5;i++)
			{
				if(KEY_State[i] != Box_IOT.LGT_KEY_State[i])              //�����б仯
				{
					Box_IOT.LGT_KEY_State[i] = KEY_State[i];                //��ֵ
					IOT_Light_Handle_Not(i,KEY_State[i]);                   //�����Ʊ�� ֵ���
//					IOT_LGT_State_Report();																	//�ϱ��仯
					delay_ms(50);
				}
			}	
		}		  
}


/*0x65****************************************************
 * ������: IOT_Light_Response
 * ����  ����ѯ������״̬     Ӧ�� 0x64������
 * ����  ��Serial ���
 * ���  ����
 ****************************************************/
void IOT_Light_Response(u8 Serial)
{
	u8 Send_Buf[11] = {0xAA,0xAB,0x30,0x65,0x03,0x00,0x01,0x01,0x00,0xBA,0xBB};
	 
	Send_Buf[6] = Serial;
	Send_Buf[7] = Box_IOT.Light_State[Serial-1];
	IOT_Send_Data(Send_Buf,11);
}


/****************************************************
 * ������: Update_LGT_KEY_Code
 * ����  ��ͨ������״̬�޸����ذ��Ӧ��Code��
 * ����  ����
 * ���  ����
 ****************************************************/
void Update_LGT_KEY_Code(void)
{
	Box_IOT.LGT_KEY_Code =Box_IOT.LGT_KEY_State[4]*16 + Box_IOT.LGT_KEY_State[3]*8 + Box_IOT.LGT_KEY_State[2]*4 + Box_IOT.LGT_KEY_State[1]*2 + Box_IOT.LGT_KEY_State[0];
}

/*0x67****************************************************
 * ������: IOT_Set_Light_Request
 * ����  ������������  ��Ӧ 0x66������
 * ����  ��Serial ���
 * ���  ����
 ****************************************************/
void IOT_Set_Light_Request(u8 Serial,u8 Value)
{
	u8 Send_Buf[9] = {0xAA,0xAB,0x30,0x67,0x01,0x00,0x00,0xBA,0xBB};
	if(Serial < 6)
	{
		if(Box_IOT.Light_State[Serial-1] == Value) //�Ƶ�״̬��Ҫ�ﵽ״̬һ��
		{
			if(Value == 1)       //���Ѵ�
				Send_Buf[5] = 0x03;
			else                 //���ѹر�
				Send_Buf[5] = 0x01;
		}
		else																//�Ƶ�״̬��Ҫ�ﵽ״̬��һ��
		{
			Box_IOT.Light_State[Serial-1] = Value;//������״̬��ֵ
			if(Value == 1)										//����
			{
				Light_ON_OFF(Serial-1,1);
				Send_Buf[5] = 0x02;             //�ƴ�			
			}
			else															//�ص�
			{	
				Light_ON_OFF(Serial-1,0);
				Send_Buf[5] = 0x00;							//�ƹر�
			}
		}
		Update_LGT_KEY_Code();
		
		IOT_Send_Data(Send_Buf,9);
	}
}


/*********************************************�Զ��ϱ�**********************************************/
/*****************************************************
 * ������: Read_EEPROM_Light_State
 * ����  ����ȡEEPROM��������״̬
 * ����  ����
 * ���  ����
 ****************************************************/
void Read_EEPROM_Interval_Time(void)
{
	u8 IIC_buf[2] = {0};     //EEPROM�洢
	u8 IIC_ADD = 0;
	u8 i = 0;
	
	IIC_ADD = IOT_IIC_Offset_ADD + 5;
	
	AT24CXX_Read(IIC_ADD,IIC_buf,2);
	Box_IOT.Interval = IIC_buf[0]*256+IIC_buf[1];
	
}
/*0x81****************************************************
 * ������: IOT_Interval_Request
 * ����  ����ѯ�Զ��ϱ����        ��Ӧ0x80������
 * ����  ����
 * ���  ����
 ****************************************************/
void IOT_Interval_Request(void)
{
	u8 Send_Buf[10] = {0xAA,0xAB,0x30,0x81,0x02,0x00,0x00,0x00,0xBA,0xBB};
	Send_Buf[5] = Box_IOT.Interval/256;
	Send_Buf[6] = Box_IOT.Interval%256;
	IOT_Send_Data(Send_Buf,10);
}

/*0x83****************************************************
 * ������: IOT_Set_Interval_Request
 * ����  �������Զ��ϱ����        ��Ӧ0x82������
 * ����  ��Time ���ʱ��
 * ���  ����
 ****************************************************/
void IOT_Set_Interval_Request(u16 Time)
{
	u8 IIC_buf[2] = {0};     //EEPROM�洢
	u8 IIC_ADD = 0;
	u8 Send_Buf[10] = {0xAA,0xAB,0x30,0x83,0x00,0x00,0xBA,0xBB};
	
	Box_IOT.Interval = Time;
	
	//IIC�洢************************************************
	IIC_ADD = IOT_IIC_Offset_ADD + 5;
	IIC_buf[0] = Time/256;
	IIC_buf[1] = Time%256;
	AT24CXX_Write(IIC_ADD,IIC_buf,2);
	
	
	
	IOT_Send_Data(Send_Buf,8);
	
}
/*0x84****************************************************
 * ������: IOT_Auto_Temperature_Report
 * ����  ���Զ��ϱ�    �ȴ� 0x85 ��Ӧ
 * ����  ��Num: �ϱ�����
 * ���  ����
 ****************************************************/
void IOT_Auto_Temperature_Report(u8 Num)
{
	u8 i = 0;
	u8 j = 0;	
	u8 Send_Buf[30] = {0xAA,0xAB,0x30,0x84,0x0B,0x05,      0x00,0x00,  0x00,0x00,  0x00,0x00,  0x00,0x00,  0x00,0x00,    0x00,0xBA,0xBB};
		
	if(Num >  6)
		Num = 6;
		
	Send_Buf[3] = 0x84;    //��������
	Send_Buf[4] = Num*3+1; //���ݳ���
	Send_Buf[5] = Num;     //����������
	j = 6;
	for(i = 0;i < Num;i++) //��� ����
	{	
		Send_Buf[j] = i+1;
		j++;
		Send_Buf[j] = Box_IOT.Tem[i]/256;
		j++;
		Send_Buf[j] = Box_IOT.Tem[i]%256;
		j++;
	}
	Send_Buf[j] = 0x00;//У��
	j++;
	Send_Buf[j] = 0xBA;//��β
	j++;
	Send_Buf[j] = 0xBB;//��β
	j++;
	IOT_Send_Data(Send_Buf,j);
}


/*0x86****************************************************
 * ������: IOT_Auto_Hum_Report
 * ����  ���Զ��ϱ�    �ȴ� 0x87 ��Ӧ
 * ����  ��Num: �ϱ�����
 * ���  ����
 ****************************************************/
void IOT_Auto_Hum_Report(u8 Num)
{
	u8 i = 0;
	u8 j = 0;	
	u8 Send_Buf[30] = {0xAA,0xAB,0x30,0x84,0x0B,0x05,      0x00,0x00,  0x00,0x00,  0x00,0x00,  0x00,0x00,  0x00,0x00,    0x00,0xBA,0xBB};
		
	if(Num >  5)
		Num = 5;
		
	Send_Buf[3] = 0x86;    //��������
	Send_Buf[4] = Num*3+1; //���ݳ���
	Send_Buf[5] = Num;     //����������
	j = 6;
	for(i = 0;i < Num;i++) //��� ����
	{	
		Send_Buf[j] = i+1;
		j++;
		Send_Buf[j] = Box_IOT.Hum[i]/256;
		j++;
		Send_Buf[j] = Box_IOT.Hum[i]%256;
		j++;
	}
	Send_Buf[j] = 0x00;//У��
	j++;
	Send_Buf[j] = 0xBA;//��β
	j++;
	Send_Buf[j] = 0xBB;//��β
	j++;
	IOT_Send_Data(Send_Buf,j);
}

/*0x88****************************************************
 * ������: IOT_Auto_Bright_Report
 * ����  ���Զ��ϱ�    �ȴ� 0x89 ��Ӧ
 * ����  ��Num: �ϱ�����
 * ���  ����
 ****************************************************/
void IOT_Auto_Bright_Report(u8 Num)
{
	u8 i = 0;
	u8 j = 0;	
	u8 Send_Buf[30] = {0xAA,0xAB,0x30,0x88,0x0B,0x05,      0x00,0x00,  0x00,0x00,  0x00,0x00,  0x00,0x00,  0x00,0x00,    0x00,0xBA,0xBB};
		
	if(Num >  5)
		Num = 5;
		
	Send_Buf[3] = 0x88;    //��������
	Send_Buf[4] = Num*3+1; //���ݳ���
	Send_Buf[5] = Num;     //����������
	j = 6;
	for(i = 0;i < Num;i++) //��� ����
	{	
		Send_Buf[j] = i+1;
		j++;
		Send_Buf[j] = Box_IOT.Bright[i];
		j++;
	}
	Send_Buf[j] = 0x00;//У��
	j++;
	Send_Buf[j] = 0xBA;//��β
	j++;
	Send_Buf[j] = 0xBB;//��β
	j++;
	IOT_Send_Data(Send_Buf,j);
}

/*0x8A****************************************************
 * ������: IOT_Auto_Watt_Meter_Report
 * ����  ���Զ��ϱ�    �ȴ� 0x8B ��Ӧ
 * ����  ��Num: �ϱ�����
 * ���  ����
 ****************************************************/
void IOT_Auto_Watt_Meter_Report(u8 Num)
{
	u8 i = 0;
	u8 j = 0;	
	u8 Send_Buf[30] = {0xAA,0xAB,0x30,0x88,0x0B,0x05,      0x00,0x00,  0x00,0x00,  0x00,0x00,  0x00,0x00,  0x00,0x00,    0x00,0xBA,0xBB};
		
	if(Num >  5)
		Num = 5;
		
	Send_Buf[3] = 0x88;    //��������
	Send_Buf[4] = Num*3+1; //���ݳ���

	j = 5;
	for(i = 0;i < Num;i++) //��� ����
	{	
		Send_Buf[j] = Box_IOT.Watt_Meter[i]/256;
		j++;
		Send_Buf[j] = Box_IOT.Watt_Meter[i]%256;
		j++;
	}
	Send_Buf[j] = 0x00;//У��
	j++;
	Send_Buf[j] = 0xBA;//��β
	j++;
	Send_Buf[j] = 0xBB;//��β
	j++;
	IOT_Send_Data(Send_Buf,j);
}

/*0x8C****************************************************
 * ������: IOT_Auto_Watt_Meter_Report
 * ����  ���Զ��ϱ�    �ȴ� 0x8D ��Ӧ
 * ����  ��Num: �ϱ�����
 * ���  ����
 ****************************************************/
void IOT_Auto_Light_Report(u8 Num)
{
	u8 i = 0;
	u8 j = 0;	
	u8 Send_Buf[30] = {0xAA,0xAB,0x30,0x88,0x0B,0x05,      0x00,0x00,  0x00,0x00,  0x00,0x00,  0x00,0x00,  0x00,0x00,    0x00,0xBA,0xBB};
		
	if(Num >  5)
		Num = 5;
		
	Send_Buf[3] = 0x88;    //��������
	Send_Buf[4] = Num*3+1; //���ݳ���
  Send_Buf[5] = Num;
	j = 6;
	for(i = 0;i < Num;i++) //��� ����
	{	
		Send_Buf[j] = Box_IOT.Light_State[i]/256;
		j++;
	}
	Send_Buf[j] = 0x00;//У��
	j++;
	Send_Buf[j] = 0xBA;//��β
	j++;
	Send_Buf[j] = 0xBB;//��β
	j++;
	IOT_Send_Data(Send_Buf,j);
}


/*****************************************************
 * ������: IOT_Auto_Report_Fun
 * ����  ���Զ��ϱ�����    
 * ����  ����
 * ���  ����
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


/*********************************************������***********************************************/
/*0x8E****************************************************
 * ������: CON_Fault_Code_Report
 * ����  ���ϱ�������        �ȴ� 0x8F������
 * ����  ����
 * ���  ����
 ****************************************************/
void IOT_Fault_Code_Report(u8 State)
{
	u8 Send_Buf[9] = {0xAA,0xAB,0x20,0x3E,0x01,0x00,0x00,0xBA,0xBB};

	IOT_Send_Data(Send_Buf,9);
}

/*********************************************��ͷ��β***********************************************/
/*****************************************************
 * ������: IOT_Head_END_Type_LEN_Check
 * ����  ����� ��ͷ ��β �豸���� ���ݳ���
 * ����  ��buf 			��������
 *         len   		�������ݳ���
 * ���  ����֤ͨ�� ����1 ���򷵻� 0
 ****************************************************/
u8 IOT_Head_END_Type_LEN_Check(u8* buf,u8 len)
{
	if((buf[0]*256+buf[1] != Head)||(buf[len-2]*256+buf[len-1] != END))//��ͷ��β
		return 0;
	if(buf[3] == 0x0F)
	{
		if(buf[2] != CTR_Device_Type)//�豸����
			return 0;
	}
	else
	{	
		if(buf[2] != IOT_Device_Type)//�豸����
			return 0;
	}
	if(buf[4] != len - 8)				 //���ݳ���
		return 0;
	
	return 1;	
}

/*********************************************�յ����Ƴ���***********************************************/
/****************************************************
 * ��������Air_Switch_Send_Data
 * ����  ���յ�����ָ��
 * ����  ��Value ����ֵ
 * ���  ����
 ****************************************************/
void Air_Switch_Send_Data(u8 Value)
{
	u8 Txbuf[8] = {0x03,0x06,0x00,0x01,0x00,0x01,0x00,0x00};
	if(Value < 2)
	{
		Txbuf[5] = Value +1;
	  ADD_CRC(Txbuf,6);            //����CRC
		RS485_1_Send_Data(Txbuf,8);   //ͨ��485��������
		Box_IOT.Air_Con_State = Value;
	}
}

/****************************************************
 * ��������Air_Mode_Send_Data
 * ����  ���յ�ģʽ�л�
 * ����  ��Value ����ֵ
 * ���  ����
 ****************************************************/
void Air_Mode_Send_Data(u8 Value)
{
	u8 Txbuf[8] = {0x03,0x06,0x00,0x02,0x00,0x01,0x00,0x00};
	if(Value < 5)
	{
		Txbuf[5] = Value;
	  ADD_CRC(Txbuf,6);            //����CRC
		RS485_1_Send_Data(Txbuf,8);   //ͨ��485��������
		Box_IOT.Air_Con_Mode = Value;
	}
}
/****************************************************
 * ��������Air_Temp_Send_Data
 * ����  ���յ��¶�����
 * ����  ��Value ����ֵ
 * ���  ����
 ****************************************************/
void Air_Temp_Send_Data(u8 Value)
{
	u8 Txbuf[8] = {0x03,0x06,0x00,0x03,0x00,0x01,0x00,0x00};
	if((Value < 31)&&(Value > 16))
	{
		Txbuf[5] = Value;
	  ADD_CRC(Txbuf,6);            //����CRC
		RS485_1_Send_Data(Txbuf,8);   //ͨ��485��������
		Box_IOT.Air_Con_Tem = Value;
	}
}

/****************************************************
 * ��������IOT_Set_AIR_Request
 * ����  ���յ�����

 * ����  ��Value ����ֵ
 * ���  ����
 ****************************************************/
void IOT_Set_AIR_Request(u8 Mode,u8 Value )
{
	switch(Mode)
	{
		case 1://����ģʽ
			Air_Switch_Send_Data(Value);
			break;
		case 2://ģʽ����
			Air_Mode_Send_Data(Value);
			break;
		case 3://�¶�����
			Air_Temp_Send_Data(Value);
			break;
		default:
			break;		
	}
	
}

/****************************************************
 * ��������IOT_AIR_Request
 * ����  ���յ�����
 * ����  ��Value ����ֵ
 * ���  ����
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

/*********************************************Э�����***********************************************/
/****************************************************
 * ��������Receive_CON_Protocol_Analysis
 * ����  ���������ݽ�����ʼ�� 
 * ����  ��buf 			��������
 *         len   		�������ݳ���
 * ���  ����
 ****************************************************/
void Receive_IOT_Protocol_Analysis(u8* buf,u8 len)
{	
	//AA AB 02 09 02 10 02 00 BA BB
	if(IOT_Head_END_Type_LEN_Check(buf,len))
	{		
			switch(buf[3])
			{
				case  0x52:													//0x52�·��豸��Ϣ��ѯ	
					IOT_Device_Info_Response();				//0x53�ϱ��豸��Ϣ��ѯӦ��	��
					break;
				case  0x0F:                         //0x56�豸������Ϣ��ѯ
				  IOT_Device_Type_Response();				//0x57�豸������Ϣ��ѯӦ��
					break;
				case  0x58:													//0x58��ѯ�¶�
					IOT_Temperature_Response(6);  		//0x59Ӧ�� 5���¶�
					break;
				case 0x5A:													//0x5A��ѯʪ��
					IOT_Hum_Response(2);         			//0x5BӦ�� 2��ʪ��
					break;
				case 0x5C:			  									//0x5C��ѯ�������?
					IOT_Watt_Meter_Response(4);		    //0x5DӦ��
					break;
				case 0x5E:													//0x5E��ѯ����?
					IOT_Bright_Response(2);	    			//0x5FӦ��
					break;
				case 0x60:													//0x60��ѯ��������?
					IOT_Set_Bright_Response();;				//0x61Ӧ��
					break;
				case 0x62:																	//0x62��������?
					IOT_Set_Bright_Request(buf[5],buf[6]);		//0x635Ӧ��
					break;
				case 0x64:													//0x64��ѯ������״̬ 
					IOT_Light_Response(buf[5]);				//0x65Ӧ��
					break;
				case 0x66:															//0x66����������
					IOT_Set_Light_Request(buf[5]*256+buf[6],buf[7]);	//0x67Ӧ��
					break;	
				case 0x68:															//0x68ѯ�ʿյ�
					IOT_AIR_Request(buf[4]);							//0x69
					break;
				case 0x6A:															//0x6A���ÿյ�
					IOT_Set_AIR_Request(buf[5],buf[6]);	//0x6B
					break;
				
				
				
				
	
				case 0x80:													//0x80��ѯ�Զ��ϱ����
					IOT_Interval_Request();						//0x81Ӧ��
					break;	
				case 0x82:													//0x82�����Զ��ϱ����
					IOT_Set_Interval_Request(buf[5]*256+buf[6]);//0x81Ӧ��
					break;
				
				default:
					break;
			}
	}
	
}
 


/*****************************************************
EEPROM  1.�Զ��ϱ�����  2.������
 ****************************************************/
/*****************************************************
 * ������: IOT_Read_EEPROM_Data
 * ����  ����ȡEEPROM��������Ϣ
 * ����  ����
 * ���  ����
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
 * ������: IOT_Device_Init
 * ����  ����ʼ�豸��ʼ��
 * ����  ����
 * ���  ����
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
 * ������: IOT_Receive_Protocol
 * ����  ����ȡ��λ��ָ�����
 * ��ʽ  ��0xAAAB	N	0x03	 len(1Byte)		data (n Bytes)	��У��	0xBABB
 * ����  ����
 * ���  ����
 ****************************************************/
void IOT_Receive_Protocol(void)
{
	u8 RE_BUF[128] = {0};
	u8 len = 0;
	u32 T = 0;
	u8 N = 0;
	
	IOT_Device_Init();
	IOT_Read_EEPROM_Data();					//��ȡEEPROM����
  IOT_Start_Report_Device_Type();	//�ϱ��豸����
	delay_ms(50);
	IOT_Start_Report_Check();			 	//�ϱ������Լ���Ϣ
	
	while(1)
	{
		
		IOT_Receive_Data(RE_BUF,&len);  
		if(len)                											//�յ�����                			 
		{
			Receive_IOT_Protocol_Analysis(RE_BUF,len);		//��������
			len = 0;  																	//���ݳ������
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
		
		IOT_Read_LGT_KEY();									//���ż��
	}
}















/*********************************************Modbus ��ʪ�Ƚ���***********************************************/
#define Modbus_ADD 0x01
/**************************************************************
 * ��������RS485_Add_TEST
 * ����  ���ж����ݵ�ַ
 * ʵ��  : 02 03 00 05 00 02 CRC
 * ����  ��rsbuf�����ܵ����� 
 * ���  : 1������ 0���Ǳ���
 **************************************************************/
u8 Modbus_Add_TEST(u8* buf)
{
	if(buf[0] == Modbus_ADD)       //��֤�����Ƿ��Ƿ�����������
		return 1;
	else
		return 0;
}

/**************************************************************
 * ��������Read_Modbus_Add
 * ����  ����ȡModbus�豸��ַ
 * ����  ��buf�����ܵ����� 
 * ���  : modbus��ַ
 **************************************************************/
u8 Read_Modbus_Add(u8* buf)
{
	return buf[0];
}

/**************************************************************
 * ��������Function_Code_Len_TEST
 * ����  ���жϹ���������ݳ���
 * ʵ��  : 01 04 04 01 31 02 22 2A CE
 * ����  ��rsbuf�����ܵ�����  FUN_Code������ Data_LEN ���ݳ���
 * ���  : 1������ 0���Ǳ���
 ***********************************************************/
u8 Function_Code_Len_TEST(u8* buf,u8 FUN_Code,u8 Data_LEN)
{
	if((buf[1]==FUN_Code)&&(buf[2]==Data_LEN))
		return 1;
	else
		return 0;
}

/**************************************************************
 * ��������Stack_Average_Value
 * ����  ����ջ����������ƽ��ֵ
 * ����  ��serial ��� data ���� Depth ��ջ���
 *         �ṹ 0 λ ��ջָ��λ��
 * ���  ����
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
 * ��������Receive_IOT_Modbus_TH_Analysis
 * ����  ��������ʪ�����ݽ���
 * ����  ��buf 			��������
 *         len   		�������ݳ���
 *         01 04 04 01 31 02 22 2A CE
 * ���  ����
 ***********************************************************/
void Receive_IOT_Modbus_TH_Analysis(u8* buf,u8 len)
{
	u16 T,H;	
	u8 Serial = 0;
	u16 Data = 0;
	
	if(CRC_TEST(buf,len))              		     //�ж�CRCУ��
	{ 
		if(Function_Code_Len_TEST(buf,0x03,0x04))//��ȡ��ʪ������
		{
			if(buf[0]<4)
			{	
				//�¶�0-5 6��  ʪ��6-7 2��
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
 * ������: IOT_Modbus_Read_TH
 * ����  ��ͨ��Modbus��ȡ��ʪ��
 * ��ʽ  �����ͣ�01 03 00 01 00 02 95 CB
 *         ���أ�01 03 04 13 4D 0B 6E E8 7C 
 *         
 *         �¶� ��ַ01 (�Ĵ��ֵ-2000)/100   
 *         ʪ�� ��ַ02 �Ĵ��ֵ/100
 *         H 0x222=546/10=54.6
 * ����  ����
 * ���  ����
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
		
		if(len)                											//�յ�����                			 
		{
			Print_RS485_Receive_data(RE_BUF,len);	
			Receive_IOT_Modbus_TH_Analysis(RE_BUF,len);		//��������
			len = 0;  																	  //���ݳ������
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
 * ������: Current_Converts_Temperature
 * ����  ��4-20MA ת��Ϊ�����¶�
 * ����  ��Data AIN0-3ת����u16����
 *  			 AIN0-1 ��Ӧ Temp4-5
* ����  ��-50-100�� ��Ӧ4-20mA   AD:100R���� ����2.048
 * ���  ����
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
 * ������: IOT_Read_4_20mA
 * ����  ��4-20MA �ɼ�
 * ����  ����
 * ���  ����
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
	while(1)//2s����һ������
	{			
			I2C_ReadNByte (0x90, 0x01, 0x00, Read_Data, 2);
			switch(DAQ_Num/50)
			{
				case 3: //AIN0
						if(DAQ_Num%50 < 5)
							InitADS1115(0x03);													//2.048
						else																					//ǰ5�����ݲ�Ҫ 
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
					//�������ݽ������
					
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
					//���
					DAQ_Num = 0;					
					for(i = 0;i < 4;i++)
						SUM[i] = 0;		
			}	
			delay_ms(10);
	}	
}





/*****************************************************
 * ������: IOT_STM32_ADC
 * ����  ��STM32 �ڲ�AD���ݲɼ�
 * ����  ����
 * ���  ����
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






