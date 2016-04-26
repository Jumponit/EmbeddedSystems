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
unsigned char ow_read_bit();
void ow_write_bit(char);
void ow_write_byte(char);
int ow_read_temperature(void);
void ow_print_temp(void);

#endif /* DS18B20_H_ */
