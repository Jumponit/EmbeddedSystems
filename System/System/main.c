/*
 * main.c
 *
 * Author : Taylor Morris
 */ 

#include <avr/io.h>
#include "Serial.h"
#include "Queues.h"


int main(void)
{
	char dataPlace;
	char * array = &dataPlace;
	char data = 'a';
	char putdata;
	
	//Serial_open(0, 9600, SERIAL_8N1);
	//Serial_write(0, data);
	//data = Serial_read(0);
	Q_create(8, array);
	
    /* Replace with your application code */
    while (1) 
    {
		Q_getc(0, &putdata);
		Q_putc(0, data);
		Q_getc(0, &putdata);
		//Serial_write(0, data);
		//data = Serial_read(0);
    }
}

