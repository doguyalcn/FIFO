/*
 * advanced.h
 *
 *  Created on: Jul 26, 2023
 *      Author: Doğu
 */

#ifndef INC_ADVANCED_H_
#define INC_ADVANCED_H_

#include "stm32f4xx_hal.h"
#include "usart.h"
#include "stdbool.h"

#define BUFFER_SIZE 9							//  | STARTBIT | MESSAGELENGTH | DATA1 | DATA2 | DATA3 | DATA4 | DATA5 | CRC | STOPBIT |

extern uint8_t rx_buff;							//oneBitDataValue; The value in which the information from UART4 will be stored one by one

extern uint8_t rxBufferToFill[BUFFER_SIZE];		//RX BUFFER; This buffer contains ring buffer data
//extern uint8_t txBufferToFill[BUFFER_SIZE];		//NOT USED...

extern uint8_t fifoReady[81];					//README.TXT
extern uint8_t FIFO[80];			//README.TXT

extern uint8_t CRC_u8;

extern int acceptedDataCount;							//DATA COUNT
extern int acceptedPackageCount;						//PACKAGE COUNT

typedef struct									//STRUCT
{
	uint8_t *buf;
	uint8_t head;
	uint8_t tail;
	uint8_t counter;
	uint8_t size;

} circularBuffer;

extern circularBuffer uartRxBuffer;			//STRUCT1
//extern circularBuffer uartTxBuffer;			//STRUCT2 NOT USED

void circularBufferInit(circularBuffer *exStruct, uint8_t *structBuffer); // INITIALIZES STRUCT

int circularBufferIsFull(circularBuffer *exStruct); // IS STRUCT BUFFER FULL? - NOT USED

int circularBufferIsEmpty(circularBuffer *exStruct); //IS STRUCT BUFFER EMPTY? - NOT USED

void circularBufferInsert(circularBuffer *exStruct, uint8_t data); // INSERT UART4 DATA TO BUFFER

void circularBufferExtract(circularBuffer *exStruct, uint8_t *data); //NOT COMPLETED - NOT USED

void fifoImplemantation(uint8_t *originBuf, uint8_t *fifoBuff, uint8_t *fifoShow);	// FIFO FILL

//void fifoShow(uint8_t *fifoBuff, uint8_t *fifoBuffShow);

uint8_t crcCalc(uint8_t crcArray[], uint8_t size);	//CRC CALCULATION

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);	// CALL-BACK FUNCTION

#endif /* INC_ADVANCED_H_ */
