/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define d0 GPIO_PIN_0
#define d1 GPIO_PIN_1
#define d2 GPIO_PIN_2
#define d3 GPIO_PIN_3
#define d4 GPIO_PIN_4
#define d5 GPIO_PIN_5
#define d6 GPIO_PIN_6
#define d7 GPIO_PIN_7

#define E  GPIO_PIN_8
#define RW GPIO_PIN_9
#define RS GPIO_PIN_10

#define keypad_c1 GPIO_PIN_0
#define keypad_c2 GPIO_PIN_1
#define keypad_c3 GPIO_PIN_2
#define keypad_r1 GPIO_PIN_3
#define keypad_r2 GPIO_PIN_4
#define keypad_r3 GPIO_PIN_5
#define keypad_r4 GPIO_PIN_6


#define a GPIO_PIN_0
#define b GPIO_PIN_1
#define c GPIO_PIN_2
#define d GPIO_PIN_10
#define e GPIO_PIN_12
#define f GPIO_PIN_13
#define g GPIO_PIN_14


#define LED_y GPIO_PIN_0
#define LED_r GPIO_PIN_1
#define LED_b GPIO_PIN_2
#define LED_g GPIO_PIN_3


#define LED_button_y GPIO_PIN_4
#define LED_button_r GPIO_PIN_5
#define LED_button_b GPIO_PIN_6
#define LED_button_g GPIO_PIN_7





//MASKS:
#define SET1(x) (1ul << (x))
#define SET0(x) (~(SET1(x)))
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
 TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;

