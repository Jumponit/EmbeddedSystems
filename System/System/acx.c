/*
 * acx.c
 *
 *  Created: 3/20/2014 11:34:55 AM
 *  Author: E. Frank Barry
 *  
 *  
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "System.h"
#include "acx.h"


//---------------------------------------------------
// Stack Control 
//---------------------------------------------------
STACK_CONTROL stack[MAX_THREADS];

//---------------------------------------------------
// Stack Memory
//---------------------------------------------------
byte x_thread_stacks[STACK_MEM_SIZE];

//---------------------------------------------------
// Thread Delay Counters
//---------------------------------------------------
unsigned int x_thread_delay[MAX_THREADS];
unsigned long x_system_counter = 0;

//---------------------------------------------------
// Exec State Variables
//---------------------------------------------------
byte x_thread_id;
byte x_thread_mask;
byte x_disable_status;
byte x_suspend_status;
byte x_delay_status;

//---------------------------------------------------
// Local Functions
//---------------------------------------------------
void init_System_Timer(void);


//---------------------------------------------------
// ACX Functions
//---------------------------------------------------
void x_init(void)
{
	cli(); //Disable interrupts
	
	// Initialize thread status variables
	x_disable_status = 0xfe;  // disable all threads except thread 0
	x_suspend_status = 0x00;  // not suspended...
	x_delay_status = 0x00;  // and not delayed
	x_thread_id = 0;        // start as thread 0
	x_thread_mask = 0x01;
	
	// Initialize Stacks
	stack[T0_ID].sp = x_thread_stacks + T0_STACK_BASE_OFFS;
	stack[T0_ID].spBase = x_thread_stacks + T0_STACK_BASE_OFFS;
	stack[T1_ID].sp = x_thread_stacks + T1_STACK_BASE_OFFS;
	stack[T1_ID].spBase = x_thread_stacks + T1_STACK_BASE_OFFS;
	stack[T2_ID].sp = x_thread_stacks + T2_STACK_BASE_OFFS;
	stack[T2_ID].spBase = x_thread_stacks + T2_STACK_BASE_OFFS;
	stack[T3_ID].sp = x_thread_stacks + T3_STACK_BASE_OFFS;
	stack[T3_ID].spBase = x_thread_stacks + T3_STACK_BASE_OFFS;
	stack[T4_ID].sp = x_thread_stacks + T4_STACK_BASE_OFFS;
	stack[T4_ID].spBase = x_thread_stacks + T4_STACK_BASE_OFFS;
	stack[T5_ID].sp = x_thread_stacks + T5_STACK_BASE_OFFS;
	stack[T5_ID].spBase = x_thread_stacks + T5_STACK_BASE_OFFS;
	stack[T6_ID].sp = x_thread_stacks + T6_STACK_BASE_OFFS;
	stack[T6_ID].spBase = x_thread_stacks + T6_STACK_BASE_OFFS;
	stack[T7_ID].sp = x_thread_stacks + T7_STACK_BASE_OFFS;
	stack[T7_ID].spBase = x_thread_stacks + T7_STACK_BASE_OFFS;
	
	// Put "canary" values at each stack boundary (low end)
	// to allow debug detection of stack overflow
	x_thread_stacks[0] = STACK_CANARY;
	x_thread_stacks[T0_STACK_BASE_OFFS+1] = STACK_CANARY;
	x_thread_stacks[T1_STACK_BASE_OFFS+1] = STACK_CANARY;
	x_thread_stacks[T2_STACK_BASE_OFFS+1] = STACK_CANARY;
	x_thread_stacks[T3_STACK_BASE_OFFS+1] = STACK_CANARY;
	x_thread_stacks[T4_STACK_BASE_OFFS+1] = STACK_CANARY;
	x_thread_stacks[T5_STACK_BASE_OFFS+1] = STACK_CANARY;
	x_thread_stacks[T6_STACK_BASE_OFFS+1] = STACK_CANARY;
	

	// Initialize System Timer
	init_System_Timer();


	// Copy return address to Thread 0 stack area
	//
	// NOTE: This works for Atmega2560 having 3-byte return addresses
	//       Does not work for parts having two byte return addresses
	byte *ps = (byte *)SP;
	stack[T0_ID].sp[0] = ps[5];   // copy low byte of ret address to T0 stack
	stack[T0_ID].sp[-1] = ps[4];  // copy mid byte of ret address to T0 stack
	stack[T0_ID].sp[-2] = ps[3];  // copy high byte of ret address to T0 stack
	stack[T0_ID].sp[-3] = ps[2];  // pushed reg
	stack[T0_ID].sp[-4] = ps[1];  // pushed reg
	
	// Update hardware SP
	SP = (int)(stack[T0_ID].sp-5);
	
	sei(); // Enable interrupts
	
	// return to caller.

}

/*--------------------------------------------------------------------------------------
   Function:       x_new

   Description:    Creates a new thread by associating a function pointer with a 
                   specified thread ID and stack. The thread may be created in 
                   either an enabled or disabled state as specified by the isEnabled
                   parameter. A thread may replace itself by another thread (or restart itself) by
                   specifying its own ID for this function. If ID ofthe calling thread is specified
                   then this function does not return to the caller but jumps to the kernel scheduler. 
                   In this case the new (replacement) thread will be scheduled according to its
                   status.

   Input:          int tid - the ID of the thread to be created/replaced (a number between 0 and 7)
                   PTHREAD pthread - a pointer to a function (PTHREAD type - see kernel.h)
                   byte isEnabled - 1=thread initially enabled, 0=thread initially disabled.

   Returns:        none
   

---------------------------------------------------------------------------------------*/

