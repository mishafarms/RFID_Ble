#include "rfid.h"

TIM_ICInitTypeDef  TIM_ICInitStructure; 
__IO int intCnt = 0;
__IO int now = 0;
__IO uint8_t currentMaxTimeHi;
__IO int capture;
__IO int lastCapture;
__IO uint32_t timerDone = FALSE;


/* set decode to point to 0 and test before using */

void (*decode)(uint32_t, uint32_t) = (void (*)()) NULL; 

/**
  * @brief  Configure the TIM3 channel2 and TIM2 IRQ Handler.
  * @param  None
  * @retval None
  */
void TIM_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

	GPIO_StructInit(&GPIO_InitStructure);

	 /* TIM3 channel2 configuration : PB.04 and PB.05 */
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Connect TIM pin to AF2 */
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_2);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_2);


	/* Enable the TIM3 global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_ARRPreloadConfig(TIM3, (FunctionalState) DISABLE);
	TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);

	/* Timer 3 running in external counter mode tim3_ch1 is clock and tim3_ch2 is capture */

	TIM_TIxExternalClockConfig(TIM3, TIM_TIxExternalCLK1Source_TI1, TIM_ICPolarity_Rising, 0);

	TIM_ICStructInit(&TIM_ICInitStructure);
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICFilter = 15;     // looks like we want a clean signal

	TIM_ICInit(TIM3, &TIM_ICInitStructure);

	/* TIM enable counter */
	TIM_Cmd(TIM3, ENABLE); 
}

void timerEnableCapture(void)
{
	// turn on interrupts

	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	TIM_ClearITPendingBit(TIM3, TIM_IT_CC2);

	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	TIM_ITConfig(TIM3, TIM_IT_CC2, ENABLE);
	TIM_SetCounter(TIM3, 0);
	lastCapture = 0;
	timerDone = FALSE;
	TIM_Cmd(TIM3, ENABLE);
}

void timerStopInts(void)
{
	// we should turn off the interrupts

	TIM_ITConfig(TIM3, TIM_IT_Update, (FunctionalState) DISABLE);
	TIM_ITConfig(TIM3, TIM_IT_CC2, (FunctionalState) DISABLE);
	TIM_Cmd(TIM3, (FunctionalState) DISABLE);
	timerDone = TRUE;
}

void timerStop(void)
{
	timerStopInts();
	TIM_Cmd(TIM3, (FunctionalState) DISABLE);
}

uint32_t timerGetCounter(void)
{
	return(TIM_GetCounter(TIM3));
}

void timerSetCounter(uint32_t cnt)
{
	TIM_SetCounter(TIM3, cnt);
}

void waitForTimer(void)
{
	Delay(2000);
}

void TIM3_IRQHandler()
{
	intCnt++;
    
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

		// get the counter value now, I think I am early
		now = TIM_GetCounter(TIM3);

		// seems they want to be able to have more then one pass through the count. I could use a 32 bit counter
		// I guess, but seems easy enough

		if (currentMaxTimeHi != 0xFF) {                   //hi datarate 
			currentMaxTimeHi = 0xFF;
		} 
		else
		{
			// we should turn off the interrupts

			TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
			TIM_ITConfig(TIM3, TIM_IT_CC2, DISABLE);
			timerDone = 1;
		} 
	}
	
	// deal with the capture interrupt
	
	if (TIM_GetITStatus(TIM3, TIM_IT_CC2) != RESET)
	{
		uint32_t diff;
		
		// I think that reading the capture register should clear the interrupt

		capture = TIM_GetCapture2(TIM3) + ((currentMaxTimeHi == 0xFF) ? 0x10000 : 0x0);
		
		/* calculate the difference between this time and last time */
		
		diff = capture - lastCapture;
		lastCapture = capture;

		// now I have to look for the opposite edge

		TIM_ICInitStructure.TIM_ICPolarity = TIM_ICInitStructure.TIM_ICPolarity == TIM_ICPolarity_Rising ?
			TIM_ICPolarity_Falling : TIM_ICPolarity_Rising;

		TIM_ICInit(TIM3, &TIM_ICInitStructure);

		if (decode != NULL)
		{
			uint32_t edge;
			
			// this is stored in a global;

			edge = TIM_ICInitStructure.TIM_ICPolarity == TIM_ICPolarity_Rising ? 1 : 0;
			(*decode)(diff, edge); 
		}
	}
}
