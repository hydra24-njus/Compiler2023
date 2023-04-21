#ifndef __SYMBOL_TABLE_H__
#define __SYMBOL_TABLE_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Type_* Type;
typedef struct FieldList_* FieldList;
typedef struct SymbolNode_* sNode;
typedef struct ScopeList_* ScopeList;

#define TABLE_SIZE (0x3fff)
#define STRUCT_SIZE (0x0fff)
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
    unsigned depth;
    int is_addr;
    int vid;
    struct SymbolNode_ *next;
    struct SymbolNode_ *tail;
};

struct FunctionList_{
    int lineno;
    char *name;
    struct FunctionList_ *next;
};

struct ScopeList_{
    struct ScopeList_  *next;
    struct SymbolNode_ *tail;
};

void symboltable_init();

void insert_node(Type type,const char *name,int deep,int kind,ScopeList scope);
void insert_node_asaddr(Type type,const char *name,int deep,int kind,ScopeList scope);

Type query_symbol(const char *name,int type,int deep);
sNode query_node(const char *name,int type,int deep);

void insert_node_struct(Type type,const char *name,int deep);
Type query_symbol_struct(const char *name,int deep);
void delete_struct_table();


void insert_function(int lineno,const char *name);
void delete_function(const char *name);
void delete_functable();

struct ScopeList_ *enter_new_scope();
struct ScopeList_ *exit_cur_scope();

void print_table();
int typecheck(Type A, Type B);

#endif