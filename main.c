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

// Опредение функции задержки
#define DELAY(T) for (i = T; i > 0; i--)

// Инициализация дисплея
void LCD_Init()
	{      
		LCD_INIT();
		CurrentFont = &Font_6x8;
		CurrentMethod = MET_AND;
	} 
// Вывод на дисплей
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
//Подключение внешнего кварца
void CLOCK_Setup(int Freq) 
	{      
		MDR_RST_CLK->HS_CONTROL = 0x1;       // переключение в режим внешнего кварца                
		while (MDR_RST_CLK->CLOCK_STATUS == 0x00) __NOP();// ожидание стабилизации кварца      
		MDR_RST_CLK->PLL_CONTROL |=((Freq/8)-1)*0x0100;		//установка множителя PLL=9 cpu clk 72МГц
	  	MDR_RST_CLK->PLL_CONTROL |= 0x04;     
    while (!(MDR_RST_CLK->CLOCK_STATUS & 0x02)) __NOP(); //ожидание стабилизации кварца       
		MDR_RST_CLK->CPU_CLOCK = 0x000000106;   //частота микроконтроллера 72 МГц     
		MDR_RST_CLK->TIM_CLOCK = 0x01000000;    //TIM1 enable
		SystemCoreClockUpdate();       
	}  
		

//Инициализация портов		
void PORTSInit() 
	{      
	//UART
		PORT_InitTypeDef SETUP;	//указание типа структуры и имени структуры			
		RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTF, ENABLE); // Тактирование порта F для функции UART		
		PORT_StructInit(&SETUP); // Настройка порта по умолчанию	
		SETUP.PORT_SPEED = PORT_SPEED_MAXFAST; // Установка короткого фронта			
		SETUP.PORT_FUNC = PORT_FUNC_OVERRID;   // Переопределение функции порта			
		SETUP.PORT_MODE = PORT_MODE_DIGITAL;   // Цифровой режим работы вывода	
		// Инициализация вывода PF1 как UART_TX (передача)				
		SETUP.PORT_Pin = PORT_Pin_1;      
		SETUP.PORT_OE = PORT_OE_OUT; //конфигурация линии порта как выхода     
		PORT_Init(MDR_PORTF,&SETUP);
		// Инициализация вывода PF0 как UART_RX (прием)				
		SETUP.PORT_Pin = PORT_Pin_0;      
		SETUP.PORT_OE = PORT_OE_IN; //конфигурация линии порта как входа      
		PORT_Init(MDR_PORTF,&SETUP);
	//Светодиоды
		RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTB, ENABLE);      
		PORT_InitTypeDef SETUP1;
		SETUP1.PORT_Pin = PORT_Pin_0|PORT_Pin_1|PORT_Pin_2|PORT_Pin_3;       
		SETUP1.PORT_OE = PORT_OE_OUT;      
		SETUP1.PORT_FUNC = PORT_FUNC_PORT; 	 //работа в режиме порта ввода-вывода     
		SETUP1.PORT_MODE = PORT_MODE_DIGITAL;      
		SETUP1.PORT_SPEED = PORT_SPEED_SLOW; //низкая скорость переключения (пологий склон)
		PORT_Init(MDR_PORTB,&SETUP1);
	} 
			
//Настройка UART
void UARTInit() 
	{      
		RST_CLK_PCLKcmd(RST_CLK_PCLK_UART2, ENABLE);  //включение тактирования UART2     
		UART_InitTypeDef UART_InitStructure;  				//объявление структуры для инициализации UART
		UART_BRGInit(MDR_UART2, UART_HCLKdiv1); 			//делитель тактовой частоты UART = 1     
		UART_InitStructure.UART_BaudRate = 57600 ;    // скорость передачи данных
		UART_InitStructure.UART_WordLength = UART_WordLength8b; // длина слова в пакете   
		UART_InitStructure.UART_StopBits = UART_StopBits1;     // один стоп-бит	
		UART_InitStructure.UART_Parity = UART_Parity_No;      // без проверки четности
		UART_InitStructure.UART_FIFOMode = UART_FIFO_ON;     // передача осуществляется по одному байту
		// Разрешить прием и передачу данных
		UART_InitStructure.UART_HardwareFlowControl = UART_HardwareFlowControl_RXE | UART_HardwareFlowControl_TXE;
		DELAY(500);					
		UART_Init(MDR_UART2, &UART_InitStructure); // Ининциализация UART2 с заданными параметрами	
		UART_Cmd(MDR_UART2, ENABLE); // Включить сконфигурированный UART   
	} 

//Моргание светодиодом
void LED() 
	{      
		static uint8_t counter = 0; //объявление переменной counter     
		if (counter++ % 2) //остаток от деления          
			PORT_SetBits(MDR_PORTB, PORT_Pin_0);  //установить бит на линии 0     
		else           
			PORT_ResetBits(MDR_PORTB, PORT_Pin_0); //сбросить бит на линии 0 
	} 

//Инициализация таймера
void TIMER_Init() 
	{      
		TIMER_CntInitTypeDef TIM1Init; //указание типа и имени структуры
		RST_CLK_PCLKcmd(RST_CLK_PCLK_TIMER1, ENABLE); //включение тактирования     
		TIMER_CntStructInit(&TIM1Init); // Заполнение структуры значениями по умолчанию
		TIMER_BRGInit(MDR_TIMER1, TIMER_HCLKdiv8); // Настройка делителя тактовой частоты
		TIM1Init.TIMER_Prescaler = 10000; // Задание предделителя тактовой частоты
		TIM1Init.TIMER_Period = 7000; // Задание период срабатывания таймера
		TIMER_CntInit(MDR_TIMER1, &TIM1Init); // Инициализация порта таймера объявленной структурой
		NVIC_EnableIRQ(Timer1_IRQn); // Включение прерывания	
		NVIC_SetPriority(Timer1_IRQn, 1); // Установка приоритета прерывания
		TIMER_ITConfig(MDR_TIMER1, TIMER_STATUS_CNT_ZERO, ENABLE); // Вкл. прерывания при =0 значения Timer1
		TIMER_Cmd(MDR_TIMER1, ENABLE); // Запуск таймера
	}  
	
//Прерывания от таймера
void Timer1_IRQHandler() 
	{ 
	//Если таймер сброшен в ноль, икрементировать счетчик и сбросить флаг прерывания
	if (TIMER_GetITStatus(MDR_TIMER1, TIMER_STATUS_CNT_ZERO))      
		{           
			g_interrupt_counter++;           
			isSendFlag = 1;
					
			TIMER_ClearITPendingBit(MDR_TIMER1, TIMER_STATUS_CNT_ZERO);      
		} 
	} 

//Главная функция
int main() 
{      
	CLOCK_Setup(72); //подключение внешнего кварца 
	LCD_Init();	 // процедура инициализации дисплея 
	PORTSInit();     //процедура инициализации портов 
	TIMER_Init();    //процедура инициализации таймера
	UARTInit();      //процедура инициализации UART 
				
	while (1)
	{
		if (isSendFlag)
			{
			LED(); //вызов LED() 
			LCD_Write(); //вывести на дисплей 
			uint8_t c;

			for (c = 0; c < sizeof(MESSAGE) - 1; c++)
				{
				UART_SendData(MDR_UART2, MESSAGE[c]); //отправить символ в порт
				}
			isSendFlag = 0;
			}
	}
}
