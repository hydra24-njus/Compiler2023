#ifndef __OPTIMIZE_H__
#define __OPTIMIZE_H__

#include "ir.h"

struct BasicBlock_{
    struct InterCodes_ * start;
    struct InterCodes_ * end;
    int dead;
    int next[2];
    int *pre;
    int precnt;
    int pre_capacity;
};

struct BB_List_{
    struct BasicBlock_ *array;
    int bb_cnt;
};
struct Global_BBlist_{
    struct BB_List_ *bblist;
    int gbb_cnt;
};

struct DAGnode_{
    enum{DAG_ASSIGN,DAG_PLUS,DAG_SUB,DAG_MUL,DAG_DIV,DAG_LEAF,DAG_READ}kind;
    Operand op;
    struct DAGnode_ * child[2];
};

struct DAGnodelist_{
    struct DAGnode_ **array;
    int DAG_cnt;
    int capacity;
};

void optimize(FILE *fp);
#endif