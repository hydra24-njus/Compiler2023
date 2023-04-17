#ifndef __IR_H__
#define __IR_H__

#include "tree.h"

typedef struct Operand_* Operand;
struct Operand_ {
    enum { IR_VARIABLE, IR_CONSTANT, IR_ADDRESS } kind;
    union {
        int var_no;
        int value;
    } u;
};

struct InterCode{
    enum { IR_ASSIGN, IR_ADD, IR_SUB, IR_MUL } kind;
    union {
        struct { Operand right, left; } assign;
        struct { Operand result, op1, op2; } binop;
    } u;
};

struct InterCodes {
    struct InterCode code;
    struct InterCodes *prev, *next; 
};


void trans_FunDec(Node *root);
void trans_Stmt(Node *root);


static void trans_Exp(Node *root);

#endif