#include "ir.h"
#include "debug.h"
#include "semantic.h"

static int _var_cnt=0;
static int _tmp_cnt=0;

InterCodes ir_head=NULL,ir_tail=NULL;

__attribute__((constructor)) void ir_init(){
    //printf("ir init\n\n");
    return;
}

void insert_code(InterCodes node){
    if(ir_head==NULL){
        ir_head=node;
        ir_tail=node;
        node->prev=NULL;
        node->next=NULL;
    }
    else{
        ir_tail->next=node;
        node->prev=ir_tail;
        node->next=NULL;
        while(ir_tail->next!=NULL){
            ir_tail=ir_tail->next;
        }
    }
}

void print_op(FILE *fp,Operand op){
    switch(op->kind){
        case IR_CONSTANT:
                        fprintf(fp,"#%d",op->u.value);
                        break;
        case IR_VARIABLE:
                        fprintf(fp,"%s",op->u.varname);
                        break;
        case IR_TMPOP:
                        fprintf(fp,"t%d",op->u.tmpno);
                        break;

    }
}

void print_ir(FILE *fp){
    InterCodes tmp=ir_head;
    while(tmp!=NULL){
        InterCode ic=tmp->code;
        switch(ic->kind){
            case IR_FUNCTION:   fprintf(fp,"FUNCTION %s : \n",ic->u.unaryop.unary->u.funcname);
                                break;
            case IR_PARAM:      fprintf(fp,"PARAM %s \n",ic->u.unaryop.unary->u.varname);
                                break;
            case IR_RETURN:     fprintf(fp,"RETURN ");print_op(fp,ic->u.unaryop.unary);fprintf(fp," \n");
                                break;
            case IR_ASSIGN:     print_op(fp,ic->u.assign.left);fprintf(fp," := ");print_op(fp,ic->u.assign.right);fprintf(fp," \n");
                                break;
            case IR_CALL:       print_op(fp,ic->u.assign.left);fprintf(fp," := CALL %s \n",ic->u.assign.right->u.funcname);
                                break;
            case IR_READ:       fprintf(fp,"READ ");print_op(fp,ic->u.unaryop.unary);fprintf(fp," \n");
                                break;

        }
        tmp=tmp->next;
    }

}

Operand new_operand(int kind){
    Operand ret=malloc(sizeof(struct Operand_));
    ret->kind=kind;
    return ret;
}

Operand new_tmpop(){
    Operand ret=malloc(sizeof(struct Operand_));
    ret->kind=IR_TMPOP;
    ret->u.tmpno=++_tmp_cnt;
    return ret;
}

InterCodes new_intercode(int kind){
    InterCodes node=malloc(sizeof(struct InterCodes_));
    node->code=malloc(sizeof(struct InterCode_));
    InterCode code=node->code;
    code->kind=kind;
    if(kind<=IR_WRITE){
        //单目
        code->u.unaryop.unary=malloc(sizeof(struct Operand_));
    }
    else if(kind<=IR_DEC){
        //双目
        code->u.assign.right =malloc(sizeof(struct Operand_));
        code->u.assign.left  =malloc(sizeof(struct Operand_));
    }
    else if(kind<=IR_DIV){
        //三目
        code->u.binop.result =malloc(sizeof(struct Operand_));
        code->u.binop.op1    =malloc(sizeof(struct Operand_));
        code->u.binop.op2    =malloc(sizeof(struct Operand_));
    }
    else if(kind==IR_IFGOTO){
        //四目
        code->u.gotop.op1    =malloc(sizeof(struct Operand_));
        code->u.gotop.op2    =malloc(sizeof(struct Operand_));
        code->u.gotop.relop  =malloc(sizeof(struct Operand_));
        code->u.gotop.lable  =malloc(sizeof(struct Operand_));
    }
    return node;
}

