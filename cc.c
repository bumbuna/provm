#include "provm.h"
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv) {
    if(argc <= 3) {
        printf("Usage: %s classname classfieldname .....\n", argv[0]);
        return 1;
    }
    class_t c;
    strcpy(c.name, argv[1]);
    for(int i = 2; i < argc; i++) {
        strcpy(c.vars[i-2], argv[i]);
    }
    c.procsc = 0;
    c.varsc = argc-2;
    fwrite(&c, sizeof(c), 1, stdout);
}
