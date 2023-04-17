#ifndef __IR_H__
#define __IR_H__

#include "tree.h"

typedef struct Operand_* Operand;
struct Operand_ {
    enum { IR_VARIABLE, IR_CONSTANT, IR_ADDRESS, IR_FUNCNAME, IR_TMPOP } kind;
    union {
        char *varname;
        char *funcname;
        int value;
        int tmpno;
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
        struct { Operand right, left; }                 assign;
        struct { Operand result, op1, op2; }            binop;
        struct { Operand op1, op2, relop, lable; }      gotop;
    } u;
};

typedef struct InterCodes_* InterCodes;
struct InterCodes_ {
    InterCode code;
    InterCodes prev, next; 
};

InterCodes new_intercode(int kind);
void insert_code(InterCodes node);

void trans_FunDec(Node *root);
void trans_Stmt(Node *root,int kind);


Operand trans_Exp(Node *root);

void print_ir(FILE *fp);

#endif