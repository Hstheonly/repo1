#include "GPIO.h"
#include "delay.h"
#include "usart.h"
#include "HS_BOX_CON.h"
#include "DEBUG.h"
/*
**********************************��װ��****************************************
***�豸��1.��������    ���						��������ת����ת��ֹͣ��1-3��
																			״̬�����С��ҡ�δ��λ��1-4��
***�豸��2.��װ����		 1# 2# 3#  			״̬���رա��򿪣�0-1��
***�豸��3.�Ѷ⳵      								״̬�����С�æµ������0-2��
***�豸��4.��ȫ��      1# 2#  				״̬���رա��򿪣�0-1��
***�豸��5.bufferλ    1# 2# 3#      	״̬����λ������λ��0-1��
***�豸��6.��ͣ��ť    1# 2#          ״̬��δ���¡����£�0-1��
 */ 


typedef struct {
	u8 Replenish_Motor_Action;//�����������״̬ 0ֹͣ 1 ��ת 2 ��ת  3���е�  4 �����
	u8 Replenish_Motor_State; //�������λ��  ״̬�����С��ҡ�δ��λ ���� ��㣨1 2 4 0 5 3��
	u8 Replenish_Motor_Site; 	//�������λ��  ͨ����ת�жϵ��λ�� ���С��ҡ�δ��λ ���� ��㣨1 2 4 0 5 3��
	u8 Replenish_Motor_Last_State; //��һ״̬	
	u8 Replenish_S_Con_State; //�����״̬  0  ��ת�������޴����
	u8 Replenish_Receive_Com; //����װ�ý��յ������� 0������ת 1 ת���Ҳ� 2ת����� 3ת���е� 4ת�����
	u8 Replenish_Motor_Move;  //�����ת���� 0������ת 1 �е�ת���Ҳ�-����ת 2�е�ת�����-����ת 3�Ҳ�ת���е�-����ת 4���ת���е�-������ת 5�����ת���е�-����ת 6.�е㡢����λת�����-����ת
	u8 Replenish_Key_State;   //������ť&&�ŵ�״̬ 1���� 0δ��
	
	u8 Con_Door_State[3];     //��װ����״̬
	u8 Con_Door_State_Code;   //��װ����״̬��
	u8 Piling_Car_State;			//�Ѷ⳵״̬
	u8 Safety_Door_State[2];  //��ȫ��״̬
	u8 Safety_Door_State_Code;//��ȫ����״̬��
	u8 buffer_State[3];       //buffer λ״̬
	u8 buffer_State_Code;			//buffer ״̬��
	u8 E_Stop_State;					//����ֹͣ��ť
	u32 Fault_Code;           //������
}HS_Box_CON;

#define CTR_Device_Type  0x00 //Box Contrl ��ַ
#define CON_Device_Type  0x20 //BOX CON ��װ����ư��ַ

#define Head 0xAAAB
#define END  0xBABB


HS_Box_CON Box_CON;

void CON_Replenish_Motor_Action_Response(u8 Value);
//void CON_Start_SCAN_Report(void);
//void CON_SCAN_Report(void);
//void CON_Read_REP_Position(void);//��ȡһ�β�������λ��
void CON_Reporte_Door_State(u8 Door);//�����ϱ���״̬
void CON_SDR_OPEN_EN(u8 Enable);			//��ȫ���Ƿ���Դ�

void CON_Safety_Door_State_Report(u8 Door);//�ϱ���ȫ��״̬�仯
void CON_Buffer_State_Report(u8 Buffer);//�ϱ�buffer״̬�仯
void CON_E_Stop_State_Report(void);     //�ϱ���ͣ��ť״̬
/*********************************************�ײ㴮������***********************************************/
/*****************************************************
 * ������: CON_Send_Data
 * ����  ����װ���豸�ϱ�
 * ����  ��buf ������ len ���ݳ���
 * ���  ����
 ****************************************************/
void CON_Send_Data(u8* buf,u8 len)
{
	 USART2_Send_Data(buf,len);
}

/*****************************************************
 * ������: CON_Receive_Data
 * ����  ����װ���豸��������
 * ����  ��buf ������ len ���ݳ���
 * ���  ����
 ****************************************************/
void CON_Receive_Data(u8* RSbuf,u8* rs_len)
{
	USART2_Receive_Data(RSbuf,rs_len);
}


/*********************************************�����ת����***********************************************/
/*****************************************************
 * ������: Motot_Turn_Right
 * ����  ���������������ת
 * ����  ����
 * ���  ����
 ****************************************************/
void Motor_Turn_Right(void)
{
		M_REP_DIR = 1;         //����
		M_REP_Start = 1;       //���� ȡ��ɲ��
}
/*****************************************************
 * ������: Motot_Turn_Left
 * ����  ���������������ת
 * ����  ����
 * ���  ����
 ****************************************************/
void Motor_Turn_Left(void)
{
		M_REP_DIR = 0;         //����
		M_REP_Start = 1;       //���� ȡ��ɲ��
}
/*****************************************************
 * ������: Motot_Turn_Stop
 * ����  ���������ֹͣ
 * ����  ����
 * ���  ����
 ****************************************************/
void Motor_Turn_Stop(void)
{
		M_REP_DIR = 0;         //����
		M_REP_Start = 0;       //���� ȡ��ɲ��
}
/*********************************************������**********************************************/
/*0x3C****************************************************
 * ������: CON_Fault_Code_Report
 * ����  ���ϱ�������        �ȴ� 0x3D������
 * ����  ����
 * ���  ����
 ****************************************************/
void CON_Fault_Code_Report(u16 Fault_Code)
{
	u8 Send_Buf[15]  = {0xAA,0xAB,0x20,0x3C,0x03,0x02,0x00,0x00,0x00,0xBA,0xBB};

	Send_Buf[6] = Fault_Code/256;
	Send_Buf[7] = Fault_Code%256;	
	
	CON_Send_Data(Send_Buf,11);
}

/*????*******************************************�豸��Ϣ**********************************************/
/*0x20****************************************************
 * ������: CON_Device_State
 * ����  ���豸״̬��Ϣ
 * ����  ��Type ������
 * ���  ����
 ****************************************************/