void x_new(byte tid, PTHREAD pthread, byte isEnabled)
{
   PTU u; //a union that gives access to a thread pointer a byte at a time
     
   // Get stack base pointer for this thread ID
   byte *psb = stack[tid].spBase;
   
   // Copy return address of thread onto its stack
   u.pthread = pthread;
   *psb-- = u.addr[0];
   *psb-- = u.addr[1];
   *psb-- = u.addr[2];   // Used for 2560 -- 3-byte return address
   psb -= THREAD_CONTEXT_SIZE;

   // Save new stack pointer in this thread's stack pointer save area.
   // If the current thread is being replaced, this new stack pointer
   // will take effect after rescheduling (below)
   stack[tid].sp = psb;

   byte tmask = bit2mask8(tid);
   if(isEnabled){
      x_disable_status &= ~tmask;
   }
   else {
      x_disable_status |= tmask;
   }
   
   //-------------------------------------------------------------------
   // If this is not the current thread just return to calling thread   
   //-------------------------------------------------------------------
   if(tid != x_getTID()){
      return;      
   }
   
   //-------------------------------------------------------------------
   // If the calling thread is being replaced/reinitialized
   // then go to rescheduling loop. 
   // NOTE: A call is OK here--modifying the stack below where
   // it has just been initialized 
   //-------------------------------------------------------------------
   x_schedule();
   
}


//-------------------------------------------------------------
// init_System_Timer
//
//     Sets system timer (Timer 0) for system tick interval
//     Start with 2 msec tick
//-------------------------------------------------------------
void init_System_Timer(void)
{
	// DDRB |= 0x20;  // temporary for testing--PB.5 is output
	
	TCCR0A = 0x02;	// Mode 2 - CTC
	OCR0A = 250;    // match count to give 1 msec 
	TIMSK0 = 2;		// enable OCR0A compare match interrupt
	TCCR0B = 0x03;		// Start timer (clkIO/64 prescaler)
	
	return;
}

/*--------------------------------------------------------------------------------------
   Function:       x_delay

   Description:    Delays the calling thread by the specified number of system "ticks" 
                   by loading the thread's delay value and setting its delay status bit
                   to '1'. The kernel timer decrements the delay values each system tick
                   tick and clears the delay status bits of any that reach zero.
   

   Input:          none

   Returns:        Does not return, but enters scheduling loop.
   

---------------------------------------------------------------------------------------*/
void x_delay(int ticks)
{
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
   {
      x_thread_delay[x_getTID()] = ticks;
      x_delay_status |= x_thread_mask;
   }
   x_yield();
}

/*--------------------------------------------------------------------------------------
   Function:       x_gtime

   Description:    Returns current value of the 'k_longtime' system tick counter.
                   Access to the system tick counter must be atomic since it involves
                   reading a multi-byte value that is written by an interrupt handler.
                   The k_longtime counter is incremented each system tick. The 32-bit
                   counter will overflow after (2**32) * (tick_period) seconds. For 
                   example, if the tick interval is 1msec, then it will overflow in
                   about 49 days. Ifthis kind of overflow will affect thread timing,
                   it is the responsibility of each thread to check the value returned
                   to make sure that if overflow occurs, the timing of the thread 
                   does not cause a failure.

   Input:          none
                  

   Returns:        long time - a copy of the system tick counter (32 bits)
   

---------------------------------------------------------------------------------------*/
unsigned long x_gtime(){
   long val;
   ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
      val = x_system_counter;
   }
   return val;
}
/*--------------------------------------------------------------------------------------
   Interrupt Service Routine:   TIMER0_COMPA_vect


   Description: This interrupt is triggered every N msec based on TIMER0 COMPARE MATCH.
                The ISR decrements the first NUM_THREADS delay values and resets 
                (to zero) the k_delay_status bits if each thread whose counter reaches
                zero. If a thread is delayed and its counter reaches zero, then it
                is made READY and may be scheduled to run.

----------------------------------------------------------------------------------------*/
ISR(TIMER0_COMPA_vect)
{

   // Increment system counter
   x_system_counter++;

   char msk = 0x01;  // start with thread 0

   unsigned int *pdelay = x_thread_delay;
   //decrement delays
   for(char i = 0; i < NUM_THREADS; i++){
      (*pdelay)--;
      if(*pdelay == 0){
         x_delay_status &= ~msk;
      }
      msk <<= 1;
      pdelay++;
   }
   
}
/*--------------------------------------------------------------------------------------
   Function:       x_suspend

   Description:    Set specified thread's suspend status bit

   Input:         tid - thread ID
   
   Returns:        none
   

---------------------------------------------------------------------------------------*/
void x_suspend(byte tid)
{
	x_suspend_status |= (1 << tid);
}
/*--------------------------------------------------------------------------------------
   Function:       x_resume

   Description:    Clears specified thread's suspend status bit

   Input:         tid - thread ID
   
   Returns:        none
   

---------------------------------------------------------------------------------------*/
void x_resume(byte tid)
{
	x_suspend_status &= ~(1 << tid);
}
/*--------------------------------------------------------------------------------------
   Function:       x_disable

   Description:    Set specified thread's DISABLE status bit

   Input:         tid - thread ID
   
   Returns:        none
   

---------------------------------------------------------------------------------------*/
void x_disable(byte tid)
{
	x_disable_status |= (1 << tid);
}
/*--------------------------------------------------------------------------------------
   Function:       x_enable

   Description:    Clears specified thread's DISABLE status bit

   Input:         tid - thread ID
   
   Returns:        none
   

---------------------------------------------------------------------------------------*/
void x_enable(byte tid)
{
	x_disable_status &= ~(1 << tid);
}

