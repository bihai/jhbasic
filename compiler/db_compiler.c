/* db_compiler.c - a simple basic compiler
 *
 * Copyright (c) 2014 by David Michael Betz.  All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>
#include <string.h>
#include <ctype.h>
#include "db_compiler.h"
#include "db_vmdebug.h"

#define RGB_SIZE    60

static uint8_t bi_delayms[] = {
    OP_FRAME, 2,
    OP_LREF, 0,
    OP_TRAP, 7,
    OP_RETURN
};

static uint8_t bi_updateleds[] = {
    OP_FRAME, 1,
    OP_TRAP, 8,
    OP_RETURN
};

/* forward declarations */
static void EnterBuiltInFunction(ParseContext *c, char *name, uint8_t *code, size_t codeSize);
static void EnterBuiltInVariable(ParseContext *c, char *name, size_t size);

/* InitCompiler - initialize the compiler */
ParseContext *InitCompiler(uint8_t *freeSpace, size_t freeSize)
{
    ParseContext *c = (ParseContext *)freeSpace;
    if (freeSize < sizeof(ParseContext))
        return NULL;
    c->heapBase = freeSpace + sizeof(ParseContext);
    c->heapTop = freeSpace + freeSize;
    return c;
}

/* Compile - compile a program */
int Compile(ParseContext *c, uint8_t *imageSpace, size_t imageSize, size_t textMax, size_t dataMax)
{
    ImageHdr *image = (ImageHdr *)imageSpace;
    VMUVALUE textSize;

    /* setup an error target */
    if (setjmp(c->errorTarget) != 0)
        return -1;

    /* initialize the image */
    if (imageSize < sizeof(ImageHdr) + textMax + dataMax)
        return -1;
    memset(image, 0, sizeof(ImageHdr));
    c->image = image;
    
    /* empty the heap */
    c->localFree = c->heapBase;
    c->globalFree = c->heapTop;

    /* initialize the image */
    c->textBase = c->textFree = imageSpace + sizeof(ImageHdr);
    c->textTop = c->textBase + textMax;
    c->dataBase = c->dataFree = c->textBase + textMax;
    c->dataTop = c->dataBase + dataMax;
    
    /* initialize the code buffer */
    c->codeFree = c->codeBuf;
	c->codeTop = c->codeBuf + sizeof(c->codeBuf);

    /* initialize block nesting table */
    c->btop = (Block *)((char *)c->blockBuf + sizeof(c->blockBuf));
    c->bptr = c->blockBuf - 1;

    /* initialize the global symbol table and string table */
    InitSymbolTable(&c->globals);
    
    /* enter the built-in functions */
    EnterBuiltInFunction(c, "delayMs", bi_delayms, sizeof(bi_delayms));
    EnterBuiltInFunction(c, "updateLeds", bi_updateleds, sizeof(bi_updateleds));
    
    /* enter the built-in variables */
    /*
        typedef struct {
            int32_t triggerTop;
            int32_t triggerBottom;
            int32_t numLeds;
            int32_t led[RGB_SIZE];
			int32_t patternnum;
        } VM_variables;
    */
    EnterBuiltInVariable(c, "triggerTop", sizeof(VMVALUE));
    EnterBuiltInVariable(c, "triggerBottom", sizeof(VMVALUE));
    EnterBuiltInVariable(c, "numLeds", sizeof(VMVALUE));
    EnterBuiltInVariable(c, "led", sizeof(VMVALUE) * RGB_SIZE);
	EnterBuiltInVariable(c, "patternNum", sizeof(VMVALUE));
    
    /* initialize the string table */
    c->strings = NULL;

    /* initialize the label table */
    c->labels = NULL;

    /* start in the main code */
    c->codeType = CODE_TYPE_MAIN;

    /* initialize scanner */
    c->inComment = VMFALSE;
    c->lineNumber = 0;
    
    /* compile each line */
    while (GetLine(c)) {
        int tkn;
        if ((tkn = GetToken(c)) != T_EOL)
            ParseStatement(c, tkn);
    }

    /* end the main code with a halt */
    putcbyte(c, OP_HALT);
    
    /* write the main code */
    StartCode(c, CODE_TYPE_MAIN);
    image->entry = StoreCode(c);
    
    /* determine the text size */
    textSize = c->textFree - c->textBase;

    /* fill in the image header */
    image->dataOffset = sizeof(ImageHdr) + textSize;
    image->dataSize = c->dataFree - c->dataBase;
    image->imageSize = image->dataOffset + image->dataSize;
    
    /* make the data contiguous with the code */
    memcpy(&imageSpace[image->dataOffset], c->dataBase, image->dataSize);

#ifdef COMPILER_DEBUG
    VM_printf("entry      "); PrintValue(image->entry); VM_printf("\n");
    VM_printf("imageSize  "); PrintValue(image->imageSize); VM_printf("\n");
    VM_printf("textSize   "); PrintValue(textSize); VM_printf("\n");
    VM_printf("dataOffset ");  PrintValue(image->dataOffset); VM_printf("\n");
    VM_printf("dataSize   "); PrintValue(image->dataSize); VM_printf("\n");
    DumpSymbols(&c->globals, "symbols");
#endif

    /* return successfully */
    return 0;
}

/* EnterBuiltInFunction - enter a built-in function */
static void EnterBuiltInFunction(ParseContext *c, char *name, uint8_t *code, size_t codeSize)
{
    uint8_t *p = ImageTextAlloc(c, codeSize);
    memcpy(p, code, codeSize);
    AddGlobal(c, name, SC_CONSTANT,  (VMVALUE)(p - (uint8_t *)c->image));
}

