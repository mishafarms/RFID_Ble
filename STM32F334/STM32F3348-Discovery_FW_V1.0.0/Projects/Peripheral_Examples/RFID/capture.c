/**
  ******************************************************************************
  * @file    RFID/capture.c 
  * @author  Michael Weiss
  * @version V1.0.0
  * @date    1/6/15
  * @brief   Capture types
  ******************************************************************************
  * @attention
  *
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

uint8_t halfDataRate = 16;

/* Private typedef -----------------------------------------------------------*/  
/* Private define ------------------------------------------------------------*/

#define TOLERANCE_SHIFT 1

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

uint16_t capts[500];
uint16_t capts_index = 0;

uint8_t captured_byte;      //store_bit current capture bits 
uint8_t captured_valid;     //store_bit current capture valid bits 
uint16_t capture_cnt;        //store_bit current capture byte index 
uint8_t captured_bit_count; //store_bit current capture bit counter 
uint8_t running_one;        //store_bit running one 
uint8_t pulse_cnt;          //how many pules have been stored

uint8_t manchester_inversion = 0;

uint8_t enable_capture = 0;

uint8_t decode_number = DECODE_NUMBER_BIPHASE;

// level 1 local variables 
 
uint8_t bit_pos;                      //bit position in demodulation 
uint8_t last_bit;                     //for biphase and miller usage 
uint8_t edge;                         //for manchester usage 
uint8_t last_valid;                   //last valid value for invalid sequence compression 

uint8_t capture_data[CAPTURE_SIZE];
uint8_t capture_valid[CAPTURE_SIZE];


uint16_t times = 0;
uint16_t myPulseCnt = 0;
uint8_t pulseLength[500];

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief   storeBit into capture buffer
  * @param  b - bit value, v - valid bit
  * @retval None
  */

void storeBit(uint8_t b, uint8_t v) 
{ 
	b &= 1; 
	v &= 1; 
  
	if( manchester_inversion != 0)
		b ^= 1;  

	if (enable_capture == 1)
	{ 
		if (b != 0)
			captured_byte |= running_one;
		
		if (v == 0)
			captured_valid ^= running_one;
		
		running_one >>= 1; 

		capture_data[capture_cnt] = captured_byte;			
		capture_valid[capture_cnt] = captured_valid; 

		if (captured_bit_count == 7)
		{ 
			captured_bit_count = 0; 
			captured_byte = 0; 
			captured_valid = 0xFF; 
			running_one = 0x80; 
			capture_cnt++; 

			if (capture_cnt == CAPTURE_SIZE)
			{
				timerStopInts();
				enable_capture = 0; 
			} 
		}
		else
		{ 
			captured_bit_count++; 
		} 
	} //enbl==1 
}  

 
/**
  * @brief   store_pulse - debug code to store pulse_length for debugging
  * @param
  * @retval None
  */
// ================================================================== 
// STORE PULSE - debug_mode == 3 
// ================================================================== 

void store_pulse(uint8_t b) 
 { 
    capture_data[capture_cnt] = b; 
 
    if (++capture_cnt == CAPTURE_SIZE) {
		timerStopInts();       //disable all (other modulations) 
  } 
} 
 

/**
  * @brief  store_pulse_length
  * @param	b - number of counts for this pulse
  * @retval None
  */
void store_pulse_length(uint8_t b) 
 { 
	times++;

	if (myPulseCnt < 500)
	{
		pulseLength[myPulseCnt] = b;
		++myPulseCnt;
	}
	else
	{
		timerStopInts();       //disable all (other modulations) 
    }
} 

/**
  * @brief   biPhaseCapture
  * @param
  * @retval None
  */
	
