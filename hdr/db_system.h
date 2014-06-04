/* db_system.h - global system context
 *
 * Copyright (c) 2014 by David Michael Betz.  All rights reserved.
 *
 */

#ifndef __DB_SYSTEM_H__
#define __DB_SYSTEM_H__

#include <stdarg.h>
#include "db_types.h"

#ifdef __cplusplus
extern "C" 
{
#endif

void VM_printf(const char *fmt, ...);
void VM_vprintf(const char *fmt, va_list ap);

int VM_getchar(void);
void VM_putchar(int ch);
void VM_flush(void);

#ifdef __cplusplus
}
#endif

#endif
