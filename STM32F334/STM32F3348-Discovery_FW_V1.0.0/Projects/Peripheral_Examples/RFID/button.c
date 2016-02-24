/**
  ******************************************************************************
  * @file    RFID/usart.c 
  * @author  Michael Weiss
  * @version V1.0.0
  * @date    1/5/16
  * @brief	 USART interface code
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

/**************************************************************************************/
  
void ledOff(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_1);
}

void ledOn(void)
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_1);
}

uint8_t Button_State(void)
{
	return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
}

void Button_Config(void)
{
	GPIO_InitTypeDef   GPIO_InitStructure;

	/* Configure PA.01 pin as output push-pull */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// set LED on

	ledOn();

	/* Configure PA.00 pin as input pulldown */
	/* this is the button and wakeup */
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

