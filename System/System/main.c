/*
 * main.c
 *
 * Author : Taylor Morris
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include "System.h"
#include "Serial.h"
#include "Queues.h"
#include "acx.h"

/*
 * The most-recently measured temperature in degrees Celsius
 */
volatile char last_temp;

/*
 * The most-recently requested temperature for the controlled box,
 * in degrees Celsius
 */
volatile char target_temp;

/*
 * Handles serial I/O
 */
void io_controller(void) {
	Serial_open(0,19200,SERIAL_8N1);
	//char * message = "H";
	while(1) {
		Serial_write(0, 'h');
		x_yield();
	}
}

/*
 * Controller for the box
 */
void box_controller(void) {
	//TODO: blink LED
	DDRB |= 0x1 << PB4;
	//PORTB |= 0x1 << PB4;
	while(1) {
		//x_delay(1000);
		_delay_ms(1000);
		PORTB ^= 0x10;
		x_yield();
	}
}

/*
 * Polls sensor for temperature every second
 */
void sensor_controller(void) {
	x_yield();
}

int main(void)
{
	x_init();
	//Launch main three threads
	x_new(2, io_controller, 1);
	x_new(1, sensor_controller, 1);
	x_new(0, box_controller, 1); //replaces main with box control logic*/
	
}

