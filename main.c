/*
Name:					MCU SOFTWARE
Version:				1.1.3
Date:					08.05.2020
Comment:				COMPLETELY STABLE VERSION (with USB)
Recent updates:				now new commands + fixed few bugs
Owner:					DVLabs
Made by:				083c083c , Sasha Grizzly
*/

#include "main.h"
#include "stm32f1xx_hal.h"
#include "usb_device.h"
#include "string.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim3;
DMA_HandleTypeDef hdma_tim3_ch4_up;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
#define PORT_M_1 GPIOB
#define WHITE_1 GPIO_PIN_6
#define RED_1 GPIO_PIN_5
#define BLACK_2 GPIO_PIN_9
#define BROWN_2 GPIO_PIN_8
#define EN_MOT_1_1 GPIO_PIN_7
#define EN_MOT_1_2 GPIO_PIN_13

#define lim_f 3900
#define lim_d 850
#define VERSION_STRING "MCU V1.1\n"  //ВЕРСИЯ ПРОШИВКИ


#define PORT_M_2 GPIOC
#define WHITE_3 GPIO_PIN_10
#define RED_3 GPIO_PIN_11
#define BLACK_4 GPIO_PIN_12
#define BROWN_4 GPIO_PIN_2 //PORTD
#define EN_MOT_2_1 GPIO_PIN_15 //PORTA
#define EN_MOT_2_2 GPIO_PIN_4 //PORTB

#define STMPS_EN GPIO_PIN_13 		//pin3 HA MAKETKE
#define STMPS_FT GPIO_PIN_12 		//pin2
#define STMPS_PORT GPIOB 			  //GPIOB

		int MAX_FLAG_F;
		int MIN_FLAG_F;
		int MAX_FLAG_D;
		int MIN_FLAG_D;
int INITF_flag=0;
int INITD_flag=0;
int lim_f_error_flag=0;
int lim_d_error_flag=0;

char uart1_data;
char uart1_rx_buf[128];	
uint8_t uart1_rx_bit; 
char input;
extern char usb_rx[128];
int speed=1;
int current_pos_f;
int current_pos_d;
int zero_check;
int init_pos_f;
int init_pos_d;

int percent_int_f = 0;
int mem_f;
int mem_perc_f;
int all_steps_f = 0;
float one_step_f = 0.0;
int go_step_f = 0;
_Bool go_dir_f = 0;
//int lim_f = 3900;

int percent_int_d = 0;
int mem_d;
int mem_perc_d;
int all_steps_d = 0;
float one_step_d = 0.0;
int go_step_d = 0;
_Bool go_dir_d = 0;
//int lim_d = 850;

char valuev[100];
char valuev1[100];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_TIM3_Init(void);
                                    
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);


/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void EXTI1_IRQHandler(void)						//edge_detect interrupt FOCUS SENSOR
{

  if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1))
								{
									MAX_FLAG_F = 1;
									MIN_FLAG_F = 0;
								}
								else
								{
									MAX_FLAG_F = 0;
									MIN_FLAG_F = 1;									
								}
								
		HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}
void EXTI0_IRQHandler(void)						//edge_detect interrupt DIAPH SENSOR
{

  if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0))
								{
									MAX_FLAG_D = 1;
									MIN_FLAG_D = 0;
								}
								else
								{
									MAX_FLAG_D = 0;
									MIN_FLAG_D = 1;									
								}
								
		HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}
