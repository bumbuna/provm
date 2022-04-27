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
#define stackpush(v) *--sp = (long) v 
#define stackpop()  *sp++
#define stackcast(t) ((t)sp)
// #define stackpopn(t,i) sp += (sizeof(t)*i)
#define stackpeek() *sp
#define stackpeekn(n) *(sp+n)
#define stackalloc(t) stackallocn(t,0)
#define stackallocn(t,n) (sp -= tolsz(t)*n)
#define stackallocsz(sz) (sp -= sz)
#define tolsz(t) (sizeof(t)/sizeof(long) + (sizeof(t)%sizeof(long) ? 1 : 0))

regl sp;//stack segment
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
        case opsetlocal:
        case opjmp:
        case opjmpe:
        case opjmpz:
        case opcall: r += sizeof(long); break;
        case opalloc:
        case opframe: r += sizeof(long)*2; break;
        default: 0;
    }
    return r;
}

int startvm(reg codesegment, regcl classinfo,...) {
    if(!mem && !(mem = malloc(STACKLIMIT))) {
        perror("malloc");
        exit(errno);
    }
    sp = (long*) &mem[STACKLIMIT];
    fp = (fh_t*) sp;
    cs = pc = codesegment;
    cls = classinfo;
    flagsclear();
    return 0;
}

long binaryop(enum opcode o, long i, long j) {
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
                int i = binaryop(o, stackpeekn(1), stackpeek());
                stackpop();
                stackpop();
                stackpush(i);
                if(!i) flagset(zero);
                break;
            }
            case oplt: case opgt: case oplte: case opgte: case opeq: case opneq: {
                if(!binaryop(o, stackpeekn(1), stackpeek())) flagset(zero);
                stackpop();
                stackpop();
                break;
            }
            case oppushc: {
                ip++;
                stackpush(*(long*)ip);
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
                } else if(o==opjmpz&&flagtest(zero)  ) {
                    pc = n;
                    flagunset(zero);
                } else if(o==opcall) {
                    fp->returnaddress = pc;
                    pc = n;
                }
                break;
            }
            case opneg: case opcompl: {
                long i = stackpop();
                stackpush(unaryop(o, i));
                break;
            }
            case opret: {
                reg tmp = (reg) fp;
                pc = fp->returnaddress;
                fp = fp->parentframe;
                long r = stackpop();
                sp = (long*) tmp;
                sp -= tolsz(fh_t);
                stackpush(r);
                break;
            }
            case opgetlocal: case opgetparam: {
                ip++;
                long *ipi = (long*) ip;
                long *vp = (long*) fp;
                vp -= fp->localscount;
                int i = fp->localscount-ipi[0];
                if(o==opgetparam) {
                    vp -= fp->paramscount;
                    i = ipi[0]; //parameters are passed in reverse order
                }
                stackpush(vp[i]);
                break;
            }
            case opsetlocal: {
                ip++;
                long *ipi = (long*) ip;
                int *vi = (int*) fp;
                vi -= fp->localscount;
                int i = fp->localscount-ipi[0]-1;
                vi[i] = stackpop();
                break;
            }
            case opalloc: { //alloc #L #P
                ip++;
                long *ipi = (long*) ip;
                stackalloc(fh_t);
                stackcast(fh_t*)->localscount = ipi[0];
                stackcast(fh_t*)->paramscount = ipi[1];
                stackcast(fh_t*)->parentframe = fp;
                stackallocn(long, ipi[0]);
                break;
            }
            case opframe: { //frame #l #p
                ip++;
                long *ipi = (long*) ip;
                long *t = stackcast(long*);
                t += (ipi[0], ipi[1]);
                fp = (fh_t*) t;
                break;
            }
            case ophalt: {
                flagset(halt);
                break;
            }
            case opwrite: {
                printf("%ld\n", stackpop());
                break;
            }
            case opdup: {
                long l = stackpeek();
                stackpush(l);
                break;
            }
            // case opdupl: {
            //     stackpush(long, stackpeekn(long, 1));
            //     break;
            // }
            case opvecmk: {
                ip++;
                long *ipi = (long*) ip;
                vec_t *v = calloc(1, sizeof(vec_t)+sizeof(long)*(ipi[0]-1));
                stackpush(v);
                v->c = ipi[0];
                break;
            }
            case opvecget: {
                #define vectorarray(v,f) (&v->f)
                long i = stackpop();
                vec_t *v = *stackcast(vec_t**);
                if(v->c <= i) {
                    printf("out of bounds: Accessing index %ld of array %p of size %d\n", i,
                                            vectorarray(v,i0), v->c);
                    flagset(halt);
                    break;
                }
                long l = (&v->i0)[i];
                stackpush(l);
                break;
            }
            case opvecset: {
                long nv = stackpop(); //new value
                long i = stackpop();
                vec_t *v = (vec_t*) stackpop();
                if(v->c<= i) {
                    printf("out of bounds: Accessing index %ld of array %p of size %d\n", i, vectorarray(v,i0), v->c);
                    flagset(halt);
                    break;
                }
                vectorarray(v,i0)[i] = nv;
                break;
            }
            case opclnew: {
                ip++;
                long i = *(long*)(ip);
                instance_t *t = malloc(1*sizeof(instance_t)+sizeof(long)*(cls[ip[0]].varsc-1));
                t->c = i;
                stackpush(t);
                break;
            }
            case opcligv: {
                ip++;
                instance_t *t = (instance_t*) stackpop();
                stackpush(vectorarray(t,f0)[*(long*)ip]);
                break;
            }
            case opclisv: {
                ip++;
                long i = *(long*)ip;
                long v = stackpop();
                instance_t *t = (instance_t*) stackpop();
                vectorarray(t,f0)[i] = v;
                break;
            }
            case opcliprint: {
                instance_t *t = (typeof(t)) stackpop();
                fputs("{\n", stdout);
                for(int i = 0; i < cls[t->c].varsc; i++) {
                    printf("  %s: %ld%c\n", cls[t->c].vars[i], vectorarray(t,f0)[i], (i+1)==cls[t->c].varsc ? 0: ',');
                }
                fputs("}\n", stdout);
                fflush(stdout);
                break;
            }
        }
        if(sp < (long*) mem) {
            printf("stackoverflow error.\n");
            flagset(halt);
        }
        if(flagtest(halt)) {
            break;
        }
    }
}
