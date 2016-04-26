/*
 * acx_asm.s
 *
 * Created: 2/9/2016
 * Author: Taylor Morris
 */ 
 #define __SFR_OFFSET 0
 #include <avr/io.h>


		.section .text
		.global x_yield
x_yield:
//------------------------------------------------------------------
// Save "callee-save" registers
//------------------------------------------------------------------
main:	
		push	r2
		push	r3
		push	r4
		push	r5
		push	r6
		push	r7
		push	r8
		push	r9
		push	r10
		push	r11
		push	r12
		push	r13
		push	r14
		push	r15
		push	r16
		push	r17
		push	r28
		push	r29

//------------------------------------------------------------------
//   Get thread ID and mask
//------------------------------------------------------------------

		lds		r17, x_thread_id
		lds		r13, x_thread_mask

//------------------------------------------------------------------
// Store SP into current thread's stack-save area
//------------------------------------------------------------------

		//ldi		r17, 0x3	//test

		ldi		r28, lo8(stack)
		ldi		r29, hi8(stack)
		add		r17, r17
		add		r17, r17
		add		r28, r17
		adc		r29, r1
		adiw	Y, 0x2
		cli
		in		r16, SPL
		st		Y+, r16
		in		r16, SPH
		st		Y, r16
		sei

		//This does not work completely
canary_check:	
		ldi		r28, lo8(stack)
		ldi		r29, hi8(stack)
		add		r17, r17
		add		r17, r17
		add		r28, r17
		adc		r29, r1

		ld		r16, Y
		ldi		r17, 0x80
		add		r16, r15

		cpi		r16, 0xAF
		//idk after this

;------------------------------------------------------------------------
; Schedule next thread -- must use caller-save registers below here
;                         because we can reach this without saving regs
;                         if entered from x_new() function
;-------------------------------------------------------------------------
		.global	x_schedule
x_schedule:
	// determine READY status of each thread

		lds		r18, disable_status
		lds		r19, suspend_status
		lds		r20, delay_status
		or		r19, r18
		or		r20, r19

;------------------------------------------------
;   Loop through all threads to test for READY
;------------------------------------------------

		lds		r23, x_thread_id
		ldi		r21, 0x0			//the current thread bit
		ldi		r22, 0x0
		ldi		r18, 0x0
		ldi		r19, 0x7
		cp		r23, r19
		breq	check
		mov		r22, r23
		inc		r22

rotate:	
		clc
		ror		r20
		brcs	rotate_carry
cont:	inc		r21
		dec		r22
		cp		r1, r22
		brne	rotate
		rjmp	check

rotate_carry:
		ori		r20, 128
		rjmp	cont

check:
		mov		r24, r20
		ori		r24, 0xFE
		cpi		r24, 0xFF
		brne	end
		ror		r20
		inc		r21
		cpi		r21, 0x8
		breq	fix
		inc		r18
		cp		r18, r19
		brne	check

original_check:
		mov		r24, r20
		ori		r24, 0xFE
		cpi		r24, 0xFF
		breq	rest

end:
		sts		x_thread_id, r21
		rjmp	restore

fix:
		ldi		r21, 0x0
		inc		r18
		cp		r18, r19
		brne	check
		rjmp		original_check

;----------------------------------------------------------
;  SLEEP HERE:  Here's where we sleep (no threads are READY)
;  but for now we'll jump back to schedule loop again
;----------------------------------------------------------
		; add sleep instructions here...

rest:	sleep
		rjmp	x_schedule			

;---------------------------------------------------
; Restore context of next READY thread
;---------------------------------------------------
restore:
		//	restore SP from thread's sp

		ldi		r30, lo8(stack)
		ldi		r31, hi8(stack)
		add		r21, r21
		add		r21, r21
		add		r30, r21
		adc		r31, r1
		adiw	Z, 0x2
		cli
		ld		r18, Z+
		out		SPL, r18
		ld		r18, Z
		out		SPH, r18
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