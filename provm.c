//bumbuna <developer@devbumbuna.com>
//2022
#include "provm.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

enum flagbit {
    halt, 
    zero,
};

#define flagset(i) (flags |= (((unsigned long)1)<<i))
#define flagunset(i) (flags &= ~(((unsigned long)1)<<i)) 
#define flagtest(i) (flags & (((unsigned long)1)<<i))
#define flagsclear() (flags = 0)
#define stackpush(t,v) sp -= sizeof(t), *(t*)sp = v
#define stackpop(t) *(t*)((sp += sizeof(t))-sizeof(t))
#define stackpopn(t,i) sp += (sizeof(t)*i)
#define stackpeek(t) *(t*)sp
#define stackpeekn(t,n) *(((t*)sp)+n)
#define stackcast(t) ((t)sp)
#define stackalloc(t) (sp -= sizeof(t))
#define stackallocsz(sz) (sp -= sz)

reg sp;//stack segment
reg ip;//instruction
regf fp;//frame
reg pc;//program counter
reg cs;//code segment
regfl flags;
regcl cls;
offset_t codeoffset = sizeof(int);

static reg mem; //main memory
/*
    
    bit (name) -> function
    0 (halt) -> when set causes program exit
    1 (zero) -> set if result of previous instruction was zero 
*/
// static int flags;

static int instructionsize(enum opcode o) {
    int r = 1; //sizeof bytecode
    switch(o) {
        case oppushc:
        case opgetlocal:
        case opgetparam:
        case opvecmk:
        case opclnew:
        case opcligv:
        case opclisv:
        case opsetlocal: r += sizeof(int); break;
        case opjmp:
        case opjmpe:
        case opjmpz:
        case opcall: r += sizeof(long); break;
        case opalloc:
        case opframe: r += sizeof(int)*2; break;
        default: 0;
    }
    return r;
}

int startvm(reg codesegment, regcl classinfo,...) {
    if(!mem && !(mem = malloc(STACKLIMIT))) {
        perror("malloc");
        exit(errno);
    }
    sp = &mem[STACKLIMIT];
    fp = (fh_t*) sp;
    cs = pc = codesegment;
    cls = classinfo;
    flagsclear();
    return 0;
}

int binaryop(enum opcode o, int i, int j) {
    switch(o) {
        case opadd: return i+j;
        case opsub: return i-j;
        case opmul: return i*j;
        case opdiv: return i/j;
        case opmod: return i%j;
        case opeq: return i==j;
        case opneq: return i!=j;
        case opgt: return i>j;
        case opgte: return i>=j;
        case oplt: return i<j;
        case oplte: return i<=j;
        case opand: return i&j;
        case opor: return i|j;
        case opxor: return i^j;
    }
    return 0;
}

int unaryop(enum opcode o, int i) {
    switch(o) {
        case opneg: return -i;
        case opcompl: return ~i;
    }
    return 0;
}