void biPhaseCapture(uint32_t capture, uint32_t edge)
{
	uint8_t delta;
	uint16_t capt; 
	int y;

	if (capts_index < 500)
	{
		capts[capts_index++] = (uint16_t) capture;
	}

	capt = (uint16_t) capture;

	if(capt>MAX_UINT8) delta = MAX_UINT8; else delta = (uint8_t)capt; 

	y = 0;
	delta += halfDataRate >> (TOLERANCE_SHIFT);	 // dynamic resolution (dependent on datarate)	 !!!!! >> 2
	delta -= halfDataRate;				// always subtract datarate/2
	if (delta > halfDataRate) {	 // when longer than that, try to subtract once more
		y++;
		delta -= halfDataRate;
	}

	if (delta > halfDataRate) {				// check overflow of counter and range
		// this check is also dynamic, depends on current datarate
		// the faster datarate the more precise must the signal be
		if (last_valid == 0) {
			storeBit(bit_pos^1, 0);			// rtf feature
			storeBit ( 0, 1);						// store bad bit
			last_valid = 1;
		}
		bit_pos = 0;

	} else {

		if (y) {
			storeBit(1, 0);
			bit_pos = 0;
			last_valid = 0;
		} else {
			if (bit_pos++) {
				storeBit(0, 0);
				bit_pos = 0;
				last_valid = 0;
			}
		}
	}
}

/**
  * @brief   biPhaseCapture
  * @param
  * @retval None
  */
	
void ClearCaptureBuffers(void)
{
	int x;
	
	for( x = 0 ; x < CAPTURE_SIZE ; x++ )
	{
		capture_data[x] = 0;
		capture_valid[x] = 0;
	}

	for( x = 0 ; x < 500 ; x++ )
	{
		pulseLength[x] = 0;
		capts[x] = 0;
	}
}

/**
  * @brief   biPhaseCapture
  * @param
  * @retval None
  */
	
// ================================================================== 
// CAPTURE 
// 
// Down Link setup function 
// Requires: maxCaptureTimeLow and maxCaptureTimeHi 
//   (located here instead of level2 because of register variables) 
// ================================================================== 
 
void Capture(uint8_t style)
{
	// make sure that we are stopped, before we really start

	timerStopInts();

	ClearCaptureBuffers();
	last_valid = 0; 
	captured_bit_count = 0; 
	pulse_cnt = 0;
	myPulseCnt = 0;
	capture_cnt = 0;                         
	bit_pos = 0; 
	// first = 1; 
	captured_byte = 0;
	captured_valid = 0xFF;
	running_one = 0x80;
	currentMaxTimeHi = 0;
 
	enable_capture = style;                   //set capture style 

	timerEnableCapture();

// wait till the interrupts are disabled
 
	waitForTimer();

	if(last_valid == 0) {                     // rtf feature 
		switch (decode_number) {
			case 1 : 
				last_bit = (~edge); 
				storeBit(last_bit, 0);
				break;
			case 2 :
				storeBit(bit_pos ^ 1, 0);
				break; 
			default : 
				break; 
		} 
	} 
  
	if (capture_cnt != CAPTURE_SIZE)          //flush captured bits
	{
		while (captured_bit_count != 0)
		{ 
			storeBit(0,1);
		}
	}
 
//  biphase_process();

	enable_capture = 0;
	timerStopInts();
  
  //save the final time for next use
//  maxCaptureTimeLow = ~timerGetCounter();               
//  maxCaptureTimeHi = ~currentMaxTimeHi;
}

/**
  * @brief   SearchPattern - look for a patterns of bits in the stream we captured
  * @param
  * @retval None
  */
	
uint8_t SearchPattern( uint32_t pattern, uint32_t mask, uint8_t start, uint8_t size )
{
	uint32_t value = 0; 
	uint32_t valid = 0xFFFFFFFF; 

	uint8_t bitcnt = start % 8; 

	uint8_t pom; 

	start = start / 8; 

	while (size > 0)
	{ 
		pom = (capture_data[start] >> (7-bitcnt)) & 1; 
		value = (value << 1) | pom;       
		valid = (valid << 1) | ((capture_valid[start] >> (7-bitcnt)) & 1); 

		size--; 
		if (bitcnt == 7)
		{ 
			bitcnt = 0; 
			start++; 
		}
		else
		{			
			bitcnt++; 
		}

		if( ((value & mask) == pattern) && ((valid & mask) == 0)) //test pattern match
		{
			return (start*8 + bitcnt);                       //return the (next) position  
		} 
	} 

	return 0; 
} 

