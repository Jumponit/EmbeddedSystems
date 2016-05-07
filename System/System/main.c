/*
 * main.c
 *
 * Author : Taylor Morris
 */

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "System.h"
#include "Serial.h"
#include "Queues.h"
#include "acx.h"
#include "DS18B20.h"

#define light_bulbs PB5 //Digital pin 11
#define fans PB4 //Digital pin 11
/*
 * The most-recently measured temperature in degrees Celsius
 */
volatile int last_temp = 0;

/*
 * The most-recently requested temperature for the controlled box,
 * in degrees Celsius
 */
volatile int target_temp = 0;

/*
 * If true, box is in service mode
 */
volatile char service_mode;

/*
 * Number of milliseconds to wait between temperature sampling.
 */
volatile unsigned int sample_rate = 5000;

/*
 * The format string used to generate output
 */
//char * format = "Last temp: 0x%x raw hex\n\r";

/*
 * If true, current reporting mode is celsius, otherwise Fahrenheit.
 */
volatile char celsius = 1;

/*
 * The over-temperature set point in celsius
 */
volatile int over_temp = 80;

/*
* Number of seconds allowed to reach target temperature.
*/
volatile int timeout = 60;


void shut_down(void) {
	
	PORTB |= (0x1 << light_bulbs) | (0x1 << fans);
	x_disable(0);
	
}

void start_up(void) {
	PORTB |= ~(0x1 << fans);
	x_enable(0);
}

/*
 * Handles serial I/O
 */
void io_controller(void) {
	Serial_open(0,19200,SERIAL_8N1);
	int command_len = 8;
	int opcode_len = 2;
	int operand_len = 6;
	char command[command_len];
	char opcode[opcode_len];
	char operand[operand_len];
	char message[64];
	char * str;
	char * formatStr;
	char * format = "Last temp: %d degrees Celsius\n\r";
	while(1) {
		//if we are able to read a command
		if(Serial_read_string(0,command,command_len)) {
			opcode[0] = command[0];
			opcode[1] = command[1];
			for (int i = 0; i < operand_len; i++)
			{
				if (command[i+2] != 0x00)
				{
					operand[i] = command[i+2];
				}
				else
				{
					operand[i] = 0x00;
				}
			}
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
					if (!strcmp(opcode, "GT")) {
						//Get temperature
						char fmt_temp = last_temp;
						if (!celsius) {
							//this is equivalent to (9/5)*C + 32
							fmt_temp = ((fmt_temp + (fmt_temp << 3))+160)/5;
						}
						sprintf((char *) message, format, fmt_temp);
						Serial_write_string(0, (char *) message, strlen((char *) message));
					} 
					else if (!strcmp(opcode, "OV")) {
						over_temp = atoi(operand);
						formatStr = "Over-temperature set to %d degrees Celsius\n\r";
						sprintf((char *) message,formatStr,over_temp);
						Serial_write_string(0, (char *) message, strlen((char *) message));
					} 
					else if (!strcmp(opcode, "SO")) {
						timeout = operand[0] * 60;
						formatStr = "Timeout set to %d seconds\n\r";
						sprintf((char *) message,formatStr,timeout);
						Serial_write_string(0, (char *) message, strlen((char *) message));
					}
					else {
						str = "Unrecognized command\n\r";
						Serial_write_string(0,str,strlen(str));
					}
				} else {
					//do operating mode things
					if (!strcmp(opcode, "ST")) {
						//set temperature
						target_temp = atoi(operand);
						if (target_temp < 0 || target_temp > 125) {
							str = "Invalid temperature selection. Sucks to suck.\n\r";
							Serial_write_string(0,str,strlen(str));
						} else {
							formatStr = "Set target temperature to %d degrees Celsius\n\r";
							sprintf((char *) message,formatStr,target_temp);
							Serial_write_string(0, (char *) message, strlen((char *) message));
						}	
					} else if (!strcmp(opcode, "SR")) {
						//set sample rate
						sample_rate = atoi(operand);
						formatStr = "Set sample rate to %u\n\r";
						sprintf((char *) message,formatStr,sample_rate);
						Serial_write_string(0, (char *) message, strlen((char *) message));
					} else if (!strcmp(opcode, "SD")) {
						//set display format
						switch (operand[0]) {
							case 'F':
								format = "Last temp: %d degrees Fahrenheit\n\r";
								celsius = 0;
								str = "Set format to Fahrenheit\n\r";
								Serial_write_string(0,str,strlen(str));
								break;
							case 'C':
								format = "Last temp: %d degrees Celsius\n\r";
								celsius = 1;
								str = "Set format to Celsius\n\r";
								Serial_write_string(0,str,strlen(str));
								break;
							case 'X':
								celsius = 1;
								format="Last temp: %x raw hex";
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
 *
 * Fans on by default
 */
void box_controller(void) {
	
	//PORTB |= 0x1 << PB4;
	DDRB |= (0x1 << light_bulbs) | (0x1 << fans);	
	PORTB &= ~(0x1 << fans);
	while(1) {
		if (last_temp >= over_temp) {
			shut_down();
		} else if (last_temp < target_temp) {
			PORTB &= ~(0x1 << light_bulbs);
		} else {
			PORTB |= (0x1 << light_bulbs);
		}
		x_delay(sample_rate);
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
		x_delay(sample_rate);
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
