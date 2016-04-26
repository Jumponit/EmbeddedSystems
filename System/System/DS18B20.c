/*
 * ds18b20.c
 *
 * Created: 3/31/2016 2:49:39 PM
 *  Author: waldonck
 */ 
#include <avr/io.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ds18b20.h"
#include "serial.h"

/************************************************************************/
/* Attempts to determine whether there is a sensor attached.            */
/************************************************************************/
unsigned char ow_reset(void)
{
	unsigned char presence;

 // pull DQ line low
	DDRE |= (1 << PE4); //set for output
	PORTE &= ~(1 << PE4); //set value low

 // leave it low for 480us
	delay_usec(479);

 // allow line to return high
	PORTE |= (1 << PE4); //set value high
	DDRE &= ~(1 << PE4); //set for input
	//PORTE &= ~(1 << PE4); //set value low?

 // wait for presence
	delay_usec(69);

 // get presence signal
	presence = !(PINE & (1 << PE4)); //read for presence

// wait for end of timeslot
	delay_usec(229); 

	return(presence); // presence signal returned
} // 1=presence, 0=no sensor

/************************************************************************/
/* Read a single bit from the sensor                                    */
/************************************************************************/
unsigned char read_bit(void)
{
	//pull low
	DDRE |= (1 << PE4);
	PORTE &= ~(1 << PE4);
	//let return to high
	PORTE |= (1 << PE4); //set value high
	DDRE &= ~(1 << PE4); //set for input
	
	delay_usec(14); //wait 15us
	
	return(PINE & (1 << PE4));
}

/************************************************************************/
/* Write a single bit                                                   */
/************************************************************************/
void write_bit(char bitval)
{
	//pull low
	DDRE |= (1 << PE4);
	PORTE &= ~(1 << PE4);
	
	//if writing a one, go high
	if (bitval == 1) {
		PORTE |= (1 << PE4); //set value high
	}
	
	delay_usec(59); //wait 60us
	
	PORTE |= (1 << PE4); //set value high
	DDRE &= ~(1 << PE4); //set for input
}

/************************************************************************/
/* Read a single byte                                                   */
/************************************************************************/
unsigned char read_byte(void) 
{
	unsigned char value = 0;
	unsigned char i;
	for (i = 0; i < 8; i++) {
		if (read_bit()) {
			value |= 0x01 << i; //reads a bit and left-shifts it into place
		}
		delay_usec(19); //wait 20us
	}
	return value;
}

void write_byte(char val)
{
	unsigned char i;
	unsigned char temp;
	for (i = 0; i < 8; i++) {
		temp = val >> i;
		temp &= 0x01;
		write_bit(temp);
	}
	
	delay_usec(119); //wait 120us
}

void read_temperature(void)
{
	char get[10];
	char temp_lsb,temp_msb;
	int k;
	char temp_f,temp_c;
	ow_reset();
	write_byte(0xCC); //Skip ROM
	write_byte(0x44); // Start Conversion
	delay_usec(119); //wait 120us
	ow_reset();
	write_byte(0xCC); // Skip ROM
	write_byte(0xBE); // Read Scratch Pad
	for (k=0; k<9; k++) {
		get[k] = read_byte();
	}
	char * message = (char *) malloc(64);
	sprintf(message, "\n\r ScratchPAD DATA = %X%X%X%X%X%X%X%X%X\n\r",get[8],get[7],get[6],get[5],get[4],get[3],get[2],get[1],get[0]);
	serial_write_string(message, strlen(message));
	temp_msb = get[1]; // Sign byte + lsbit
	temp_lsb = get[0]; // Temp data plus lsb
	if (temp_msb <= 0x80){temp_lsb = (temp_lsb/2);} // shift to get whole degree
	temp_msb = temp_msb & 0x80; // mask all but the sign bit
	if (temp_msb >= 0x80) {temp_lsb = (~temp_lsb)+1;} // twos complement
	if (temp_msb >= 0x80) {temp_lsb = (temp_lsb/2);}// shift to get whole degree
	if (temp_msb >= 0x80) {temp_lsb = ((-1)*temp_lsb);} // add sign bit
	sprintf(message, "\n\rTempC= %d degrees C\n\r", (int)temp_lsb ); // print temp. C
	serial_write_string(message, strlen(message));
	temp_c = temp_lsb; // ready for conversion to Fahrenheit
	temp_f = (((int)temp_c)* 9)/5 + 32;
	
	sprintf(message, "\n\rTempF= %d degrees F\n\r", (int)temp_f ); // print temp. F
	serial_write_string(message, strlen(message));
	free(message);
}
