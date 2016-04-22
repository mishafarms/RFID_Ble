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

/**************************************************************************************/

uint8_t Button_State(void)
{
#if 0
	return(1);
#else
	return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
#endif
}

void Button_Config(void)
{
	GPIO_InitTypeDef   GPIO_InitStructure;

	/* Configure PA.00 pin as input pulldown */
	/* this is the button and wakeup */
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

