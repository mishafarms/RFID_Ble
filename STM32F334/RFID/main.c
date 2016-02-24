/**
  ******************************************************************************
  * @file    TIM_PWM_Input/main.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    20-June-2014
  * @brief   Main program body
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

/* Includes ------------------------------------------------------------------*/
#include "rfid.h"

/** @addtogroup STM32F3348_DISCOVERY_Examples
  * @{
  */

/** @addtogroup RFID
  * @{
  */

/* Private typedef -----------------------------------------------------------*/  
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static __IO uint32_t TickCounter;
/* Private function prototypes -----------------------------------------------*/

void GPIO_Configuration(void);

/* Private functions ---------------------------------------------------------*/

int testMe = 0;
char sentStr[32] = "";  // I know the string cannot be 32 bytes

/**
  * @brief   Main program
  * @param  None
  * @retval None
  */
int main(void)
{  
	uint32_t ButtonReleasedTime = 0;
	
	/*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       files (startup_stm32f334x8.s) before to branch to application main. 
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f30x.c file
     */  

  /* Setup SysTick Timer for 1 msec interrupts.
     ------------------------------------------
    1. The SysTick_Config() function is a CMSIS function which configure:
       - The SysTick Reload register with value passed as function parameter.
       - Configure the SysTick IRQ priority to the lowest value (0x0F).
       - Reset the SysTick Counter register.
       - Configure the SysTick Counter clock source to be Core Clock Source (HCLK).
       - Enable the SysTick Interrupt.
       - Start the SysTick Counter.
    
    2. You can change the SysTick Clock source to be HCLK_Div8 by calling the
       SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8) just after the
       SysTick_Config() function call. The SysTick_CLKSourceConfig() is defined
       inside the stm32f30x_misc.c file.

    3. You can change the SysTick IRQ priority by calling the
       NVIC_SetPriority(SysTick_IRQn,...) just after the SysTick_Config() function 
       call. The NVIC_SetPriority() is defined inside the core_cm0.h file.

    4. To adjust the SysTick time base, use the following formula:
                            
         Reload Value = SysTick Counter Clock (Hz) x  Desired Time base (s)
    
       - Reload Value is the parameter to be passed for SysTick_Config() function
       - Reload Value should not exceed 0xFFFFFF
   */

if (SysTick_Config(SystemCoreClock / 1000))
  { 
    /* Capture error */ 
      while (1)
      {
      }
  }

  /* TIM configuration --------------------------------------------------------- 
    TIM3 configuration: External clk Counter mode
     The external CLK signal is connected to TIM3 CH1 pin (PA.06)
		 The external capture signal is connected to TIM3 CH2 pin (PA.07)
     TIM3 CCR2 is used to tell how many clocks have passed since the last capture
     TIM3 Update is used to keep track of how long we have been looking so we don't
			lock up. A timed interrupt is probably better.


  Note: 
  SystemCoreClock variable holds HCLK frequency and is defined in system_stm32f30x.c file.
  Each time the core clock (HCLK) changes, user had to call SystemCoreClockUpdate()
  function to update SystemCoreClock variable value. Otherwise, any configuration
  based on this variable will be incorrect.  
  --------------------------------------------------------------------------- */

	RCC_Configuration();
  
	/* We need to init some pins for the harware we have */
  
	GPIO_Configuration();
	
	TIM_Config();
	
	USART1_Configuration();
	
	Button_Config();
	
	/* Infinite loop */
	
	while (1)
	{
		USART1_SendStr("Hello World\n");
		Delay(1000);
	}

	while (1)
	{
		char *animalStr;

		if (Button_State() )
		{
			// if the button is pressed then it cannot be released, so clear the releaseTime
			
			ButtonReleasedTime = 0;

			// it also means that the RFID chip should be awake and scanning */
			
			// see if we can find an RFID chip and read it in ISO 11784/11785 format
			
			animalStr = AnimalCapture();
	  
			if (animalStr != (char *) NULL)
			{
				// is it the same as the last RFID chip read?
				// if not then send it 
				
				if (strcmp(animalStr, sentStr) != 0)
				{
					USART1_SendStr(animalStr);
					
					// this is now the last RFID chip read
					
					strcpy(sentStr, animalStr);
				}
			}
			else
			{
				// I think not reading the value means they moved the RFID chip out of range
				// if they move it back in, they want to see it again
				
				sentStr[0] = '\0';
			}
		}
		else
		{
			// how long has it been? (That is a rather personal question!) - that the button was down
			
			if (!ButtonReleasedTime)
			{
				// just released it
				// what time is now
				
				ButtonReleasedTime = TickCounter;

				// it also means that the RFID chip should be put to sleep, in  nice way */
			}
			else if (TickCounter >= (ButtonReleasedTime + SLEEP_TIMEOUT))
			{
				// we have been idle long enough, go to standby mode
				
				// but before we do, tll the nRF51822 so it will sleep as well */
				
				PWR_WakeUpPinCmd(PWR_WakeUpPin_1, ENABLE);

				// this routine will not return. We will come out of this into reset
				// there is a way to tell if it was a reset or wakeup if we care.

				PWR_EnterSTANDBYMode();
			}
		}
	}
}

/**************************************************************************************/
  
void RCC_Configuration(void)
{
  /* Enable GPIO clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
 
  /* Enable USART clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
  /* TIM3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);  

  /* Enable SYSCFG clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
}
	
void GPIO_Configuration(void)
{
	GPIO_InitTypeDef   GPIO_InitStructure;

	/* pin PB.00 is connected to the nRF51822 pin P0.04 This pin will be an output (act like a button to the */
	/* nRF51822. This will be used to put the nRF51822 to sleep and wake it up. Pin PB.01 is connected to    */
	/* nRF51822 pin P0.03 This pin will be an input, The nRF51822 is using this pin as a LED to indicate     */
	/* it's status. We will have to look at the activity on this pin and figure out what the nRF51822 is     */
	/* trying to tell us */
	
	/* Configure PB.00 pin as output OD */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/* Configure PB.01 pin as input */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/* we also have 2 pins mapped for the EM4095 chip they are the SHD and the MOD pins. */
	/* pin PA.03 is SHD and PA.15 is MOD */
	
	/* Configure PB.03 pin as output push-pull (SHD) */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/* set SHD initial state as LOW, for now, This will need to be disabled when we go to sleep. */
	/* actually this should only be LOW when the button is pressed (looking for tags). */
	
	GPIO_ResetBits(GPIOB, GPIO_Pin_3);
	
	/* Configure PA.15 pin as output push-pull (MOD) */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* set MOD initial state as LOW, for this version of hardware it is probably not needed. */
	
	GPIO_ResetBits(GPIOA, GPIO_Pin_15);
}

void Timing_Tick(void)
{
	extern void ledOn(void);
	extern void ledOff(void);
	
	TickCounter++;
	
	if((TickCounter % 1000) == 0)
	{
		ledOn();
	}
	else if((TickCounter % 1000) == 500)
	{
		ledOff();
	}
}

/**
* @brief  Inserts a delay time.
* @param  nTime: specifies the delay time length, in 1 ms.
* @retval None
*/
void Delay(__IO uint32_t nTime)

{
  uint32_t TimingDelay = TickCounter + nTime;
  
  while(TickCounter < TimingDelay)
  {
	  if(timerDone)
	  {
		  break;
	  }
  }
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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
