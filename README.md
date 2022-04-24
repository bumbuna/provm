# PROVM

Virtual machine providing backend for both interpreted procedural and OOP languages

>**NOTE:** The project name might be confusing since the vm was first designed without OOP in mind.

## BUILD

```bash

$ cd cloned-project-dir

$ cmake -B build

$ cmake --build build

```

## SUPPORTED INSTRUCTIONS

  
|instruction| opcode | operand| use|
|--|--|--|--|
|halt | 0x1 || halt the virtual machine for the current program
|add|0x2||binary addition|
|sub|0x3||binary subtraction|
|div|0x4||division
|mul|0x5||multiplication
|mod|0x6||modulus
|neg|0x7||unary minus
|eq|0x8||equality test
|neq|0x9||not equal test
|gt|0xa||greater than test
|lt|0xb||less than test
|gte|0xc||greater than or equal to test
|lte|0xd||less than or equal to test
|and|0xe||bit operation AND
|or|0xf||bit operation OR
|xor|0x10||bit operation XOR
|compl|0x11||one's complement
|call|0x12|Long E|invoke function at offset E  
|ret|0x13||return from a function
|jmp|0x14|Long E|unconditional jump to instruction at offset E
|jmpe|0x15|Long E|conditional jump to instruction at offset E if ZERO flag is not set
|jmpz|0x16|Long E|conditional jump to instruction at offset E if ZERO  flag is set
|write|0x17||pop value on TOS and print it to standard output
|getlocal|0x18|Int I|get value of local variable at index I in active function
|setlocal|0x19|Int I|pop value at TOS and set it as value of local variable at index I in active function
|getparam|0x1a|Int I|get value of parameter variable at index I in active function
|alloc|0x1b|Int L P|Allocate a stack frame for a function with L number of local variables and P parameters
|frame|0x1c|Int L P|update value of FP register to that of the new stack frame allocated by recent alloc instruction. **THE VALUE OF *L* AND *P* SHOULD BE THE SAME AS THOSE SUPPLIED TO THE MOST RECENT alloc instruction**
|dup|0x1d||duplicate value at TOS
|vecmk|0x1e|Int I|create a vector of size I and  Push the address onto the stack.
TBD
> Written with [StackEdit](https://stackedit.io/).
