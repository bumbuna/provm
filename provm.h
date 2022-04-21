//bumbuna <developer@devbumbuna.com>
//2022
#ifndef provm_h
#define provm_h
#include "limits.h"
#include "opcodes.h"

typedef char* reg;
//registers
extern reg sp;//stack segment
extern reg ip;//instruction
extern reg fp;//frame
extern reg pc;//program counter
extern reg cs;//code segment

/*  stackframe

    _________________________
    | RETURNADDRESS         |\
    | PARENTFRAME           | \ header 
    | #LOCAL                | /
    | #PARAMETER            |/ 
    |-----------------------|
    | space for locals      |
    |-----------------------|
    | space for parameters  |
    |-----------------------|
    | scratch area          |
NOTE:
    - Direction of stack growth is downwards
    - Direction of items on the stack is upwards
*/

typedef struct frameheader {
    int paramscount;
    int localscount;
    long *parentframe;
    long *returnaddress;
} frameheader_t, fh_t;

typedef struct vector {
    int c;
    int *start;
} vector_t, vec_t;

#define RETURNADDRESS ((fh_t*)fp)->returnaddress
#define PARENTFRAME ((fh_t*)fp)->parentframe
#define LOCALSCOUNT ((fh_t*)fp)->localscount
#define PARAMSCOUNT ((fh_t*)fp)->paramscount

extern int startvm(reg , ...);
extern void runvm();
#endif