/* USER CODE BEGIN PV */
static volatile char array[16];
static volatile int size = 0;
static volatile int status = 0; // 0 => starting(waiting for the student ID)    1 => showing name of the student ID owner(waiting for reset)
//2 => init Game		3 => start of the game			4 => game is finished
static volatile int mistakeNumber = 0;
static volatile uint32_t current = -1;
static volatile uint32_t pre = -1;
static volatile int counter = 0;
static volatile int numberOfReset = 0;
static volatile int time = 60;
static volatile int isWinnerLoserShown = 0;
static volatile int onLED = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM5_Init(void);
/* USER CODE BEGIN PFP */
void MyInitialize(void);
void LCD_put_char(char data);
void HAL_Delay(uint32_t Delay);
void LCD_init(void);
void LCD_command(unsigned char command);
void LCD_resetCommand(void);
void LCD_setCommand(void);
char findKeypadButton_column1(void);
char findKeypadButton_column2(void);
char findKeypadButton_column3(void);
void init_keypadInput(void);
void handle(void);
void initGame(void);
void LCD_clear(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/*
 ****************** Editing HAL_Delay Function Because It Added The Delay Time **************
 */
 void HAL_Delay(uint32_t Delay){ // It is same as the function in stm32f4xx_hal.c File, Line 390, But we commented the part which 
//was adding delay time
  uint32_t tickstart = HAL_GetTick();
  uint32_t wait = Delay;

  ///* Add a freq to guarantee minimum wait */
  //if (wait < HAL_MAX_DELAY)
  //{
  //  wait += (uint32_t)(uwTickFreq);
  //}

  while((HAL_GetTick() - tickstart) < wait)
  {
  }
}
/*
 ******************************************* My Functions ******************************************
 */
void MyInitialize(void){
	//MX_TIM2_Init();
	//MX_TIM3_Init();
	//HAL_NVIC_DisableIRQ(TIM2_IRQn);
	HAL_NVIC_DisableIRQ(TIM3_IRQn); // TIM3 is sed for making intrrupt each second for game timer
	HAL_NVIC_DisableIRQ(TIM5_IRQn); // TIM5 is used for showing wiiner or loser
	init_keypadInput();
	
	for(int i = 0; i < 16; i++)
		array[i] = ' ';
	
	LCD_init();
	LCD_setCommand();
}

void LCD_put_char(char data) {
	HAL_GPIO_WritePin(GPIOA, d0, SET1(0) & data);
	HAL_GPIO_WritePin(GPIOA, d1, SET1(1) & data);
	HAL_GPIO_WritePin(GPIOA, d2, SET1(2) & data);
	HAL_GPIO_WritePin(GPIOA, d3, SET1(3) & data);
	HAL_GPIO_WritePin(GPIOA, d4, SET1(4) & data);
	HAL_GPIO_WritePin(GPIOA, d5, SET1(5) & data);
	HAL_GPIO_WritePin(GPIOA, d6, SET1(6) & data);
	HAL_GPIO_WritePin(GPIOA, d7, SET1(7) & data);
	HAL_GPIO_WritePin(GPIOA, RS, 1); // RS
	HAL_GPIO_WritePin(GPIOA, RW, 0); // RW
	HAL_GPIO_WritePin(GPIOA, E, 1); // E
	HAL_GPIO_WritePin(GPIOA, E, 0); // E
	// making the least possible delay for writing characters correctly in the LCD
	for(int i = 0; i < 17 ; i++)
		HAL_Delay(0);
}

void LCD_init(void){
	LCD_command(0x38);
	LCD_command(0x06);
	LCD_command(0x08);
	LCD_command(0x0C);
}

void LCD_command(unsigned char command){
	HAL_GPIO_WritePin(GPIOA, d0, SET1(0) & command);
	HAL_GPIO_WritePin(GPIOA, d1, SET1(1) & command);
	HAL_GPIO_WritePin(GPIOA, d2, SET1(2) & command);
	HAL_GPIO_WritePin(GPIOA, d3, SET1(3) & command);
	HAL_GPIO_WritePin(GPIOA, d4, SET1(4) & command);
	HAL_GPIO_WritePin(GPIOA, d5, SET1(5) & command);
	HAL_GPIO_WritePin(GPIOA, d6, SET1(6) & command);
	HAL_GPIO_WritePin(GPIOA, d7, SET1(7) & command);
	HAL_GPIO_WritePin(GPIOA, RS, 0); // RS
	HAL_GPIO_WritePin(GPIOA, RW, 0); // RW
	HAL_GPIO_WritePin(GPIOA, E, 1); // E
	HAL_GPIO_WritePin(GPIOA, E, 0); // E
	
  if (command < 4)
      HAL_Delay(2);           /* command 1 and 2 needs up to 1.64ms */
  else
      HAL_Delay(1);           /* all others 40 us */
}
void LCD_setCommand(void){ 
	LCD_command(0x01);
	LCD_command(0x38);
}

void LCD_resetCommand(void){
	for(int i = 0; i < 16; i++)
		LCD_put_char(array[i]);
}

void LCD_clear(void){
	LCD_command(0x80);
	for(int i = 0; i < 16; i++)
		LCD_put_char(' ');
	LCD_command(0xC0);
	for(int i = 0; i < 16; i++)
		LCD_put_char(' ');
	LCD_command(0x80);
}

void set_7Segment(int number){
	switch(number){
		 case -1 : {
			HAL_GPIO_WritePin(GPIOC, a, 0);
			HAL_GPIO_WritePin(GPIOC, b, 0);
			HAL_GPIO_WritePin(GPIOC, c, 0);
			HAL_GPIO_WritePin(GPIOB, d, 0);
			HAL_GPIO_WritePin(GPIOB, e, 0);
			HAL_GPIO_WritePin(GPIOB, f, 0);
			HAL_GPIO_WritePin(GPIOB, g, 0);
		 } break;
		 case 0 :{
			HAL_GPIO_WritePin(GPIOC, a, 1);
			HAL_GPIO_WritePin(GPIOC, b, 1);
			HAL_GPIO_WritePin(GPIOC, c, 1);
			HAL_GPIO_WritePin(GPIOB, d, 1);
			HAL_GPIO_WritePin(GPIOB, e, 1);
			HAL_GPIO_WritePin(GPIOB, f, 1);
			HAL_GPIO_WritePin(GPIOB, g, 0);
		 } break;
		 case 1 : {
			HAL_GPIO_WritePin(GPIOC, a, 0);
			HAL_GPIO_WritePin(GPIOC, b, 1);
			HAL_GPIO_WritePin(GPIOC, c, 1);
			HAL_GPIO_WritePin(GPIOB, d, 0);
			HAL_GPIO_WritePin(GPIOB, e, 0);
			HAL_GPIO_WritePin(GPIOB, f, 0);
			HAL_GPIO_WritePin(GPIOB, g, 0);
		 } break;
		 case 2 :{
			HAL_GPIO_WritePin(GPIOC, a, 1);
			HAL_GPIO_WritePin(GPIOC, b, 1);
			HAL_GPIO_WritePin(GPIOC, c, 0);
			HAL_GPIO_WritePin(GPIOB, d, 1);
			HAL_GPIO_WritePin(GPIOB, e, 1);
			HAL_GPIO_WritePin(GPIOB, f, 0);
			HAL_GPIO_WritePin(GPIOB, g, 1);
		 } break;
		 case 3 :{
			HAL_GPIO_WritePin(GPIOC, a, 1);
			HAL_GPIO_WritePin(GPIOC, b, 1);
			HAL_GPIO_WritePin(GPIOC, c, 1);
			HAL_GPIO_WritePin(GPIOB, d, 1);
			HAL_GPIO_WritePin(GPIOB, e, 0);
			HAL_GPIO_WritePin(GPIOB, f, 0);
			HAL_GPIO_WritePin(GPIOB, g, 1);
		 } break;
		 case 4 : {
			HAL_GPIO_WritePin(GPIOC, a, 0);
			HAL_GPIO_WritePin(GPIOC, b, 1);
			HAL_GPIO_WritePin(GPIOC, c, 1);
			HAL_GPIO_WritePin(GPIOB, d, 0);
			HAL_GPIO_WritePin(GPIOB, e, 0);
			HAL_GPIO_WritePin(GPIOB, f, 1);
			HAL_GPIO_WritePin(GPIOB, g, 1);
		 } break;
		 case 5 : {
			HAL_GPIO_WritePin(GPIOC, a, 1);
			HAL_GPIO_WritePin(GPIOC, b, 0);
			HAL_GPIO_WritePin(GPIOC, c, 1);
			HAL_GPIO_WritePin(GPIOB, d, 1);
			HAL_GPIO_WritePin(GPIOB, e, 0);
			HAL_GPIO_WritePin(GPIOB, f, 1);
			HAL_GPIO_WritePin(GPIOB, g, 1);
		 } break;
		 case 6 : {
			HAL_GPIO_WritePin(GPIOC, a, 1);
			HAL_GPIO_WritePin(GPIOC, b, 0);
			HAL_GPIO_WritePin(GPIOC, c, 1);
			HAL_GPIO_WritePin(GPIOB, d, 1);
			HAL_GPIO_WritePin(GPIOB, e, 1);
			HAL_GPIO_WritePin(GPIOB, f, 1);
			HAL_GPIO_WritePin(GPIOB, g, 1);
		 } break;
		 case 7 : {
			HAL_GPIO_WritePin(GPIOC, a, 1);
			HAL_GPIO_WritePin(GPIOC, b, 1);
			HAL_GPIO_WritePin(GPIOC, c, 1);
			HAL_GPIO_WritePin(GPIOB, d, 0);
			HAL_GPIO_WritePin(GPIOB, e, 0);
			HAL_GPIO_WritePin(GPIOB, f, 0);
			HAL_GPIO_WritePin(GPIOB, g, 0);
		 } break;
		 case 8 : {
			HAL_GPIO_WritePin(GPIOC, a, 1);
			HAL_GPIO_WritePin(GPIOC, b, 1);
			HAL_GPIO_WritePin(GPIOC, c, 1);
			HAL_GPIO_WritePin(GPIOB, d, 1);
			HAL_GPIO_WritePin(GPIOB, e, 1);
			HAL_GPIO_WritePin(GPIOB, f, 1);
			HAL_GPIO_WritePin(GPIOB, g, 1);
		 } break;
		 case 9 : {
			HAL_GPIO_WritePin(GPIOC, a, 1);
			HAL_GPIO_WritePin(GPIOC, b, 1);
			HAL_GPIO_WritePin(GPIOC, c, 1);
			HAL_GPIO_WritePin(GPIOB, d, 1);
			HAL_GPIO_WritePin(GPIOB, e, 0);
			HAL_GPIO_WritePin(GPIOB, f, 1);
			HAL_GPIO_WritePin(GPIOB, g, 1);
		 } break;
	}
}

void init_keypadInput(void){
	HAL_GPIO_WritePin(GPIOB, keypad_r1, 1);
	HAL_GPIO_WritePin(GPIOB, keypad_r2, 1);
	HAL_GPIO_WritePin(GPIOB, keypad_r3, 1);
	HAL_GPIO_WritePin(GPIOB, keypad_r4, 1);
}

char getChar(int digit){
	switch(digit){
		case 0 : return '0';
		case 1 : return '1';
		case 2 : return '2';
		case 3 : return '3';
		case 4 : return '4';
		case 5 : return '5';
		case 6 : return '6';
		case 7 : return '7';
		case 8 : return '8';
		case 9 : return '9';
	}
	return '&';
}

void LCD_writeNumber(int input){
	if (input < 0){
		input = -input;
		LCD_put_char('M');
	}
	if(input == 0){
		LCD_put_char('0');
		return;
	}
	char arr[10];
	int len = 0;
	while(input > 0){
		arr[len++]=getChar(input % 10);
		input /= 10;
	}
	while(len >= 0)
		LCD_put_char(arr[--len]);
}

char findKeypadButton_column1(void){
	HAL_GPIO_WritePin(GPIOB, keypad_r1, 0);
	if(HAL_GPIO_ReadPin(GPIOB, keypad_c1) == 0){
		init_keypadInput();
		return '1';
	}
	HAL_GPIO_WritePin(GPIOB, keypad_r2, 0);
	if(HAL_GPIO_ReadPin(GPIOB, keypad_c1) == 0){
		init_keypadInput();
		return '4';
	}
	HAL_GPIO_WritePin(GPIOB, keypad_r3, 0);
	if(HAL_GPIO_ReadPin(GPIOB, keypad_c1) == 0){
		init_keypadInput();
		return '7';
	}
	HAL_GPIO_WritePin(GPIOB, keypad_r4, 0);
	if(HAL_GPIO_ReadPin(GPIOB, keypad_c1) == 0){
		init_keypadInput();
		return '*';
	}
}
char findKeypadButton_column2(void){
	HAL_GPIO_WritePin(GPIOB, keypad_r1, 0);
	if(HAL_GPIO_ReadPin(GPIOB, keypad_c2) == 0){
		init_keypadInput();
		return '2';
	}
	HAL_GPIO_WritePin(GPIOB, keypad_r2, 0);
	if(HAL_GPIO_ReadPin(GPIOB, keypad_c2) == 0){
		init_keypadInput();
		return '5';
	}
	HAL_GPIO_WritePin(GPIOB, keypad_r3, 0);
	if(HAL_GPIO_ReadPin(GPIOB, keypad_c2) == 0){
		init_keypadInput();
		return '8';
	}
	HAL_GPIO_WritePin(GPIOB, keypad_r4, 0);
	if(HAL_GPIO_ReadPin(GPIOB, keypad_c2) == 0){
		init_keypadInput();
		return '0';
	}
}
char findKeypadButton_column3(void){
	HAL_GPIO_WritePin(GPIOB, keypad_r1, 0);
	if(HAL_GPIO_ReadPin(GPIOB, keypad_c3) == 0){
		init_keypadInput();
		return '3';
	}
	HAL_GPIO_WritePin(GPIOB, keypad_r2, 0);
	if(HAL_GPIO_ReadPin(GPIOB, keypad_c3) == 0){
		init_keypadInput();
		return '6';
	}
	HAL_GPIO_WritePin(GPIOB, keypad_r3, 0);
	if(HAL_GPIO_ReadPin(GPIOB, keypad_c3) == 0){
		init_keypadInput();
		return '9';
	}
	HAL_GPIO_WritePin(GPIOB, keypad_r4, 0);
	if(HAL_GPIO_ReadPin(GPIOB, keypad_c3) == 0){
		init_keypadInput();
		return '#';
	}
}

void checkStudentID(void){
	if(size == 8){
		if(array[0] == '9' && array[1] == '8'  && array[2] == '2' && array[3] == '4' && array[4] == '3' && array[5] == '0' && array[6] == '3' && array[7] == '5'){
			LCD_command(0xC0);
			array[0] = 'F';
			array[1] = 'a';
			array[2] = 't';
			array[3] = 'e';
			array[4] = 'm';
			array[5] = 'e';
			array[6] = ' ';
			array[7] = 'S';
			array[8] = 'e';
			array[9] = 'i';
			array[10] = 'f';
			array[11] = 'i';
			array[12] = ' ';
			array[13] = ' ';
			array[14] = ' ';
			array[15] = ' ';
		  status = 1;
		  LCD_resetCommand();
		}else if(array[0] == '9' && array[1] == '8'  && array[2] == '2' && array[3] == '4' && array[4] == '3' && array[5] == '0' && array[6] == '5' && array[7] == '0'){
			LCD_command(0xC0);
			array[0] = 'P';
			array[1] = 'a';
			array[2] = 'r';
			array[3] = 's';
			array[4] = 'a';
			array[5] = ' ';
			array[6] = 'M';
			array[7] = 'o';
			array[8] = 'h';
			array[9] = 'a';
			array[10] = 'm';
			array[11] = 'a';
			array[12] = 'd';
			array[13] = 'p';
			array[14] = 'o';
			array[15] = 'r';
		  status = 1;
			LCD_resetCommand();
		}
	}
}

void handle(void){
	HAL_Delay(120);
	LCD_command(0xC0);
	for(int i = 0; i < 16; i++)
		LCD_put_char(' ');
	HAL_Delay(120);
	LCD_command(0xC0);
	LCD_resetCommand();
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin & SET1(0)){
		char result = findKeypadButton_column1();
		HAL_NVIC_ClearPendingIRQ(EXTI0_IRQn); // to avoid making an infinite loop that write result in LCD
		if(status > 1){// after pushing reset twice and the game was inited for the first time
			if(result != '*')
				return;
			status = 2;
			return;
		}
		array[size++] = result;
		LCD_put_char(result);
		checkStudentID();
	} else if(GPIO_Pin & SET1(1)){ 
		if(status != 0)
			return;
		char result = findKeypadButton_column2();
		HAL_NVIC_ClearPendingIRQ(EXTI1_IRQn); // to avoid making an infinite loop that write result in LCD
		array[size++] = result;
		LCD_put_char(result);
		checkStudentID();
	} else if(GPIO_Pin & SET1(2)){
		if(status != 0)
			return;
		char result = findKeypadButton_column3();
		HAL_NVIC_ClearPendingIRQ(EXTI2_IRQn); // to avoid making an infinite loop that write result in LCD
		if(result == '#'){
			size = 0;
			for(int i = 0; i < 16 ; i++)
				array[i] = ' ';
			LCD_clear();
			return;
		}
		HAL_NVIC_ClearPendingIRQ(EXTI2_IRQn); // to avoid making an infinite loop that write result in LCD
		array[size++] = result;
		LCD_put_char(result);
		checkStudentID();
	} else if (GPIO_Pin & SET1(3)){
		
	} else if (GPIO_Pin & SET1(4)){
		
	}else  if (GPIO_Pin & SET1(10)){
		if(status == 1 /*|| status == 0 || status == 2*/){
			//set_7Segment(counter);
			if(counter < 2){
				//LCD_command(0xC0);
				//LCD_put_char('/');
				numberOfReset++;
				current = TIM2 -> CCR1;
				if(numberOfReset % 2 == 0){
					if(pre != -1 && (current - pre) >= 75){ // in the real time it should be 2000
						counter++;
						//LCD_put_char('!');
						if(counter == 2){
							status = 2;
							//LCD_put_char('=');
						}
					}
				}
				//LCD_writeNumber(current);
				//LCD_put_char('=');
				//LCD_writeNumber(pre);
				pre = current;
			}
		}else if(status == 3 || status == 4 || status == 2){
			numberOfReset++;
				current = TIM2 -> CCR1;
				if(numberOfReset % 2 == 0){
					if(pre != -1 && (current - pre) >= 105){ // in the real time it should be 3000
						status = 2; // we can't call initGame function due to the priority we set for them(the priority of thin method with this call, is higher than
						//SysTick. so it can't write any thing or make any delay)
						return;
					}
				}
				pre = current;
		}
	}
}

