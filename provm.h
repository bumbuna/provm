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
    struct frameheader *parentframe;
    char *returnaddress;
} frameheader_t, fh_t;

typedef struct vector {
    int c;
    long i0; //index 0
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
    variable_t vars[FIELDLIMIT];
    // classproc_t procs[IDLIMIT]; 
} class_t;

typedef struct instance {
    int c; //class index 
    long f0; //field 0
} instance_t;

typedef char *reg, *regc;
typedef fh_t* regf;
typedef unsigned long regfl;
typedef long *regl;
typedef class_t *regcl;
typedef int offset_t;
//registers
extern regl sp;//stack segment
extern reg ip;//instruction
extern regf fp;//frame
extern reg pc;//program counter
extern reg cs;//code segment
extern regfl flags; //flags
extern regcl cls; //classes table
extern offset_t codeoffset; //offset of code section in obj file


extern int startvm(reg , regcl, ...);
extern void runvm();
#endif