void CON_Device_State(u8 Type)
{
	u8 i = 0;
	
	u8 Send_Buf[30] = {0xAA,0xAB,0x20};	
	
	CON_SCAN_Device_State();//ɨ���豸״̬	
	
	Send_Buf[3] = Type;
	
	i = 5;
	Send_Buf[i] = Box_CON.Replenish_Motor_State;//��������λ��
	i++;
	Send_Buf[i] = 3;//��װ��������
	i++;
	Send_Buf[i] = Box_CON.Con_Door_State[0];//��װ����״̬
	i++;
	Send_Buf[i] = Box_CON.Con_Door_State[1];
	i++;
	Send_Buf[i] = Box_CON.Con_Door_State[2];
	
	i++;
	Send_Buf[i] = 2;//��ȫ������
	i++;
	Send_Buf[i] = Box_CON.Safety_Door_State[0];//��ȫ��״̬
	i++;
	Send_Buf[i] = Box_CON.Safety_Door_State[1];
	
	i++;
	Send_Buf[i] = 3;//buffer ����
	i++;
	Send_Buf[i] = Box_CON.buffer_State[0];//buffer ״̬
	i++;
	Send_Buf[i] = Box_CON.buffer_State[1];
	i++;
	Send_Buf[i] = Box_CON.buffer_State[2];
	
	i++;
	Send_Buf[i] = Box_CON.E_Stop_State;//��ͣ״̬
	
	i++;
	Send_Buf[i] = 0;//У��
	
	i++;
	Send_Buf[i] = 0xBA;//У��
	i++;
	Send_Buf[i] = 0xBB;//У��
	
	i++;//����
	Send_Buf[4] = i-8;//���ݳ���
	
	CON_Send_Data(Send_Buf,i);
}

/*0x20****************************************************
 * ������: CON_Start_Report_Check
 * ����  �������ϱ��豸��Ϣ   �ȴ�0x21 
 * ����  ����
 * ���  ����
 ****************************************************/
void CON_Start_Report_Check()
{
	CON_Device_State(0x20);
}
/*0x23****************************************************
 * ������: CON_Device_Info_Response
 * ����  ���豸��Ϣ��ѯ   �ظ�0x22
 * ����  ����
 * ���  ����
 ****************************************************/
void CON_Device_Info_Response(void)
{
	CON_Device_State(0x23);
}


/*********************************************�豸����***********************************************/
/*0x24****************************************************
 * ������: Start_Report_Device_Type
 * ����  �������ϱ��豸���� �ȴ� 0x25
 * ����  ����
 * ���  ����
 ****************************************************/
void CON_Start_Report_Device_Type(void)
{
	u8 Send_Buf[9] = {0xAA,0xAB,0x20,0x24,0x01,0x20,0x00,0xBA,0xBB};
	CON_Send_Data(Send_Buf,9);
}

/*0x27****************************************************
 * ������: CON_Device_Type_Response
 * ����  ���豸����Ӧ��  �ظ� 0x26
 * ����  ����
 * ���  ����
 ****************************************************/
void CON_Device_Type_Response(void)
{
	u8 Send_Buf[9] = {0xAA,0xAB,0x20,0x27,0x01,0x20,0x00,0xBA,0xBB};
	CON_Send_Data(Send_Buf,9);
}



/*********************************************����λ***********************************************/
/*****************************************************
 * ������: CON_Replenish_Motor_Stop
 * ����  ������λ���ֹͣ
  #define RM0          //ɲ�� 0����ɲ�� 1ȡ��ɲ��
  #define RM1          //����ֹͣ 0ֹͣ 1����
  #define RM2          //������� 0��ת 1��ת
 ****************************************************/
void CON_Replenish_Motor_Stop(void)
{
		M_REP_Start = 0;//ֹͣ ɲ��
	  M_REP_DIR = 0;  //�رշ���̵���
	  Box_CON.Replenish_Motor_Action = 0;//״̬����Ϊֹͣ       
}

#define TIME_CON_REP  2500  //ms ��ʱʱ�䵥λms


/*****************************************************
 * ������: Read_REP_Sit_State
 * ����  ����ȡ����λλ��״̬
 ****************************************************/
u8 Read_REP_Sit_State(void)
{
	u8 REP_TEMP = 0;	
	REP_TEMP = REP_KEY_SCAN(); 
	
	switch(REP_TEMP)
	{
		case 0:
			Box_CON.Replenish_Motor_Site = 0;//�������λ��   ���С��ҡ�δ��λ ���� ��㣨1 2 4 0 5 3��  ����λ
			break;
		case 1:
			Box_CON.Replenish_Motor_Site = 1;//�������λ��   ���С��ҡ�δ��λ ���� ��㣨1 2 4 0 5 3��  ��
			break;
		case 2:
			Box_CON.Replenish_Motor_Site = 2;//�������λ��   ���С��ҡ�δ��λ ���� ��㣨1 2 4 0 5 3��  �е�
			break;
		case 4:
			Box_CON.Replenish_Motor_Site = 4;//�������λ��   ���С��ҡ�δ��λ ���� ��㣨1 2 4 0 5 3��  ��
			break;
		case 8:
			Box_CON.Replenish_Motor_Site = 3;//�������λ��   ���С��ҡ�δ��λ ���� ��㣨1 2 4 0 5 3��   ���
			break;
		default:
			Box_CON.Replenish_Motor_Site = 5;//�������λ��   ���С��ҡ�δ��λ ���� ��㣨1 2 4 0 5 3��   ����
			break;
	}	
}

/*****************************************************
 * ������: CON_SCAN_Report
 * ����  ��������������ֹͣ�ϱ�
 * ����  ����
 * ���  ����
 ****************************************************/
