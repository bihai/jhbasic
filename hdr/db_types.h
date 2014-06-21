/* db_types.h - type definitions for a simple virtual machine
 *
 * Copyright (c) 2014 by David Michael Betz.  All rights reserved.
 *
 */

#ifndef __DB_TYPES_H__
#define __DB_TYPES_H__

#ifdef __cplusplus
extern "C" 
{
#endif

/*****************/
/* Configuration */
/*****************/

/* size of a virtual machine value */
//#define VM_VALUE_16
#define VM_VALUE_32

/* size of a virtual machine address */
//#define VM_ADDRESS_32
#define VM_ADDRESS_16

/**********/
/* Common */
/**********/

#ifdef WIN32
// Ugh! Visual Studio C++ doesn't supply stdint.h
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
#else
#include <stdint.h>
#endif

#define VMTRUE      1
#define VMFALSE     0

/* host alignment mask */
#define HOST_ALIGN_MASK     (sizeof(void *) - 1)

/* minimum stack size */
#define MIN_STACK_SIZE      32

/* type for virtual machine values */
#if defined(VM_VALUE_16)
typedef int16_t VMVALUE;
typedef uint16_t VMUVALUE;
#elif defined(VM_VALUE_32)
typedef int32_t VMVALUE;
typedef uint32_t VMUVALUE;
#else
#error define either VM_VALUE_16 or VM_VALUE_32
#endif

typedef int16_t VMWORD;

/* definitions for virtual machine addresses */
/* TEXT always starts at zero and DATA_OFFSET is the offset where SRAM starts */
#if defined(VM_ADDRESS_16)
#define DATA_OFFSET             0x8000U
#define ALIGN_MASK              1
#elif defined(VM_ADDRESS_32)
#define DATA_OFFSET             0x80000000U
#define ALIGN_MASK              3
#else
#error define either VM_ADDRESS_16 or VM_ADDRESS_32
#endif

/* AVR uses special functions to access data in flash */
#ifdef AVR

#include <avr/pgmspace.h>
#define VMCODEBYTE(p)           ((uint8_t)pgm_read_byte(p))

#if defined(VM_VALUE_16)
#define VMCODEVALUE(p)          ((VMVALUE)pgm_read_word(p))
#define VMCODEUVALUE(p)         ((VMUVALUE)pgm_read_word(p))
#elif defined (VM_VALUE_32)
#define VMCODEVALUE(p)          ((VMVALUE)pgm_read_dword(p))
#define VMCODEUVALUE(p)         ((VMUVALUE)pgm_read_dword(p))
#else
#error define either VM_VALUE_16 or VM_VALUE_32
#endif

/* format for printing a value */
#define VMVALUE_FMT             "%ld"

#else

/* other processors can use normal pointer dereferencing */
#define VMCODEBYTE(p)           *(uint8_t *)(p)
#define VMCODEVALUE(p)          *(VMVALUE *)(p)
#define VMCODEUVALUE(p)         *(VMUVALUE *)(p)

/* format for printing a value */
#define VMVALUE_FMT             "%d"

#endif

#ifdef __cplusplus
}
#endif

#endif