void runvm() {
    while(1) {
        ip = pc;
        pc += instructionsize((char) *ip);
        enum opcode o;
        switch((o = *ip)) {
            /*
                Expected stack state
                BEFORE:
                | other stuff   |
                | left operand  |
                | right operand |<------------- SP
                
                AFTER:
                | other stuff   |
                | result        |<------------- SP
            */
            case opadd: case opsub: case opdiv: case opmul: case opmod: case opand: case opor: case opxor:  {
                int i = binaryop(o, stackpeekn(int,1), stackpeek(int));
                stackpopn(int,2);
                stackpush(int, i);
                if(!i) flagset(zero);
                break;
            }
            case oplt: case opgt: case oplte: case opgte: case opeq: case opneq: {
                if(!binaryop(o, stackpeekn(int,1), stackpeekn(int, 0))) flagset(zero);
                stackpopn(int,2);
                break;
            }
            case oppushc: {
                ip++;
                stackpush(int, *(int*)ip);
                break;
            }
            case opjmp: case opjmpe: case opcall: case opjmpz: {
                ip++;
                reg n = (cs+*(long*)ip)-codeoffset;
                if(o==opjmp) {
                    pc = n;
                } else if (o==opjmpe&&!flagtest(zero)) {
                    pc = (reg) n;
                    flagunset(zero);
                } else if(o==opjmpz&&flagtest(zero)) {
                    pc = n;
                    flagunset(zero);
                } else if(o==opcall) {
                    fp->returnaddress = pc;
                    pc = n;
                }
                break;
            }
            case opneg: case opcompl: {
                int i = stackpop(int);
                stackpush(int, unaryop(o, i));
                break;
            }
            case opret: {
                reg tmp = (reg) fp;
                pc = fp->returnaddress;
                fp = fp->parentframe;
                int r = stackpop(int);
                sp = tmp;
                stackpop(fh_t);
                stackpush(int, r);
                break;
            }
            case opgetlocal: case opgetparam: {
                ip++;
                int *ipi = (int*) ip;
                int *vp = (int*) fp;
                vp -= fp->localscount;
                int i = fp->localscount-ipi[0];
                if(o==opgetparam) {
                    vp -= fp->paramscount;
                    i = fp->paramscount-ipi[0]-1;
                }
                stackpush(int, vp[i]);
                break;
            }
            case opsetlocal: {
                ip++;
                int *ipi = (int*) ip;
                int *vi = (int*) fp;
                vi -= fp->localscount;
                int i = fp->localscount-ipi[0]-1;
                vi[i] = stackpop(int);
                break;
            }
            case opalloc: { //alloc #L #P
                ip++;
                int *ipi = (int*) ip;
                stackalloc(sizeof(fh_t));
                stackcast(fh_t*)->localscount = ipi[0];
                stackcast(fh_t*)->paramscount = ipi[1];
                stackcast(fh_t*)->parentframe = fp;
                stackallocsz(sizeof(int)*ip[0]);
                break;
            }
            case opframe: { //frame #l #p
                ip++;
                int *ipi = (int*) ip;
                int *t = stackcast(int*);
                t += (ipi[0], ipi[1]);
                fp = (fh_t*) t;
                break;
            }
            case ophalt: {
                flagset(halt);
                break;
            }
            case opwrite: {
                printf("%d\n", stackpop(int));
                break;
            }
            case opdup: {
                stackpush(int, stackpeekn(int, 1));
                break;
            }
            case opdupl: {
                stackpush(long, stackpeekn(long, 1));
                break;
            }
            case opvecmk: {
                ip++;
                int *ipi = (int*) ip;
                stackalloc(vec_t);
                stackcast(vec_t*)->start = calloc(ipi[0], sizeof(int));
                stackcast(vec_t*)->c = ipi[0];
                break;
            }
            case opvecget: {
                int i = stackpop(int);
                if(stackcast(vec_t*)->c <= i) {
                    printf("out of bounds: Accessing index %d of array %p of size %d\n", i,
                                            stackcast(vec_t*)->start, stackcast(vec_t*)->c);
                    flagset(halt);
                    break;
                }
                int v = stackcast(vec_t*)->start[i];
                stackpush(int, v);
                break;
            }
            case opvecset: {
                int v = stackpeek(int);
                int i = stackpeekn(int, 1);
                stackpopn(int,2);
                vec_t *vt = stackcast(vec_t*);
                if(vt->c<= i) {
                    printf("out of bounds: Accessing index %d of array %p of size %d\n", i, vt->start, vt->c);
                    flagset(halt);
                    break;
                }
                vt->start[i] = v;
                break;
            }
            case opclnew: {
                ip++;
                int i = *(int*)(ip);
                instance_t *t = malloc(1*sizeof(instance_t));
                t->c = i;
                t->address = malloc(sizeof(int)*cls[i].varsc);
                stackpush(void*, t);
                break;
            }
            case opcligv: {
                ip++;
                instance_t *t = stackpop(instance_t*);
                stackpush(int, t->address[*(int*)ip]);
                break;
            }
            case opclisv: {
                ip++;
                int i = *(int*)ip;
                int v = stackpop(int);
                instance_t *t = stackpop(instance_t*);
                t->address[i] = v;
                break;
            }
            case opcliprint: {
                instance_t *t = stackpop(instance_t*);
                fputs("{\n", stdout);
                for(int i = 0; i < cls[t->c].varsc; i++) {
                    printf("  %s: %d%c\n", cls[t->c].vars[i], t->address[i], (i+1)==cls[t->c].varsc ? 0: ',');
                }
                fputs("}\n", stdout);
                fflush(stdout);
                break;
            }
        }
        if(sp < mem) {
            printf("stackoverflow error.\n");
            flagset(halt);
        }
        if(flagtest(halt)) {
            break;
        }
    }
}
