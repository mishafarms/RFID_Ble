/**
  ******************************************************************************
  * @file    RFID/animal.c 
  * @author  Michael Weiss
  * @version V1.0.0
  * @date    1/6/15
  * @brief   Deal with all the animal specifics here
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "rfid.h"
#include <stdio.h>

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

//Animal Mode data
uint8_t AnimalData[16];                
uint16_t crc; 

/* Private function prototypes -----------------------------------------------*/

ErrorStatus AnimalCapture_lowLevel(void);


uint8_t reverseArray[256] = {
	0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0, 0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0, 
	0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8, 0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8, 
	0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4, 0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4, 
	0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec, 0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc, 
	0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2, 0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2, 
	0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea, 0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa, 
	0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6, 0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6, 
	0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee, 0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe, 
	0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1, 0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1, 
	0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9, 0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9, 
	0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5, 0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5, 
	0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed, 0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd, 
	0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3, 0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3, 
	0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb, 0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb, 
	0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7, 0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7, 
	0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef, 0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};

char *AnimalString(void)
{
	uint8_t animal[16];
	uint8_t value;
	uint8_t newArray[16];
	int starts[13] = { 11, 20, 29, 38, 47, 56, 65, 74, 83, 92, 101, 110, 119 };
	int start;
	int bitCnt;
	long long nationalCode = 0;
	int countryCode = 0;
	int block = 0;
	int blockValid = 0;
	int animalFlag = 0;
	static char buffer[160];
	int x;
	int y;

	/* turn the array into something that we can actually work with */

	for(x = 0 ; x < 16 ; x++)
	{
		newArray[x] = reverseArray[AnimalData[x]];  // this does a bit reversal lsb -> msb
	}

	/* extract the 13 bytes of actual data */

	for( x = 0; x < 13 ; x++ )
	{
		start = starts[x] / 8;
		bitCnt = starts[x] % 8;

		value = 0;

		for( y = 0; y < 8; y++)
		{
			value |= ((newArray[start] >> (7 - bitCnt)) & 0x1) << y;

			if( ++bitCnt == 8 )
			{
				start++;
				bitCnt = 0;
			}
		}

		animal[x] = value;
	}

	/* now I need to get the actual values stored in the record, 38 bits of national code, 10 bits of country code, */
	/* then some flags and a possible 24 bit block at the end */

	nationalCode = animal[0] | (animal[1] << 8) | (animal[2] << 16) | (animal[3] << 24) |
		((long long) (animal[4] & 0x3f) << 32);

	countryCode = ((animal[4] >> 6) & 0x3) | (animal[5] << 2);

	blockValid = animal[6] & 0x1;
	animalFlag = (animal[7] >> 7) & 0x01;

	/* skip crc it was checked before we were called */

	block = animal[10] | (animal[11] << 8) | (animal[12] << 16);

	sprintf(buffer, "%d-%06ld%06ld %s %d%s",
			countryCode,
			(long) (nationalCode / 1000000L),
			(long) (nationalCode % 1000000L),
			animalFlag ? "A" : "",
			block,
			blockValid ? "V" : "");

	return buffer;
}

//======================================================================== 
//  iso11785CRC:  compute one bit CRC using CRC-CCITT for ISO 11785 
//  Results:		16 bit binary CRC 
//======================================================================== 
 
//------------------------------------------------------------------------- 
void iso11785CRC(uint8_t data)
{
	uint8_t bit, carry; 
#define poly 0x8408 
 
	carry = crc & 1;                        // set carry [expr $crc & 1] 
	crc = crc >> 1;                         // set crc [expr $crc >> 1] 
	bit = data & 1;                         // set bit [string range $in $i $i] 

	if (bit == 1)
	{
		crc = crc | 0x8000;  // set crc [expr $crc | 0x8000]
	}
	
	if (carry == 1)
	{
		crc ^= poly;
	}		
#undef poly 
} 

//-------------------------------------------------------------------- 
ErrorStatus AnimalTestValidRange(uint16_t start, uint8_t size )
{ 
	uint8_t valid = 0; 
	uint8_t bitcnt = start % 8; 

	uint8_t pom, data = 0; 
	uint8_t ptr = 0; 

	uint8_t strt = start / 8;  

	while (ptr < size)
	{ 
		pom = (capture_data[strt] >> (7-bitcnt)) & 1; 
		data >>= 1;
		
		if (pom != 0)
			data |= 0x80;
		
		valid = valid | ((capture_valid[strt] >> (7-bitcnt)) & 1); 

		if (ptr % 8 == 7)
		{			
			AnimalData[ptr/8] = data;
		}
		
		ptr++; 

		if (bitcnt == 7)
		{ 
			bitcnt = MAX_UINT8;  
			strt++; 
		}
		
		bitcnt++; 
	} 

	if (valid == 0)
	{
		return SUCCESS;
	}
	else
	{
		return ERROR;
	}		
} 

//======================================================================== 
// Animal Mode crc check of data in specified range 
//======================================================================== 
 
//------------------------------------------------------------------------- 
void AnimalTestCrcCheck(uint8_t start, uint8_t size )
{
	uint8_t bitcnt = start % 8; 
	uint8_t pom; 

	start = start / 8; 

	while (size-- > 0)
	{   
		pom = (AnimalData[start] >> bitcnt) & 1; 

		iso11785CRC(pom); 

		if (bitcnt == 7)
		{ 
			bitcnt = MAX_UINT8; 
			start++; 
		}
		
		bitcnt++; 
	} 
}

//======================================================================== 
//Animal Mode data check - suggested smallest code size 
//======================================================================== 
 

ErrorStatus CheckAnimalData(void)
{ 
 
  crc = 0;                       //init CRC 
 
  AnimalTestCrcCheck( 11, 8 ); 
  AnimalTestCrcCheck( 20, 8 ); 
  AnimalTestCrcCheck( 29, 8 ); 
  AnimalTestCrcCheck( 38, 8 ); 
  AnimalTestCrcCheck( 47, 8 ); 
  AnimalTestCrcCheck( 56, 8 ); 
  AnimalTestCrcCheck( 65, 8 ); 
  AnimalTestCrcCheck( 74, 8 ); 
  AnimalTestCrcCheck( 83, 8 ); 
  AnimalTestCrcCheck( 92, 8 ); 
 
	if (crc == 0)
	{	  
		return SUCCESS;
	}		
	else
	{		
		return ERROR;
	}		
} 
 
/**
  * @brief  Start an biphase capture and try to apply ISO 11784/5
  * @param  None
  * @retval None
  */
char *AnimalCapture(void)
{ 
	ErrorStatus status;
	char *str = NULL;
	
	decode = biPhaseCapture;
	status = AnimalCapture_lowLevel(); 

	if (status == SUCCESS)
	{
		str = AnimalString();
	}

	return "935-0000045135\n";
	return str; 
} 
 
 
//======================================================================== 
//Animal Mode Capture - performs one default read and checks the header presence 
//                    - header is (1)00000000001 
//======================================================================== 
 
//------------------------------------------------------------------------- 
ErrorStatus AnimalCapture_lowLevel(void)
{
	ErrorStatus st;
	uint8_t status,	start;
	
	Capture(1); 
 
	status = SearchPattern( 0x801, 0xFFF, 0, 150 );  //search r/o header within first 150 captured bits 

	if (status != 0)
	{	  
		start = status - 11; 
		st = AnimalTestValidRange(start, 128);

		if(st == SUCCESS)
		{ 
			st = CheckAnimalData(); 
		} 
	}  
	else
	{
		st = ERROR; 
	}
	
	return st; 
} 
 
