/* ebasic.c - a simple basic interpreter
 *
 * Copyright (c) 2014 by David Michael Betz.  All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "db_vm.h"

#define STACK_SIZE 32

int main(int argc, char *argv[])
{
    Interpreter i;
    ImageHdr *image = NULL;
    size_t imageSize;
    VMVALUE stack[STACK_SIZE];
    int stackSize = STACK_SIZE;
    FILE *fp;
    
    /* check the argument list */
    if (argc != 2) {
        fprintf(stderr, "usage: execute <image>\n");
        return 1;
    }
    
    /* open the image file */
    if (!(fp = fopen(argv[1], "rb"))) {
        fprintf(stderr, "error: can't open %s\n", argv[1]);
        return 1;
    }
    
    /* get the size of the image file */
    fseek(fp, 0, SEEK_END);
    imageSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    /* allocate space for the image */
    if (!(image = (ImageHdr *)malloc(imageSize))) {
        fprintf(stderr, "error: insufficient memory\n");
        return 1;
    }
    
    /* read the image file */
    fread(image, 1, imageSize, fp);
    fclose(fp);
    
    /* initialize the image */
    i.image = image;
	i.data = (uint8_t *)image + VMCODEUVALUE(&image->dataOffset) - DATA_OFFSET;

    /* execute the code */
    Execute(&i, stack, stackSize);

    return 0;
}