void trans_FunDec(Node *root){
    Type ftype=query_symbol(root->child->info_char,0,0);
    if(!ftype){
        debug("error in trans_FunDec: ftype==NULL\n");
        exit(1);
    }
    InterCodes node=new_intercode(IR_FUNCTION);
    node->code->u.unaryop.unary->kind=IR_FUNCNAME;
    node->code->u.unaryop.unary->u.funcname=root->child->info_char;
    insert_code(node);
    if(ftype->u.function.paramscnt!=0){
        FieldList field=ftype->u.function.paramlist;
        while(field!=NULL){
            InterCodes tmp=new_intercode(IR_PARAM);
            tmp->code->u.unaryop.unary->kind=IR_VARIABLE;
            tmp->code->u.unaryop.unary->u.varname=field->name;
            insert_code(tmp);
            field=field->tail;
        }
    }
}

void trans_Stmt(Node *root,int kind){

}

Operand trans_Exp(Node *root){
    if(gencheck(root,1,"INT")){
        debug("Exp -> INT\n");
        Operand ret=new_operand(IR_CONSTANT);
        ret->u.value=root->child->info_int;
        return ret;
    }
    else if(gencheck(root,1,"FLOAT")){
        debug("error! float\n");exit(1);
    }
    else if(gencheck(root,1,"ID")){
        debug("Exp -> ID\n");
        Operand ret=new_operand(IR_VARIABLE);
        ret->u.varname=root->child->info_char;
        return ret;
    }
    else if(gencheck(root,2,"MINUS","Exp")){
        debug("Exp -> MINUS Exp\n");
    }
    else if(gencheck(root,2,"NOT","Exp")){
        debug("Exp -> NOT Exp\n");
    }
    else if(gencheck(root,3,"Exp","ASSIGNOP","Exp")){
        debug("Exp -> Exp ASSIGNOP Exp\n");
        //默认exp1是ID TODO: 结构体和数组
        Operand op2=trans_Exp(root->child->next->next);
        Operand op1=trans_Exp(root->child);
        InterCodes code=new_intercode(IR_ASSIGN);
        free(code->code->u.assign.left);
        free(code->code->u.assign.right);
        code->code->u.assign.left=op1;
        code->code->u.assign.right=op2;
        insert_code(code);
        return op1;
    }
    else if(gencheck(root,3,"Exp","RELOP","Exp")){
        debug("Exp -> Exp RELOP Exp\n");
    }
    else if(gencheck(root,3,"Exp","OP","Exp")){
        debug("Exp -> Exp OP Exp\n");
    }
    else if(gencheck(root,3,"Exp","DOT","ID")){
        debug("Exp -> Exp DOT ID\n");
    }
    else if(gencheck(root,4,"Exp","LB","Exp","RB")){
        debug("Exp -> Exp LB Exp RB\n");
    }
    else if(gencheck(root,3,"LP","Exp","RP")){
        debug("Exp -> LP Exp RP\n");
    }
    else if(gencheck(root,3,"ID","LP","RP")){
        debug("Exp -> ID LP RP\n");
        if(strcmp(root->child->info_char,"read")==0){
            Operand op=new_tmpop();
            InterCodes node=new_intercode(IR_READ);
            free(node->code->u.unaryop.unary);
            node->code->u.unaryop.unary=op;
            insert_code(node);
            return op;
        }
        else{
            Operand op=new_tmpop();
            InterCodes node=new_intercode(IR_CALL);
            free(node->code->u.assign.left);
            node->code->u.assign.left=op;
            node->code->u.assign.right->u.funcname=root->child->info_char;
            insert_code(node);
            return op;
        }
    }
    else if(gencheck(root,4,"ID","LP","Args","RP")){
        debug("Exp -> ID LP Args RP\n");
    }
    else{
        debug("error in trans_Exp\n");
    }
    return NULL;
}

void trans_args(Node *root){
    if(gencheck(root,1,"Exp")){
        debug("Args -> Exp\n");
    }
    else if(gencheck(root,3,"Exp","COMMA","Args")){
        debug("Args -> Exp COMMA Args\n");
    }
    else{
        debug("error in trans_Args\n");
    }
}

void trans_Cond(Node *root){

}