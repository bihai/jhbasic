#include <stdio.h>
#include "db_system.h"

void VM_flush(void)
{
    fflush(stdout);
}

int VM_getchar(void)
{
    return getchar();
}

#ifndef CUSTOM_FUNCTIONS
void VM_putchar(int ch)
{
    putchar(ch);
}
#endif //CUSTOM_FUNCTIONS