void CON_REP_SCAN_Report(void)
{	
	u8 REP_TEMP = 0;
	u8 KEY = 0;
	u8 Send_Buf[9] = {0xAA,0xAB,0x20,0x2B,0x01,0x01,0x00,0xBA,0xBB};//��λ
	u8 Send_Timeout_Buf[9] = {0xAA,0xAB,0x20,0x2B,0x01,0x00,0x00,0xBA,0xBB};//��ʱδ��λ
	u8 Send_Suspend_Buf[9] = {0xAA,0xAB,0x20,0x2B,0x01,0x03,0x00,0xBA,0xBB};//��ת��ֹ
	static u16 T = 0;
	
	REP_TEMP = Read_REP_Sit_State();//��ȡλ��
	KEY = REP_BUT_SCAN();//�������Ŵ�״̬
//	Box_CON.Replenish_Motor_Move =6;  //�����ת���� 0������ת 1 �е�ת���Ҳ�-����ת 2�е�ת�����-����ת 3�Ҳ�ת���е�-����ת 4���ת���е�-������ת 5�����ת���е�-����ת 6.�е㡢����λת�����-����ת
	
/*************����*************/	
	if(KEY == 1)
	{
		switch(Box_CON.Replenish_Motor_Move)//�����ת���� 0������ת 1 �е�ת���Ҳ�-����ת 2�е�ת�����-����ת 3�Ҳ�ת���е�-����ת 4���ת���е�-������ת 5�����ת���е�-����ת 6.�е㡢����λת�����-����ת
		{
			case 1://�е�ת���Ҳ�-����ת
				Motor_Turn_Right();
				T++;	//��ʱ��¼ һ���ۻ�ʱ��10ms
			break;
			case 2://�е�ת�����-����ת
				Motor_Turn_Left();
				T++;	//��ʱ��¼ һ���ۻ�ʱ��10ms
			break;
			case 3://�Ҳ�ת���е�-����ת 
				Motor_Turn_Left();
				T++;	//��ʱ��¼ һ���ۻ�ʱ��10ms
			break;
			case 4://���ת���е�-������ת 
				Motor_Turn_Right();
				T++;	//��ʱ��¼ һ���ۻ�ʱ��10ms
			break;
			case 5://���ת���е�-����ת
				Motor_Turn_Left();
				T++;	//��ʱ��¼ һ���ۻ�ʱ��10ms
			break;
			case 6://�е㡢����λת�����-����ת
				Motor_Turn_Right();
				T++;	//��ʱ��¼ һ���ۻ�ʱ��10ms
			break;
			default:
				break;		
		}		
		Box_CON.Replenish_Key_State = KEY;
	}
	else
	{
		Motor_Turn_Stop();//ֹͣ
		if(Box_CON.Replenish_Key_State == 1)
		{
			CON_Send_Data(Send_Suspend_Buf,9);//�ϱ���ֹ
		}
		Box_CON.Replenish_Key_State = 0;		
	}
	
/************************��ʱ**********************/	
	if((Box_CON.Replenish_Motor_Move>0)&&(Box_CON.Replenish_Motor_Move<5))
	{
		if(T == TIME_CON_REP/10)	
		{
			Motor_Turn_Stop();//ֹͣ
			Box_CON.Replenish_Motor_Move =0;  //�����ת���� 0������ת 1 �е�ת���Ҳ�-����ת 2�е�ת�����-����ת 3�Ҳ�ת���е�-����ת 4���ת���е�-������ת 5�����ת���е�-����ת 6.�е㡢����λת�����-����ת
			CON_Send_Data(Send_Timeout_Buf,9);//�ϱ���ʱֹͣ
			T = 0;	//��ʱ��¼ ���
		}			
	}
	if((Box_CON.Replenish_Motor_Move==5)||(Box_CON.Replenish_Motor_Move==6))
	{
		if(T == TIME_CON_REP/5)	
		{
			Motor_Turn_Stop();//ֹͣ
			Box_CON.Replenish_Motor_Move =0;  //�����ת���� 0������ת 1 �е�ת���Ҳ�-����ת 2�е�ת�����-����ת 3�Ҳ�ת���е�-����ת 4���ת���е�-������ת 5�����ת���е�-����ת 6.�е㡢����λת�����-����ת
			CON_Send_Data(Send_Timeout_Buf,9);//�ϱ���ʱֹͣ
			T = 0;	//��ʱ��¼ ���
		}			
	}	
			
/************************��λ���**********************/	
	switch(Box_CON.Replenish_Motor_Move)//�����ת���� 0������ת 1 �е�ת���Ҳ�-����ת 2�е�ת�����-����ת 3�Ҳ�ת���е�-����ת 4���ת���е�-������ת 5�����ת���е�-����ת 6.�е㡢����λת�����-����ת
	{
		case 1://�е�ת���Ҳ�-����ת
			if(Box_CON.Replenish_Motor_Site == 4)//�������λ��   ���С��ҡ�δ��λ ���� ��㣨1 2 4 0 5 3��   
			{
				Motor_Turn_Stop();//ֹͣ
				Box_CON.Replenish_Motor_Move =0;  //�����ת���� 0������ת 1 �е�ת���Ҳ�-����ת 2�е�ת�����-����ת 3�Ҳ�ת���е�-����ת 4���ת���е�-������ת 5�����ת���е�-����ת 6.�е㡢����λת�����-����ת
				CON_Send_Data(Send_Buf,9);//�ϱ���λ
				T = 0;	//��ʱ��¼ ���
			}
			break;
		case 2://�е�ת�����-����ת
			if(Box_CON.Replenish_Motor_Site == 1)//�������λ��   ���С��ҡ�δ��λ ���� ��㣨1 2 4 0 5 3��   
			{
				Motor_Turn_Stop();//ֹͣ
				Box_CON.Replenish_Motor_Move =0;  //�����ת���� 0������ת 1 �е�ת���Ҳ�-����ת 2�е�ת�����-����ת 3�Ҳ�ת���е�-����ת 4���ת���е�-������ת 5�����ת���е�-����ת 6.�е㡢����λת�����-����ת
				CON_Send_Data(Send_Buf,9);//�ϱ���λ
				T = 0;	//��ʱ��¼ ���
			}
			break;
		case 3://�Ҳ�ת���е�-����ת 
			if(Box_CON.Replenish_Motor_Site == 2)//�������λ��   ���С��ҡ�δ��λ ���� ��㣨1 2 4 0 5 3��   
			{
				Motor_Turn_Stop();//ֹͣ
				Box_CON.Replenish_Motor_Move =0;  //�����ת���� 0������ת 1 �е�ת���Ҳ�-����ת 2�е�ת�����-����ת 3�Ҳ�ת���е�-����ת 4���ת���е�-������ת 5�����ת���е�-����ת 6.�е㡢����λת�����-����ת
				CON_Send_Data(Send_Buf,9);//�ϱ���λ
				T = 0;	//��ʱ��¼ ���
			}
			break;
		case 4://���ת���е�-������ת 
			if(Box_CON.Replenish_Motor_Site == 2)//�������λ��   ���С��ҡ�δ��λ ���� ��㣨1 2 4 0 5 3��   
			{
				Motor_Turn_Stop();//ֹͣ
				Box_CON.Replenish_Motor_Move =0;  //�����ת���� 0������ת 1 �е�ת���Ҳ�-����ת 2�е�ת�����-����ת 3�Ҳ�ת���е�-����ת 4���ת���е�-������ת 5�����ת���е�-����ת 6.�е㡢����λת�����-����ת
				CON_Send_Data(Send_Buf,9);//�ϱ���λ
				T = 0;	//��ʱ��¼ ���
			}
			break;
		case 5://���ת���е�-����ת
			if(Box_CON.Replenish_Motor_Site == 2)//�������λ��   ���С��ҡ�δ��λ ���� ��㣨1 2 4 0 5 3��   
			{
				Motor_Turn_Stop();//ֹͣ
				Box_CON.Replenish_Motor_Move =0;  //�����ת���� 0������ת 1 �е�ת���Ҳ�-����ת 2�е�ת�����-����ת 3�Ҳ�ת���е�-����ת 4���ת���е�-������ת 5�����ת���е�-����ת 6.�е㡢����λת�����-����ת
				CON_Send_Data(Send_Buf,9);//�ϱ���λ
				T = 0;	//��ʱ��¼ ���
			}
			break;
		case 6://�е㡢����λת�����-����ת
			if(Box_CON.Replenish_Motor_Site == 3)//�������λ��   ���С��ҡ�δ��λ ���� ��㣨1 2 4 0 5 3��   
			{
				Motor_Turn_Stop();//ֹͣ
				Box_CON.Replenish_Motor_Move =0;  //�����ת���� 0������ת 1 �е�ת���Ҳ�-����ת 2�е�ת�����-����ת 3�Ҳ�ת���е�-����ת 4���ת���е�-������ת 5�����ת���е�-����ת 6.�е㡢����λת�����-����ת
				CON_Send_Data(Send_Buf,9);//�ϱ���λ
				T = 0;	//��ʱ��¼ ���
			}
			break;
		default://����
			break;
	}	
}

