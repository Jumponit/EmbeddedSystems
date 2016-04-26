/*
 * acx.h
 *
 * Created: 2/1/2016
 * Author : Taylor Morris
 */ 

/*
 * thread 0 base = 0x21ff
 * thread 1 base = 0x217f
 * thread 2 base = 0x20ff
 * thread 3 base = 0x207f
 * thread 4 base = 0x1fff
 * thread 5 base = 0x1f7f
 * thread 6 base = 0x1eff
 * thread 7 base = 0x1e7f
 */

#define		x_getTID()		(x_thread_id)
#define		MAX_THREADS		8
#define		NUM_THREADS		8
#define		STACK_SIZE		128
#define		MEM_BASE		0x21FF
typedef		uint8_t			byte;
typedef		uint8_t			bool;
typedef 	void			(*PTHREAD)(void);
typedef struct  
{
	byte *pbase;
	byte *pstack;
}stack_pointer;	
typedef union
{
	byte low;
	byte high;
}access;