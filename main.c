#include <MDR32F9Qx_port.h> 
#include <MDR32F9Qx_uart.h> 
#include <MDR32F9Qx_rst_clk.h> 
#include <MDR32F9Qx_timer.h>
#include <stdio.h>
#include "lcd.h" 
#include "text.h" 
#include "font_defs.h" 

const char static STR_MYNAME[] = "Somov PE";
const char static STR_MYGROUP[] = "RL1-112";
const char static STR_FREQUENCY[] = "72 MHz";
const char static STR_DELAY_FREQUENCY[] = "7 sec";
const char static STR_UART_SPEED[] = "57 600 bit/sec";
const char static MESSAGE[] = "72 MHz\n\r";
static int i, g_interrupt_counter = 0;

int isSendFlag;

// ��������� ������� ��������
#define DELAY(T) for (i = T; i > 0; i--)

// ������������� �������
void LCD_Init()
	{      
		LCD_INIT();
		CurrentFont = &Font_6x8;
		CurrentMethod = MET_AND;
	} 
// ����� �� �������
void LCD_Write()
	{      
		char str_interrupt_counter[32];      
		sprintf(str_interrupt_counter, "%d", g_interrupt_counter);      
		LCD_CLS();
		LCD_PUTS(0, 0, STR_MYNAME);
		LCD_PUTS(0, 8, STR_MYGROUP);
		LCD_PUTS(0, 17, STR_FREQUENCY);
		LCD_PUTS(0, 26, STR_DELAY_FREQUENCY);
		LCD_PUTS(0, 35, STR_UART_SPEED);
		LCD_PUTS(0, 44, str_interrupt_counter);
	}  
//����������� �������� ������
void CLOCK_Setup(int Freq) 
	{      
		MDR_RST_CLK->HS_CONTROL = 0x1;       // ������������ � ����� �������� ������                
		while (MDR_RST_CLK->CLOCK_STATUS == 0x00) __NOP();// �������� ������������ ������      
		MDR_RST_CLK->PLL_CONTROL |=((Freq/8)-1)*0x0100;		//��������� ��������� PLL=9 cpu clk 72���
	  	MDR_RST_CLK->PLL_CONTROL |= 0x04;     
    while (!(MDR_RST_CLK->CLOCK_STATUS & 0x02)) __NOP(); //�������� ������������ ������       
		MDR_RST_CLK->CPU_CLOCK = 0x000000106;   //������� ���������������� 72 ���     
		MDR_RST_CLK->TIM_CLOCK = 0x01000000;    //TIM1 enable
		SystemCoreClockUpdate();       
	}  
		

//������������� ������		
void PORTSInit() 
	{      
	//UART
		PORT_InitTypeDef SETUP;	//�������� ���� ��������� � ����� ���������			
		RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTF, ENABLE); // ������������ ����� F ��� ������� UART		
		PORT_StructInit(&SETUP); // ��������� ����� �� ���������	
		SETUP.PORT_SPEED = PORT_SPEED_MAXFAST; // ��������� ��������� ������			
		SETUP.PORT_FUNC = PORT_FUNC_OVERRID;   // ��������������� ������� �����			
		SETUP.PORT_MODE = PORT_MODE_DIGITAL;   // �������� ����� ������ ������	
		// ������������� ������ PF1 ��� UART_TX (��������)				
		SETUP.PORT_Pin = PORT_Pin_1;      
		SETUP.PORT_OE = PORT_OE_OUT; //������������ ����� ����� ��� ������     
		PORT_Init(MDR_PORTF,&SETUP);
		// ������������� ������ PF0 ��� UART_RX (�����)				
		SETUP.PORT_Pin = PORT_Pin_0;      
		SETUP.PORT_OE = PORT_OE_IN; //������������ ����� ����� ��� �����      
		PORT_Init(MDR_PORTF,&SETUP);
	//����������
		RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTB, ENABLE);      
		PORT_InitTypeDef SETUP1;
		SETUP1.PORT_Pin = PORT_Pin_0|PORT_Pin_1|PORT_Pin_2|PORT_Pin_3;       
		SETUP1.PORT_OE = PORT_OE_OUT;      
		SETUP1.PORT_FUNC = PORT_FUNC_PORT; 	 //������ � ������ ����� �����-������     
		SETUP1.PORT_MODE = PORT_MODE_DIGITAL;      
		SETUP1.PORT_SPEED = PORT_SPEED_SLOW; //������ �������� ������������ (������� �����)
		PORT_Init(MDR_PORTB,&SETUP1);
	} 
			
