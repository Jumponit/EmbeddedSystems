/*
 * ds18b20.h
 *
 * Created: 3/31/2016 2:49:20 PM
 *  Author: waldonck
 */ 


#ifndef DS18B20_H_
#define DS18B20_H_

void delay_usec(int);
unsigned char ow_reset(void);
unsigned char read_bit();
void write_bit(char);
void write_byte(char);
void read_temperature(void);


#endif /* DS18B20_H_ */