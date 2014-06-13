/* db_vmint.c - bytecode interpreter for a simple virtual machine
 *
 * Copyright (c) 2014 by David Michael Betz.  All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <setjmp.h>
#include <ctype.h>
#include "db_vm.h"
#include "db_system.h"
#include "db_vmdebug.h"
#include "db_abort.h"

/* stack manipulation macros */
#define Reserve(i, n)   do {                                    \
                            if ((i)->sp - (n) < (i)->stack)     \
                                StackOverflow(i);               \
                            else                                \
                                (i)->sp -= (n);                 \
                        } while (0)
#define CPush(i, v)     do {                                    \
                            if ((i)->sp - 1 < (i)->stack)       \
                                StackOverflow(i);               \
                            else                                \
                                Push(i, v);                     \
                        } while (0)
#define Push(i, v)      (*--(i)->sp = (v))
#define Pop(i)          (*(i)->sp++)
#define Top(i)          (*(i)->sp)
#define Drop(i, n)      ((i)->sp += (n))

/* prototypes for local functions */
static void DoTrap(Interpreter *i, int op);
static void StackOverflow(Interpreter *i);
#ifdef VM_DEBUG
static void ShowStack(Interpreter *i);
#endif

/* Execute - execute the main code */
int Execute(Interpreter *i, VMVALUE *stack, int stackSize)
{
    VMVALUE tmp;
    VMWORD tmpw;
    int8_t tmpb;
    int cnt;

	/* make sure there is enough space for the runtime structures */
	if (stackSize < MIN_STACK_SIZE)
	    return -1;
	    
	/* setup the stack */
    i->stack = stack;
    i->stackTop = stack + stackSize;

    /* initialize */    
    i->text = (uint8_t *)i->image;
    i->pc = i->text + VMCODEUVALUE(&i->image->entry);
    i->sp = i->fp = i->stackTop;

    if (setjmp(i->errorTarget))
        return -1;

    for (;;) {
#ifdef VM_DEBUG
        ShowStack(i);
        DecodeInstruction(i->text, i->pc);
#endif
        switch (VMCODEBYTE(i->pc++)) {
        case OP_HALT:
            return 0;
        case OP_BRT:
            for (tmpw = 0, cnt = sizeof(VMWORD); --cnt >= 0; )
                tmpw = (tmpw << 8) | VMCODEBYTE(i->pc++);
            if (i->tos)
                i->pc += tmpw;
            i->tos = Pop(i);
            break;
        case OP_BRTSC:
            for (tmpw = 0, cnt = sizeof(VMWORD); --cnt >= 0; )
                tmpw = (tmpw << 8) | VMCODEBYTE(i->pc++);
            if (i->tos)
                i->pc += tmpw;
            else
                i->tos = Pop(i);
            break;
        case OP_BRF:
            for (tmpw = 0, cnt = sizeof(VMWORD); --cnt >= 0; )
                tmpw = (tmpw << 8) | VMCODEBYTE(i->pc++);
            if (!i->tos)
                i->pc += tmpw;
            i->tos = Pop(i);
            break;
        case OP_BRFSC:
            for (tmpw = 0, cnt = sizeof(VMWORD); --cnt >= 0; )
                tmpw = (tmpw << 8) | VMCODEBYTE(i->pc++);
            if (!i->tos)
                i->pc += tmpw;
            else
                i->tos = Pop(i);
            break;
        case OP_BR:
            for (tmpw = 0, cnt = sizeof(VMWORD); --cnt >= 0; )
                tmpw = (tmpw << 8) | VMCODEBYTE(i->pc++);
            i->pc += tmpw;
            break;
        case OP_NOT:
            i->tos = (i->tos ? VMFALSE : VMTRUE);
            break;
        case OP_NEG:
            i->tos = -i->tos;
            break;
        case OP_ADD:
            tmp = Pop(i);
            i->tos = tmp + i->tos;
            break;
        case OP_SUB:
            tmp = Pop(i);
            i->tos = tmp - i->tos;
            break;
        case OP_MUL:
            tmp = Pop(i);
            i->tos = tmp * i->tos;
            break;
        case OP_DIV:
            tmp = Pop(i);
            i->tos = (i->tos == 0 ? 0 : tmp / i->tos);
            break;
        case OP_REM:
            tmp = Pop(i);
            i->tos = (i->tos == 0 ? 0 : tmp % i->tos);
            break;
        case OP_BNOT:
            i->tos = ~i->tos;
            break;
        case OP_BAND:
            tmp = Pop(i);
            i->tos = tmp & i->tos;
            break;
        case OP_BOR:
            tmp = Pop(i);
            i->tos = tmp | i->tos;
            break;
        case OP_BXOR:
            tmp = Pop(i);
            i->tos = tmp ^ i->tos;
            break;
        case OP_SHL:
            tmp = Pop(i);
            i->tos = tmp << i->tos;
            break;
        case OP_SHR:
            tmp = Pop(i);
            i->tos = tmp >> i->tos;
            break;
        case OP_LT:
            tmp = Pop(i);
            i->tos = (tmp < i->tos ? VMTRUE : VMFALSE);
            break;
        case OP_LE:
            tmp = Pop(i);
            i->tos = (tmp <= i->tos ? VMTRUE : VMFALSE);
            break;
        case OP_EQ:
            tmp = Pop(i);
            i->tos = (tmp == i->tos ? VMTRUE : VMFALSE);
            break;
        case OP_NE:
            tmp = Pop(i);
            i->tos = (tmp != i->tos ? VMTRUE : VMFALSE);
            break;
        case OP_GE:
            tmp = Pop(i);
            i->tos = (tmp >= i->tos ? VMTRUE : VMFALSE);
            break;
        case OP_GT:
            tmp = Pop(i);
            i->tos = (tmp > i->tos ? VMTRUE : VMFALSE);
            break;
        case OP_LIT:
            for (tmp = 0, cnt = sizeof(VMUVALUE); --cnt >= 0; )
                tmp = (tmp << 8) | VMCODEBYTE(i->pc++);
            CPush(i, i->tos);
            i->tos = tmp;
            break;
        case OP_SLIT:
            tmpb = (int8_t)VMCODEBYTE(i->pc++);
            CPush(i, i->tos);
            i->tos = tmpb;
            break;
        case OP_LOAD:
            if ((VMUVALUE)i->tos >= DATA_OFFSET)
                i->tos = *(VMVALUE *)(i->data + (VMUVALUE)i->tos);
            else
                i->tos = VMCODEUVALUE(i->text + (VMUVALUE)i->tos);
            break;
        case OP_LOADB:
            if ((VMUVALUE)i->tos >= DATA_OFFSET)
                i->tos = *(uint8_t *)(i->data + (VMUVALUE)i->tos);
            else
                i->tos = VMCODEBYTE(i->text + (VMUVALUE)i->tos);
            break;
        case OP_STORE:
            tmp = Pop(i);
            if ((VMUVALUE)i->tos >= DATA_OFFSET)
                *(VMVALUE *)(i->data + (VMUVALUE)i->tos) = tmp;
            i->tos = Pop(i);
            break;
        case OP_STOREB:
            tmp = Pop(i);
            if ((VMUVALUE)i->tos >= DATA_OFFSET)
                *(uint8_t *)(i->data + (VMUVALUE)i->tos) = tmp;
            i->tos = Pop(i);
            break;
        case OP_LREF:
            tmpb = (int8_t)VMCODEBYTE(i->pc++);
            CPush(i, i->tos);
            i->tos = i->fp[(int)tmpb];
            break;
        case OP_LSET:
            tmpb = (int8_t)VMCODEBYTE(i->pc++);
            i->fp[(int)tmpb] = i->tos;
            i->tos = Pop(i);
            break;
        case OP_INDEX:
            i->tos = Pop(i) + i->tos * sizeof (VMVALUE);
            break;
        case OP_CALL:
            ++i->pc; // skip over the argument count
            tmp = i->tos;
            i->tos = (VMVALUE)(i->pc - i->text);
            i->pc = (uint8_t *)(i->text + tmp);
            break;
        case OP_FRAME:
            cnt = VMCODEBYTE(i->pc++);
            tmp = (VMVALUE)((uint8_t *)i->fp - i->text);
            i->fp = i->sp;
            Reserve(i, cnt);
            i->sp[0] = i->tos;
            i->sp[1] = tmp;
            break;
        case OP_RETURN:
            i->pc = i->text + Top(i);
            i->sp = i->fp;
            Drop(i, VMCODEBYTE(&i->pc[-1]));
            i->fp = (VMVALUE *)(i->text + i->fp[-1]);
            break;
        case OP_DROP:
            i->tos = Pop(i);
            break;
        case OP_DUP:
            CPush(i, i->tos);
            break;
        case OP_NATIVE:
            for (tmp = 0, cnt = sizeof(VMUVALUE); --cnt >= 0; )
                tmp = (tmp << 8) | VMCODEBYTE(i->pc++);
            break;
        case OP_TRAP:
            DoTrap(i, VMCODEBYTE(i->pc++));
            break;
        default:
            Abort(i, "undefined opcode 0x%02x", VMCODEBYTE(i->pc - 1));
            break;
        }
    }
    
    /* never reached */
    return -1;
}