/*****************************************************
 * ������: CON_Read_REP_Position
 * ����  ����ȡһ�β�������λ��
 * ����  ����
 * ���  ����
 ****************************************************/
void CON_Read_REP_Position(void)
{
	Box_CON.Replenish_Motor_State = REP_KEY_SCAN();  
	Box_CON.Replenish_Motor_Site = Box_CON.Replenish_Motor_State;
}


/*0x29****************************************************
 * ������: CON_Replenish_State_Response
 * ����  ������λ���λ��Ӧ��    ��Ӧ 0x28
 * ����  ����
 * ���  ����
 ****************************************************/
void CON_Replenish_State_Response(void)
{
	u8 Send_Buf[9] = {0xAA,0xAB,0x20,0x29,0x01,0x01,0x00,0xBA,0xBB};
	
	Read_REP_Sit_State();//��ȡλ��
	Send_Buf[5] = Box_CON.Replenish_Motor_Site;//�������λ��   ���С��ҡ�δ��λ ���� ��㣨1 2 4 0 5 3��  ����λ
	CON_Send_Data(Send_Buf,9);
}


/*****************************************************
 * ������: CON_Replenish_Motor_A_Response
 * ����  ������λ����˶�����
 * ����  ��Dir 0:ֹͣ 1:ת���Ҳ� 2ת����� 3��ת���е� 4ת�����
 * ���  ����
 ****************************************************/
