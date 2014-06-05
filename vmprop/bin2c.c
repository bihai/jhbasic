#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    FILE *ifp, *ofp;
    int byte, cnt;

    if (argc != 3) {
        fprintf(stderr, "usage: bin2c <infile> <outfile>\n");
        exit(1);
    }
    else if (!(ifp = fopen(argv[1], "rb"))) {
        fprintf(stderr, "error: can't open: %s\n", argv[1]);
        exit(1);
    }
    else if (!(ofp = fopen(argv[2], "wb"))) {
        fprintf(stderr, "error: can't create: %s\n", argv[2]);
        exit(1);
    }

    cnt = 0;
    while ((byte = getc(ifp)) != EOF) {
        fprintf(ofp, " 0x%02x,", byte);
        if (++cnt == 8) {
            putc('\n', ofp);
            cnt = 0;
        }
    }

    if (cnt > 0)
        putc('\n', ofp);
    
    fclose(ifp);
    fclose(ofp);

    return 0;
}
