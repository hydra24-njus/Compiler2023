#include "ir.h"
#include "debug.h"
#include "semantic.h"

static int _lable_cnt=0;
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
                        fprintf(fp,"v%s",op->u.varname);
                        break;
        case IR_FUNCNAME:
                        if(strcmp(op->u.funcname,"main")==0)fprintf(fp,"%s",op->u.funcname);
                        else fprintf(fp,"f%s",op->u.funcname);
                        break;
        case IR_TMPOP:
                        fprintf(fp,"t%d",op->u.tmpno);
                        break;
        case IR_RELOP:
                        fprintf(fp,"%s",op->u.relopid);
                        break;
        case IR_LABELOP:
                        fprintf(fp,"lable%d",op->u.lableno);
                        break;
        default:        printf("in op:%d\n",op->kind);exit(1);break;
    }
}

void print_ir(FILE *fp){
    InterCodes tmp=ir_head;
    while(tmp!=NULL){
        InterCode ic=tmp->code;
        switch(ic->kind){
            case IR_LABEL:      fprintf(fp,"LABEL ");print_op(fp,ic->u.unaryop.unary);fprintf(fp," : \n");
                                break;
            case IR_FUNCTION:   fprintf(fp,"FUNCTION ");print_op(fp,ic->u.unaryop.unary);fprintf(fp," : \n");
                                break;
            case IR_PARAM:      fprintf(fp,"PARAM ");print_op(fp,ic->u.unaryop.unary);fprintf(fp," \n");
                                break;
            case IR_RETURN:     fprintf(fp,"RETURN ");print_op(fp,ic->u.unaryop.unary);fprintf(fp," \n");
                                break;
            case IR_ASSIGN:     print_op(fp,ic->u.assign.left);fprintf(fp," := ");print_op(fp,ic->u.assign.right);fprintf(fp," \n");
                                break;
            case IR_CALL:       print_op(fp,ic->u.assign.left);fprintf(fp," := CALL ");print_op(fp,ic->u.assign.right);fprintf(fp," \n");
                                break;
            case IR_READ:       fprintf(fp,"READ ");print_op(fp,ic->u.unaryop.unary);fprintf(fp," \n");
                                break;
            case IR_ARG:        fprintf(fp,"ARG ");print_op(fp,ic->u.unaryop.unary);fprintf(fp," \n");
                                break;
            case IR_WRITE:      fprintf(fp,"WRITE ");print_op(fp,ic->u.unaryop.unary);fprintf(fp," \n");
                                break;
            case IR_IFGOTO:     fprintf(fp,"IF ");
                                print_op(fp,ic->u.gotop.op1);fprintf(fp," ");
                                print_op(fp,ic->u.gotop.relop);fprintf(fp," ");
                                print_op(fp,ic->u.gotop.op2);fprintf(fp," GOTO ");
                                print_op(fp,ic->u.gotop.lable);fprintf(fp," \n");
                                break;
            case IR_GOTO:       fprintf(fp,"GOTO ");
                                print_op(fp,ic->u.unaryop.unary);fprintf(fp," \n");
                                break;
            case IR_ADD:        print_op(fp,ic->u.binop.result);fprintf(fp," := ");
                                print_op(fp,ic->u.binop.op1);fprintf(fp," + ");
                                print_op(fp,ic->u.binop.op2);fprintf(fp," \n");
                                break;
            case IR_SUB:        print_op(fp,ic->u.binop.result);fprintf(fp," := ");
                                print_op(fp,ic->u.binop.op1);fprintf(fp," - ");
                                print_op(fp,ic->u.binop.op2);fprintf(fp," \n");
                                break;
            case IR_MUL:        print_op(fp,ic->u.binop.result);fprintf(fp," := ");
                                print_op(fp,ic->u.binop.op1);fprintf(fp," * ");
                                print_op(fp,ic->u.binop.op2);fprintf(fp," \n");
                                break;
            case IR_DIV:        print_op(fp,ic->u.binop.result);fprintf(fp," := ");
                                print_op(fp,ic->u.binop.op1);fprintf(fp," / ");
                                print_op(fp,ic->u.binop.op2);fprintf(fp," \n");
                                break;
            default:            printf("in code:%d\n",ic->kind);exit(1);break;
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

Operand get_relop(const char* name){
    Operand tmp=new_operand(IR_RELOP);
    tmp->u.relopid=name;
    return tmp;
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

Operand new_lable(){
    Operand lab=new_operand(IR_LABEL);
    lab->kind=IR_LABELOP;
    lab->u.lableno=++_lable_cnt;
    return lab;
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
        Operand tmp=trans_Exp(root->child->next);
        Operand ret=new_tmpop();
        InterCodes node=new_intercode(IR_SUB);
        free(node->code->u.binop.op2);
        free(node->code->u.binop.result);
        node->code->u.binop.op1->kind=IR_CONSTANT;
        node->code->u.binop.op1->u.value=0;
        node->code->u.binop.op2=tmp;
        node->code->u.binop.result=ret;
        insert_code(node);
        return ret;
    }
    else if(gencheck(root,2,"NOT","Exp")){
        debug("Exp -> Exp RELOP Exp\n");
        Operand lable1=new_lable();
        Operand lable2=new_lable();
        Operand tmp=new_tmpop();
        InterCodes code0=new_intercode(IR_ASSIGN);
        free(code0->code->u.assign.left);
        code0->code->u.assign.left=tmp;
        code0->code->u.assign.right->kind=IR_CONSTANT;
        code0->code->u.assign.right->u.value=0;
        insert_code(code0);
        trans_Cond(root,lable1,lable2);
        InterCodes code2=new_intercode(IR_LABEL);
        free(code2->code->u.unaryop.unary);
        code2->code->u.unaryop.unary=lable1;
        insert_code(code2);
        code2=new_intercode(IR_ASSIGN);
        free(code2->code->u.assign.left);
        code2->code->u.assign.left=tmp;
        code2->code->u.assign.right->kind=IR_CONSTANT;
        code2->code->u.assign.right->u.value=1;
        insert_code(code2);
        code2=new_intercode(IR_LABEL);
        free(code2->code->u.unaryop.unary);
        code2->code->u.unaryop.unary=lable2;
        insert_code(code2);
        return tmp;
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
        Operand lable1=new_lable();
        Operand lable2=new_lable();
        Operand tmp=new_tmpop();
        InterCodes code0=new_intercode(IR_ASSIGN);
        free(code0->code->u.assign.left);
        code0->code->u.assign.left=tmp;
        code0->code->u.assign.right->kind=IR_CONSTANT;
        code0->code->u.assign.right->u.value=0;
        insert_code(code0);
        trans_Cond(root,lable1,lable2);
        InterCodes code2=new_intercode(IR_LABEL);
        free(code2->code->u.unaryop.unary);
        code2->code->u.unaryop.unary=lable1;
        insert_code(code2);
        code2=new_intercode(IR_ASSIGN);
        free(code2->code->u.assign.left);
        code2->code->u.assign.left=tmp;
        code2->code->u.assign.right->kind=IR_CONSTANT;
        code2->code->u.assign.right->u.value=1;
        insert_code(code2);
        code2=new_intercode(IR_LABEL);
        free(code2->code->u.unaryop.unary);
        code2->code->u.unaryop.unary=lable2;
        insert_code(code2);
        return tmp;
    }
    else if(gencheck(root,3,"Exp","AND","Exp")){
        debug("Exp -> Exp RELOP Exp\n");
        Operand lable1=new_lable();
        Operand lable2=new_lable();
        Operand tmp=new_tmpop();
        InterCodes code0=new_intercode(IR_ASSIGN);
        free(code0->code->u.assign.left);
        code0->code->u.assign.left=tmp;
        code0->code->u.assign.right->kind=IR_CONSTANT;
        code0->code->u.assign.right->u.value=0;
        insert_code(code0);
        trans_Cond(root,lable1,lable2);
        InterCodes code2=new_intercode(IR_LABEL);
        free(code2->code->u.unaryop.unary);
        code2->code->u.unaryop.unary=lable1;
        insert_code(code2);
        code2=new_intercode(IR_ASSIGN);
        free(code2->code->u.assign.left);
        code2->code->u.assign.left=tmp;
        code2->code->u.assign.right->kind=IR_CONSTANT;
        code2->code->u.assign.right->u.value=1;
        insert_code(code2);
        code2=new_intercode(IR_LABEL);
        free(code2->code->u.unaryop.unary);
        code2->code->u.unaryop.unary=lable2;
        insert_code(code2);
        return tmp;
    }
    else if(gencheck(root,3,"Exp","OR","Exp")){
        debug("Exp -> Exp RELOP Exp\n");
        Operand lable1=new_lable();
        Operand lable2=new_lable();
        Operand tmp=new_tmpop();
        InterCodes code0=new_intercode(IR_ASSIGN);
        free(code0->code->u.assign.left);
        code0->code->u.assign.left=tmp;
        code0->code->u.assign.right->kind=IR_CONSTANT;
        code0->code->u.assign.right->u.value=0;
        insert_code(code0);
        trans_Cond(root,lable1,lable2);
        InterCodes code2=new_intercode(IR_LABEL);
        free(code2->code->u.unaryop.unary);
        code2->code->u.unaryop.unary=lable1;
        insert_code(code2);
        code2=new_intercode(IR_ASSIGN);
        free(code2->code->u.assign.left);
        code2->code->u.assign.left=tmp;
        code2->code->u.assign.right->kind=IR_CONSTANT;
        code2->code->u.assign.right->u.value=1;
        insert_code(code2);
        code2=new_intercode(IR_LABEL);
        free(code2->code->u.unaryop.unary);
        code2->code->u.unaryop.unary=lable2;
        insert_code(code2);
        return tmp;
    }
    else if(gencheck(root,3,"Exp","OP","Exp")){
        debug("Exp -> Exp OP Exp\n");//加减乘除
        Operand t1=trans_Exp(root->child);
        Operand t2=trans_Exp(root->child->next->next);
        Operand t3=new_tmpop();
        InterCodes node=new_intercode(IR_ADD);
        char *tmpchar=root->child->next->node_info;
        if(strcmp(tmpchar,"PLUS")==0)node->code->kind=IR_ADD;
        else if(strcmp(tmpchar,"MINUS")==0)node->code->kind=IR_SUB;
        else if(strcmp(tmpchar,"STAR")==0)node->code->kind=IR_MUL;
        else if(strcmp(tmpchar,"DIV")==0)node->code->kind=IR_DIV;
        else{debug("error colculate\n");exit(1);}
        free(node->code->u.binop.op1);
        free(node->code->u.binop.op2);
        free(node->code->u.binop.result);
        node->code->u.binop.op1=t1;
        node->code->u.binop.op2=t2;
        node->code->u.binop.result=t3;
        insert_code(node);
        return t3;
    }
    else if(gencheck(root,3,"Exp","DOT","ID")){
        debug("Exp -> Exp DOT ID\n");
    }
    else if(gencheck(root,4,"Exp","LB","Exp","RB")){
        debug("Exp -> Exp LB Exp RB\n");
    }
    else if(gencheck(root,3,"LP","Exp","RP")){
        debug("Exp -> LP Exp RP\n");
        return trans_Exp(root->child->next);
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
        if(strcmp(root->child->info_char,"write")==0){
            Operand op=trans_Exp(root->child->next->next->child);
            InterCodes node=new_intercode(IR_WRITE);
            free(node->code->u.unaryop.unary);
            node->code->u.unaryop.unary=op;
            insert_code(node);
            return op;
        }
        else{
            InterCodes head=trans_args(root->child->next->next);
            InterCodes tmp=head;
            while(tmp!=NULL){
                InterCodes tmp2=tmp;
                tmp=tmp->next;
                insert_code(tmp2);
            }
            Operand op=new_tmpop();
            InterCodes node=new_intercode(IR_CALL);
            free(node->code->u.assign.left);
            node->code->u.assign.left=op;
            node->code->u.assign.right->u.funcname=root->child->info_char;
            node->code->u.assign.right->kind=IR_FUNCNAME;
            insert_code(node);
            return op;
        }
    }
    else{
        debug("error in trans_Exp\n");
    }
    return NULL;
}

InterCodes trans_args(Node *root){
    InterCodes node=new_intercode(IR_ARG);
    free(node->code->u.unaryop.unary);
    if(gencheck(root,1,"Exp")){
        debug("Args -> Exp\n");
        Operand op=trans_Exp(root->child);
        node->code->u.unaryop.unary=op;
        return node;
    }
    else if(gencheck(root,3,"Exp","COMMA","Args")){
        debug("Args -> Exp COMMA Args\n");
        Operand op=trans_Exp(root->child);
        node->code->u.unaryop.unary=op;
        InterCodes next=trans_args(root->child->next->next);
        InterCodes ret=next;
        while(next->next!=NULL)next=next->next;
        next->next=node;
        return ret;
    }
    else{
        debug("error in trans_Args\n");
    }
    return NULL;
}

void trans_Cond(Node *root,Operand label1,Operand label2){
    if(gencheck(root,3,"Exp","RELOP","Exp")){
        Operand t1=trans_Exp(root->child);
        Operand t2=trans_Exp(root->child->next->next);
        Operand relop=get_relop(root->child->next->info_char);
        InterCodes node1=new_intercode(IR_IFGOTO);
        free(node1->code->u.gotop.op1);
        free(node1->code->u.gotop.op2);
        free(node1->code->u.gotop.lable);
        free(node1->code->u.gotop.relop);
        node1->code->u.gotop.op1=t1;
        node1->code->u.gotop.op2=t2;
        node1->code->u.gotop.lable=label1;
        node1->code->u.gotop.relop=relop;
        insert_code(node1);
        node1=new_intercode(IR_GOTO);
        free(node1->code->u.unaryop.unary);
        node1->code->u.unaryop.unary=label2;
        insert_code(node1);
    }
    else if(gencheck(root,2,"NOT","Exp")){

    }
    else if(gencheck(root,3,"Exp","AND","Exp")){

    }
    else if(gencheck(root,3,"Exp","OR","Exp")){

    }
    else {
        debug("error in trans_cond\n");
        exit(1);
    }
}