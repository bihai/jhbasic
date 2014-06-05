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

/**********/
/* Common */
/**********/

#define VMTRUE      1
#define VMFALSE     0

/* host alignment mask */
#define HOST_ALIGN_MASK     (sizeof(void *) - 1)

/* minimum stack size */
#define MIN_STACK_SIZE      32

/*****************/
/* MAC and LINUX */
/*****************/

#if defined(MAC) || defined(LINUX)

#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef int16_t VMWORD;
typedef int32_t VMVALUE;
typedef uint32_t VMUVALUE;

#define DATA_OFFSET             0x80000000

#define ALIGN_MASK              3

#define VMCODEBYTE(p)           *(uint8_t *)(p)
#define VMCODEVALUE(p)          *(VMVALUE *)(p)
#define VMCODEUVALUE(p)         *(VMUVALUE *)(p)

#endif  // MAC

/*******/
/* AVR */
/*******/

#if defined(AVR)

#include <stdio.h>
#ifdef WIN32
// Ugh! Visual Studio C++ doesn't supply stdint.h
typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
#else
#include <stdint.h>
#endif
#include <string.h>

typedef int16_t VMWORD;

#ifdef AVR32
typedef int32_t VMVALUE;
typedef uint32_t VMUVALUE;
#define VMVALUE_FMT             "%ld"
#else
typedef int16_t VMVALUE;
typedef uint16_t VMUVALUE;
#endif

#define DATA_OFFSET             0x8000U

#define ALIGN_MASK              1

#ifdef AVR_VM
#include <avr/pgmspace.h>
#define VMCODEBYTE(p)           ((uint8_t)pgm_read_byte(p))
#ifdef AVR32
#define VMCODEVALUE(p)          ((VMVALUE)pgm_read_dword(p))
#define VMCODEUVALUE(p)         ((VMUVALUE)pgm_read_dword(p))
#else
#define VMCODEVALUE(p)          ((VMVALUE)pgm_read_word(p))
#define VMCODEUVALUE(p)         ((VMUVALUE)pgm_read_word(p))
#endif
#else
#define VMCODEBYTE(p)           *(uint8_t *)(p)
#define VMCODEVALUE(p)          *(VMVALUE *)(p)
#define VMCODEUVALUE(p)         *(VMUVALUE *)(p)
#endif

#endif  // MAC

/*****************/
/* PROPELLER_GCC */
/*****************/

#ifdef PROPELLER_GCC

#include <string.h>
#include <stdint.h>

typedef int16_t VMWORD;
typedef int32_t VMVALUE;
typedef uint32_t VMUVALUE;

#define DATA_OFFSET             0x00008000

#define ALIGN_MASK              3

int strcasecmp(const char *s1, const char *s2);

#define VMCODEBYTE(p)           *(uint8_t *)(p)
#define VMCODEVALUE(p)          *(VMVALUE *)(p)
#define VMCODEUVALUE(p)         *(VMUVALUE *)(p)

#endif  // PROPELLER_GCC

#ifndef VMVALUE_FMT
#define VMVALUE_FMT             "%d"
#endif

#ifdef __cplusplus
}
#endif

#endif
