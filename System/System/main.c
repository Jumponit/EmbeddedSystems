/*
 * main.c
 *
 * Author : Taylor Morris
 */

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
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
volatile int target_temp;

/*
 * If true, box is in service mode
 */
volatile char service_mode;


volatile char * format = "%x";
volatile char celsius = 1;
/*
 * Handles serial I/O
 */
void io_controller(void) {
	Serial_open(0,19200,SERIAL_8N1);
	int command_len = 5;
	int opcode_len = 3;
	char command[command_len];
	char opcode[opcode_len];
	char operand[3];
	char message[64];
	char * str = message;
	char * Fstr;
	while(1) {
		//if we are able to read a command
		if(Serial_read_string(0,command,command_len)) {
			opcode[0] = command[0];
			opcode[1] = command[1];
			/************************************************************************/
			/* Mode-selection commands                                              */
			/************************************************************************/
			if(!strcmp(opcode,"SM")) {//set service mode to true;
				str = "Entering Service Mode\n\r";
				Serial_write_string(0,str,strlen(str));
				service_mode = 1;
			} else if (!strcmp(opcode,"TM")) {//toggle service mode
				service_mode = !service_mode;
				if (service_mode) {
					str = "Entering Service Mode\n\r";
					Serial_write_string(0,str,strlen(str));
				} else {
					str = "Entering Operating Mode\n\r";
					Serial_write_string(0,str,strlen(str));
				}
			} else if (!strcmp(opcode, "OM")) {//set service mode to false
				str = "Entering Operating Mode\n\r";
				Serial_write_string(0,str,strlen(str));
				service_mode = 0;
			} else {
				/************************************************************************/
				/* Mode-specific commands                                               */
				/************************************************************************/
				if(service_mode) {
					//do service mode things
		
				} else {
					operand[0] = command[2];
					operand[1] = 0x00;
					if (command[3] != 0x00)
					{
						operand[1] = command[3];
					}
					//do operating mode things
					if (!strcmp(opcode, "ST")) {
						//set temperature
						target_temp = atoi(operand);
						Fstr = "Set target temperature to %d degrees Celsius\n\r";
						if (sprintf(str,Fstr,target_temp) < 0) {
							str = "Formatting Error\n\r";
						}
						Serial_write_string(0,str,strlen(str));
					} else if (!strcmp(opcode, "SR")) {
						//set sample rate
					} else if (!strcmp(opcode, "SD")) {
						//set display format
						switch (operand[0]) {
							case 'F':
								format = "%d";
								celsius = 0;
								str = "Set format to Fahrenheit\n\r";
								Serial_write_string(0,str,strlen(str));
								break;
							case 'C':
								format = "%d";
								celsius = 1;
								str = "Set format to Celsius\n\r";
								Serial_write_string(0,str,strlen(str));
								break;
							case 'X':
								celsius = 1;
								format="%x";
								str = "Set format to Celsius Hexadecimal\n\r";
								Serial_write_string(0,str,strlen(str));
								break;
							default:
								str = "Unrecognized format\n\r";
								Serial_write_string(0,str,strlen(str));
								break;
						}
					} else {
						str = "Unrecognized command\n\r";
						Serial_write_string(0,str,strlen(str));
					}
				}
			}

		} else {
			str = "Error reading command\n\r";
			Serial_write_string(0,str,strlen(str));
		}
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
