#ifndef __OPTIMIZE_H__
#define __OPTIMIZE_H__

#include "ir.h"

struct BasicBlock_{
    struct InterCodes_ * start;
    struct InterCodes_ * end;
};

struct node_{
    int kind;// 0:常量 1:t 2:v
    union{
        int id;
        int value;
    };//若是常量则为value;变量为id
    int cnt;//若是常量此项为0
}

void _build_bb();

#endif