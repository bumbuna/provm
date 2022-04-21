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

#define flagset(i) (flags |= (1<<i))
#define flagunset(i) (flags &= ~((unsigned)1<<i)) 
#define flagtest(i) (flags & (1<<i))
#define flagsclear() (flags = 0)

reg sp;//stack segment
reg ip;//instruction
reg fp;//frame
reg pc;//program counter
reg cs;//code segment
static reg mem; //main memory
/*
    
    bit (name) -> function
    0 (halt) -> when set causes program exit
    1 (zero) -> set if result of previous instruction was zero 
*/
static int flags;

static int instructionsize(enum opcode o) {
    int r = 1; //sizeof bytecode
    switch(o) {
        case oploadc:
        case opgetlocal:
        case opgetparam:
        case opvecmk:
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

int startvm(reg codesegment, ...) {
    if(!mem && !(mem = malloc(STACKLIMIT))) {
        perror("malloc");
        exit(errno);
    }
    fp = sp = &mem[STACKLIMIT];
    cs = pc = codesegment;
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
                int *spi = sp;
                spi[1] = binaryop(o, spi[1], spi[0]);
                sp = &spi[1];
                if(!spi[1]) flagset(zero);
                break;
            }
            case oplt: case opgt: case oplte: case opgte: case opeq: case opneq: {
                int *spi = sp;
                if(!binaryop(o, spi[1], spi[0])) flagset(zero);
                sp = &spi[2];
                break;
            }
            case oploadc: {
                ip++;
                int *spi = sp;
                spi--;
                spi[0] = *(int*)ip;
                sp = spi;
                break;
            }
            case opjmp: case opjmpe: case opcall: case opjmpz: {
                ip++;
                long n = cs+*(reg)ip;
                int *spi = sp;
                if(o==opjmp) {
                    pc = n;
                } else if (o==opjmpe&&!flagtest(zero)) {
                    pc = n;
                    flagunset(zero);
                } else if(o==opjmpz&&flagtest(zero)) {
                    pc = n;
                    flagunset(zero);
                } else if(o==opcall) {
                    RETURNADDRESS = pc;
                    pc = n;
                }
                break;
            }
            case opneg: case opcompl: {
                int *spi = sp;
                spi[0] = unaryop(o, spi[0]);
                break;
            }
            case opret: {
                int *spi = sp;
                fh_t *tfp = fp;
                pc = tfp->returnaddress;
                fp = tfp->parentframe;
                int r = spi[0];
                sp = ++tfp;
                spi = sp;
                --spi;
                spi[0] = r;
                sp = spi;
                break;
            }
            case opgetlocal: case opgetparam: {
                ip++;
                int *ipi = ip;
                int *spi = sp;
                spi--;
                int *vp = fp;
                vp -= LOCALSCOUNT;
                int i = LOCALSCOUNT-ipi[0];
                if(o==opgetparam) {
                    vp -= PARAMSCOUNT;
                    i = PARAMSCOUNT-ipi[0]-1;
                }
                spi[0] = vp[i];
                break;
            }
            case opsetlocal: {
                ip++;
                int *spi = sp;
                int *ipi = ip;
                int *vi = ((fh_t*)fp);
                vi -= LOCALSCOUNT;
                int i = LOCALSCOUNT-ipi[0]-1;
                vi[i] = spi[0];
                spi++;
                break;
            }
            case opalloc: { //alloc #L #P
                ip++;
                fh_t *spf = sp;
                int *ipi = ip;
                --spf;
                spf->localscount = ipi[0];
                spf->paramscount = ipi[1];
                spf->parentframe = fp;
                sp = spf;
                sp -= sizeof(int)*ipi[0];
                break;
            }
            case opframe: { //frame #l #p
                ip++;
                int *ipi = ip;
                int *t = sp;
                t += (ipi[0]+ipi[1]);
                fh_t *f = t;
                fp = f;
                break;
            }
            case ophalt: {
                flagset(halt);
                break;
            }
            case opwrite: {
                int *spi = sp;
                printf("%d\n", spi[0]);
                sp = ++spi;
                break;
            }
            case opdup: {
                int *spi = sp;
                --spi;
                spi[0] = spi[1];
                sp = spi;
                break;
            }
            case opvecmk: {
                ip++;
                int *ipi = ip;
                vec_t *vt = sp;
                vt--;
                vt->start = calloc(ipi[0], sizeof(int));
                vt->c = ipi[0];
                sp = vt;
                break;
            }
            case opvecget: {
                int *spi = sp;
                int i = spi[0];
                spi++;
                vec_t *vt = spi;
                if(vt->c <= i) {
                    printf("out of bounds: Accessing index %d of array %p of size %d\n", i, vt->start, vt->c);
                    flagset(halt);
                    break;
                }
                --spi;
                spi[0] = vt->start[i];
                sp = spi;
                break;
            }
            case opvecset: {
                int *spi = sp;
                int v = spi[0];
                int i = spi[1];
                spi++;
                spi++;
                vec_t *vt = spi;
                if(vt->c<= i) {
                    printf("out of bounds: Accessing index %d of array %p of size %d\n", i, vt->start, vt->c);
                    flagset(halt);
                    break;
                }
                vt->start[i] = v;
                sp = spi;
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
