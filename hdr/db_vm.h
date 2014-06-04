/* db_vm.h - definitions for a simple virtual machine
 *
 * Copyright (c) 2014 by David Michael Betz.  All rights reserved.
 *
 */

#ifndef __DB_VM_H__
#define __DB_VM_H__

#include <setjmp.h>
#include "db_image.h"

#ifdef __cplusplus
extern "C" 
{
#endif

/* interpreter state structure */
typedef struct {
    jmp_buf errorTarget;
    ImageHdr *image;
    uint8_t *text;
    uint8_t *data;
    VMVALUE *stack;
    VMVALUE *stackTop;
    uint8_t *pc;
    VMVALUE *fp;
    VMVALUE *sp;
    VMVALUE tos;
} Interpreter;

/* prototypes from db_vmint.c */
int Execute(Interpreter *i, VMVALUE *stack, int stackSize);
void Abort(Interpreter *i, const char *fmt, ...);

#ifdef AVR_VM
void VM_DelayMs(VMVALUE ms);
void VM_UpdateLeds(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
