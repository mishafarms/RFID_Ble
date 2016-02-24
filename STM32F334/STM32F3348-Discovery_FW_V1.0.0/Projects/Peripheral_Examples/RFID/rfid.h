/**
  ******************************************************************************
  * @file    TIM_PWM_Input/main.h 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    20-June-2014
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RFID_H
#define __RFID_H

/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include <string.h>

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

#define MAX_UINT8 255
#define CAPTURE_SIZE 88

#define FALSE (0)
#define TRUE (!FALSE)

//#define SLEEP_TIMEOUT  (1000 * 60 * 2)   // 2 minutes in milliseconds
#define SLEEP_TIMEOUT  (1000 *10)   // 30 seconds in milliseconds

/* There will be more if I decide to support other formats (likely) */

#define DECODE_NUMBER_BIPHASE 2

/* Exported functions ------------------------------------------------------- */

extern void USART1_Configuration(void);
extern void USART1_SendStr(char *str);
extern void USART2_SendStr(char *str);

extern void Timing_Tick(void);
extern void Delay(__IO uint32_t nTime);

/* button.c */

extern void Button_Config(void);
extern void Button_Handler(void);
extern uint8_t Button_State(void);
extern void Sleep_Button_Press(void);

/* timer.c */

extern void (*decode)(uint32_t, uint32_t);      //decoder routine variable

extern void TIM_Config(void);
extern void timerEnableCapture(void);

void timerStopInts(void);
void timerStop(void);

uint32_t timerGetCounter(void);
void timerSetCounter(uint32_t cnt);
void waitForTimer(void);

/* capture.c */

char *AnimalCapture(void);
void Capture(uint8_t style);
uint8_t SearchPattern( uint32_t pattern, uint32_t mask, uint8_t start, uint8_t size );

/* main.c still */

extern void RCC_Configuration(void);

extern uint8_t capture_data[CAPTURE_SIZE];
extern uint8_t capture_valid[CAPTURE_SIZE];
extern __IO uint8_t currentMaxTimeHi;

extern void biPhaseCapture(uint32_t, uint32_t);
extern __IO uint32_t timerDone;

#endif /* __RFID_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

