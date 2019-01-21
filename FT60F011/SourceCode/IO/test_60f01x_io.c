//********************************************************* 
/*  �ļ�����TEST_60F01x_IO.c
*	���ܣ�  FT60F01x-IO������ʾ
*   IC:    FT60F011A SOP8
*   ����  16M/4T                    
*   ˵����  ��DemoPortIn���ջ��߸ߵ�ƽʱ,
*		   DemoPortOut���50Hzռ�ձ�50%�Ĳ���
*		   ��DemoPortIn�ӵ�ʱ,DemoPortOut����ߵ�ƽ

*                  FT60F011A  SOP8 
*                 ----------------
*  VDD-----------|1(VDD)    (GND)8|------------GND     
*  DemoPortIn----|2(PA2)    (PA4)7|----DemoPortOut 
*  NC------------|3(PA1)    (PA5)6|-------------NC
*  NC------------|4(PA3)    (PA0)5|-------------NC
*			      ----------------
*/
//*********************************************************
#include "SYSCFG.h"
#define OSC_16M  0X70
#define OSC_8M   0X60
#define OSC_4M   0X50
#define OSC_2M   0X40
#define OSC_1M   0X30
#define OSC_500K 0X20
#define OSC_250K 0X10
#define OSC_32K  0X00

#define WDT_256K 0X80
#define WDT_32K  0X00
//**********************************************************
//***********************�궨��*****************************
#define  unchar     unsigned char 
#define  unint      unsigned int
#define  unlong     unsigned long

#define  DemoPortOut	RA4   
#define  DemoPortIn		RA2
/*-------------------------------------------------
 *  ��������POWER_INITIAL
 *	���ܣ�  �ϵ�ϵͳ��ʼ��
 *  ���룺  ��
 *  �����  ��
 --------------------------------------------------*/	
void POWER_INITIAL (void) 
{ 
	OSCCON = WDT_32K|OSC_16M|0X01;	//INROSC
//	OSCCON = 0B01110001;			//WDT 32KHZ IRCF=111=16MHZ/4=4MHZ,0.25US/T
					 				//Bit0=1,ϵͳʱ��Ϊ�ڲ�����(60F01x ����λ��
					 				//Bit0=0,ʱ��Դ��FOSC<2��0>����������ѡ��ʱѡ��

	INTCON = 0;  					//�ݽ�ֹ�����ж�
	PORTA = 0B00000000;				
	TRISA = 0B11101111;				//PA������� 0-��� 1-����
	//TRISA4 =0;					//PA4->���
	WPUA = 0B00000100;     			//PA�˿��������� 1-������ 0-������
	//WPUA2 = 1;					//��PA2����

	OPTION = 0B00001000;			//Bit3=1 WDT MODE,PS=000=1:1 WDT RATE
					 				//Bit7(PAPU)=0 ENABLED PULL UP PA
    MSCKCON = 0B00000000;	        //Bit6->0,��ֹPA4, PC5��ѹ���(60F01x����λ)
					  			    //Bit5->0,TIMER2ʱ��ΪFosc(60F01x����λ)
					  			    //Bit4->0,��ֹLVR(60F01x O��֮ǰ)
                                    //Bit4->0, LVRENʹ��ʱ,����LVR(60F01x O�漰O���)  
                                	//Bit4->1, LVRENʹ��ʱ,����ʱ����LVR, ˯��ʱ�Զ��ر�LVR(60F01x O�漰O��֮��)  
       
	 
}
/*-------------------------------------------------
 *  �������ƣ�DelayUs
 *  ���ܣ�   ����ʱ���� --16M-4T--��ſ�1%����.
 *  ���������Time��ʱʱ�䳤�� ��ʱʱ��Time*2Us
 * 	���ز������� 
 -------------------------------------------------*/
void DelayUs(unsigned char Time)
{
	unsigned char a;
	for(a=0;a<Time;a++)
	{
		NOP();
	}
}                  
/*------------------------------------------------- 
 * 	�������ƣ�DelayMs
 * 	���ܣ�   ����ʱ����
 * 	���������Time��ʱʱ�䳤�� ��ʱʱ��Time ms
 * 	���ز������� 
 -------------------------------------------------*/
void DelayMs(unsigned char Time)
{
	unsigned char a,b;
	for(a=0;a<Time;a++)
	{
		for(b=0;b<5;b++)
		{
		 	DelayUs(98); //��1%
		}
	}
}
/*------------------------------------------------- 
 * 	�������ƣ�DelayS
 * 	���ܣ�   ����ʱ����
 * 	���������Time ��ʱʱ�䳤�� ��ʱʱ��Time S
 * 	���ز������� 
 -------------------------------------------------*/
void DelayS(unsigned char Time)
{
	unsigned char a,b;
	for(a=0;a<Time;a++)
	{
		for(b=0;b<10;b++)
		{
		 	DelayMs(100); 
		}
	}
}
/*-------------------------------------------------
 *  ������:  main 
 *	���ܣ�  ������
 *  ���룺  ��
 *  �����  ��
 --------------------------------------------------*/
void main()
{
	POWER_INITIAL();				//ϵͳ��ʼ��
	while(1)
	{
		DemoPortOut = 1; 			
		DelayMs(10);     			//10ms
		if(DemoPortIn == 1) 		//�ж������Ƿ�Ϊ�ߵ�ƽ 
		{
			DemoPortOut = 0;
		}
		DelayMs(10); 
	}
}