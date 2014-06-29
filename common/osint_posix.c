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

void VM_putchar(int ch)
{
    putchar(ch);
}


void VM_DelayMs(VMVALUE ms)
{
	//_delay_ms((double)ms);
	Sleep(ms);
}

void VM_UpdateLeds(void)
{
	VM_printf("UpdateLeds called\n");
}
