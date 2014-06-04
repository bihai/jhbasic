/* db_compiler.h - definitions for a simple basic compiler
 *
 * Copyright (c) 2014 by David Michael Betz.  All rights reserved.
 *
 */

#ifndef __DB_COMPILER_H__
#define __DB_COMPILER_H__

#include <stdio.h>
#include <setjmp.h>
#include "db_types.h"
#include "db_image.h"
#include "db_system.h"

#ifdef __cplusplus
extern "C" 
{
#endif

#ifdef WIN32
#define strcasecmp  _stricmp
#endif

/* program limits */
#define MAXTOKEN        32

/* forward type declarations */
typedef struct ParseTreeNode ParseTreeNode;
typedef struct ExprListEntry ExprListEntry;

/* program limits */
#define MAXLINE         128
#define MAXCODE         32768

/* line input handler */
typedef int GetLineHandler(void *cookie, char *buf, int len);

/* lexical tokens */
enum {
    T_NONE,
    T_REM = 0x100,  /* keywords start here */
    T_DEF,
    T_DIM,
    T_AS,
    T_LET,
    T_IF,
    T_THEN,
    T_ELSE,
    T_END,
    T_FOR,
    T_TO,
    T_STEP,
    T_NEXT,
    T_DO,
    T_WHILE,
    T_UNTIL,
    T_LOOP,
    T_GOTO,
    T_MOD,
    T_AND,
    T_OR,
    T_NOT,
    T_STOP,
    T_RETURN,
    T_PRINT,
#ifdef USE_ASM
    T_ASM,
#endif
    T_ELSE_IF,  /* compound keywords */
    T_END_DEF,
    T_END_IF,
#ifdef USE_ASM
    T_END_ASM,
#endif
    T_DO_WHILE,
    T_DO_UNTIL,
    T_LOOP_WHILE,
    T_LOOP_UNTIL,
    T_LE,       /* non-keyword tokens */
    T_NE,
    T_GE,
    T_SHL,
    T_SHR,
    T_IDENTIFIER,
    T_NUMBER,
    T_STRING,
    T_EOL,
    T_EOF
};

/* block type */
typedef enum {
    BLOCK_NONE,
    BLOCK_IF,
    BLOCK_ELSE,
    BLOCK_FOR,
    BLOCK_DO
} BlockType;

/* block structure */
typedef struct Block Block;
struct Block {
    BlockType type;
    union {
        struct {
            int nxt;
            int end;
        } IfBlock;
        struct {
            int end;
        } ElseBlock;
        struct {
            int nxt;
            int end;
        } ForBlock;
        struct {
            int nxt;
            int end;
        } DoBlock;
    } u;
};

/* label structure */
typedef struct Label Label;
struct Label {
    Label *next;
    int placed;
    int fixups;
    int offset;
    char name[1];
};

/* storage class ids */
typedef enum {
    SC_CONSTANT,
    SC_VARIABLE
} StorageClass;

/* symbol table */
typedef struct Symbol Symbol;
typedef struct {
    Symbol *head;
    Symbol **pTail;
    int count;
} SymbolTable;

/* symbol structure */
struct Symbol {
    Symbol *next;
    StorageClass storageClass;
    VMVALUE value;
    char name[1];
};

/* string structure */
typedef struct String String;
struct String {
    char *data;
    String *next;
};

/* code types */
typedef enum {
    CODE_TYPE_MAIN,
    CODE_TYPE_FUNCTION
} CodeType;

/* parse context */
typedef struct {
    jmp_buf errorTarget;        /* error target */
    GetLineHandler *getLine;    /* function to get a line of input */
    void *getLineCookie;        /* cookie for the getLine function */
    int lineNumber;             /* current line number */
    char lineBuf[MAXLINE];      /* current input line */
    char *linePtr;              /* pointer to the current character */
    uint8_t *heapBase;          /* code staging buffer (start of heap) */
    uint8_t *localFree;         /* next free local heap location */
    uint8_t *globalFree;        /* next free global heap location */
    uint8_t *heapTop;           /* top of heap */
    int savedToken;             /* lookahead token */
    int tokenOffset;            /* offset to the start of the current token */
    char token[MAXTOKEN];       /* current token string */
    VMVALUE value;              /* current token integer value */
    int inComment;              /* inside of a slash/star comment */
    Label *labels;              /* local labels */
    CodeType codeType;          /* type of code under construction */
    Symbol *codeSymbol;         /* symbol table entry of code under construction */
    SymbolTable arguments;      /* arguments of current function definition */
    SymbolTable locals;         /* local variables of current function definition */
    int localOffset;            /* offset to next available local variable */
    Block blockBuf[10];         /* stack of nested blocks */
    Block *bptr;                /* current block */
    Block *btop;                /* top of block stack */
    SymbolTable globals;        /* global variables and constants */
    String *strings;            /* string constants */
    uint8_t codeBuf[MAXCODE];   /* code staging buffer */
    uint8_t *codeFree;          /* next free location in code stating buffer */
    uint8_t *codeTop;           /* top of code staging buffer */
    ImageHdr *image;            /* header of image being constructed */
    uint8_t *textBase;          /* base of text buffer */
    uint8_t *textFree;          /* next free text location */
    uint8_t *textTop;           /* top of text buffer */
    uint8_t *dataBase;          /* base of data buffer */
    uint8_t *dataFree;          /* next free data location */
    uint8_t *dataTop;           /* top of data buffer */
} ParseContext;

/* partial value function codes */
typedef enum {
    PV_ADDR,
    PV_LOAD,
    PV_STORE
} PValOp;

/* partial value structure */
typedef struct PVAL PVAL;
struct PVAL {
    void (*fcn)(ParseContext *c, PValOp op, PVAL *pv);
    union {
        Symbol *sym;
        String *str;
        VMVALUE val;
    } u;
};

/* parse tree node types */
enum {
    NodeTypeSymbolRef,
    NodeTypeStringLit,
    NodeTypeIntegerLit,
    NodeTypeUnaryOp,
    NodeTypeBinaryOp,
    NodeTypeArrayRef,
    NodeTypeFunctionCall,
    NodeTypeDisjunction,
    NodeTypeConjunction
};

/* parse tree node structure */
struct ParseTreeNode {
    int nodeType;
    union {
        struct {
            Symbol *symbol;
            void (*fcn)(ParseContext *c, PValOp op, PVAL *pv);
            int offset;
        } symbolRef;
        struct {
            String *string;
        } stringLit;
        struct {
            VMVALUE value;
        } integerLit;
        struct {
            int op;
            ParseTreeNode *expr;
        } unaryOp;
        struct {
            int op;
            ParseTreeNode *left;
            ParseTreeNode *right;
        } binaryOp;
        struct {
            ParseTreeNode *array;
            ParseTreeNode *index;
        } arrayRef;
        struct {
            ParseTreeNode *fcn;
            ExprListEntry *args;
            int argc;
        } functionCall;
        struct {
            ExprListEntry *exprs;
        } exprList;
    } u;
};

/* expression list entry structure */
struct ExprListEntry {
    ParseTreeNode *expr;
    ExprListEntry *next;
};

/* db_compiler.c */
ParseContext *InitCompiler(uint8_t *freeSpace, size_t freeSize);
int Compile(ParseContext *c, uint8_t *imageSpace, size_t imageSize, size_t textMax, size_t dataMax);
void StartCode(ParseContext *c, CodeType type);
VMVALUE StoreCode(ParseContext *c);
String *AddString(ParseContext *c, char *value);
VMVALUE AddStringRef(String *str, int offset);
void *LocalAlloc(ParseContext *c, size_t size);
void *GlobalAlloc(ParseContext *c, size_t size);
void *ImageTextAlloc(ParseContext *c, size_t size);
void *ImageDataAlloc(ParseContext *c, size_t size);
void Abort(ParseContext *c, const char *fmt, ...);
void PrintValue(VMVALUE value);

/* db_statement.c */
void ParseStatement(ParseContext *c, int tkn);
BlockType CurrentBlockType(ParseContext *c);
void CheckLabels(ParseContext *c);

/* db_expr.c */
void ParseRValue(ParseContext *c);
ParseTreeNode *ParseExpr(ParseContext *c);
ParseTreeNode *ParsePrimary(ParseContext *c);
ParseTreeNode *GetSymbolRef(ParseContext *c, char *name);
int IsIntegerLit(ParseTreeNode *node);

/* db_scan.c */
int GetLine(ParseContext *c);
void FRequire(ParseContext *c, int requiredToken);
void Require(ParseContext *c, int token, int requiredToken);
int GetToken(ParseContext *c);
void SaveToken(ParseContext *c, int token);
char *TokenName(int token);
int SkipSpaces(ParseContext *c);
int GetChar(ParseContext *c);
void UngetC(ParseContext *c);
void ParseError(ParseContext *c, char *fmt, ...);

/* db_symbols.c */
void InitSymbolTable(SymbolTable *table);
Symbol *AddGlobal(ParseContext *c, const char *name, StorageClass storageClass, VMVALUE value);
Symbol *AddArgument(ParseContext *c, const char *name, StorageClass storageClass, int value);
Symbol *AddLocal(ParseContext *c, const char *name, StorageClass storageClass, int value);
Symbol *FindSymbol(SymbolTable *table, const char *name);
int IsConstant(Symbol *symbol);
void DumpSymbols(SymbolTable *table, char *tag);

/* db_generate.c */
void code_lvalue(ParseContext *c, ParseTreeNode *expr, PVAL *pv);
void code_rvalue(ParseContext *c, ParseTreeNode *expr);
void rvalue(ParseContext *c, PVAL *pv);
void chklvalue(ParseContext *c, PVAL *pv);
void code_global(ParseContext *c, PValOp fcn, PVAL *pv);
void code_local(ParseContext *c, PValOp fcn, PVAL *pv);
int codeaddr(ParseContext *c);
int putcbyte(ParseContext *c, int v);
int putcword(ParseContext *c, VMWORD v);
void fixupbranch(ParseContext *c, VMUVALUE chn, VMUVALUE val);

#ifdef __cplusplus
}
#endif

#endif

