//bumbuna <developer@devbumbuna.com>
//2022
#ifndef provm_h
#define provm_h
#include "limits.h"
#include "opcodes.h"

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

typedef char variable_t[IDLIMIT];

typedef struct classproc {
    char name[IDLIMIT];
    int lc;
    int pc;
    long p; //offset of function entry in code 
} classproc_t;

typedef struct class{
    int varsc, procsc;
    char name[IDLIMIT];
    variable_t vars[IDLIMIT];
    classproc_t procs[IDLIMIT]; 
} class_t;

typedef struct instance {
    class_t *c; //class 
    void *address;
} instance_t;

typedef char *reg, *regc;
typedef fh_t* regf;
typedef unsigned long regfl;
//registers
extern reg sp;//stack segment
extern reg ip;//instruction
extern regf fp;//frame
extern reg pc;//program counter
extern reg cs;//code segment
extern regfl flags; //flags

extern int startvm(reg , ...);
extern void runvm();
#endif
