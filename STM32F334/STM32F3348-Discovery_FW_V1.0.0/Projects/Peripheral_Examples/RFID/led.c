/**
  ******************************************************************************
  * @file    RFID/button.c 
  * @author  Michael Weiss
  * @version V1.0.0
  * @date    3/21/16
  * @brief	 button interface code
  ******************************************************************************
  * @attention
  *
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

#define LED_SHIFT 2
#define LED_BITS 3
#define BLE_CONN_TIME (1* 1000)  // time to wait till we think we are connected in ms

/**************************************************************************************/

/* I will implement a blink feature so we don't have to do much */

static uint8_t _ledColor = LED_BLACK;
static uint32_t _ledTempTime = 0;
static uint32_t _ledCounter = 0;
static uint32_t _ledPerTime = LED_SLOW_CYCLE;  // How many ticks for a full led cycle
static uint32_t _ledOnTime = 100;   // the led will be off for this mount of time per ledTimePeriod

static uint32_t _bleLastTime = 0;
static uint8_t _bleState = 0;

void Led_Set_Cycle(uint32_t time)
{
	_ledPerTime = time;
}

void Led_Show_Ble_State(void)
{
	uint32_t curTime =  getTick();
	
	if((_bleState == Bit_RESET) && ((_bleLastTime + BLE_CONN_TIME) < curTime))
	{
		if (goodStr)
		{
			Led_Set(LED_GREEN);
		}
		else
		{
			Led_Set(LED_PURPLE);
		}
	}
	else
	{
		if (goodStr)
		{
			Led_Set(LED_YELLOW);
		}
		else
		{
			Led_Set(LED_CYAN);
		}
	}
}

/*
 * This code is called when the BLE led changes from on to off or visa versa
 *
 * figure out if it is one or off and keep track of the last time it changed
 */

void Ble_Led_Flash(void)
{
	// ok so look at the GPIO pin and see if it is high or lowand get the tickCounter

	_bleLastTime = getTick();
	
	_bleState = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1);
	
#if 0
	if (_bleState == Bit_RESET)
	{
		// this means LED on so set it on now 
		Led_Set(LED_PURPLE);
	}
	else
	{
		// this way when we do an Led_On we will still be dark
		Led_Set(LED_BLACK);
	}
#endif
}

void Led_Tick(void)
{
	// if we temporarily have a different color then count it and move on
	
	if( _ledTempTime != 0)
	{
		if( --_ledTempTime <= 0 )
		{
			_ledTempTime = 0;  // stop doing this
	
			Led_Show_Ble_State();
		}
	}
	else if( _ledPerTime != 0 )
	{
		_ledCounter++;
	
		if((_ledCounter % _ledPerTime) == 0)
		{
			Led_Show_Ble_State();
		}
		else if((_ledCounter % _ledPerTime) == _ledOnTime)
		{
			Led_Off();
		}
	}
}

uint8_t Led_Off(void)
{
	for( uint8_t bit = 0 ; bit < LED_BITS ; bit++)
	{
		GPIO_SetBits(GPIOA,  1 << (bit + LED_SHIFT));
	}
	
	return _ledColor;
}

uint8_t Led_On(void)
{
	Led_Set(_ledColor);
	return _ledColor;
}

uint8_t Led_Set_Temp(uint8_t color, uint32_t time)
{
	uint8_t oldColor = _ledColor;
	
	color &= LED_WHITE;
	
	for( uint8_t bit = 0 ; bit < LED_BITS ; bit++)
	{
		if( color & (1 << bit) )
		{
			GPIO_ResetBits(GPIOA, 1 << (bit + LED_SHIFT));
		}
		else
		{
			GPIO_SetBits(GPIOA,  1 << (bit + LED_SHIFT));
		}
	}
	
	// reset the led timer
	
	_ledCounter = 0;
	return oldColor;
}

void Led_Set(uint8_t color)
{
	_ledColor = color & LED_WHITE;
	
	for( uint8_t bit = 0 ; bit < LED_BITS ; bit++)
	{
		if( _ledColor & (1 << bit) )
		{
			GPIO_ResetBits(GPIOA, 1 << (bit + LED_SHIFT));
		}
		else
		{
			GPIO_SetBits(GPIOA,  1 << (bit + LED_SHIFT));
		}
	}
	
	// reset the led timer
	
	_ledCounter = 0;
}


void Led_Config(void)
{
	GPIO_InitTypeDef   GPIO_InitStructure;

	/* Configure PA.02, PA.03, PA.04 pin as output push-pull */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// do a simple LED pattern to show we are alive

	for( int8_t col = LED_WHITE ; col >= LED_BLACK ; col-- )
	{
		Led_Set(col);
		
		/* delay a small amount */
		
		Delay(100);
	}
	
	// leave it at blinking RED
	
	Led_Set(LED_RED);
}
