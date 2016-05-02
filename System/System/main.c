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
#include "DS18B20.h"

/*
 * The most-recently measured temperature in degrees Celsius
 */
volatile char last_temp = 0;

/*
 * The most-recently requested temperature for the controlled box,
 * in degrees Celsius
 */
volatile char target_temp;

/*
 * If true, box is in service mode
 */
volatile char service_mode;

/*
 * Handles serial I/O
 */
void io_controller(void) {
	Serial_open(0,19200,SERIAL_8N1);
	char message[64];
	while(1) {
		if(service_mode) {
			//do service mode things
			
		} else {
			//do non-service mode things
			
		}
		
		//Serial_write(0, last_temp);
		Serial_read_string(0, message, 64);
		Serial_write_string(0, message, strlen(message));
		x_delay(1000);
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
		x_delay(1000);
		//_delay_ms(1000);
		PORTB ^= 0x10;
		//x_yield();
	}
}

/*
 * Polls sensor for temperature every second
 */
void sensor_controller(void) {
	//Check for sensor presence
	char presence = ow_reset();
	//keep checking until we detect a sensor
	while (! presence) {
		presence = ow_reset();
		//give other threads a chance to act during this process
		x_yield();
	}
	//monitor temperature
	while(1) {
		last_temp = ow_read_temperature();
		x_delay(250);
	}
}

int main(void)
{
	x_init();
	//Launch main three threads
	x_new(2, io_controller, 1);
	x_new(1, sensor_controller, 1);
	x_new(0, box_controller, 1); //replaces main with box control logic
}
