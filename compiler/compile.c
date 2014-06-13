#include <stdio.h>
#include "db_compiler.h"

/* compiler heap size */
#define HEAPSIZE            5000

/* image buffer size */
#define TEXTMAX             8192
#define DATAMAX             1024

static uint8_t space[sizeof(ParseContext) + HEAPSIZE];
static uint8_t imageSpace[sizeof(ImageHdr) + TEXTMAX + DATAMAX];

static int MyGetLine(void *cookie, char *buf, int len);

int main(int argc, char *argv[])
{
    ParseContext *c;
    FILE *fp;
    
    /* check the argument list */
    if (argc != 3) {
        fprintf(stderr, "usage: compile <source> <image>\n");
        return 1;
    }
    
    /* open the input file */
    if (!(fp = fopen(argv[1], "r"))) {
        fprintf(stderr, "error: can't open %s\n", argv[1]);
        return 1;
    }
    
    if (!(c = InitCompiler(space, sizeof(space)))) {
        VM_printf("error: insufficient memory\n");
        return 1;
    }
    c->getLine = MyGetLine;
    c->getLineCookie = fp;

    if (Compile(c, imageSpace, sizeof(imageSpace), TEXTMAX, DATAMAX) != 0) {
        VM_printf("error: compile failed\n");
        return 1;
    }
    
    /* close the input file */
    fclose(fp);
    
    /* create the image file */
    if (!(fp = fopen(argv[2], "wb"))) {
        fprintf(stderr, "error: can't create %s\n", argv[2]);
        return 1;
    }
        
    /* write the image file */
    fwrite(imageSpace, 1, ((ImageHdr *)imageSpace)->imageSize, fp);
    fclose(fp);

    return 0;
}

static int MyGetLine(void *cookie, char *buf, int len)
{
    FILE *fp = (FILE *)cookie;
    return fgets(buf, len, fp) ? 0 : -1;
}
