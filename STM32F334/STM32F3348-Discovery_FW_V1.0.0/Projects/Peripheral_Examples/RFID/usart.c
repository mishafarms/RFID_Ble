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

#define FLOW_CONTROL 1
 
static void _GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
 
  /* Connect PA.09 to USART1_Tx */

  GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_7);

  /* Connect PA.10 to USART1_Rx */
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_7);

#if defined(FLOW_CONTROL)
  /* Connect PA.11 to USART1_CTS */
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_7);
 
  /* Connect PA.12 to USART2_RTS */
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_7);
 
  GPIO_StructInit(&GPIO_InitStructure);
#endif
	
#if defined(FLOW_CONTROL)
  /* Configure USART Tx and RTS as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_12;
#else
  /* Configure USART Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
#endif

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
 
#if defined(FLOW_CONTROL)
  /* Configure USART Rx and CTS as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
#else
  /* Configure USART Rx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
#endif

  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

#if 0
	/* Configure PA.08 pin as output push-pull */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_11;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_8 | GPIO_Pin_11);
#endif
}
 
/**************************************************************************************/
 
void USART1_Configuration(void)
{
  USART_InitTypeDef USART_InitStructure;
 
  _GPIO_Configuration();

  /* USART resources configuration (Clock, GPIO pins and USART registers) ----*/
  /* USART configured as follow:
        - BaudRate = 115200 baud
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
  */
	
  USART_InitStructure.USART_BaudRate = 38400;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
#if defined(FLOW_CONTROL)
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_RTS_CTS;
#else
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
#endif

  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
 
  /* USART configuration */
  USART_Init(USART1, &USART_InitStructure);
 
  /* Enable USART */
  USART_Cmd(USART1, ENABLE);
 
//  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
}
 
/**************************************************************************************/

void USART1_SendStr(char *str)
{
	int len = strlen(str);
	int x;
	
	for( x = 0 ; x < len ; x++)
	{
			/* wait for the UASRT to go empty , before we send */
		
			while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); // Wait for Empty
  
			USART_SendData(USART1, str[x]); // Send next char
	}
}
