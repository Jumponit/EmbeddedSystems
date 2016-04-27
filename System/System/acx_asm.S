
/*
 * Kernel_SP13_asm.s
 *
 *  Created: 4/11/2013 11:09:06 AM
 *  Author: E. Frank Barry
 */ 
 #define __SFR_OFFSET 0
 #include <avr/io.h>
 #include "acx.h"


		.section .text
		.global x_yield
x_yield:
	// Save "callee-save" registers
		push  r2
		push  r3
		push  r4
		push  r5
		push  r6
		push  r7
		push  r8
		push  r9
		push  r10
		push  r11
		push  r12
		push  r13
		push  r14
		push  r15
		push  r16
		push  r17
		push  r28
		push  r29

//------------------------------------------------------------------
//   Get thread ID and mask
//------------------------------------------------------------------
		lds   r18,x_thread_id
		lds   r19,x_thread_mask

//------------------------------------------------------------------
// Store SP into current thread's stack-save area
//------------------------------------------------------------------
		cli					;disable interrupts
		in		r14,SPL
		in		r15,SPH
		sei					;re-enable interrupts--assumes that interrupts must be enabled--does not save state of flags


		ldi		r30,lo8(stack)
		ldi		r31,hi8(stack)
		lsl		r18
		lsl		r18
		add		r30,r18
		adc		r31,r1  ;z = stacks+(id*4) to give pointer to stacks[thread_id].ps
		st		z+,r14	;save SPL
		st		z,r15	;save SPH

;------------------------------------------------------------------------
; Schedule next thread -- must use caller save registers below here
;                         because we can reach this without saving regs
;                         if reached from k_new() function
;-------------------------------------------------------------------------
		.global	x_schedule
x_schedule:
	// determine READY status of each thread
		lds		r18,x_disable_status
		lds		r19,x_delay_status
		lds		r20,x_suspend_status
		or		r18,r19
		or		r18,r20
		lds		r19, x_thread_id		;get current thread
		lds		r20, x_thread_mask
		ldi		r22,NUM_THREADS		;max number of threads
		clc							;make sure Carry flag is clear
;------------------------------------------------
;   Loop through all threads to test for READY
;------------------------------------------------
1:
		inc		r19					;inc to next thread id...
		andi	r19,7				;...modulo 8
		rol		r20					;shift mask left
		brcc	2f					;skip next instruction if C is clear
		rol		r20					;perform second rotate if bit7 was set (to rotate through Carry flag)
2:
		mov		r23,r20				;save copy of mask
		and		r20,r18				;test bit corresponding to status of next thread
		breq	restore				;break out if READY
		dec		r22					;decrement thread count
		brne	1b					;back to test next thread
;----------------------------------------------------------
;  SLEEP HERE:  Here's where we sleep (no threads are READY)
;  but for now we'll jump back to schedule loop again
;----------------------------------------------------------
		; add sleep instructions here...

		rjmp	x_schedule			

;---------------------------------------------------
; Restore context of next READY thread
;---------------------------------------------------
restore:
		sts		x_thread_id,r19
		sts		x_thread_mask,r23

		ldi		r30,lo8(stack)
		ldi		r31,hi8(stack)
		lsl		r19
		lsl		r19
		add		r30,r19
		adc		r31,r1	;z = stacks+(id*4) to give pointer to stacks[thread_id].sp
		ld		r18,z+	;get SPL
		ld		r19,z	;get SPH
		
		cli
		out		SPL,r18		;restore SP
		out		SPH,r19
		sei


		// Restore registers
		pop		r29
		pop		r28
		pop		r17
		pop		r16
		pop		r15
		pop		r14
		pop		r13
		pop		r12
		pop		r11
		pop		r10
		pop		r9
		pop		r8
		pop		r7
		pop		r6
		pop		r5
		pop		r4
		pop		r3
		pop		r2

		// Return to next thread
		ret

//----------------------------------------------------------------------------------------------
//  Function: bit2mask8
//
//  Description: Returns a bit mask corresponding to thread ID parameter.
//
//  Input params:  byte id  -- must by integer between 0 and 7
//  Returns:       mask - a bit mask having a single bit set corresponding to input ID
//						  (interpreted as bit number)
//-----------------------------------------------------------------------------------------------
		.global bit2mask8
bit2mask8:
		;r24 containts thread ID input
		ldi r30,lo8(bitmask8_table)
		ldi r31,hi8(bitmask8_table)
		add r30,r24
		adc r31,r1		;r1 always has 0 in it--so just add carry
		lpm r24,Z		;lpm accesses program memory (FLASH) indirectly using Z reg pair (r31:r30)
		ret
//
// Put table of mask values in .text section and use LPM instruction to access...
//
		.text
bitmask8_table:
		.byte 0x01
		.byte 0x02
		.byte 0x04
		.byte 0x08
		.byte 0x10
		.byte 0x20
		.byte 0x40
		.byte 0x80
