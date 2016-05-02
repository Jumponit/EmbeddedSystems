/*
 * Serial.c
 *
 * Created: 3/3/2016 2:24:42 PM
 *  Author: joycemj, taylor morris
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "Queues.h"
#include "acx.h"
#include "Serial.h"

char buffer[QCB_MAX_COUNT][64];

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
	
	regs[port]->ucsra |= (1<<U2X0); //Changed made here
	
	long reg_set = -1;
	
	switch(speed)
	{
		case 2400:
		reg_set = 832;
		break;
		
		case 4800:
		reg_set = 416;
		break;
		
		case 9600:
		reg_set = 207;
		break;
		
		case 14400:
		reg_set = 138;
		break;
		
		case 19200:
		reg_set = 103;
		break;
		
		case 28800:
		reg_set = 68;
		break;
		
		case 38400L:
		reg_set = 51;
		break;
		
		case 57600L:
		reg_set = 34;
		break;
		
		case 76800L:
		reg_set = 25;
		break;
		
		case 115200L:
		reg_set = 16;
		break;
		
		case 230400L:
		reg_set = 8;
		break;
		
		case 250000L:
		reg_set = 7;
		break;
	}
	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		regs[port]->ubrr = reg_set;
		regs[port]->ucsrc = config;
		regs[port]->ucsrb = (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0);
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
}

int Serial_write_string(int port, char * data, int data_length) {
	for (int i = 0; i < data_length; i++) {
		Serial_write(port, data[i]);
	}
	return 1;
}

int Serial_write(int port, char data)
{
	if (Q_putc(ports[port].tx_qid, data))
	{
		//regs[port].ucsrb |= (0x1 << 5); //Commented out line	
		regs[port]->ucsrb |= (1<<UDRIE0); 
		//regs[port].ucsra |= (0x1 << 5); //This might be wrong.
		return 1;	
	}
	return -1;
	
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
		regs[0]->ucsrb &= ~(0x1<<UDRIE0);
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


void serial_open(long speed, int config)
{
   
    UCSR0A |= (1<<U2X0);
    switch(speed)
	{
		case 2400:
		UBRR0 = 832;
		break;
		
		case 4800:
		UBRR0 = 416;
		break;
		
		case 9600:
		UBRR0 = 207;
		break;
		
		case 14400:
		UBRR0 = 138;
		break;
		
		case 19200:
		UBRR0 = 103;
		break;
		
		case 28800:
		UBRR0 = 68;
		break;
		
		case 38400L:
		UBRR0 = 51;
		break;
		
		case 57600L:
		UBRR0 = 34;
		break;
		
		case 76800L:
		UBRR0 = 25;
		break;
		
		case 115200L:
		UBRR0 = 16;
		break;
		
		case 230400L:
		UBRR0 = 8;
		break;
		
		case 250000L:
		UBRR0 = 7;
		break;	
	}
	
	UCSR0C = config;

	//UBRR0H = (UBRR0 >> 8);
	//UBRR0L = (unsigned char)UBRR0;
	//Receiver and Transmitter Enable
	UCSR0B = (1<<TXEN0) | (1<<RXEN0); 

}

char serial_read()
{
	//Contents of the Received Data Buffer Register
	while (!(UCSR0A & (1<<RXC0)))
	{
		//Wait for data to be received. 
		//x_yield();
	}
	return UDR0;
}

void serial_write(char data)
{
	while (!(UCSR0A & (1<<UDRE0)) )
	{
		//Wait for empty transmit buffer
		//x_yield();
	}
	UDR0 = data; 
}


