//bumbuna <developer@devbumbuna.com>
//2022
#ifndef provm_opcodes_h
#define provm_opcodes_h
/*
opcodes with no parameter(s) work with item(s) ontop of the stack 
*/
enum opcode {
    ophalt = 1,
    opadd,
    opsub,
    opdiv,
    opmul,
    opmod,
    opneg,
    opeq,
    opneq,
    opgt,
    oplt,
    opgte,
    oplte,
    opand,
    opor,
    /*xor*/
    opxor,
    /*complement*/
    opcompl,
    /*PARAMETER: E
      invoke procedure E
     */
    opcall,
    /* return from a procedure */
    opret,
    /*PARAMETER: E
      unconditional jump to E
     */
    opjmp,
    /*PARAMETER: E
      jump to E if zero flag is not set
     */
    opjmpe,
    /*PARAMETER: E
      jump to E if zero flag is set
     */
    opjmpz,
    /*output to stdout*/
    opwrite,
    /*PARAMETER: I
      get procedure's local variable at index I
     */
    opgetlocal,
    /*PARAMETER: I
      set procedure's local variable at index I
     */
    opsetlocal,
    /*PARAMETER: I
      get procedure's parameter at index I
     */
    opgetparam,
    /*PARAMETER: C
      push constant C to the stack
     */
    oppushc,
    /*PARAMETER: L P
      allocate space for a new stackframe big enough to hold frame header and L variables
     */
    opalloc,
    /*PARAMETER: L P
      update FP register to point to new stackframe
     */
    opframe,
    /*duplicate*/
    opdup,
    /*PARAMETER: N
      create a vector of size N
     */
    opvecmk,
    /*PARAMETER: I
      get value of TOS[I]
     */
    opvecget,
    /*PARAMETER:  I
      set value of TOS[I]
     */
    opvecset,
    /*PARAMETER I
      create instance for class at index I of class table
    */
    opclnew,
    /*
     initialize instance at top of stack
     */
    opclii,
    /*PARAMETER: I
      from instance on top of stack get variable  named as string starting at offset I in literal segment
      */
    opcligv,
    /*load content of address on TOS*/
    oploada,
    /*set content of address TOS to TOS value*/
    opseta,
};


#endif