/* EnterBuiltInVariable - enter a built-in variable */
static void EnterBuiltInVariable(ParseContext *c, char *name, size_t size)
{
    uint8_t *p = ImageDataAlloc(c, size);
    AddGlobal(c, name, SC_VARIABLE,  (VMVALUE)(DATA_OFFSET + (p - (uint8_t *)c->dataBase)));
}

/* StartCode - start a function or method under construction */
void StartCode(ParseContext *c, CodeType type)
{
    /* all methods must precede the main code */
    if (type != CODE_TYPE_MAIN && c->codeFree > c->codeBuf)
        ParseError(c, "subroutines and functions must precede the main code");

    /* don't allow nested functions or subroutines (for now anyway) */
    if (type != CODE_TYPE_MAIN && c->codeType != CODE_TYPE_MAIN)
        ParseError(c, "nested subroutines and functions are not supported");

    /* initialize the code object under construction */
    InitSymbolTable(&c->arguments);
    InitSymbolTable(&c->locals);
    c->localOffset = 0;
    c->codeType = type;
    
    /* write the code prolog */
    if (type != CODE_TYPE_MAIN) {
        putcbyte(c, OP_FRAME);
        putcbyte(c, 0);
    }
}

/* StoreCode - store the function or method under construction */
VMVALUE StoreCode(ParseContext *c)
{
    size_t codeSize;
    VMVALUE code;
    uint8_t *p;

    /* check for unterminated blocks */
    switch (CurrentBlockType(c)) {
    case BLOCK_IF:
    case BLOCK_ELSE:
        ParseError(c, "expecting END IF");
    case BLOCK_FOR:
        ParseError(c, "expecting NEXT");
    case BLOCK_DO:
        ParseError(c, "expecting LOOP");
    case BLOCK_NONE:
        break;
    }

    /* fixup the RESERVE instruction at the start of the code */
    if (c->codeType != CODE_TYPE_MAIN) {
        c->codeBuf[1] = 2 + c->localOffset;
        putcbyte(c, OP_RETURN);
    }

    /* make sure all referenced labels were defined */
    CheckLabels(c);
    
    /* allocate code space */
    codeSize = (int)(c->codeFree - c->codeBuf);
    p = (uint8_t *)ImageTextAlloc(c, codeSize);
    memcpy(p, c->codeBuf, codeSize);
    
    /* get the address of the compiled code */
    code = (VMVALUE)(p - (uint8_t *)c->image);

#ifdef COMPILER_DEBUG
{
    VM_printf("%s:\n", c->codeSymbol ? c->codeSymbol->name : "<main>");
    DecodeFunction((uint8_t *)c->image, (uint8_t *)c->image + code, codeSize);
    DumpSymbols(&c->arguments, "arguments");
    DumpSymbols(&c->locals, "locals");
    VM_printf("\n");
}
#endif

    /* prepare the buffer for the next function */
    c->codeFree = c->codeBuf;

    /* empty the local heap */
    c->localFree = c->heapBase;
    InitSymbolTable(&c->arguments);
    InitSymbolTable(&c->locals);
    c->labels = NULL;

    /* reset to compile the next code */
    c->codeType = CODE_TYPE_MAIN;
    
    /* return the code vector */
    return code;
}

/* AddString - add a string to the string table */
String *AddString(ParseContext *c, char *value)
{
    String *str;
    
    /* check to see if the string is already in the table */
    for (str = c->strings; str != NULL; str = str->next)
        if (strcmp(value, str->data) == 0)
            return str;

    /* allocate the string structure */
    str = GlobalAllocBasic(c, sizeof(String));
    str->data = (char *)ImageTextAlloc(c, strlen(value) + 1);
    strcpy(str->data, value);
    str->next = c->strings;
    c->strings = str;

    /* return the string table entry */
    return str;
}

/* LocalAllocBasic - allocate memory from the local heap */
void *LocalAllocBasic(ParseContext *c, size_t size)
{
    void *addr = c->localFree;
    size = (size + HOST_ALIGN_MASK) & ~HOST_ALIGN_MASK;
    if (c->localFree + size > c->globalFree)
        Abort(c, "insufficient local memory");
    c->localFree += size;
    return addr;
}

/* GlobalAllocBasic - allocate memory from the global heap */
void *GlobalAllocBasic(ParseContext *c, size_t size)
{
    size = (size + HOST_ALIGN_MASK) & ~HOST_ALIGN_MASK;
    if (c->globalFree - size < c->localFree)
        Abort(c, "insufficient global memory");
    c->globalFree -= size;
    return c->globalFree;
}

/* ImageTextAlloc - allocate image text space */
void *ImageTextAlloc(ParseContext *c, size_t size)
{
    void *addr = c->textFree;
    size = (size + ALIGN_MASK) & ~ALIGN_MASK;
    if (c->textFree + size > c->textTop)
        Abort(c, "insufficient image text space");
    c->textFree += size;
    return addr;
}

/* ImageDataAlloc - allocate image data space */
void *ImageDataAlloc(ParseContext *c, size_t size)
{
    void *addr = c->dataFree;
    size = (size + ALIGN_MASK) & ~ALIGN_MASK;
    if (c->dataFree + size > c->dataTop)
        Abort(c, "insufficient image data space");
    c->dataFree += size;
    return addr;
}

void Abort(ParseContext *c, const char *fmt, ...)
{
    char buf[100], *p = buf;
    va_list ap;
    va_start(ap, fmt);
    VM_printf("error: ");
    vsnprintf(buf, sizeof(buf), fmt, ap);
    while (*p != '\0')
        VM_putchar(*p++);
    VM_putchar('\n');
    va_end(ap);
    longjmp(c->errorTarget, 1);
}

void PrintValue(VMVALUE value)
{
    int i;
    for (i = sizeof(VMVALUE); --i >= 0 ; )
        VM_printf("%02x", (value >> (8 * i)) & 0xff);
}

