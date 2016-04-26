/*
 * Queues.h
 *	Defines constants, types and function prototypes for general purpose
 *  FIFO (byte-oriented) queue functions.
 *
 * Created: 3/2/2015 3:32:56 PM
 *  Author: barryef
 */ 


#ifndef QUEUES_H_
#define QUEUES_H_

#include "System.h"

//
// Queue Control Block
//
#define		QCB_MAX_COUNT  	8	// defines maximum number of system queues.

#define		Q_FULL	0			// bit position of FULL status bit in QCB flags byte
#define		Q_EMPTY	1			// bit position of EMPTY status bit in QCB flags byte

//
//   Defined System Queues
//
#define		SERIAL0_IN_Q		0    // ID of USART0 serial inbound queue
#define		SERIAL0_OUT_Q		1    // ID of USART0 serial outbound queue

//
//   Queue Control Block Type
//
typedef struct {
    byte in;            // index of next char to be placed in queue (if not full)
    byte out;           // index of next char to be removed from queue (if not empty)
    byte smask;         // mask used to maintain circular queue access (mod size)
    byte flags;         // stores full and empty flags
    int available;      // number of bytes available to be read from queue
    char *pQ;           // pointer to queue data buffer
} QCB;


//
// Function Prototypes
//
byte Q_putc(byte , char);
byte Q_getc(byte , char * );
uint8_t Q_create(int , char * );
void Q_delete(byte);
int Q_used(byte);
int Q_unused(byte);

#endif /* QUEUES_H_ */