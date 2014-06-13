#ifndef _ABORT_
#define _ABORT_

#include <setjmp.h>

void Abort(jmp_buf errorTarget, const char *fmt, ...);

#endif