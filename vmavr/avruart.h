#ifndef __DB_UART_H
#define __DB_UART_H__

#include <stdint.h>

void UART_init(uint32_t baudrate);
void UART_term(void);
int UART_putc(int ch);
int UART_getc(void);

#endif