u8  CON_Replenish_Motor_Action(u8 Dir)
{	
	u8 Rep_ERROR = 0;
	u8 Send_Buf[9] = {0xAA,0xAB,0x20,0x2B,0x01,0x01,0x00,0xBA,0xBB};
	
	Read_REP_Sit_State();//ȷ����������λ��

	if(DEBUG)
		printf("Receive rotation command  State:%d",Box_CON.Replenish_Motor_Site);//�������λ��   ���С��ҡ�δ��λ ���� ��㣨1 2 4 0 5 3��

	switch(Dir)//Dir 0:ֹͣ 1:ת���Ҳ� 2ת����� 3��ת���е� 4ת�����
	{
		case 0://ֹͣ
			Motor_Turn_Stop();//ֹͣ
		  Box_CON.Replenish_Motor_Move = 0;  //�����ת���� 0������ת 1 �е�ת���Ҳ�-����ת 2�е�ת�����-����ת 3�Ҳ�ת���е�-����ת 4���ת���е�-������ת 5�����ת���е�-����ת 6.�е㡢����λת�����-����ת
			break;
	case 1://�е�ת���Ҳ�
			if(Box_CON.Replenish_Motor_Site == 2)
			{
				Motor_Turn_Right();//����ת
				Box_CON.Replenish_Motor_Move = 1;  //�����ת���� 0������ת 1 �е�ת���Ҳ�-����ת 2�е�ת�����-����ת 3�Ҳ�ת���е�-����ת 4���ת���е�-������ת 5�����ת���е�-����ת 6.�е㡢����λת�����-����ת
//				Box_CON.Replenish_Receive_Com = Dir; //����װ�ý��յ������� 0������ת 1 ת���Ҳ� 2ת����� 3ת���е� 4ת�����
			}				
			else
				Rep_ERROR = 1;
			break;
	case 2://�е�ת�����
			if(Box_CON.Replenish_Motor_Site == 2)
			{
				Motor_Turn_Left();//����ת
				Box_CON.Replenish_Motor_Move =2;  //�����ת���� 0������ת 1 �е�ת���Ҳ�-����ת 2�е�ת�����-����ת 3�Ҳ�ת���е�-����ת 4���ת���е�-������ת 5�����ת���е�-����ת 6.�е㡢����λת�����-����ת
//				Box_CON.Replenish_Receive_Com = Dir; //����װ�ý��յ������� 0������ת 1 ת���Ҳ� 2ת����� 3ת���е� 4ת�����
			}				
			else
				Rep_ERROR = 1;
			break;
	case 3://ת���е�
			switch(Box_CON.Replenish_Motor_Site)
			{
				case 1://�����������ת
					Motor_Turn_Right();//����ת
				  Box_CON.Replenish_Motor_Move =4;  //�����ת���� 0������ת 1 �е�ת���Ҳ�-����ת 2�е�ת�����-����ת 3�Ҳ�ת���е�-����ת 4���ת���е�-������ת 5�����ת���е�-����ת 6.�е㡢����λת�����-����ת
					break;
				case 4://���Ҳ�������ת
					Motor_Turn_Left();//����ת
					Box_CON.Replenish_Motor_Move =3;  //�����ת���� 0������ת 1 �е�ת���Ҳ�-����ת 2�е�ת�����-����ת 3�Ҳ�ת���е�-����ת 4���ת���е�-������ת 5�����ת���е�-����ת 6.�е㡢����λת�����-����ת
					break;
				case 3://�����������ת180
					Motor_Turn_Right();//����ת
				  Box_CON.Replenish_Motor_Move =5;  //�����ת���� 0������ת 1 �е�ת���Ҳ�-����ת 2�е�ת�����-����ת 3�Ҳ�ת���е�-����ת 4���ת���е�-������ת 5�����ת���е�-����ת 6.�е㡢����λת�����-����ת
					break;
				default:
					Rep_ERROR = 0;
					break;
			}
			break;
	case 4://ת�����  �е�Ͳ���λ
			switch(Box_CON.Replenish_Motor_Site)//�������λ��   ���С��ҡ�δ��λ ���� ��㣨1 2 4 0 5 3��
			{
				case 0://����λת����� ������ת
					Motor_Turn_Right();//����ת
				  Box_CON.Replenish_Motor_Move =6;  //�����ת���� 0������ת 1 �е�ת���Ҳ�-����ת 2�е�ת�����-����ת 3�Ҳ�ת���е�-����ת 4���ת���е�-������ת 5�����ת���е�-����ת 6.�е㡢����λת�����-����ת
					break;
				case 2://�е�ת����� ������ת
					Motor_Turn_Right();//����ת
				  Box_CON.Replenish_Motor_Move =6;  //�����ת���� 0������ת 1 �е�ת���Ҳ�-����ת 2�е�ת�����-����ת 3�Ҳ�ת���е�-����ת 4���ת���е�-������ת 5�����ת���е�-����ת 6.�е㡢����λת�����-����ת
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
		Motor_Turn_Stop();//ֹͣ
		Box_CON.Replenish_Motor_Move =0;  //�����ת���� 0������ת 1 �е�ת���Ҳ�-����ת 2�е�ת�����-����ת 3�Ҳ�ת���е�-����ת 4���ת���е�-������ת 5�����ת���е�-����ת 6.�е㡢����λת�����-����ת
		
		Send_Buf[5] = 0x04;//��ת����
		CON_Send_Data(Send_Buf,9);
		INFO_Print_M("Receive rotation command Atction ERROR!");
	}
		
	return ERROR;
		
	
}
/*0x2B****************************************************
 * ������: CON_Replenish_Motor_A_Response
 * ����  ������λ�������Ӧ��   ��Ӧ 0x2A
 * ����  ��Value �Ѿ����� ���� ���м� �Ѿ����� ���� ���ϣ���ʱ��
 * ���  ����
 ****************************************************/
void CON_Replenish_Motor_Action_Response(u8 Value)
{
	u8 Send_Buf[9] = {0xAA,0xAB,0x20,0x2B,0x01,0x01,0x00,0xBA,0xBB};
	
	Send_Buf[5] = Value;
	CON_Send_Data(Send_Buf,9);
}

/*****************************************************
 * ������: CON_Replenish_Motor_Start_Init
 * ����  ���豸�������������ʼ�� �������λ�ظ����м�
 * ����  ����
 * ���  ����
 ****************************************************/
void CON_Replenish_Motor_Start_Init(void)
{
	
	printf("Motor_Site:%d\r\n",Box_CON.Replenish_Motor_Site);
	if(Box_CON.Replenish_Motor_Site == 0)		//�������λ
	{
		printf("Motor_Site:%d\r\n",Box_CON.Replenish_Motor_Site);
		CON_Replenish_Motor_Action(1);
		
		while(Box_CON.Replenish_Motor_Action)//ֱ��ֹͣ
		{
			CON_REP_SCAN_Report();
			delay_ms(10);
		}
		delay_ms(1000);
		CON_Replenish_Motor_Action(1);		
		while(Box_CON.Replenish_Motor_Action)//ֱ��ֹͣ
		{
			CON_REP_SCAN_Report();
			delay_ms(10);
		}
		delay_ms(1000);
		CON_Replenish_Motor_Action(2);		
		while(Box_CON.Replenish_Motor_Action)//ֱ��ֹͣ
		{
			CON_REP_SCAN_Report();
			delay_ms(10);
		}
		
	}
	
}

/*********************************************��װ����״̬***********************************************/
/*****************************************************
 * ������: CON_Read_CDR_Position
 * ����  ��������ȡһ�μ�װ���Ź�λ��
 * ����  ����
 * ���  ����
 ****************************************************/
void CON_Read_CDR_Position(void)
{
	u8 CDR_TEMP = 0;
	CDR_TEMP = CDR_KEY_SCAN();                    //ɨ��ֵ

  Box_CON.Con_Door_State_Code	= CDR_TEMP;       //��ֵ
			
	Box_CON.Con_Door_State[0] = CDR_TEMP%2;				//�ȵ�1#������״̬
	CDR_TEMP = CDR_TEMP/2;
	Box_CON.Con_Door_State[1] = CDR_TEMP%2;				//�ȵ�2#������״̬
	CDR_TEMP = CDR_TEMP/2;
	Box_CON.Con_Door_State[2] = CDR_TEMP%2;  			//�ȵ�3#������״̬	
}
/*****************************************************
 * ������: CON_CDR_SCAN_Report
 * ����  ����װ��״̬�仯�ϱ�
 * ����  ����
 * ���  ����
 ****************************************************/
void CON_CDR_SCAN_Report(void)
{
	u8 CDR_TEMP = 0;
	u8 State[3] = {0};
	u8 i = 0;
	
	
		CDR_TEMP = CDR_KEY_SCAN();                  		//��λ   IN_CDR_1 IN_CDR_2 IN_CDR_3     0-7
		if(CDR_TEMP !=Box_CON.Con_Door_State_Code)			//λ���б仯��¼
		{
			Box_CON.Con_Door_State_Code	= CDR_TEMP;       //��ֵ
			
			State[0] = CDR_TEMP%2;																			//�ȵ�1#������״̬
			CDR_TEMP = CDR_TEMP/2;
			State[1] = CDR_TEMP%2;																			//�ȵ�2#������״̬
			CDR_TEMP = CDR_TEMP/2;
			State[2] = CDR_TEMP%2;                                      //�ȵ�3#������״̬
			
			for(i = 0;i < 3;i++)
			{
				if(State[i] != Box_CON.Con_Door_State[i])                 //��״̬�б仯
				{
					Box_CON.Con_Door_State[i] = State[i];                   //��ֵ
					CON_Reporte_Door_State(i);															//�ϱ���״̬
					delay_ms(50);
				}
			}			
		}		
}

/*0x2D****************************************************
 * ������: CON_Ask_Door_Response
 * ����  ����ѯ��װ����Ӧ��   ��Ӧ 0x2C
 * ����  ����
 * ���  ����
 ****************************************************/
void CON_Ask_Door_Response(u8 Door)
{
	u8 Send_Buf[11] = {0xAA,0xAB,0x20,0x2D,0x03,0x00,0x00,0x00,0x00,0xBA,0xBB};
	Send_Buf[6] = Door;																		//�ű��
	Send_Buf[7] = Box_CON.Con_Door_State[Door-1];					//��״̬
	CON_Send_Data(Send_Buf,11);
}

/*0x2E****************************************************
 * ������: CON_Reporte_Door_State
 * ����  �������ϱ���״̬   �ȴ�0x2F
 * ����  ����
 * ���  ����
 ****************************************************/
void CON_Reporte_Door_State(u8 Door)
{
	u8 Send_Buf[11] = {0xAA,0xAB,0x20,0x2E,0x03,0x00,0x00,0x00,0x00,0xBA,0xBB};
	Send_Buf[6] = Door+1;					     												//�ű��
	Send_Buf[7] = Box_CON.Con_Door_State[Door];					      //��״̬
	CON_Send_Data(Send_Buf,11);
}


/*********************************************�Ѷ⳵״̬***********************************************/
/*0x30****************************************************
 * ������: CON_Ask_Piling_Car_State
 * ����  ��������ѯ�Ѷ⳵״̬   �ȴ�0x31
 * ����  ����
 * ���  ����
 ****************************************************/
void CON_Ask_Piling_Car_State(void)
{
	u8 Send_Buf[8] = {0xAA,0xAB,0x20,0x30,0x00,0x00,0xBA,0xBB};
	CON_Send_Data(Send_Buf,8);
}

/*0x31****************************************************
 * ������: CON_Ask_Piling_Car_State_Response
 * ����  ��������ѯ�Ѷ⳵״̬  �ȵ���Ӧ
 * ����  ����
 * ���  ����
 ****************************************************/
void CON_Ask_Piling_Car_State_Response(u8 State)
{
	Box_CON.Piling_Car_State = State;
}



/*0x33****************************************************
 * ������: CON_Piling_Car_State_Response
 * ����  ���Ѷ⳵״̬�仯Ӧ��  ��Ӧ 0x32������
 * ����  ����
 * ���  ����
 ****************************************************/
void CON_Piling_Car_State_Response(u8 State)
{
	u8 Send_Buf[8] = {0xAA,0xAB,0x20,0x33,0x00,0x00,0xBA,0xBB};
	Box_CON.Piling_Car_State = State;
	if(State <= 1)									//æµ״̬
		CON_SDR_OPEN_EN(1);
	if(State == 2) //���� ����״̬
		CON_SDR_OPEN_EN(0);
	CON_Send_Data(Send_Buf,8);
}


/*********************************************��ȫ��***********************************************/
/*****************************************************
 * ������: CON_Read_SDR_Position
 * ����  ��������ȡһ�ΰ�ȫ��״̬
 * ����  ����
 * ���  ����
 ****************************************************/
void CON_Read_SDR_Position(void)
{
	u8 SDR_TEMP = 0;
	SDR_TEMP = SDR_KEY_SCAN();                    //ɨ��ֵ

  Box_CON.Safety_Door_State_Code	= SDR_TEMP;   //��ֵ
			
	Box_CON.Safety_Door_State[0] = SDR_TEMP%2;		//�ȵ�1#������״̬
	SDR_TEMP = SDR_TEMP/2;
	Box_CON.Safety_Door_State[1] = SDR_TEMP%2;		//�ȵ�2#������״̬

}
/*****************************************************
 * ������: CON_SDR_SCAN_Report
 * ����  ����ȫ��״̬�仯�ϱ�
 * ����  ����
 * ���  ����
 ****************************************************/
void CON_SDR_SCAN_Report(void)
{
	u8 SDR_TEMP = 0;
	u8 State[3] = {0};
	u8 i = 0;
	
	
		SDR_TEMP = SDR_KEY_SCAN();                  		//��λ   0 IN_CDR_1 IN_CDR_2 IN_CDR_3 0-3
		if(SDR_TEMP !=Box_CON.Safety_Door_State_Code)		//λ���б仯��¼
		{
			Box_CON.Safety_Door_State_Code	= SDR_TEMP;  //��ֵ
			
			State[0] = SDR_TEMP%2;																			//�ȵ�1#������״̬
			SDR_TEMP = SDR_TEMP/2;
			State[1] = SDR_TEMP%2;																			//�ȵ�2#������״̬
			
			for(i = 0;i < 2;i++)
			{
				if(State[i] != Box_CON.Safety_Door_State[i])              //��״̬�б仯
				{
					Box_CON.Safety_Door_State[i] = State[i];                //��ֵ
					CON_Safety_Door_State_Report(i);											  //�ϱ���״̬
					delay_ms(50);
				}
			}			
		}		
}
/*****************************************************
 * ������: CON_SDR_OPEN_EN
 * ����  ����ȫ���Ƿ���Դ�
 * ����  ��1 ���� 0 ������
 * ���  ����
 ****************************************************/
void CON_SDR_OPEN_EN(u8 Enable)
{
	Q_SDR = Enable;
	

}

/*0x35****************************************************
 * ������: CON_Open_Safety_Door_Request
 * ����  ����ѯ��ȫ��״̬   ��Ӧ 0x34������
 * ����  ����
 * ���  ����
 ****************************************************/
void CON_Safety_Door_State_Response(u8 Door)
{
	u8 Send_Buf[10] = {0xAA,0xAB,0x20,0x35,0x02,0x00,0x00,0x00,0xBA,0xBB};
	Send_Buf[5] = Door;
	Send_Buf[6] = Box_CON.Safety_Door_State[Door-1] ;
	CON_Send_Data(Send_Buf,10);
}

/*0x36****************************************************
 * ������: CON__Safety_Door_State_Report
 * ����  �������ϱ���ȫ��״̬�仯  �ȴ� 0x37������
 * ����  ����
 * ���  ����
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
 * ������: CON_Read_BUF_Position
 * ����  ��������ȡһ��Buffer ״̬
 * ����  ����
 * ���  ����
 ****************************************************/
void CON_Read_BUF_Position(void)
{
	u8 BUF_TEMP = 0;
	BUF_TEMP = CDR_KEY_SCAN();                    //ɨ��ֵ

  Box_CON.buffer_State_Code	= BUF_TEMP;      		//��ֵ ״̬��
			
	Box_CON.buffer_State[0] = BUF_TEMP%2;					//�ȵ�1#����״̬
	BUF_TEMP = BUF_TEMP/2;
	Box_CON.buffer_State[1] = BUF_TEMP%2;					//�ȵ�2#����״̬
	BUF_TEMP = BUF_TEMP/2;
	Box_CON.buffer_State[2] = BUF_TEMP%2;  				//�ȵ�3#����״̬	
}
/*****************************************************
 * ������: CON_BUF_SCAN_Report
 * ����  ��Buffer״̬�仯�ϱ�
 * ����  ����
 * ���  ����
 ****************************************************/
void CON_BUF_SCAN_Report(void)
{
	u8 BUF_TEMP = 0;
	u8 State[3] = {0};
	u8 i = 0;	
	
	BUF_TEMP = BUF_KEY_SCAN();                  		//��λ   IN_BUF_1 IN_BUF_2 IN_BUF_3     0-7
	if(BUF_TEMP !=Box_CON.buffer_State_Code)				//λ���б仯��¼
	{
		Box_CON.buffer_State_Code	= BUF_TEMP;       //��ֵ
		
		State[0] = BUF_TEMP%2;																			//�ȵ�1#����״̬
		BUF_TEMP = BUF_TEMP/2;
		State[1] = BUF_TEMP%2;																			//�ȵ�2#����״̬
		BUF_TEMP = BUF_TEMP/2;
		State[2] = BUF_TEMP%2;                                      //�ȵ�3#����״̬
		
		for(i = 0;i < 3;i++)
		{
			if(State[i] != Box_CON.buffer_State[i])                 	//��״̬�б仯
			{
				Box_CON.buffer_State[i] = State[i];                   	//��ֵ ״ֵ̬
				CON_Buffer_State_Report(i);															//�ϱ���״̬
				delay_ms(50);
			}
		}			
	}		
}
/*0x38****************************************************
 * ������: CON_Buffer_State_Report
 * ����  ��bufferλ״̬�仯�ϱ�  �ȴ� 0x39������
 * ����  ����
 * ���  ����
 ****************************************************/
void CON_Buffer_State_Report(u8 Buffer)
{
	u8 Send_Buf[11] = {0xAA,0xAB,0x20,0x38,0x03,0x00,0x01,0x01,0x00,0xBA,0xBB};
	Send_Buf[6] = Buffer+1;
	Send_Buf[7] = Box_CON.buffer_State[Buffer];	
	CON_Send_Data(Send_Buf,11);
}

/*0x3B****************************************************
 * ������: CON_Buffer_State_Response
 * ����  ����ѯbufferλ״̬�仯  ��Ӧ 0x3A������
 * ����  ����
 * ���  ����
 ****************************************************/
void CON_Buffer_State_Response(u8 Buffer)
{
	u8 Send_Buf[11] = {0xAA,0xAB,0x20,0x3B,0x03,0x00,0x00,0x00,0x00,0xBA,0xBB};
	Send_Buf[6] = Buffer;
	Send_Buf[7] = Box_CON.buffer_State[Buffer-1];
	CON_Send_Data(Send_Buf,11);
}





/*********************************************��ͣ��ť***********************************************/
/*****************************************************
 * ������: CON_Read_STP_Position
 * ����  ��������ȡһ��STP ״̬
 * ����  ����
 * ���  ����
 ****************************************************/
void CON_Read_STP_Position(void)
{
	
	Box_CON.E_Stop_State = STP_KEY_SCAN();        //ɨ��ֵ  ״̬��

}
/*****************************************************
 * ������: CON_STP_SCAN_Report
 * ����  ����ͣ״̬�仯�ϱ�
 * ����  ����
 * ���  ����
 ****************************************************/
void CON_STP_SCAN_Report(void)
{
	u8 STP_TEMP = 0;
	
	STP_TEMP = STP_KEY_SCAN();                  		//��λ   IN_STP 0-1
	if(STP_TEMP !=Box_CON.E_Stop_State)							//λ���б仯��¼
	{
		Box_CON.E_Stop_State	= STP_TEMP;       			//��ֵ
		CON_E_Stop_State_Report();										//�ϱ�״̬
	}		
}
/*0x3E****************************************************
 * ������: CON_E_Stop_State_Report
 * ����  ���ϱ���ͣ��ť״̬       �ȴ� 0x3F������
 * ����  ����
 * ���  ����
 ****************************************************/
void CON_E_Stop_State_Report(void)
{
	u8 Send_Buf[9] = {0xAA,0xAB,0x20,0x3E,0x01,0x00,0x00,0xBA,0xBB};
	Send_Buf[5] = Box_CON.E_Stop_State ;
	CON_Send_Data(Send_Buf,9);
}

/*0x41****************************************************
 * ������: CON_E_Stop_State_Response
 * ����  ����ѯ��ͣ��ť״̬  ��Ӧ 0x40������
 * ����  ����
 * ���  ����
 ****************************************************/
void CON_E_Stop_State_Response(void)
{
	u8 Send_Buf[9] = {0xAA,0xAB,0x20,0x41,0x01,0x00,0x00,0xBA,0xBB};
	Send_Buf[5] = Box_CON.E_Stop_State;
	CON_Send_Data(Send_Buf,9);
}

/*0x41****************************************************
 * ������: CON_Device_State_Regular_Report
 * ����  ���豸״̬��ʱ�ϱ�   �ȴ� 0x42
 * ����  ����
 * ���  ����
 ****************************************************/
void CON_Device_State_Regular_Report(void)
{
	CON_Device_State(0x41);
}


/*********************************************��ͷ��β***********************************************/
/*****************************************************
 * ������: CON_Head_END_Type_LEN_Check
 * ����  ����� ��ͷ ��β �豸���� ���ݳ���
 * ����  ��buf 			��������
 *         len   		�������ݳ���
 * ���  ����֤ͨ�� ����1 ���򷵻� 0
 ****************************************************/
u8 CON_Head_END_Type_LEN_Check(u8* buf,u8 len)
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
		if(buf[2] != CON_Device_Type)//�豸����
			return 0;
	}
	if(buf[4] != len - 8)				 //���ݳ���
		return 0;
	
	return 1;	
}

