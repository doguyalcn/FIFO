/*
 * advanced.c
 *
 *  Created on: Jul 26, 2023
 *      Author: Doğu
 */

/*README.TXT

DATA PACKAGE ---> | 0x51 | 0x8 | 0x11 | 0x22 | 0x33 | 0x44 | 0x55 | 0xFF |

Firstly, the received data package is inserted into the ring buffer to ensure the correctness of the package.
If the packet is correct, the received data is transferred to the FIFO buffer one by one. Process example given below

FIFO has 50 elements.

0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ............ 0 0 0.
STEP 1
First data inserted first FIFO's first element.
51 0 0 0 0 0 0 0 0 0 0 0 0 0 0...
All FIFO right shifted by one.
0 51 0 0 0 0 0 0 0 0 0 0 0 0 0...

STEP 2
08 51 0 0 0 0 0 0 0 0 0 0 0 0 0...
0 08 51 0 0 0 0 0 0 0 0 0 0 0 0...

STEP3

11 08 51 0 0 0 0 0 0 0 0 0 0 0 0...
0 11 08 51 0 0 0 0 0 0 0 0 0 0 0...
.
.
.
STEP 9

FF 48 55 44 33 22 11 08 51 0 0 0...
0 FF 48 55 44 33 22 11 08 51 0 0 0...

NOTE THAT CRC INCLUDED AND FIFO STRUCTURE COMPLETED.
THE VALUE IN THE FIRST INDEX OF THE FIFO MUST ALWAYS BE ZERO FOR INCOMING DATA THUS
WE AVOID THE DATA LOSS.

FINALLY THIS BUFFER CAN BE SHIFTED TO LEFT BY ONE BIT.

 */

#include "advanced.h"
#include "usart.h"
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#define BUFFER_SIZE 9

int acceptedDataCount;
int acceptedPackageCount;

uint8_t rx_buff = 0;
uint8_t CRC_u8;

uint8_t rxBufferToFill[BUFFER_SIZE];
//uint8_t txBufferToFill[BUFFER_SIZE];

uint8_t fifoReady[81];
uint8_t FIFO[80];

circularBuffer uartRxBuffer;
//circularBuffer uartTxBuffer;

static int counter1 = 0;

void circularBufferInit(circularBuffer *exStruct, uint8_t *structBuffer)
{
	exStruct->head = 0;
	exStruct->tail = 0;
	exStruct->counter = 0;
	exStruct->size = BUFFER_SIZE;
	exStruct->buf = structBuffer;
}

void circularBufferInsert(circularBuffer *exStruct, uint8_t data)
{
		static uint8_t startbit = 0x51;		//starbit
		static uint8_t stopbit 	= 0xFF;		//stopbit
		static uint8_t ringLength = 0x08;	//ringlength
		static uint8_t ringCounter = 0;

		if(ringCounter == 0)
		{
			if(data == startbit)
			{
		exStruct->buf[exStruct->head] = data;
		exStruct->head = (exStruct->head + 1) % exStruct->size;
		ringCounter++;
			}
			else
			{
				for(size_t i = 0;i<exStruct->size;i++)
				{
					exStruct->buf[i] = 0;
				}
				exStruct->head=0;
				ringCounter =0;
			}

		}

		else if(ringCounter == 1)
		{
			if(data == ringLength)
			{
			exStruct->buf[exStruct->head] = data;
			exStruct->head = (exStruct->head + 1) % exStruct->size;
			ringCounter++;
			}
			else
			{
				for(size_t i = 0;i<exStruct->size;i++)
				{
					exStruct->buf[i] = 0;
				}
				exStruct->head=0;
				ringCounter =0;
			}
		}

		else if(ringCounter >= 2 && ringCounter < ringLength-1)
		{
			exStruct->buf[exStruct->head] = data;
			exStruct->head = (exStruct->head + 1) % exStruct->size;
			ringCounter++;
		}

		else
		{
			if(ringCounter == ringLength-1)
			{
				if(data == stopbit)
				{

					exStruct->buf[exStruct->head+1] = data;

					CRC_u8 = 0;
					exStruct->buf[ringLength-1] = 0;
					exStruct->buf[ringLength-1] = crcCalc(rxBufferToFill, sizeof(rxBufferToFill));

					for(size_t i = 0; i<BUFFER_SIZE-1;i++)
					{
						fifoImplemantation(rxBufferToFill, fifoReady, FIFO);
					}

					exStruct->head = 0;
					ringCounter = 0;

				}
				else
				{
					for(size_t i = 0;i<exStruct->size;i++)
					{
						exStruct->buf[i] = 0;
					}
					exStruct->head=0;
					ringCounter =0;
				}
			}
		}


}

uint8_t crcCalc(uint8_t crcArray[], uint8_t size)
{

	for(size_t i = 0; i<size; i++)
	{
		CRC_u8 = crcArray[i] ^ CRC_u8;
	}

	CRC_u8 = CRC_u8 ^ 0x00FF;
	return CRC_u8;
}

void fifoImplemantation(uint8_t *originBuf, uint8_t *fifoBuff, uint8_t *FifoShow)
{
		fifoBuff[0] = originBuf[counter1];
		for(int t = 80;t>=0;t--)
		{
			fifoBuff[t] = fifoBuff[t-1];
		}

		for(size_t j = 0; j<80; j++)
		{
			FifoShow[j] = fifoBuff[j+1];
		}

			if(counter1 == 7)
			{
					fifoBuff[0] = originBuf[counter1+1];
					for(int t = 80;t>=0;t--)
					{
						fifoBuff[t] = fifoBuff[t-1];
					}
					for(size_t j = 0; j<80; j++)
					{
						FifoShow[j] = fifoBuff[j+1];
					}
					counter1 = counter1 + 1;
			}

		acceptedDataCount++;
		acceptedPackageCount=acceptedDataCount/8;

		counter1 = (counter1+1) % 9;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == UART4)
	{
		HAL_UART_Receive_IT(&huart4, &rx_buff, 1);
		circularBufferInsert(&uartRxBuffer, rx_buff);
	}
}

//int circularBufferIsFull(circularBuffer *exStruct)
//{
//	return (exStruct->head + 1) % exStruct->size == exStruct->tail;
//}
//
//int circularBufferIsEmpty(circularBuffer *exStruct)
//{
//	return exStruct->head == exStruct->tail;
//}

//int circularBufferExtract(circularBuffer *exStruct, uint8_t *data)
//{
//	if(!circularBufferIsEmpty(exStruct))
//	{
//		*data = exStruct->buf[exStruct->tail];
//		exStruct->tail = (exStruct->tail + 1) % exStruct->size;
//		return 1;
//	}
//	return 0;
//}
