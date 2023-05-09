#ifndef __ASH_H__
#define __ASH_H__

#include "ir.h"

typedef struct Stack_ *stack;
struct Stack_{
    struct{
        int kind;
        int id;
        int offset;
    };
    struct Stack_* next;
};



void print_asm(FILE *fp);
void init_data(FILE *fp);
void init_io(FILE *fp);
void init_regs();
void trans_codes(FILE *fp,InterCodes head);
void trans_one_code(FILE *fp,InterCodes cur);

#endif