void EXTI12_IRQHandler(void)						//interrupt FAULT
{

  if (HAL_GPIO_ReadPin(STMPS_PORT, STMPS_FT))
					{
						HAL_GPIO_WritePin(STMPS_PORT, STMPS_EN, GPIO_PIN_SET);
						HAL_Delay(50);
						HAL_GPIO_WritePin(STMPS_PORT, STMPS_EN, GPIO_PIN_RESET);
					}
								
		HAL_GPIO_EXTI_IRQHandler(STMPS_FT);
}
void stop_motor_F() 																	//функция остановки движка фокуса
{
		HAL_GPIO_WritePin(PORT_M_2,WHITE_3,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(PORT_M_2,RED_3,GPIO_PIN_RESET);		
		HAL_GPIO_WritePin(PORT_M_2,BLACK_4,GPIO_PIN_RESET);	
		HAL_GPIO_WritePin(GPIOD,BROWN_4,GPIO_PIN_RESET);
}

void stop_motor_D() 																	//функция остановки движка диафрагмы
{
		HAL_GPIO_WritePin(PORT_M_1,WHITE_1,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(PORT_M_1,RED_1,GPIO_PIN_RESET);		
		HAL_GPIO_WritePin(PORT_M_1,BLACK_2,GPIO_PIN_RESET);	
		HAL_GPIO_WritePin(PORT_M_1,BROWN_2,GPIO_PIN_RESET);
}

void motor_F(int speed, int steps, int dir)						//функция управления обмотками движка
{ 
		
		if (dir==1)	
{																					//ПОЛОЖИТ
		
		for(int i = 0; i<steps; i++)
	{

HAL_GPIO_WritePin(PORT_M_2,RED_3,GPIO_PIN_RESET);				//1
HAL_GPIO_WritePin(PORT_M_2,WHITE_3,GPIO_PIN_SET);
HAL_GPIO_WritePin(GPIOD,BROWN_4,GPIO_PIN_RESET);
HAL_GPIO_WritePin(PORT_M_2,BLACK_4,GPIO_PIN_SET);	
				
HAL_Delay(speed);

HAL_GPIO_WritePin(PORT_M_2,RED_3,GPIO_PIN_RESET);				//2
HAL_GPIO_WritePin(PORT_M_2,WHITE_3,GPIO_PIN_SET);
HAL_GPIO_WritePin(GPIOD,BROWN_4,GPIO_PIN_SET);
HAL_GPIO_WritePin(PORT_M_2,BLACK_4,GPIO_PIN_RESET);	
				
HAL_Delay(speed);

HAL_GPIO_WritePin(PORT_M_2,RED_3,GPIO_PIN_SET);					//3
HAL_GPIO_WritePin(PORT_M_2,WHITE_3,GPIO_PIN_RESET);
HAL_GPIO_WritePin(GPIOD,BROWN_4,GPIO_PIN_SET);
HAL_GPIO_WritePin(PORT_M_2,BLACK_4,GPIO_PIN_RESET);			
		
HAL_Delay(speed);

HAL_GPIO_WritePin(PORT_M_2,RED_3,GPIO_PIN_SET);					//4
HAL_GPIO_WritePin(PORT_M_2,WHITE_3,GPIO_PIN_RESET);
HAL_GPIO_WritePin(GPIOD,BROWN_4,GPIO_PIN_RESET);
HAL_GPIO_WritePin(PORT_M_2,BLACK_4,GPIO_PIN_SET);			

HAL_Delay(speed);

current_pos_f++;
	}
}

		else if (dir==0)	
{																					//ОТРИЦАТ
		
		for(int i = 0; i<steps; i++)
	{
HAL_GPIO_WritePin(PORT_M_2,RED_3,GPIO_PIN_SET);					//4
HAL_GPIO_WritePin(PORT_M_2,WHITE_3,GPIO_PIN_RESET);
HAL_GPIO_WritePin(GPIOD,BROWN_4,GPIO_PIN_RESET);
HAL_GPIO_WritePin(PORT_M_2,BLACK_4,GPIO_PIN_SET);			

HAL_Delay(speed);

HAL_GPIO_WritePin(PORT_M_2,RED_3,GPIO_PIN_SET);					//3
HAL_GPIO_WritePin(PORT_M_2,WHITE_3,GPIO_PIN_RESET);
HAL_GPIO_WritePin(GPIOD,BROWN_4,GPIO_PIN_SET);
HAL_GPIO_WritePin(PORT_M_2,BLACK_4,GPIO_PIN_RESET);			
		
HAL_Delay(speed);
		
HAL_GPIO_WritePin(PORT_M_2,RED_3,GPIO_PIN_RESET);				//2
HAL_GPIO_WritePin(PORT_M_2,WHITE_3,GPIO_PIN_SET);
HAL_GPIO_WritePin(GPIOD,BROWN_4,GPIO_PIN_SET);
HAL_GPIO_WritePin(PORT_M_2,BLACK_4,GPIO_PIN_RESET);	
				
HAL_Delay(speed);
		
HAL_GPIO_WritePin(PORT_M_2,RED_3,GPIO_PIN_RESET);				//1
HAL_GPIO_WritePin(PORT_M_2,WHITE_3,GPIO_PIN_SET);
HAL_GPIO_WritePin(GPIOD,BROWN_4,GPIO_PIN_RESET);
HAL_GPIO_WritePin(PORT_M_2,BLACK_4,GPIO_PIN_SET);	
				
HAL_Delay(speed);

current_pos_f--;
		}
	}
}

void motor_D(int speed, int steps, int dir)						//функция управления обмотками движка
{ 
		
		if (dir==0)	
{																					//ПОЛОЖИТ
		
		for(int i = 0; i<steps; i++)
	{

HAL_GPIO_WritePin(PORT_M_1,RED_1,GPIO_PIN_RESET);				//1
HAL_GPIO_WritePin(PORT_M_1,WHITE_1,GPIO_PIN_SET);
HAL_GPIO_WritePin(PORT_M_1,BROWN_2,GPIO_PIN_RESET);
HAL_GPIO_WritePin(PORT_M_1,BLACK_2,GPIO_PIN_SET);	
				
HAL_Delay(speed);

HAL_GPIO_WritePin(PORT_M_1,RED_1,GPIO_PIN_RESET);				//2
HAL_GPIO_WritePin(PORT_M_1,WHITE_1,GPIO_PIN_SET);
HAL_GPIO_WritePin(PORT_M_1,BROWN_2,GPIO_PIN_SET);
HAL_GPIO_WritePin(PORT_M_1,BLACK_2,GPIO_PIN_RESET);	
				
HAL_Delay(speed);

HAL_GPIO_WritePin(PORT_M_1,RED_1,GPIO_PIN_SET);					//3
HAL_GPIO_WritePin(PORT_M_1,WHITE_1,GPIO_PIN_RESET);
HAL_GPIO_WritePin(PORT_M_1,BROWN_2,GPIO_PIN_SET);
HAL_GPIO_WritePin(PORT_M_1,BLACK_2,GPIO_PIN_RESET);			
		
HAL_Delay(speed);

HAL_GPIO_WritePin(PORT_M_1,RED_1,GPIO_PIN_SET);					//4
HAL_GPIO_WritePin(PORT_M_1,WHITE_1,GPIO_PIN_RESET);
HAL_GPIO_WritePin(PORT_M_1,BROWN_2,GPIO_PIN_RESET);
HAL_GPIO_WritePin(PORT_M_1,BLACK_2,GPIO_PIN_SET);			

HAL_Delay(speed);

current_pos_d--;
	}
}

		else if (dir==1)	
{																					//отрицат
		
		for(int i = 0; i<steps; i++)
	{
HAL_GPIO_WritePin(PORT_M_1,RED_1,GPIO_PIN_SET);					//4
HAL_GPIO_WritePin(PORT_M_1,WHITE_1,GPIO_PIN_RESET);
HAL_GPIO_WritePin(PORT_M_1,BROWN_2,GPIO_PIN_RESET);
HAL_GPIO_WritePin(PORT_M_1,BLACK_2,GPIO_PIN_SET);			

HAL_Delay(speed);

HAL_GPIO_WritePin(PORT_M_1,RED_1,GPIO_PIN_SET);					//3
HAL_GPIO_WritePin(PORT_M_1,WHITE_1,GPIO_PIN_RESET);
HAL_GPIO_WritePin(PORT_M_1,BROWN_2,GPIO_PIN_SET);
HAL_GPIO_WritePin(PORT_M_1,BLACK_2,GPIO_PIN_RESET);			
		
HAL_Delay(speed);
		
HAL_GPIO_WritePin(PORT_M_1,RED_1,GPIO_PIN_RESET);				//2
HAL_GPIO_WritePin(PORT_M_1,WHITE_1,GPIO_PIN_SET);
HAL_GPIO_WritePin(PORT_M_1,BROWN_2,GPIO_PIN_SET);
HAL_GPIO_WritePin(PORT_M_1,BLACK_2,GPIO_PIN_RESET);	
				
HAL_Delay(speed);
		
HAL_GPIO_WritePin(PORT_M_1,RED_1,GPIO_PIN_RESET);				//1
HAL_GPIO_WritePin(PORT_M_1,WHITE_1,GPIO_PIN_SET);
HAL_GPIO_WritePin(PORT_M_1,BROWN_2,GPIO_PIN_RESET);
HAL_GPIO_WritePin(PORT_M_1,BLACK_2,GPIO_PIN_SET);	
				
HAL_Delay(speed);

current_pos_d++;
		}
	}
}
void go_to_min_f()																		//движение до min фокуса
      {
				stop_motor_F();																//остановка перед разворотом
				HAL_Delay(speed);															//пауза
				
				while(MIN_FLAG_F != 1)
           { 
						 motor_F(1,5,1);
						 
						 if (current_pos_f>lim_f)
						 {
							 		MAX_FLAG_F = 1;
									MIN_FLAG_F = 1;
									lim_f_error_flag = 1;
						 }
           }       
      }
void go_to_min_d()																		//движение до min диафрагмы
      {
				stop_motor_D();																//остановка перед разворотом
				HAL_Delay(speed);															//пауза
				
				while(MIN_FLAG_D != 1)
           { 
						 motor_D(1,5,0);
						 
						 if (current_pos_d<-lim_d)
						 {
							 		MAX_FLAG_D = 1;
									MIN_FLAG_D = 1;
									lim_d_error_flag = 1;
						 }
           }       
      }			
void go_to_max_f()																		//движение до max фокуса
      {
				stop_motor_F();																//остановка перед разворотом
				HAL_Delay(speed);															//пауза
				
				while(MAX_FLAG_F != 1)
           {
						 motor_F(1,5,0);
						 
						 if (current_pos_f<-lim_f)
						 {
							 		MAX_FLAG_F = 1;
									MIN_FLAG_F = 1;
									lim_f_error_flag = 1;
						 }
           }
      }
void go_to_max_d()																		//движение до max диафрагмы
      {
				stop_motor_D();																//остановка перед разворотом
				HAL_Delay(speed);															//пауза
				
				while(MAX_FLAG_D != 1)
           {
						 motor_D(1,5,1);
						 
						 if (current_pos_d>lim_d)
						 {
							 		MAX_FLAG_D = 1;
									MIN_FLAG_D = 1;
									lim_d_error_flag = 1;
						 }
           }
      }			

int init_F()
{		
			init_pos_f=0;
			all_steps_f=0;
			current_pos_f=0;
	if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1)==1)
	{
			go_to_min_f();
			init_pos_f=current_pos_f;
			current_pos_f=0;
			go_to_max_f();
	}
	else if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1)==0)
	{
			go_to_max_f();
			init_pos_f=current_pos_f;
			current_pos_f=0;
			go_to_min_f();	
	}	
	int steps_f = abs(current_pos_f);

     return steps_f;

}
int init_D()
{
			init_pos_d=0;
			all_steps_d=0;
			current_pos_d=0;
	if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)==1)
	{
			go_to_min_d();
			init_pos_d=current_pos_d;
			current_pos_d=0;
			go_to_max_d();
	}
	else if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)==0)
	{
			go_to_max_d();
			init_pos_d=current_pos_d;
			current_pos_d=0;
			go_to_min_d();	
	}	
	int steps_d = abs(current_pos_d);

     return steps_d;
	
}
static void terminal()		//функция обработки команд из терминала
{
int stps_f;
int stps_d;
	
		uint16_t len = strlen((const char*)usb_rx);
	
	    if(len > 0)
			
			{		
				 const char *str=usb_rx;
					 if			(strncmp(usb_rx,"AT+FIN\r", 7)==0)   				//INITF
					 {
							input=1;
					 }
					 else if(strncmp(usb_rx,"AT+AIN\r", 7)==0) 				  //INITD
					 {
							input=2;
					 }					 
					 else if(strncmp(usb_rx,"AT+FMIN\r", 8)==0) 				//min focus
					 {
							input=3;
					 }
					 else if(strncmp(usb_rx,"AT+AMIN\r",8)==0) 					//min diaph
					 {
							input=4;
					 } 					 
					 else if(strncmp(usb_rx,"AT+FMAX\r",8)==0) 					//max focus
					 {
							input=5;
					 }
					 else if(strncmp(usb_rx,"AT+AMAX\r",8)==0) 					//max diaph
					 {
							input=6;
					 }
					 else if(strncmp(usb_rx,"AT+FMOV=",8)==0) 					//FMOV
					 {
							input=7;
					 }					 
					 else if(strncmp(usb_rx,"AT+AMOV=",8)==0) 					//AMOV
					 {
							input=8;
					 }					 
					 else if(strncmp(usb_rx,"AT+CMGR?\r",9)==0) 		  	//запрос версии прошивки
					 {
							input=11;
					 }
					 else if(strncmp(usb_rx,"AT+APOS?\r",9)==0) 			  //запрос позиции diaph движка
					 {
							input=12;
					 }
					 else if(strncmp(usb_rx,"AT+FPOS?\r",9)==0) 			  //запрос позиции focus движкa
					 {
							input=13;
					 }					 
					 else if(strcmp(usb_rx,str)==0) 										//  APOS= / FPOS=
					 {
							input=14;
					 }
					 else 
					 {
							input=0;
					 }		 
					 
		switch (input)
		{
					
					case 1:																																				//INITF		 +FIN
								HAL_Delay(1);
								CDC_Transmit_FS((uint8_t *)"OK\n", 3);
								all_steps_f = init_F();
							if ((all_steps_f>lim_f) || (lim_f_error_flag==1))
									{
											HAL_Delay(1);
											CDC_Transmit_FS((uint8_t *)"+CME ERROR: 21\n", 15);
											INITF_flag=0;						
									}
							else
									{
											if (current_pos_f < 0) current_pos_f = 0;
											else current_pos_f = all_steps_f;
											HAL_Delay(1);
											CDC_Transmit_FS((uint8_t *)"+FIN OK\n", 8);	
											go_step_f = all_steps_f - init_pos_f;
											motor_F(1,go_step_f,1);
										INITF_flag=1;
									}
						break;
					
					case 2:																																				//INITD     +AIN
								HAL_Delay(1);					
								CDC_Transmit_FS((uint8_t *)"OK\n", 3);						
								all_steps_d = init_D();
							if ((all_steps_d>lim_d) || (lim_d_error_flag==1))
									{
											HAL_Delay(1);								
											CDC_Transmit_FS((uint8_t *)"+CME ERROR: 11\n", 15);	
											INITD_flag=0;						
									}
							else
									{
											if (current_pos_d < 0) current_pos_d = 0;
											else current_pos_d = all_steps_d;
											HAL_Delay(1);										
											CDC_Transmit_FS((uint8_t *)"+AIN OK\n", 8);
											go_step_d = all_steps_d - init_pos_d;
											motor_D(1,go_step_d,1);	
										INITD_flag=1;										
									}
						break;					
					
					case 3:																																			  //min_f    +FMIN
							HAL_Delay(1);		
							CDC_Transmit_FS((uint8_t *)"OK\n", 3);
							if (INITF_flag==0)
									{
											HAL_Delay(1);
											CDC_Transmit_FS((uint8_t *)"+CME ERROR: 23\n", 15);
									}
					
							else
								{
									percent_int_f = 0;
									mem_perc_f=percent_int_f;
									one_step_f = all_steps_f/100.0;          
									stps_f = ((float)percent_int_f * one_step_f);
											
									if (current_pos_f > stps_f) 
										{
											go_step_f = abs(current_pos_f-stps_f);
											go_dir_f = 0;
										}
									else if (current_pos_f < stps_f) 
										{
											go_step_f = stps_f-current_pos_f;
											go_dir_f = 1;
										} 
																
									motor_F(1,go_step_f,go_dir_f);
									go_step_f = 0;

								HAL_Delay(1);										
								CDC_Transmit_FS((uint8_t *)"+FMIN OK\n", 9);									
								}								
						break;
								
					case 4:																																			//min_d	   +AMIN
							HAL_Delay(1);
							CDC_Transmit_FS((uint8_t *)"OK\n", 3);				
							if (INITD_flag==0)
									{
											HAL_Delay(1);									
											CDC_Transmit_FS((uint8_t *)"+CME ERROR: 12\n", 15);
									}	
		
							else
								{					
									percent_int_d = 0;
									mem_perc_d=percent_int_d;
									one_step_d = all_steps_d/100.0;          
									stps_d = ((float)percent_int_d * one_step_d);
											
									if (current_pos_d > stps_d) 
										{
											go_step_d = abs(current_pos_d-stps_d);
											go_dir_d = 0;
										}
									else if (current_pos_d < stps_d) 
										{
											go_step_d = stps_d-current_pos_d;
											go_dir_d = 1;
										} 
																
									motor_D(1,go_step_d,go_dir_d);
									go_step_d = 0;

								HAL_Delay(1);
								CDC_Transmit_FS((uint8_t *)"+AMIN OK\n", 9);
								}								
						break;								
					
					case 5:																																			//max_f    +FMAX
							HAL_Delay(1);
							CDC_Transmit_FS((uint8_t *)"OK\n", 3);
							if (INITF_flag==0)
									{
											HAL_Delay(1);									
											CDC_Transmit_FS((uint8_t *)"+CME ERROR: 23\n", 15);
									}
		
							else
								{					
									percent_int_f = 100;
									mem_perc_f=percent_int_f;
									one_step_f = all_steps_f/100.0;          
									stps_f = ((float)percent_int_f * one_step_f);
														
									if (current_pos_f > stps_f) 
										{
											go_step_f = abs(current_pos_f-stps_f);
											go_dir_f = 0;
										}
									else if (current_pos_f < stps_f) 
										{
											go_step_f = stps_f-current_pos_f;
											go_dir_f = 1;
										} 
																
									motor_F(1,go_step_f,go_dir_f);
									go_step_f = 0;

								HAL_Delay(1);
								CDC_Transmit_FS((uint8_t *)"+FMAX OK\n", 9);
								}								
						break;
								
					case 6:																																			//max_d	    +AMAX
							HAL_Delay(1);
							CDC_Transmit_FS((uint8_t *)"OK\n", 3);
					
							if (INITD_flag==0)
									{
											HAL_Delay(1);										
											CDC_Transmit_FS((uint8_t *)"+CME ERROR: 12\n", 15);
									}	
		
							else
							{						
									percent_int_d = 100;
									mem_perc_d=percent_int_d;
									one_step_d = all_steps_d/100.0;          
									stps_d = ((float)percent_int_d * one_step_d);
														
									if (current_pos_d > stps_d) 
										{
											go_step_d = abs(current_pos_d-stps_d);
											go_dir_d = 0;
										}
									else if (current_pos_d < stps_d) 
										{
											go_step_d = stps_d-current_pos_d;
											go_dir_d = 1;
										} 
																
									motor_D(1,go_step_d,go_dir_d);
									go_step_d = 0;

								HAL_Delay(1);		
								CDC_Transmit_FS((uint8_t *)"+AMAX OK\n", 9);	
		}								
						break;

					case 7:																																			//  +FMOV
							HAL_Delay(1);
							CDC_Transmit_FS((uint8_t *)"OK\n", 3);
							percent_int_f = atoi(str+9);
								if (percent_int_f<1)
												{
													HAL_Delay(1);
													CDC_Transmit_FS((uint8_t *)"+CME ERROR: 3\n", 14);
												}
								else if (percent_int_f>99)
												{
													HAL_Delay(1);
													CDC_Transmit_FS((uint8_t *)"+CME ERROR: 3\n", 14);
												}
								else
								{
					
										if (INITF_flag==0)
												{
													HAL_Delay(1);
													CDC_Transmit_FS((uint8_t *)"+CME ERROR: 23\n", 15);
												}
		
										else
												{
													one_step_f = all_steps_f/100.0;          
													stps_f = ((float)percent_int_f * one_step_f);
																
											if(strncmp(usb_rx,"AT+FMOV=-", 9)==0) 
												{
													if ((current_pos_f-stps_f)<0)
														{
															HAL_Delay(1);
															CDC_Transmit_FS((uint8_t *)"+CME ERROR: 23\n", 15);
														}
													else
														{
															go_step_f = stps_f;
															go_dir_f = 0;
															mem_perc_f=(-1)*percent_int_f;
											motor_F(1,go_step_f,go_dir_f);
											go_step_f = 0;
											HAL_Delay(1);
											sprintf(valuev1,"+FMOV: %d\n", mem_perc_f);
											CDC_Transmit_FS((uint8_t *)valuev1, strlen(valuev1));															
														}
												}
											else if(strncmp(usb_rx,"AT+FMOV=+", 9)==0)
												{
													if ((current_pos_f+stps_f)>all_steps_f)
														{
															HAL_Delay(1);
															CDC_Transmit_FS((uint8_t *)"+CME ERROR: 23\n", 15);
														}
													else
														{
															go_step_f = stps_f;
															go_dir_f = 1;
															mem_perc_f=percent_int_f;
											motor_F(1,go_step_f,go_dir_f);
											go_step_f = 0;
											HAL_Delay(1);
											sprintf(valuev1,"+FMOV: %d\n", mem_perc_f);
											CDC_Transmit_FS((uint8_t *)valuev1, strlen(valuev1));
														}
												}									
										}
								}		
								
						break;

					case 8:																																			//  +AMOV 
							HAL_Delay(1);
							CDC_Transmit_FS((uint8_t *)"OK\n", 3);
							percent_int_d = atoi(str+9);
								if (percent_int_d<1)
												{
													HAL_Delay(1);
													CDC_Transmit_FS((uint8_t *)"+CME ERROR: 3\n", 14);
												}
								else if (percent_int_d>99)
												{
													HAL_Delay(1);
													CDC_Transmit_FS((uint8_t *)"+CME ERROR: 3\n", 14);
												}
								else
								{
					
										if (INITD_flag==0)
												{
													HAL_Delay(1);
													CDC_Transmit_FS((uint8_t *)"+CME ERROR: 12\n", 15);
												}
		
										else
												{
													one_step_d = all_steps_d/100.0;          
													stps_d = ((float)percent_int_d * one_step_d);
																
											if(strncmp(usb_rx,"AT+AMOV=-", 9)==0) 
												{
													if ((current_pos_d-stps_d)<0)
														{
															HAL_Delay(1);
															CDC_Transmit_FS((uint8_t *)"+CME ERROR: 12\n", 15);
														}
													else
														{
															go_step_d = stps_d;
															go_dir_d = 0;
															mem_perc_d=(-1)*percent_int_d;
											motor_D(1,go_step_d,go_dir_d);
											go_step_d = 0;
											HAL_Delay(1);
											sprintf(valuev1,"+AMOV: %d\n", mem_perc_d);
											CDC_Transmit_FS((uint8_t *)valuev1, strlen(valuev1));															
														}
												}
											else if(strncmp(usb_rx,"AT+AMOV=+", 9)==0)
												{
													if ((current_pos_d+stps_d)>all_steps_d)
														{
															HAL_Delay(1);
															CDC_Transmit_FS((uint8_t *)"+CME ERROR: 12\n", 15);
														}
													else
														{
															go_step_d = stps_d;
															go_dir_d = 1;
															mem_perc_d=percent_int_d;
											motor_D(1,go_step_d,go_dir_d);
											go_step_d = 0;
											HAL_Delay(1);
											sprintf(valuev1,"+AMOV: %d\n", mem_perc_d);
											CDC_Transmit_FS((uint8_t *)valuev1, strlen(valuev1));
														}
												}									
										}
								}		
								
						break;		
					
					case 11:																																			//запрос версии прошивки      +CMGR?
								HAL_Delay(1);
								CDC_Transmit_FS((uint8_t *)"OK\n", 3);
								HAL_Delay(1);
								CDC_Transmit_FS((uint8_t *)VERSION_STRING, strlen(VERSION_STRING));

						break;
				
					case 12:																																			//запрос позиции diaph			  +APOS?
								HAL_Delay(1);						
								CDC_Transmit_FS((uint8_t *)"OK\n", 3);
									if (INITD_flag==0)
											{
												HAL_Delay(1);																	
												CDC_Transmit_FS((uint8_t *)"+CME ERROR: 11\n", 15);
											}	
									else
											{
												mem_d = current_pos_d / one_step_d;
												HAL_Delay(1);											
												sprintf(valuev1,"+APOS: %d\n", mem_d);
												CDC_Transmit_FS((uint8_t *)valuev1, strlen(valuev1));							
											}													

						break;
				
					case 13:																																			//запрос позиции focus					+FPOS?
								HAL_Delay(1);	
								CDC_Transmit_FS((uint8_t *)"OK\n", 3);
									if (INITF_flag==0)
											{
												HAL_Delay(1);																	
												CDC_Transmit_FS((uint8_t *)"+CME ERROR: 21\n", 15);												
											}	
				
									else
											{
												mem_f = current_pos_f / one_step_f;
												HAL_Delay(1);
												sprintf(valuev1,"+FPOS: %d\n", mem_f);
												CDC_Transmit_FS((uint8_t *)valuev1, strlen(valuev1));							
											}									

						break;
															
					case 14:																																			//  dxxx / fxxx
								HAL_Delay(1);	
								CDC_Transmit_FS((uint8_t *)"OK\n", 3);					
					if(strncmp(usb_rx,"AT+FPOS=", 8)==0)
					{
							percent_int_f = atoi(str+8);
							zero_check = atoi(str+9);
						
							if (strncmp(usb_rx,"AT+FPOS=0", 9)==0)
								{
									if (zero_check=='\0')
									{	
											if (INITF_flag==0)
													{
														HAL_Delay(1);
														CDC_Transmit_FS((uint8_t *)"+CME ERROR: 23\n", 15);
													}
				
											else
											{									
													percent_int_f=0;
													mem_perc_f=percent_int_f;
													one_step_f = all_steps_f/100.0;          
													stps_f = ((float)percent_int_f * one_step_f);
																		
													if (current_pos_f > stps_f) 
														{
															go_step_f = abs(current_pos_f-stps_f);
															go_dir_f = 0;
														}
													else if (current_pos_f < stps_f) 
														{
															go_step_f = stps_f-current_pos_f;
															go_dir_f = 1;
														} 							
														motor_F(1,go_step_f,go_dir_f);
														go_step_f = 0;
														HAL_Delay(1);
														sprintf(valuev1,"+FPOS: %d\n", percent_int_f);
														CDC_Transmit_FS((uint8_t *)valuev1, strlen(valuev1));					
											}
								}
								else
								{
									HAL_Delay(1);
									CDC_Transmit_FS((uint8_t *)"+CME ERROR: 3\n", 14);	
								}
							}
							else if (percent_int_f<1)
								{
									HAL_Delay(1);
									CDC_Transmit_FS((uint8_t *)"+CME ERROR: 3\n", 14);
								}
							else if (percent_int_f>100)
								{
									HAL_Delay(1);
									CDC_Transmit_FS((uint8_t *)"+CME ERROR: 3\n", 14);
								}
							else
								{
									if (INITF_flag==0)
											{
												HAL_Delay(1);
												CDC_Transmit_FS((uint8_t *)"+CME ERROR: 23\n", 15);
											}
		
									else
									{
											mem_perc_f=percent_int_f;										
											one_step_f = all_steps_f/100.0;          
											stps_f = ((float)percent_int_f * one_step_f);
																
											if (current_pos_f > stps_f) 
												{
													go_step_f = abs(current_pos_f-stps_f);
													go_dir_f = 0;
												}
											else if (current_pos_f < stps_f) 
												{
													go_step_f = stps_f-current_pos_f;
													go_dir_f = 1;
												} 
												motor_F(1,go_step_f,go_dir_f);
												go_step_f = 0;
												HAL_Delay(1);
												sprintf(valuev1,"+FPOS: %d\n", percent_int_f);
												CDC_Transmit_FS((uint8_t *)valuev1, strlen(valuev1));
									}
								}
					}
						
					else if(strncmp(usb_rx,"AT+APOS=", 8)==0)
					{						
							percent_int_d = atoi(str+8);
							zero_check = atoi(str+9);						
	
							if (strncmp(usb_rx,"AT+APOS=0", 9)==0)
								{
									if (zero_check=='\0')
									{
												
											if (INITD_flag==0)
													{
														HAL_Delay(1);
														CDC_Transmit_FS((uint8_t *)"+CME ERROR: 12\n", 15);
													}	
				
											else
											{									
													percent_int_d=0;
													mem_perc_d=percent_int_d;
													one_step_d = all_steps_d/100.0;          
													stps_d = ((float)percent_int_d * one_step_d);
																		
													if (current_pos_d > stps_d) 
														{
															go_step_d = abs(current_pos_d-stps_d);
															go_dir_d = 0;
														}
													else if (current_pos_d < stps_d) 
														{
															go_step_d = stps_d-current_pos_d;
															go_dir_d = 1;
														} 
														motor_D(1,go_step_d,go_dir_d);
														go_step_d = 0;
														HAL_Delay(1);
														sprintf(valuev1,"+APOS: %d\n", percent_int_d);
														CDC_Transmit_FS((uint8_t *)valuev1, strlen(valuev1));	
											}
								}
								else
								{
									HAL_Delay(1);
									CDC_Transmit_FS((uint8_t *)"+CME ERROR: 3\n", 14);	
								}
							}
						  else if (percent_int_d<1)
								{
									HAL_Delay(1);
									CDC_Transmit_FS((uint8_t *)"+CME ERROR: 3\n", 14);
								}
							else if (percent_int_d>100)
								{
									HAL_Delay(1);
									CDC_Transmit_FS((uint8_t *)"+CME ERROR: 3\n", 14);
								}										
							else
								{
									
									if (INITD_flag==0)
											{
												HAL_Delay(1);
												CDC_Transmit_FS((uint8_t *)"+CME ERROR: 12\n", 15);
											}	
		
									else
									{
											mem_perc_d=percent_int_d;										
											one_step_d = all_steps_d/100.0;          
											stps_d = ((float)percent_int_d * one_step_d);
																
											if (current_pos_d > stps_d) 
												{
													go_step_d = abs(current_pos_d-stps_d);
													go_dir_d = 0;
												}
											else if (current_pos_d < stps_d) 
												{
													go_step_d = stps_d-current_pos_d;
													go_dir_d = 1;
												} 
												motor_D(1,go_step_d,go_dir_d);
												go_step_d = 0;
												HAL_Delay(1);
												sprintf(valuev1,"+APOS: %d\n", percent_int_d);
												CDC_Transmit_FS((uint8_t *)valuev1, strlen(valuev1));
									}
								}
						}
						
					else
							{
									HAL_Delay(1);
									CDC_Transmit_FS((uint8_t *)"+CME ERROR: 3\n", 14);
							}			
						break;
		
					case 0:
									HAL_Delay(1);
									CDC_Transmit_FS((uint8_t *)"+CME ERROR: 3\n", 14);
						break;													
		}
											percent_int_d=0;
											one_step_d = all_steps_d/100.0;          
											mem_d = current_pos_d / one_step_d;
		
											percent_int_f=0;
											one_step_f = all_steps_f/100.0;
											mem_f = current_pos_f / one_step_f;		
													
		
		
					memset(uart1_rx_buf, '\0', strlen(uart1_rx_buf)); // очистка памяти
					uart1_rx_bit=0;                                // очистка счётчика			
					memset(usb_rx, 0, sizeof(usb_rx));
					memset(usb_rx, '\0', sizeof(usb_rx));
					stop_motor_F();
					stop_motor_D();
		}
	
}
void USART1_IRQHandler (void) 												// прерывание
{
   if (USART1->SR & USART_SR_RXNE) //проверяем пришло ли что нибудь в УАРТ
			{
				uart1_data=USART1->DR; // считываем то что пришло в переменную
				uart1_rx_buf[uart1_rx_bit]=USART1->DR; // помещаем принятый байт в буфер
				uart1_rx_bit++;  // наращиваем счётчик байтов буфера
			}
}
void EXTI_Init (void)
{
  EXTI->PR = EXTI_PR_PR1;      //Сбрасываем флаг прерывания 
  NVIC_EnableIRQ(EXTI1_IRQn);
	EXTI->PR = EXTI_PR_PR0;      //Сбрасываем флаг прерывания 
  NVIC_EnableIRQ(EXTI0_IRQn);
	EXTI->PR = EXTI_PR_PR12;      //Сбрасываем флаг прерывания 
  NVIC_EnableIRQ(EXTI15_10_IRQn);
}
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
	MX_USB_DEVICE_Init();
  MX_ADC1_Init();
  MX_USART3_UART_Init();
  MX_TIM3_Init();
    /* USER CODE BEGIN 2 */
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
	HAL_GPIO_EXTI_IRQHandler(STMPS_FT);
	
	HAL_GPIO_WritePin(STMPS_PORT, STMPS_EN, GPIO_PIN_RESET);
	
  HAL_GPIO_WritePin(PORT_M_1,EN_MOT_1_1,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC,EN_MOT_1_2,GPIO_PIN_SET);
	
	HAL_GPIO_WritePin(GPIOA,EN_MOT_2_1,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB,EN_MOT_2_2,GPIO_PIN_SET);
	
	HAL_GPIO_WritePin(STMPS_PORT,STMPS_EN,GPIO_PIN_RESET);

			uint8_t reset = 0;
			HAL_UART_Receive_IT(&huart1, &reset, 1);			
			stop_motor_F();
			stop_motor_D();
			EXTI_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
 
  while (1)
  {
		terminal();	
	}	
		
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
    
  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_USB;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV4;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* ADC1 init function */
static void MX_ADC1_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Common config 
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = ENABLE;
  hadc1.Init.NbrOfDiscConversion = 1;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 3;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM3 init function */
static void MX_TIM3_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 399;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 5000;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 100;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim3);

}

/* USART1 init function */
static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* USART3 init function */
static void MX_USART3_UART_Init(void)
{

  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
	
	// reset USB DP (D+)
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET); 
  for(uint16_t i = 0; i < 10000; i++) {}; 

  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  for(uint16_t i = 0; i < 10000; i++) {}; 

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_10 
                          |GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14 
                          |GPIO_PIN_15|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6 
                          |GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC13 PC4 PC5 PC10 
                           PC11 PC12 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_10 
                          |GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PC0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	  /*Configure GPIO pin : PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
		  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA3 PA7 PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_7|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB14 
                           PB15 PB4 PB5 PB6 
                           PB7 PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_14 
                          |GPIO_PIN_15|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6 
                          |GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	  /*Configure GPIO pin : PB13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	/*Configure GPIO pin : PB12 */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PD2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);
	HAL_NVIC_SetPriority(EXTI0_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
