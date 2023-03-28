#ifndef __SYMBOL_TABLE_H__
#define __SYMBOL_TABLE_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Type_* Type;
typedef struct FieldList_* FieldList;
typedef struct SymbolNode_* sNode_p;

#define TABLE_SIZE 0x3fff

struct Type_{
enum { BASIC, ARRAY, STRUCTURE } kind;
    union{
    // 基本类型
    int basic;
    // 数组类型信息包括元素类型与数组大小构成
    struct { Type elem; int size; } array;
    // 结构体类型信息是一个链表
    FieldList structure;
    }u;
};

struct FieldList_{
    char* name; // 域的名字
    Type type; // 域的类型
    FieldList tail; // 下一个域
};

struct SymbolNode_{
    struct SymbolNode_ *next;
    struct SymbolNode_ *behind;
    enum { VARIABLE=0,STRUCT,FUNCTION} kind;
    FieldList field;
};

void symboltable_init();
#endif