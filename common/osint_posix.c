#include <stdio.h>
#include "db_system.h"

#ifdef WIN32
	#include <Windows.h>
#endif

void VM_flush(void)
{
    fflush(stdout);
}

int VM_getchar(void)
{
    return getchar();
}

void VM_DelayMs(VMVALUE ms)
{
#if defined(WIN32)
    Sleep(ms);
#else
//    _delay_ms((double)ms);
#endif
}

#ifndef CUSTOM_FUNCTIONS
void VM_putchar(int ch)
{
    putchar(ch);
}

void VM_UpdateLeds(void)
{
	VM_printf("UpdateLeds called\n");
}
#endif //CUSTOM_FUNCTIONS
