#include <stdio.h>
#include <propeller.h>
#include "db_vm.h"
#include "db_system.h"

#define STACK_SIZE      32

/* code space */
uint8_t vmimage[] = {
#include "vmimage.h"
};

/* stack space in sram */
VMVALUE stack[STACK_SIZE];

int main(int argc, char *argv[])
{
    Interpreter i;
    
    i.image = (ImageHdr *)vmimage;
    i.data = vmimage + i.image->dataOffset - DATA_OFFSET;

    Execute(&i, stack, STACK_SIZE);
    
    for (;;)
        ;
    
    return 0;
}

int VM_getchar(void)
{
    return getchar();
}

void VM_putchar(int ch)
{
    putchar(ch);
}

void VM_flush(void)
{
}

void VM_DelayMs(VMVALUE ms)
{
    VM_printf("DelayMs called; %d\n", ms);
}

void VM_UpdateLeds(void)
{
    VM_printf("UpdateLeds called\n");
}