void initGame(void){
	HAL_NVIC_DisableIRQ(TIM5_IRQn);
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);
	HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_3);
	mistakeNumber = 0;
	set_7Segment(mistakeNumber);
	LCD_clear();
	time = 60;
	LCD_writeNumber(time);
	status = 3;
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
}

void endGame(void){
	status = 4;
	HAL_NVIC_DisableIRQ(TIM3_IRQn);
	if(mistakeNumber < 3){
		LCD_command(0xC0);
		array[0] = 'W';
		array[1] = 'I';
		array[2] = 'N';
		array[3] = 'N';
		array[4] = 'E';
		array[5] = 'R';
		array[6] = ' ';
		array[7] = ' ';
		array[8] = ' ';
		array[9] = ' ';
		array[10] = ' ';
		array[11] = ' ';
		array[12] = ' ';
		array[13] = ' ';
		array[14] = ' ';
		array[15] = ' ';
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
	}else {
		LCD_command(0xC0);
		array[0] = 'L';
		array[1] = 'O';
		array[2] = 'S';
		array[3] = 'E';
		array[4] = 'R';
		array[5] = ' ';
		array[6] = ' ';
		array[7] = ' ';
		array[8] = ' ';
		array[9] = ' ';
		array[10] = ' ';
		array[11] = ' ';
		array[12] = ' ';
		array[13] = ' ';
		array[14] = ' ';
		array[15] = ' ';
		HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
	}
	HAL_NVIC_EnableIRQ(TIM5_IRQn);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim -> Instance == TIM11){
		
	}else if(htim -> Instance == TIM2){
		
	}else if(htim -> Instance == TIM3){
		if(status == 3){
			if(time  < 0){
				endGame();
				return;
			}
			LCD_command(0x80);
			LCD_put_char(' ');
			LCD_put_char(' ');
			LCD_command(0x80);
			LCD_writeNumber(time--);
		}
	}else if(htim -> Instance == TIM5){
		if(status == 4){
			if(isWinnerLoserShown == 0){
				LCD_command(0xC0);
				LCD_resetCommand();
				isWinnerLoserShown = 1;
			}else{
				LCD_command(0xC0);
				for(int i = 0; i < 16; i++)
					LCD_put_char(' ');
				LCD_command(0xC0);
				isWinnerLoserShown = 0;
			}
		}
	}
}

