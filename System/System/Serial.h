/*
 * Serial.h
 *
 * Created: 3/3/2016 2:25:08 PM
 *  Author: joycemj
 */ 

#ifndef SERIAL_H_
#define SERIAL_H_
#define SERIAL_5N1 0x00
#define SERIAL_6N1 0x02
#define SERIAL_7N1 0x04
#define SERIAL_8N1 0x06
#define SERIAL_5N2 0x08
#define SERIAL_6N2 0x0A
#define SERIAL_7N2 0x0C
#define SERIAL_8N2 0x0E
#define SERIAL_5E1 0x20
#define SERIAL_6E1 0x22
#define SERIAL_7E1 0x24
#define SERIAL_8E1 0x26 
#define SERIAL_5E2 0x28
#define SERIAL_6E2 0x2A
#define SERIAL_7E2 0x2C
#define SERIAL_8E2 0x2E
#define SERIAL_5O1 0x30
#define SERIAL_6O1 0x32
#define SERIAL_7O1 0x34
#define SERIAL_8O1 0x36
#define SERIAL_5O2 0x38
#define SERIAL_6O2 0x3A
#define SERIAL_7O2 0x3C
#define SERIAL_8O2 0x3E

typedef struct {
	volatile uint8_t ucsra;
	volatile uint8_t ucsrb;
	volatile uint8_t ucsrc;
	volatile uint8_t reserved;
	volatile uint16_t ubrr;
	volatile uint8_t udr;
}SERIAL_PORT_REGS;

typedef struct {
	uint8_t rx_qid;
	uint8_t tx_qid;
	char *rx_buffer;
	int rx_bufsize;
	char *tx_buffer;
	int tx_bufsize;
}SERIAL_PORT;


#define P0_RX_BUFFER_SIZE   64
#define P0_TX_BUFFER_SIZE   64
#define P1_RX_BUFFER_SIZE   32
#define P1_TX_BUFFER_SIZE   32
#define P2_RX_BUFFER_SIZE   32
#define P2_TX_BUFFER_SIZE   32
#define P3_RX_BUFFER_SIZE   32
#define P3_TX_BUFFER_SIZE   32


void serial_open(long speed, int config);
char serial_read();
void serial_write(char data);

int Serial_open(int port, long speed, int config);
void Serial_close(int);
void Serial_config(int, long, int);
int Serial_available(int);
int Serial_read(int);
int Serial_write(int, char);
void Serial0_config(long,int);
char Serial0_poll_read();
void Serial0_poll_write(char);
void Serial0_poll_print(char *);
int Serial_write_string(int port, char * data, int data_length);
int Serial_read_string(int port, char * data, int data_length);
#endif /* SERIAL_H_ */