#include <stdio.h>
#include <avr/pgmspace.h>
#define __DELAY_BACKWARD_COMPATIBLE__
#include <util/delay.h>
#include "db_vm.h"
#include "db_system.h"
#include "avruart.h"

#define VMTEXT_SIZE     2048
#define VMDATA_SIZE     1024
#define STACK_SIZE      32

/* code space in flash */
uint8_t __attribute__((section(".vmimage"))) vmimage[VMTEXT_SIZE] = {
#include "vmimage.h"
};

/* data space in sram */
uint8_t vmdata[VMDATA_SIZE];

/* stack space in sram */
VMVALUE stack[STACK_SIZE];

int main(int argc, char *argv[])
{
    Interpreter i;
    
    UART_init(115200);
    
    i.image = (ImageHdr *)vmimage;
    i.data = vmdata - DATA_OFFSET;
    memcpy_P(vmdata, vmimage + VMCODEUVALUE(&i.image->dataOffset), VMCODEUVALUE(&i.image->dataSize));

    Execute(&i, stack, STACK_SIZE);
    
    for (;;)
        ;
    
    return 0;
}

int VM_getchar(void)
{
    int ch;
    while ((ch = UART_getc()) == -1)
        ;
    return ch;
}

void VM_putchar(int ch)
{
    if (ch == '\n')
        UART_putc('\r');
    UART_putc(ch);
}

void VM_flush(void)
{
}

void VM_DelayMs(VMVALUE ms)
{
    _delay_ms((double)ms);
}

void VM_UpdateLeds(void)
{
    VM_printf("UpdateLeds called\n");
}
