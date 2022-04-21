#include "provm.h"
#include <stdio.h>

static char filebuffer[64*1024]; //64kb

int main(int argc, char **argv) {
    if(argc < 2) {
        printf("Usage: %s binfile ...\n", argv[0]);
        return 1;
    }
    int i = 1;
    FILE *f;
    for(;i < argc; i++) {
        if(!(f = fopen(argv[i], "rb"))) {
            perror(argv[i]);
            continue;
        }
        fread(filebuffer, 1, sizeof(filebuffer), f);
        startvm(filebuffer);
        runvm();
    }
}
