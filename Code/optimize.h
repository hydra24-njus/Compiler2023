#ifndef __OPTIMIZE_H__
#define __OPTIMIZE_H__

#include "ir.h"

struct BasicBlock_{
    struct InterCodes_ * start;
    struct InterCodes_ * end;
    int dead;
    struct BasicBlock_ *next[2];
    struct BasicBlock_ **pre;
    int precnt;
};

struct BB_List_{
    struct BasicBlock_ *array;
    int bb_cnt;
};

struct DGAnode_{
    enum{DGA_ASSIGN,DGA_PLUS,DGA_SUB,DGA_MUL,DGA_DIV,DGA_LEAF}kind;
    Operand op;
    struct DGAnode_ * child[2];
};

struct DGAnodelist_{
    struct DGAnode_ **array;
    int DGA_cnt;
    int capacity;
};

void _build_bblist();
#endif