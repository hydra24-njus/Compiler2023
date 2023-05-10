#ifndef __INPUT_H__
#define __INPUT_H__

#include <stdlib.h>
#include "ir.h"

//从文件读入中间代码并转化为ir的InterCodes_双向链表
struct InterCodes_ *input(FILE *fp);

#endif