/*********************************************����***********************************************/
/**************************************************************
 * ��������Receive_CON_Protocol_Analysis
 * ����  ���������ݽ�����ʼ�� 
 * ����  ��buf 			��������
 *         len   		�������ݳ���
 * ���  ����
 ****************************************************/
void Receive_CON_Protocol_Analysis(u8* buf,u8 len)
{	
	//AA AB 02 09 02 10 02 00 BA BB
	if(CON_Head_END_Type_LEN_Check(buf,len))
	{
		
			switch(buf[3])
			{
				case  0x22:													//0x22�·��豸��Ϣ��ѯ	
					CON_Device_Info_Response();				//0x23�ϱ��豸��Ϣ��ѯӦ��	��
					break;
				case  0x0F:                         //0x26�豸������Ϣ��ѯ
				  CON_Device_Type_Response();				//0x27�豸������Ϣ��ѯӦ��
					break;
				
				case  0x28:													//0x28��ѯ��������λ�� 	
					CON_Replenish_State_Response();   //0x29Ӧ��
					break;
				case 0x2A:																		//0x2A���������ת
					CON_Replenish_Motor_Action(buf[5]);         //��ת��λ����Ӧ��
//					CON_Replenish_Motor_Action_Response();//0x2BӦ��
					break;
				case 0x2C:																		//��ѯ��װ����״̬
					if(buf[5 == 0])
						CON_Ask_Door_Response(buf[6]);							//Ӧ��
					break;
				case 0x31:																		//0x31������ѯ�Ѷ⳵״̬�յ���Ӧ
					CON_Ask_Piling_Car_State_Response(buf[5]);	
					break;
				case 0x32:																		//0x32�Ѷ⳵�仯״̬
					CON_Piling_Car_State_Response(buf[5]);			//0x33Ӧ�� �򿪰�ȫ�ŷ�ֹ��
					break;
				case 0x34:																		//0x34��ѯ��ȫ��״̬ 
					CON_Safety_Door_State_Response(buf[5]);			//0x35Ӧ��
					break;
				case 0x3A:																		//0x3A��ѯBuffer״̬ 
					if(buf[5 == 0])
					CON_Buffer_State_Response(buf[6]);					//0x3BӦ��
					break;
				case 0x40:																		//0x40��ѯ��ͣ��ť״̬ 
					CON_E_Stop_State_Response();								//0x41Ӧ��
					break;				
				default:
					break;
			}
		
	}
}
 
