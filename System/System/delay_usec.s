
// Name:    delay_usec
// Desc:    Busy-wait delay of of N + 1 microseconds, 
//          where N is the input parameter. Assumes 16 MHz Atmega2560 processor.
//          Accounts for parameter passing overhead.
// Author:  Frank Barry
// Date:    3/24/16
//
		.section .text
		.global delay_usec
delay_usec:			//when called, takes 2 cycles to load parameter
		mov	r22,r24
		or  r22,r25
		nop
		breq 1f	        // 5 cycles if branch taken, 4 if not
2:
		rcall	1f	// 9 cycles for call/ret
		rjmp	3f      // 2 cycles
3:	
		nop		// 1 cycle
		sbiw	r24,1   // 2 cycles
		brne	2b      // 2 cycles branch taken, 1 cycle not taken
		nop             // 1 (added for prev branch not taken)
1:
		ret