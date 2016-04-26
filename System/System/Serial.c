/*
 * Serial.c
 *
 * Created: 2/24/2016
 * Author : Taylor Morris
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "acx.h"
#include "Serial.h"
#include "Queues.h"

int Serial_open(int port, long speed, int config);
void Serial_close(int port);
int Serial_available(int port);
int Serial_read(int port);
int Serial_write(int port, char data);

//To hold the buffers.
char buffer[8][64];

//Initialize serial ports.
SERIAL_PORT ports[4] = {
	{0, 0, buffer[0], P0_RX_BUFFER_SIZE, buffer[1], P0_TX_BUFFER_SIZE},
	{0, 0, buffer[2], P1_RX_BUFFER_SIZE, buffer[3], P1_TX_BUFFER_SIZE},
	{0, 0, buffer[4], P2_RX_BUFFER_SIZE, buffer[5], P2_TX_BUFFER_SIZE},
	{0, 0, buffer[6], P3_RX_BUFFER_SIZE, buffer[7], P3_TX_BUFFER_SIZE}
};

SERIAL_PORT_REGS *regs[4] = {
	(SERIAL_PORT_REGS *) 0xC0,
	(SERIAL_PORT_REGS *) 0xC8,
	(SERIAL_PORT_REGS *) 0xD0,
	(SERIAL_PORT_REGS *) 0x130
};


int Serial_open(int port, long speed, int config)
{
	if (port < 0 || port > 3)
	{
		return -1;
	}
	
	ports[port].rx_qid = Q_create(ports[port].rx_bufsize, ports[port].rx_buffer);
	ports[port].tx_qid = Q_create(ports[port].tx_bufsize, ports[port].tx_buffer);
	
	regs[port]->ucsra |= (U2X0 << 1);
	long reg_set;
	switch (speed)
	{
		case 2400:
			reg_set = 832;
		case 4800:
			reg_set = 416;
		case 9600:
			reg_set = 207;
		case 14400:
			reg_set = 138;
		case 19200:
			reg_set = 103;
		case 28800:
			reg_set = 68;
		case 38400:
			reg_set = 51;
		case 57600:
			reg_set = 34;
		case 115200:
			reg_set = 16;
		default:
			return -1;
	}
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		regs[port]->ubrr = reg_set;
		regs[port]->ucsrb = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
		regs[port]->ucsrc = config;
	}
	sei();
	return 0;
}

void Serial_close(int port)
{
	regs[port]->ucsrb = 0;
	Q_delete(ports[port].rx_qid);
	Q_delete(ports[port].tx_qid);
}

int Serial_available(int port)
{
	return Q_used(ports[port].rx_qid);
}

int Serial_read(int port)
{
	char qdata = 0;
	int data;
	
	if (Q_getc(ports[port].rx_qid, &qdata))
	{
		data = qdata;
		return data;
	}
	else
	{
		return -1;
	}
	//regs[port].ucsra |= (0x1 << 7);
	
// 	while(1)
// 	{
// 		if ((UCSR0A | 0x7F) == 0xFF)
// 		{
// 			data = UDR0;
// 			return data;
// 		}
// 	}
}

int Serial_write(int port, char data)
{
	Q_putc(ports[port].tx_qid, data);
	regs[port]->ucsrb |= (0x1 << 5);
	return 1;
	
// 	while(1)
// 	{
// 		if((UCSR0A | 0xDF) == 0xFF)
// 		{
// 			UDR0 = data;
// 		}
// 	}
}

ISR(USART0_UDRE_vect)
{
	char data;
	if (Q_getc(ports[0].tx_qid, &data))
	{
		UDR0 = data;
	}
	else
	{
		regs[0]->ucsrb &= ~(0x1 << 5);
	}
}

ISR(USART1_UDRE_vect)
{
	char data;
	if (Q_getc(ports[1].tx_qid, &data))
	{
		UDR1 = data;
	}
	else
	{
		regs[1]->ucsrb &= ~(0x1 << 5);
	}
}

ISR(USART2_UDRE_vect)
{
	char data;
	if (Q_getc(ports[2].tx_qid, &data))
	{
		UDR2 = data;
	}
	else
	{
		regs[2]->ucsrb &= ~(0x1 << 5);
	}
}

ISR(USART3_UDRE_vect)
{
	char data;
	if (Q_getc(ports[3].tx_qid, &data))
	{
		UDR3 = data;
	}
	else
	{
		regs[3]->ucsrb &= ~(0x1 << 5);
	}
}

ISR(USART0_RX_vect)
{
	Q_putc(ports[0].rx_qid, UDR0);
}

ISR(USART1_RX_vect)
{
	Q_putc(ports[1].rx_qid, UDR1);
}

ISR(USART2_RX_vect)
{
	Q_putc(ports[2].rx_qid, UDR2);
}

ISR(USART3_RX_vect)
{
	Q_putc(ports[3].rx_qid, UDR3);
}