//��������� UART
void UARTInit() 
	{      
		RST_CLK_PCLKcmd(RST_CLK_PCLK_UART2, ENABLE);  //��������� ������������ UART2     
		UART_InitTypeDef UART_InitStructure;  				//���������� ��������� ��� ������������� UART
		UART_BRGInit(MDR_UART2, UART_HCLKdiv1); 			//�������� �������� ������� UART = 1     
		UART_InitStructure.UART_BaudRate = 57600 ;    // �������� �������� ������
		UART_InitStructure.UART_WordLength = UART_WordLength8b; // ����� ����� � ������   
		UART_InitStructure.UART_StopBits = UART_StopBits1;     // ���� ����-���	
		UART_InitStructure.UART_Parity = UART_Parity_No;      // ��� �������� ��������
		UART_InitStructure.UART_FIFOMode = UART_FIFO_ON;     // �������� �������������� �� ������ �����
		// ��������� ����� � �������� ������
		UART_InitStructure.UART_HardwareFlowControl = UART_HardwareFlowControl_RXE | UART_HardwareFlowControl_TXE;
		DELAY(500);					
		UART_Init(MDR_UART2, &UART_InitStructure); // �������������� UART2 � ��������� �����������	
		UART_Cmd(MDR_UART2, ENABLE); // �������� ������������������ UART   
	} 

//�������� �����������
void LED() 
	{      
		static uint8_t counter = 0; //���������� ���������� counter     
		if (counter++ % 2) //������� �� �������          
			PORT_SetBits(MDR_PORTB, PORT_Pin_0);  //���������� ��� �� ����� 0     
		else           
			PORT_ResetBits(MDR_PORTB, PORT_Pin_0); //�������� ��� �� ����� 0 
	} 

//������������� �������
void TIMER_Init() 
	{      
		TIMER_CntInitTypeDef TIM1Init; //�������� ���� � ����� ���������
		RST_CLK_PCLKcmd(RST_CLK_PCLK_TIMER1, ENABLE); //��������� ������������     
		TIMER_CntStructInit(&TIM1Init); // ���������� ��������� ���������� �� ���������
		TIMER_BRGInit(MDR_TIMER1, TIMER_HCLKdiv8); // ��������� �������� �������� �������
		TIM1Init.TIMER_Prescaler = 10000; // ������� ������������ �������� �������
		TIM1Init.TIMER_Period = 7000; // ������� ������ ������������ �������
		TIMER_CntInit(MDR_TIMER1, &TIM1Init); // ������������� ����� ������� ����������� ����������
		NVIC_EnableIRQ(Timer1_IRQn); // ��������� ����������	
		NVIC_SetPriority(Timer1_IRQn, 1); // ��������� ���������� ����������
		TIMER_ITConfig(MDR_TIMER1, TIMER_STATUS_CNT_ZERO, ENABLE); // ���. ���������� ��� =0 �������� Timer1
		TIMER_Cmd(MDR_TIMER1, ENABLE); // ������ �������
	}  
	
//���������� �� �������
void Timer1_IRQHandler() 
	{ 
	//���� ������ ������� � ����, ��������������� ������� � �������� ���� ����������
	if (TIMER_GetITStatus(MDR_TIMER1, TIMER_STATUS_CNT_ZERO))      
		{           
			g_interrupt_counter++;           
			isSendFlag = 1;
					
			TIMER_ClearITPendingBit(MDR_TIMER1, TIMER_STATUS_CNT_ZERO);      
		} 
	} 

//������� �������
int main() 
{      
	CLOCK_Setup(72); //����������� �������� ������ 
	LCD_Init();	 // ��������� ������������� ������� 
	PORTSInit();     //��������� ������������� ������ 
	TIMER_Init();    //��������� ������������� �������
	UARTInit();      //��������� ������������� UART 
				
	while (1)
	{
		if (isSendFlag)
			{
			LED(); //����� LED() 
			LCD_Write(); //������� �� ������� 
			uint8_t c;

			for (c = 0; c < sizeof(MESSAGE) - 1; c++)
				{
				UART_SendData(MDR_UART2, MESSAGE[c]); //��������� ������ � ����
				}
			isSendFlag = 0;
			}
	}
}
