#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "avruart.h"

#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif

#if 0

void UART_init(uint32_t baudrate)
{
    uint16_t brr = (((F_CPU / (baudrate * 16UL))) - 1);
    UBRR0H = brr >> 8;
    UBRR0L = brr;
    UCSR0A &= ~_BV(U2X0);
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);
}

int UART_getc(void)
{
    while (!(UCSR0A & _BV(RXC0)))
        ;
    return UDR0;
}

int UART_putc(int ch)
{
    while (!(UCSR0A & _BV(UDRE0)))
        ;
    UDR0 = ch;
}

#else

#define RX_BUFFER_SIZE	16
#define TX_BUFFER_SIZE	16

typedef struct {
    volatile uint8_t head;
    volatile uint8_t tail;
    volatile uint8_t *data;
    uint8_t size;
	uint8_t overwrite;
} Buffer;

static Buffer rx_buf;
static uint8_t rx_data[RX_BUFFER_SIZE + 1];
static Buffer tx_buf;
static uint8_t tx_data[TX_BUFFER_SIZE + 1];

static void BufInit(Buffer *buf, uint8_t *data, uint8_t size, int overwrite);
static int BufPutByte(Buffer *buf, int byte);
static int BufGetByte(Buffer *buf);

void UART_init(uint32_t baudrate)
{
    uint16_t brr;
    baudrate /= 2;
    brr = (uint16_t)(F_CPU / (baudrate * 16UL) - 1);
	BufInit(&rx_buf, rx_data, sizeof(rx_data), TRUE);
	BufInit(&tx_buf, tx_data, sizeof(tx_data), FALSE);
    UBRR0L = (uint8_t)brr;
    UBRR0H = (uint8_t)(brr >> 8);
    UCSR0A |= (1 << U2X0);
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0); 
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	sei();
}

void UART_term(void)
{
	UCSR0B = 0; 
}

int UART_putc(int ch)
{
	/* wait for space */
	while (!BufPutByte(&tx_buf, ch))
		;
	
	/* enable the buffer empty interrupt */
	UCSR0B |= 1 << UDRIE0;

	/* return the character */
	return ch;
}

int UART_getc(void)
{
    int byte;
	
    /* disable the receive interrupt */
    UCSR0B &= ~(1 << RXCIE0);

	/* get a character from the buffer */
	byte = BufGetByte(&rx_buf);

    /* enable the receive interrupt */
    UCSR0B |= 1 << RXCIE0;

	/* return the character */
	return byte;
}

static void BufInit(Buffer *buf, uint8_t *data, uint8_t size, int overwrite)
{
    buf->head = buf->tail = 0;
    buf->data = data;
    buf->size = size;
	buf->overwrite = overwrite;
}

static int BufPutByte(Buffer *buf, int byte)
{
    uint8_t head, tail;
    
    /* advance the write pointer */
    if ((head = buf->head + 1) >= buf->size)
        head = 0;
    
    /* check for the buffer being full */
    if (head == buf->tail) {

		/* overwrite the oldest data */
		if (buf->overwrite) {
			if ((tail = buf->tail + 1) >= buf->size)
				tail = 0;
			buf->tail = tail;
		}

		/* return failure */
		else
        	return FALSE;
	}
        
    /* add the new byte to the buffer */
    buf->data[buf->head] = byte;
    buf->head = head;
    
    /* return successfully */
    return TRUE;
}

static int BufGetByte(Buffer *buf)
{
    uint8_t tail;
    int byte;

    /* check for the buffer being empty */
    if (buf->head == buf->tail)
        return -1;
        
    /* advance the write pointer */
    if ((tail  = buf->tail + 1) >= buf->size)
        tail = 0;
    
    /* add the new byte to the buffer */
    byte = buf->data[buf->tail];
    buf->tail = tail;
    
    /* return successfully */
    return byte;
}

ISR(USART_RX_vect)
{
	BufPutByte(&rx_buf, UDR0);
}

ISR(USART_UDRE_vect)
{
	int byte;
	if ((UCSR0A & (1 << UDRE0)) && (byte = BufGetByte(&tx_buf)) != -1)
		UDR0 = byte;
	else
	    UCSR0B &= ~(1 << UDRIE0);
}

#endif