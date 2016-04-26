/*
 * acx.c
 *
 * Created: 2/1/2016
 * Author : Taylor Morris
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "acx.h"


unsigned long absolute_tick_counter;
unsigned long global_timer;
byte x_thread_id;
byte x_thread_mask;
byte disable_status;
byte suspend_status;
byte delay_status;
stack_pointer stack[8];
volatile unsigned int x_thread_delay[8];

void x_init()
{
	disable_status = 0xFE;
	suspend_status = 0x00;
	delay_status = 0x00;
	x_thread_id = 0;
	x_thread_mask = 0x01;
	absolute_tick_counter = 0;
	global_timer = 0;
	
	for (int i = 0; i < 8; i++)
	{
		stack[i].pbase = MEM_BASE - (i * STACK_SIZE);
		stack[i].pstack = stack[i].pbase;
	}
	for (int i = 0; i < 8; i++)
	{
		*(stack[i].pbase - 127) = 0xAF;
	}
	stack[0].pstack = SP;
}

void x_new(byte tid, PTHREAD pthread, byte isDisabled)
{
	byte low = pthread;
	byte high = ((int)pthread >> 8);
	stack[tid].pstack = stack[tid].pbase;
	*(stack[tid].pstack) = low;
	*(stack[tid].pstack - 1) = high;
	stack[tid].pstack -= 21;
	if (isDisabled == 1)
	{
		disable_status = disable_status | (1 << tid);
	}
	else if (isDisabled == 0)
	{
		disable_status = disable_status & (0xFF - (1 << tid));
	}
}

void x_delay(unsigned int ticks)
{	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		delay_status = delay_status | (0x1 << x_thread_id);
		x_thread_delay[x_thread_id] = ticks;
	}
	x_yield();
}

void x_suspend(unsigned int tid)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		suspend_status = suspend_status | (0x1 << tid);
	}
}

void x_resume(unsigned int tid)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		suspend_status = suspend_status & (~(0x1 << tid));
	}
}

void x_disable(unsigned int tid)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		disable_status = disable_status | (0x1 << tid);
	}
}

void x_enable(unsigned int tid)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		disable_status = disable_status & (~(0x1 << tid));
	}
}

unsigned long gtime()
{
	return absolute_tick_counter;
}

byte x_getID()
{
	return x_thread_id;
}

ISR(TIMER0_COMPA_vect)
{
	absolute_tick_counter++;
	for (int i = 0; i < 7; i++)
	{
		if (x_thread_delay[i] > 0)
		{
			x_thread_delay[i]--;
			if (x_thread_delay[i] == 0)
			{
				delay_status = delay_status & (~(0x1 << i));
			}
		}
	}
}