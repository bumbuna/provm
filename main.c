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
        int noofclasses; //no of classes in object file
        if(!(f = fopen(argv[i], "rb"))) {
            perror(argv[i]);
            continue;
        }
        fread(&noofclasses, sizeof(int), 1, f);
        fread(filebuffer, 1, sizeof(filebuffer), f);
        codeoffset = sizeof(int)+sizeof(class_t)*noofclasses;
        startvm(filebuffer+sizeof(class_t)*noofclasses, (class_t*) filebuffer);
        runvm();
    }
}
