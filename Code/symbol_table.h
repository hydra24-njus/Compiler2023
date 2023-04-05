#ifndef __SYMBOL_TABLE_H__
#define __SYMBOL_TABLE_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Type_* Type;
typedef struct FieldList_* FieldList;
typedef struct SymbolNode_* sNode;

#define TABLE_SIZE 0x3fff

struct Type_{
enum { BASIC, ARRAY, STRUCTURE, FUNCTION_T } kind;
    union{
    // 基本类型
    enum { INT, FLOAT } basic;
    // 数组类型信息包括元素类型与数组大小构成
    struct { Type elem; int size; } array;
    // 结构体类型信息是一个链表
    FieldList structure;
    struct { Type returntype;int paramscnt;FieldList paramlist;int isdef;}function;
    }u;
};

struct FieldList_{
    char* name; // 域的名字
    Type type; // 域的类型
    FieldList tail; // 下一个域
};

struct SymbolNode_{
    enum { VARIABLE=0,STRUCT=2,FUNCTION=3} kind;
    char *name;
    Type type;
    struct SymbolNode_ *next;
    unsigned depth;
};

void symboltable_init();
void insert_node(Type type,char *name);
Type query_symbol(char *name);
void print_table();
int typecheck(Type A, Type B);
#endif