/*****************************************************
 * ������: CON_Start_SCAN_Report
 * ����  ������ɨ��һ�������豸״̬
 * ����  ����
 * ���  ����
 ****************************************************/
void CON_SCAN_Device_State(void)
{
	Read_REP_Sit_State();		//����λ״̬
	CON_Read_CDR_Position();//��װ����״̬
	CON_Read_SDR_Position();//��ȫ��״̬
	CON_Read_BUF_Position();//buffer״̬
	CON_Read_STP_Position();//��ͣ��ť״̬
}
#define Regular_Report_Time  30000
/*****************************************************
 * ������: CON_Receive_Protocol
 * ����  ����ȡ��λ��ָ�����
 * ��ʽ  ��0xAAAB	N	0x03	 len(1Byte)		data (n Bytes)	��У��	0xBABB
 * ����  ����
 * ���  ����
 ****************************************************/
void CON_Receive_Protocol(void)
{
	u8 RE_BUF[128] = {0};
	u8 len = 0;
	u32 T = 0;
  CON_Start_Report_Device_Type();	//���������豸����
	delay_ms(200);
	CON_Start_Report_Check();			 	//���������豸��Ϣ

	while(1)
	{
		
		CON_Receive_Data(RE_BUF,&len);  								//10ms�ӳ�
		if(len)                													//�յ�����                			 
		{
			Receive_CON_Protocol_Analysis(RE_BUF,len);		//��������
			len = 0;  																		//���ݳ������
		}
		
		CON_REP_SCAN_Report();													//ɨ�貹�������豸״̬
		CON_CDR_SCAN_Report();                          //ɨ�輯װ����״̬
		CON_SDR_SCAN_Report();                          //ɨ�谲ȫ��״̬
		CON_BUF_SCAN_Report();                          //ɨ��Buffer״̬
		CON_STP_SCAN_Report();													//ɨ�輱ͣ��ť״̬
		
		T++;
		if(T == Regular_Report_Time/15)
		{
			T = 0;
//			CON_Device_State_Regular_Report();
		}
	}
}
























