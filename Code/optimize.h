#ifndef __OPTIMIZE_H__
#define __OPTIMIZE_H__

#include "ir.h"

struct BasicBlock_{
    struct InterCodes_ * start;
    struct InterCodes_ * end;
};


void _build_bb();

#endif