static void DoTrap(Interpreter *i, int op)
{
    switch (op) {
    case TRAP_GetChar:
        Push(i, i->tos);
        i->tos = VM_getchar();
        break;
    case TRAP_PutChar:
        VM_putchar(i->tos);
        i->tos = Pop(i);
        break;
    case TRAP_PrintStr:
        if ((VMUVALUE)i->tos >= DATA_OFFSET) {
            uint8_t *p = i->data + (VMUVALUE)i->tos;
            while (*p != '\0')
                VM_putchar(*p++);
        }
        else {
            uint8_t *p = i->text + (VMUVALUE)i->tos;
            int ch;
            while ((ch = VMCODEBYTE(p++)) != '\0')
                VM_putchar(ch);
        }
        i->tos = Pop(i);
        break;
    case TRAP_PrintInt:
        VM_printf(VMVALUE_FMT, i->tos);
        i->tos = Pop(i);
        break;
    case TRAP_PrintTab:
        VM_putchar('\t');
        break;
    case TRAP_PrintNL:
        VM_putchar('\n');
        break;
    case TRAP_PrintFlush:
        VM_flush();
        break;
#ifdef AVR_VM
    case TRAP_DelayMs:
        VM_DelayMs(i->tos);
        i->tos = Pop(i);
        break;
    case TRAP_UpdateLeds:
        VM_UpdateLeds();
        break;
#endif
    default:
        Abort(i, "undefined trap %d", op);
        break;
    }
}

static void StackOverflow(Interpreter *i)
{
    Abort(i, "stack overflow");
}

// void Abort(Interpreter *i, const char *fmt, ...)
// {
//     char buf[100], *p = buf;
//     va_list ap;
//     va_start(ap, fmt);
//     VM_printf("error: ");
//     vsnprintf(buf, sizeof(buf), fmt, ap);
//     while (*p != '\0')
//         VM_putchar(*p++);
//     VM_putchar('\n');
//     va_end(ap);
//     longjmp(i->errorTarget, 1);
// }

#ifdef VM_DEBUG
static void ShowStack(Interpreter *i)
{
    VMVALUE *p;
    if (i->sp < i->stackTop) {
        VM_printf(" %d", i->tos);
        for (p = i->sp; p < i->stackTop - 1; ++p) {
            if (p == i->fp)
                VM_printf(" <fp>");
            VM_printf(" %d", *p);
        }
        VM_printf("\n");
    }
}
#endif
