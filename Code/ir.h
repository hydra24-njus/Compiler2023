#ifndef __IR_H__
#define __IR_H__

#include "tree.h"
typedef struct Type_* Type;
typedef struct Operand_* Operand;
struct Operand_ {
    enum { IR_VARIABLE, IR_CONSTANT, IR_FUNCNAME, IR_TMPOP, IR_LABELOP, IR_RELOP } kind;
    enum { IR_ADDR=-1, IR_NOMAL, IR_POINT,} access;
    int is_addr;
    int version;
    union {
        int vid;
        char *funcname;
        int value;
        int tmpno;
        int lableno;
        char *relopid;
    } u;
};

typedef struct InterCode_* InterCode;
struct InterCode_{
    enum {  
        IR_LABEL,//1
        IR_FUNCTION,//1
        IR_GOTO,//1
        IR_RETURN,//1
        IR_ARG,//1
        IR_PARAM,//1
        IR_READ,//1
        IR_WRITE,//1
        IR_ASSIGN,//2
        IR_CALL,//2
        IR_DEC,//2
        IR_ADD,//3
        IR_SUB,//3
        IR_MUL,//3
        IR_DIV,//3
        IR_IFGOTO//4
    } kind;
    union {
        struct { Operand unary; }                       unaryop;
        struct { Operand left, right; }                 assign;
        struct { Operand result, op1, op2; }            binop;
        struct { Operand op1, op2, relop, lable; }      gotop;
    } u;
};

typedef struct InterCodes_* InterCodes;
struct InterCodes_ {
    InterCode code;
    int ishead;
    InterCodes prev, next; 
};

Operand new_lable();
Operand new_operand(int kind);
InterCodes new_intercode(int kind);
void insert_code(InterCodes node);
int getsize(Type type);

void trans_FunDec(Node *root);


Operand trans_Exp(Node *root);
InterCodes trans_args(Node *root);
void trans_Cond(Node *root,Operand lable1,Operand lable2);

void print_ir(FILE *fp);

#endif