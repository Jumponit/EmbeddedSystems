/*
 * Queues.c
 *
 * Author : Taylor Morris
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <stdbool.h>
#include "System.h"
#include "Queues.h"

byte Q_putc(byte qid, char data);
byte Q_getc(byte qid, char *pdata );
uint8_t Q_create(int qsize, char * pbuffer);
void Q_delete(byte qid);
int Q_used(byte qid);
int Q_unused(byte qid);

QCB queues[QCB_MAX_COUNT];
bool occupied[8] = {false, false, false, false, false, false, false, false};

byte Q_putc(byte qid, char data)
{
	QCB *qcb = &queues[qid];
	if (qcb->flags != 1)
	{
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			*(qcb->pQ + qcb->in) = data;
			qcb->available += 1;
			if (qcb->flags == 2)
			{
				qcb->flags = 0;
			}
			
			if (((qcb->in + 1) & qcb->smask) != qcb->out)
			{
				qcb->in = (qcb->in + 1) & qcb->smask;
			}
			else
			{
				qcb->in = (qcb->in + 1) & qcb->smask;
				qcb->flags = 1;
			}
		}
		return 1;
	}
	else
	{
		return 0;
	}
}

byte Q_getc(byte qid, char *pdata)
{
	QCB *qcb = &queues[qid];
	if (qcb->flags != 2)
	{
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			*pdata = *(qcb->pQ + qcb->out);
			qcb->available -= 1;
			if (qcb->flags == 1)
			{
				qcb->flags = 0;
			}
			
			if (((qcb->out + 1) & qcb->smask) != qcb->in)
			{
				qcb->out = (qcb->out + 1) & qcb->smask;
			}
			else
			{
				qcb->out = (qcb->out + 1) & qcb->smask;
				qcb->flags = 2;
			}
		}
		return 1;
	}
	return 0;
}

uint8_t Q_create(int qsize, char * pbuffer)
{
	if ((qsize <= 0) || (qsize > 256) || (qsize & (qsize - 1)) != 0)
	{
		return -1;
	}
	
	for (int i = 0; i < QCB_MAX_COUNT; i++)
	{
		if (occupied[i] == false)
		{
			queues[i].in = 0;
			queues[i].out = 0;
			queues[i].smask = qsize - 1;
			queues[i].flags = 2;
			queues[i].available = 0;
			queues[i].pQ = pbuffer;
			occupied[i] = true;
			return i;
		}
	}
	return -1;
}

void Q_delete(byte qid)
{
	queues[qid].in = 0;
	queues[qid].out = 0;
	queues[qid].smask = 0;
	queues[qid].flags = 0;
	queues[qid].available = 0;
	queues[qid].pQ = NULL;
	occupied[qid] = false;
}

int Q_used(byte qid)
{
	return queues[qid].available;
}

int Q_unused(byte qid)
{
	if (qid >= QCB_MAX_COUNT)
	{
		return -1;
	}
	
	if (queues[qid].out > queues[qid].in)
	{
		return (queues[qid].out - queues[qid].in);
	}
	else if (queues[qid].in > queues[qid].out)
	{
		return ((queues[qid].smask + 1) - (queues[qid].in - queues[qid].out));
	}
	else
	{
		return 0;
	}
}
