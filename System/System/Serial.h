/*
 * Serial.h
 *
 * Created: 2/19/2015 3:48:34 PM
 *  Author: EFB
 */ 


#ifndef SERIAL_H_
#define SERIAL_H_

#define DATABITS 1
#define STOPBITS 3
#define PARITYBITS 4

#define SERIAL_5N1  (0x00 | (0 << DATABITS))
#define SERIAL_6N1  (0x00 | (1 << DATABITS))
#define SERIAL_7N1  (0x00 | (2 << DATABITS))
#define SERIAL_8N1  (0x00 | (3 << DATABITS))     // (the default)
#define SERIAL_5N2  (0x08 | (0 << DATABITS))
#define SERIAL_6N2  (0x08 | (1 << DATABITS))
#define SERIAL_7N2  (0x08 | (2 << DATABITS))
#define SERIAL_8N2  (0x08 | (3 << DATABITS))
#define SERIAL_5E1  (0x20 | (0 << DATABITS))
#define SERIAL_6E1  (0x20 | (1 << DATABITS))
#define SERIAL_7E1  (0x20 | (2 << DATABITS))
#define SERIAL_8E1  (0x20 | (3 << DATABITS))
#define SERIAL_5E2  (0x28 | (0 << DATABITS))
#define SERIAL_6E2  (0x28 | (1 << DATABITS))
#define SERIAL_7E2  (0x28 | (2 << DATABITS))
#define SERIAL_8E2  (0x28 | (3 << DATABITS))
#define SERIAL_5O1  (0x30 | (0 << DATABITS))
#define SERIAL_6O1  (0x30 | (1 << DATABITS))
#define SERIAL_7O1  (0x30 | (2 << DATABITS))
#define SERIAL_8O1  (0x30 | (3 << DATABITS))
#define SERIAL_5O2  (0x38 | (0 << DATABITS))
#define SERIAL_6O2  (0x38 | (1 << DATABITS))
#define SERIAL_7O2  (0x38 | (2 << DATABITS))
#define SERIAL_8O2  (0x38 | (3 << DATABITS))

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

int Serial_open(int, long, int );
void Serial_close(int);
void Serial_config(int, long, int);
int Serial_available(int);
int Serial_read(int);
int Serial_write(int, char);
void Serial0_config(long,int);
char Serial0_poll_read();
void Serial0_poll_write(char);
void Serial0_poll_print(char *);

#endif /* SERIAL_H_ */