void LED_RED(int state){
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, state); // red LED is on or off
}

void LED_GREEN(int state){
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, state); // green LED is on or off
}

void LED_YELLOW(int state){
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, state); // yellow LED is on or off
}

void LED_BLUE(int state){
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, state); // blue LED is on or off
}

void game(void){
	volatile int numb = 0;
	
	/******************************************************* 3 seconds ********************************************/
	for(int i = 0; i < 3; i++){
		LED_BLUE(0);
		LED_GREEN(0);
		LED_RED(0);
		LED_YELLOW(0);
		if(numb % 4 == 0)
			LED_BLUE(1);
		else if(numb % 4 == 1)
			LED_GREEN(1);
		else if(numb % 4 == 2)
			LED_RED(1);
		else if(numb % 4 == 3)
			LED_YELLOW(1);
				
		uint32_t tickstart = HAL_GetTick();
		uint32_t wait = 150;
	
		/* Add a freq to guarantee minimum wait */
		if (wait < HAL_MAX_DELAY)
		{
			wait += (uint32_t)(uwTickFreq);
		}
	
		volatile int hasPressed = 0;
		
		while((HAL_GetTick() - tickstart) < wait)
		{
			if(status != 3){
				LED_BLUE(0);
				LED_GREEN(0);
				LED_RED(0);
				LED_YELLOW(0);
				return;
			}
			if(time <= 0){
				status = 4;
				endGame();
				LED_BLUE(0);
				LED_GREEN(0);
				LED_RED(0);
				LED_YELLOW(0);
				HAL_NVIC_DisableIRQ(TIM3_IRQn);
				return;
			}
			if(HAL_GPIO_ReadPin(GPIOC, LED_button_y) == 1){
				if(numb % 4 != 3){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_y) == 1)
					;
			}else if(HAL_GPIO_ReadPin(GPIOC, LED_button_b) == 1){
				if(numb % 4 != 0){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_b) == 1)
					;
			}else if(HAL_GPIO_ReadPin(GPIOC, LED_button_g) == 1){
				if(numb % 4 != 1){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_g) == 1)
					;
			} else if(HAL_GPIO_ReadPin(GPIOC, LED_button_r) == 1){
				if(numb % 4 != 2){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_r) == 1)
					;
			}
		}
		numb++;
		if(hasPressed == 0){
			mistakeNumber++;
		}
		set_7Segment(mistakeNumber);
		if(mistakeNumber >= 3){
			status = 4;
			LED_BLUE(0);
			LED_GREEN(0);
			LED_RED(0);
			LED_YELLOW(0);
			HAL_NVIC_DisableIRQ(TIM3_IRQn);
			endGame();
			return;
		}
	}
	
	/****************************************************** 2 seconds ***************************************************/
	for(int i = 0; i < 3; i++){
		LED_BLUE(0);
		LED_GREEN(0);
		LED_RED(0);
		LED_YELLOW(0);;
		if(numb % 4 == 0)
			LED_BLUE(1);
		else if(numb % 4 == 1)
			LED_GREEN(1);
		else if(numb % 4 == 2)
			LED_RED(1);
		else if(numb % 4 == 3)
			LED_YELLOW(1);
				
		uint32_t tickstart = HAL_GetTick();
		uint32_t wait = 100;
	
		/* Add a freq to guarantee minimum wait */
		if (wait < HAL_MAX_DELAY)
		{
			wait += (uint32_t)(uwTickFreq);
		}
	
		int hasPressed = 0;
		
		while((HAL_GetTick() - tickstart) < wait)
		{
			if(status != 3){
				LED_BLUE(0);
				LED_GREEN(0);
				LED_RED(0);
				LED_YELLOW(0);
				return;
			}
			if(time <= 0){
				status = 4;
				endGame();
				LED_BLUE(0);
				LED_GREEN(0);
				LED_RED(0);
				LED_YELLOW(0);
				HAL_NVIC_DisableIRQ(TIM3_IRQn);
				return;
			}
			if(HAL_GPIO_ReadPin(GPIOC, LED_button_y) == 1){
				if(numb % 4 != 3){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_y) == 1)
					;
			}else if(HAL_GPIO_ReadPin(GPIOC, LED_button_b) == 1){
				if(numb % 4 != 0){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_b) == 1)
					;
			}else if(HAL_GPIO_ReadPin(GPIOC, LED_button_g) == 1){
				if(numb % 4 != 1){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_g) == 1)
					;
			} else if(HAL_GPIO_ReadPin(GPIOC, LED_button_r) == 1){
				if(numb % 4 != 2){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_r) == 1)
					;
			}
		}
		numb++;
		if(hasPressed == 0){
			mistakeNumber++;
		}
		set_7Segment(mistakeNumber);
		if(mistakeNumber >= 3){
			status = 4;
			LED_BLUE(0);
			LED_GREEN(0);
			LED_RED(0);
			LED_YELLOW(0);
			HAL_NVIC_DisableIRQ(TIM3_IRQn);
			endGame();
			return;
		}
	}
	
	
	/******************************************************* 1 second ***************************************************/
	for(int i = 0; i < 10; i++){
		LED_BLUE(0);
		LED_GREEN(0);
		LED_RED(0);
		LED_YELLOW(0);
		if(numb % 4 == 0)
			LED_BLUE(1);
		else if(numb % 4 == 1)
			LED_GREEN(1);
		else if(numb % 4 == 2)
			LED_RED(1);
		else if(numb % 4 == 3)
			LED_YELLOW(1);
				
		uint32_t tickstart = HAL_GetTick();
		uint32_t wait = 95;
	
		/* Add a freq to guarantee minimum wait */
		if (wait < HAL_MAX_DELAY)
		{
			wait += (uint32_t)(uwTickFreq);
		}
	
		int hasPressed = 0;
		
		while((HAL_GetTick() - tickstart) < wait)
		{
			if(status != 3){
				LED_BLUE(0);
				LED_GREEN(0);
				LED_RED(0);
				LED_YELLOW(0);
				return;
			}
			if(time <= 0){
				status = 4;
				endGame();
				LED_BLUE(0);
				LED_GREEN(0);
				LED_RED(0);
				LED_YELLOW(0);
				HAL_NVIC_DisableIRQ(TIM3_IRQn);
				return;
			}
			if(HAL_GPIO_ReadPin(GPIOC, LED_button_y) == 1){
				if(numb % 4 != 3){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_y) == 1)
					;
			}else if(HAL_GPIO_ReadPin(GPIOC, LED_button_b) == 1){
				if(numb % 4 != 0){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_b) == 1)
					;
			}else if(HAL_GPIO_ReadPin(GPIOC, LED_button_g) == 1){
				if(numb % 4 != 1){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_g) == 1)
					;
			} else if(HAL_GPIO_ReadPin(GPIOC, LED_button_r) == 1){
				if(numb % 4 != 2){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_r) == 1)
					;
			}
		}
		numb++;
		if(hasPressed == 0){
			mistakeNumber++;
		}
		set_7Segment(mistakeNumber);
		if(mistakeNumber >= 3){
			status = 4;
			LED_BLUE(0);
			LED_GREEN(0);
			LED_RED(0);
			LED_YELLOW(0);
			HAL_NVIC_DisableIRQ(TIM3_IRQn);
			endGame();
			return;
		}
	}
	
	
	/******************************************************** 900 ms ****************************************************/
	for(int i = 0; i < 10; i++){
		LED_BLUE(0);
		LED_GREEN(0);
		LED_RED(0);
		LED_YELLOW(0);
		if(numb % 4 == 0)
			LED_BLUE(1);
		else if(numb % 4 == 1)
			LED_GREEN(1);
		else if(numb % 4 == 2)
			LED_RED(1);
		else if(numb % 4 == 3)
			LED_YELLOW(1);
				
		uint32_t tickstart = HAL_GetTick();
		uint32_t wait = 90;
	
		/* Add a freq to guarantee minimum wait */
		if (wait < HAL_MAX_DELAY)
		{
			wait += (uint32_t)(uwTickFreq);
		}
	
		int hasPressed = 0;
		
		while((HAL_GetTick() - tickstart) < wait)
		{
			if(status != 3){
				LED_BLUE(0);
				LED_GREEN(0);
				LED_RED(0);
				LED_YELLOW(0);
				return;
			}
			if(time <= 0){
				status = 4;
				endGame();
				LED_BLUE(0);
				LED_GREEN(0);
				LED_RED(0);
				LED_YELLOW(0);
				HAL_NVIC_DisableIRQ(TIM3_IRQn);
				return;
			}
			if(HAL_GPIO_ReadPin(GPIOC, LED_button_y) == 1){
				if(numb % 4 != 3){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_y) == 1)
					;
			}else if(HAL_GPIO_ReadPin(GPIOC, LED_button_b) == 1){
				if(numb % 4 != 0){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_b) == 1)
					;
			}else if(HAL_GPIO_ReadPin(GPIOC, LED_button_g) == 1){
				if(numb % 4 != 1){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_g) == 1)
					;
			} else if(HAL_GPIO_ReadPin(GPIOC, LED_button_r) == 1){
				if(numb % 4 != 2){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_r) == 1)
					;
			}
		}
		numb++;
		if(hasPressed == 0){
			mistakeNumber++;
		}
		set_7Segment(mistakeNumber);
		if(mistakeNumber >= 3){
			status = 4;
			LED_BLUE(0);
			LED_GREEN(0);
			LED_RED(0);
			LED_YELLOW(0);
			HAL_NVIC_DisableIRQ(TIM3_IRQn);
			endGame();
			return;
		}
	}
	
	
	/******************************************************** 800 ms ****************************************************/
	for(int i = 0; i < 10; i++){
		LED_BLUE(0);
		LED_GREEN(0);
		LED_RED(0);
		LED_YELLOW(0);
		if(numb % 4 == 0)
			LED_BLUE(1);
		else if(numb % 4 == 1)
			LED_GREEN(1);
		else if(numb % 4 == 2)
			LED_RED(1);
		else if(numb % 4 == 3)
			LED_YELLOW(1);
				
		uint32_t tickstart = HAL_GetTick();
		uint32_t wait = 85;
	
		/* Add a freq to guarantee minimum wait */
		if (wait < HAL_MAX_DELAY)
		{
			wait += (uint32_t)(uwTickFreq);
		}
	
		int hasPressed = 0;
		
		while((HAL_GetTick() - tickstart) < wait)
		{
			if(status != 3){
				LED_BLUE(0);
				LED_GREEN(0);
				LED_RED(0);
				LED_YELLOW(0);
				return;
			}
			if(time <= 0){
				status = 4;
				endGame();
				LED_BLUE(0);
				LED_GREEN(0);
				LED_RED(0);
				LED_YELLOW(0);
				HAL_NVIC_DisableIRQ(TIM3_IRQn);
				return;
			}
			if(HAL_GPIO_ReadPin(GPIOC, LED_button_y) == 1){
				if(numb % 4 != 3){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_y) == 1)
					;
			}else if(HAL_GPIO_ReadPin(GPIOC, LED_button_b) == 1){
				if(numb % 4 != 0){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_b) == 1)
					;
			}else if(HAL_GPIO_ReadPin(GPIOC, LED_button_g) == 1){
				if(numb % 4 != 1){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_g) == 1)
					;
			} else if(HAL_GPIO_ReadPin(GPIOC, LED_button_r) == 1){
				if(numb % 4 != 2){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_r) == 1)
					;
			}
		}
		numb++;
		if(hasPressed == 0){
			mistakeNumber++;
		}
		set_7Segment(mistakeNumber);
		if(mistakeNumber >= 3){
			status = 4;
			LED_BLUE(0);
			LED_GREEN(0);
			LED_RED(0);
			LED_YELLOW(0);
			HAL_NVIC_DisableIRQ(TIM3_IRQn);
			endGame();
			return;
		}
	}
	
	
	/******************************************************** 700 ms ****************************************************/
	for(int i = 0; i < 10; i++){
		LED_BLUE(0);
		LED_GREEN(0);
		LED_RED(0);
		LED_YELLOW(0);
		if(numb % 4 == 0)
			LED_BLUE(1);
		else if(numb % 4 == 1)
			LED_GREEN(1);
		else if(numb % 4 == 2)
			LED_RED(1);
		else if(numb % 4 == 3)
			LED_YELLOW(1);
				
		uint32_t tickstart = HAL_GetTick();
		uint32_t wait = 80;
	
		/* Add a freq to guarantee minimum wait */
		if (wait < HAL_MAX_DELAY)
		{
			wait += (uint32_t)(uwTickFreq);
		}
	
		int hasPressed = 0;
		
		while((HAL_GetTick() - tickstart) < wait)
		{
			if(status != 3){
				LED_BLUE(0);
				LED_GREEN(0);
				LED_RED(0);
				LED_YELLOW(0);
				return;
			}
			if(time <= 0){
				status = 4;
				endGame();
				LED_BLUE(0);
				LED_GREEN(0);
				LED_RED(0);
				LED_YELLOW(0);
				HAL_NVIC_DisableIRQ(TIM3_IRQn);
				return;
			}
			if(HAL_GPIO_ReadPin(GPIOC, LED_button_y) == 1){
				if(numb % 4 != 3){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_y) == 1)
					;
			}else if(HAL_GPIO_ReadPin(GPIOC, LED_button_b) == 1){
				if(numb % 4 != 0){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_b) == 1)
					;
			}else if(HAL_GPIO_ReadPin(GPIOC, LED_button_g) == 1){
				if(numb % 4 != 1){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_g) == 1)
					;
			} else if(HAL_GPIO_ReadPin(GPIOC, LED_button_r) == 1){
				if(numb % 4 != 2){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_r) == 1)
					;
			}
		}
		numb++;
		if(hasPressed == 0){
			mistakeNumber++;
		}
		set_7Segment(mistakeNumber);
		if(mistakeNumber >= 3){
			status = 4;
			LED_BLUE(0);
			LED_GREEN(0);
			LED_RED(0);
			LED_YELLOW(0);
			HAL_NVIC_DisableIRQ(TIM3_IRQn);
			endGame();
			return;
		}
	}
	
	
	
	
	/******************************************************** 600 ms ****************************************************/
	for(int i = 0; i < 10; i++){
		LED_BLUE(0);
		LED_GREEN(0);
		LED_RED(0);
		LED_YELLOW(0);
		if(numb % 4 == 0)
			LED_BLUE(1);
		else if(numb % 4 == 1)
			LED_GREEN(1);
		else if(numb % 4 == 2)
			LED_RED(1);
		else if(numb % 4 == 3)
			LED_YELLOW(1);
				
		uint32_t tickstart = HAL_GetTick();
		uint32_t wait = 75;
	
		/* Add a freq to guarantee minimum wait */
		if (wait < HAL_MAX_DELAY)
		{
			wait += (uint32_t)(uwTickFreq);
		}
	
		int hasPressed = 0;
		
		while((HAL_GetTick() - tickstart) < wait)
		{
			if(status != 3){
				LED_BLUE(0);
				LED_GREEN(0);
				LED_RED(0);
				LED_YELLOW(0);
				return;
			}
			if(time <= 0){
				status = 4;
				endGame();
				LED_BLUE(0);
				LED_GREEN(0);
				LED_RED(0);
				LED_YELLOW(0);
				HAL_NVIC_DisableIRQ(TIM3_IRQn);
				return;
			}
			if(HAL_GPIO_ReadPin(GPIOC, LED_button_y) == 1){
				if(numb % 4 != 3){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_y) == 1)
					;
			}else if(HAL_GPIO_ReadPin(GPIOC, LED_button_b) == 1){
				if(numb % 4 != 0){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_b) == 1)
					;
			}else if(HAL_GPIO_ReadPin(GPIOC, LED_button_g) == 1){
				if(numb % 4 != 1){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_g) == 1)
					;
			} else if(HAL_GPIO_ReadPin(GPIOC, LED_button_r) == 1){
				if(numb % 4 != 2){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_r) == 1)
					;
			}
		}
		numb++;
		if(hasPressed == 0){
			mistakeNumber++;
		}
		set_7Segment(mistakeNumber);
		if(mistakeNumber >= 3){
			status = 4;
			LED_BLUE(0);
			LED_GREEN(0);
			LED_RED(0);
			LED_YELLOW(0);
			HAL_NVIC_DisableIRQ(TIM3_IRQn);
			endGame();
			return;
		}
	}
	
	
	/******************************************************** 500 ms ****************************************************/
	for(int i = 0; i < 10; i++){
		LED_BLUE(0);
		LED_GREEN(0);
		LED_RED(0);
		LED_YELLOW(0);
		if(numb % 4 == 0)
			LED_BLUE(1);
		else if(numb % 4 == 1)
			LED_GREEN(1);
		else if(numb % 4 == 2)
			LED_RED(1);
		else if(numb % 4 == 3)
			LED_YELLOW(1);
				
		uint32_t tickstart = HAL_GetTick();
		uint32_t wait = 70;
	
		/* Add a freq to guarantee minimum wait */
		if (wait < HAL_MAX_DELAY)
		{
			wait += (uint32_t)(uwTickFreq);
		}
	
		int hasPressed = 0;
		
		while((HAL_GetTick() - tickstart) < wait)
		{
			if(status != 3){
				LED_BLUE(0);
				LED_GREEN(0);
				LED_RED(0);
				LED_YELLOW(0);
				return;
			}
			if(time <= 0){
				status = 4;
				endGame();
				LED_BLUE(0);
				LED_GREEN(0);
				LED_RED(0);
				LED_YELLOW(0);
				HAL_NVIC_DisableIRQ(TIM3_IRQn);
				return;
			}
			if(HAL_GPIO_ReadPin(GPIOC, LED_button_y) == 1){
				if(numb % 4 != 3){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_y) == 1)
					;
			}else if(HAL_GPIO_ReadPin(GPIOC, LED_button_b) == 1){
				if(numb % 4 != 0){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_b) == 1)
					;
			}else if(HAL_GPIO_ReadPin(GPIOC, LED_button_g) == 1){
				if(numb % 4 != 1){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_g) == 1)
					;
			} else if(HAL_GPIO_ReadPin(GPIOC, LED_button_r) == 1){
				if(numb % 4 != 2){
					mistakeNumber++;
					set_7Segment(mistakeNumber);
				}
				hasPressed = 1;
				while(HAL_GPIO_ReadPin(GPIOC, LED_button_r) == 1)
					;
			}
		}
		numb++;
		if(hasPressed == 0){
			mistakeNumber++;
		}
		set_7Segment(mistakeNumber);
		if(mistakeNumber >= 3){
			status = 4;
			LED_BLUE(0);
			LED_GREEN(0);
			LED_RED(0);
			LED_YELLOW(0);
			HAL_NVIC_DisableIRQ(TIM3_IRQn);
			endGame();
			return;
		}
	}
	
	LED_BLUE(0);
	LED_GREEN(0);
	LED_RED(0);
	LED_YELLOW(0);
	HAL_NVIC_DisableIRQ(TIM3_IRQn);
	endGame();
	status = 4;
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);
	HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_3);
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

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
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM1_Init();
  MX_TIM4_Init();
  MX_TIM5_Init();
  /* USER CODE BEGIN 2 */
	//int i = 0;
	//while(i <= 9){
	//	set_7Segment(i++);
	//	HAL_Delay(30);
	//}
	//HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
	//HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
	MyInitialize();
	//status = 2;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		if(status == 1)
			handle();
		
		if(status == 2)
			initGame();
		
		if(status == 3)
			game();
		
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 1499 ; // real time = 15999 but for making it real in proteus
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 1000 ; // real time = 1000 but for making it real in proteus
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 500 / 9 ; // real time = 500 but for making it real in proteus
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 15999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 999999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_BOTHEDGE;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */
	//TIM2 -> CR1 = 1; 
	//htim2.Instance->CR1 = 1; // active
	//htim2.Instance->DIER |= 1;
	//TIM2 -> DIER |= 1;
	
	
	//RCC -> APB1ENR |= 1;
	//TIM2 -> PSC = 16000 - 1;
	TIM2 -> CCMR1 = 0x41;
	TIM2 -> CCER = 0x0B;
	TIM2 -> CR1 = 1;
  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 709; // real time should be 15999 but for makins it real in proteus
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1000;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */
	//TIM2 -> CR1 = 1; 
	htim3.Instance->CR1 = 1; // active
	htim3.Instance->DIER |= 1;
	//TIM2 -> DIER |= 1;
  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 1499 ; // real time = 15999 but for making it real in proteus
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 1000 ; // real time = 1000 but for making it real in proteus
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 500 / 9 ; // real time = 500 but for making it real in proteus
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief TIM5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM5_Init(void)
{

  /* USER CODE BEGIN TIM5_Init 0 */

  /* USER CODE END TIM5_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM5_Init 1 */

  /* USER CODE END TIM5_Init 1 */
  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 1599;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 500;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM5_Init 2 */
	//TIM5 -> CR1 = 1; 
	htim5.Instance->CR1 = 1; // active
	htim5.Instance->DIER |= 1;
	//TIM5 -> DIER |= 1;
  /* USER CODE END TIM5_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_8
                          |GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
                          |GPIO_PIN_7|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC0 PC1 PC2 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA1 PA2 PA3 PA4
                           PA5 PA6 PA7 PA8
                           PA9 PA10 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4
                          |GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8
                          |GPIO_PIN_9|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PC4 PC5 PC6 PC7 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB2 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB10 PB12 PB13 PB14
                           PB3 PB4 PB5 PB6
                           PB7 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
                          |GPIO_PIN_7|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PC8 PC9 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PC10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI2_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
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
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
