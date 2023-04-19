#include "ir.h"
#include "debug.h"
#include "semantic.h"
#include <assert.h>
static int _lable_cnt=0;
static int _tmp_cnt=0;
extern int _depth;
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
    if(op->access==IR_ADDR&&!op->is_addr)fprintf(fp,"&");
    else if(op->access==IR_POINT)fprintf(fp,"*");
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
        default:        printf("in op:%d\n",op->kind);assert(0);break;
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
            case IR_DEC:        fprintf(fp,"DEC ");print_op(fp,ic->u.assign.left);fprintf(fp," %d \n",ic->u.assign.right->u.value);
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
            default:            printf("in code:%d\n",ic->kind);assert(0);break;
        }
        tmp=tmp->next;
    }

}

Operand new_operand(int kind){
    Operand ret=malloc(sizeof(struct Operand_));
    memset(ret,0,sizeof(struct Operand_));
    ret->kind=kind;
    ret->access=0;
    return ret;
}

Operand new_tmpop(){
    Operand ret=malloc(sizeof(struct Operand_));
    memset(ret,0,sizeof(struct Operand_));
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
    memset(node,0,sizeof(struct InterCodes_));
    node->code=malloc(sizeof(struct InterCode_));
    memset(node->code,0,sizeof(struct InterCode_));
    InterCode code=node->code;
    code->kind=kind;
    return node;
}

Operand new_lable(){
    Operand lab=new_operand(IR_LABEL);
    lab->kind=IR_LABELOP;
    lab->u.lableno=++_lable_cnt;
    return lab;
}

int getsize(Type type){
    if(type->kind==BASIC){
        return 4;
    }
    else if(type->kind==ARRAY){
        return type->u.array.size*getsize(type->u.array.elem);
    }
    else if(type->kind==STRUCTURE){
        int ret=0;
        FieldList tmp=type->u.structure;
        while(tmp!=NULL){
            ret+=getsize(tmp->type);
            tmp=tmp->tail;
        }
        return ret;
    }
    return 0;
}

void trans_FunDec(Node *root){
    Type ftype=query_symbol(root->child->info_char,0,0);
    if(!ftype){
        debug("error in trans_FunDec: ftype==NULL\n");
        assert(0);
    }
    InterCodes node=new_intercode(IR_FUNCTION);
    node->code->u.unaryop.unary=new_operand(IR_FUNCNAME);
    node->code->u.unaryop.unary->u.funcname=root->child->info_char;
    insert_code(node);
    if(ftype->u.function.paramscnt!=0){
        FieldList field=ftype->u.function.paramlist;
        while(field!=NULL){
            InterCodes tmp=new_intercode(IR_PARAM);
            tmp->code->u.unaryop.unary=new_operand(IR_VARIABLE);
            tmp->code->u.unaryop.unary->u.varname=field->name;
            if(field->type->kind!=BASIC)tmp->code->u.unaryop.unary->is_addr=1;
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
        debug("error! float\n");assert(0);
    }
    else if(gencheck(root,1,"ID")){
        debug("Exp -> ID\n");
        Operand ret=new_operand(IR_VARIABLE);
        ret->u.varname=root->child->info_char;
        ret->is_addr=query_if_addr(ret->u.varname,1,_depth);
        return ret;
    }
    else if(gencheck(root,2,"MINUS","Exp")){
        debug("Exp -> MINUS Exp\n");
        Operand tmp=trans_Exp(root->child->next);
        Operand ret=new_tmpop();
        InterCodes node=new_intercode(IR_SUB);
        node->code->u.binop.op1=new_operand(IR_CONSTANT);
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
        code0->code->u.assign.left=tmp;
        code0->code->u.assign.right=new_operand(IR_CONSTANT);
        code0->code->u.assign.right->u.value=0;
        insert_code(code0);
        trans_Cond(root,lable1,lable2);
        InterCodes code2=new_intercode(IR_LABEL);
        code2->code->u.unaryop.unary=lable1;
        insert_code(code2);
        code2=new_intercode(IR_ASSIGN);
        code2->code->u.assign.left=tmp;
        code2->code->u.assign.right=new_operand(IR_CONSTANT);
        code2->code->u.assign.right->u.value=1;
        insert_code(code2);
        code2=new_intercode(IR_LABEL);
        code2->code->u.unaryop.unary=lable2;
        insert_code(code2);
        return tmp;
    }
    else if(gencheck(root,3,"Exp","ASSIGNOP","Exp")){
        debug("Exp -> Exp ASSIGNOP Exp\n");
        //默认exp1是ID TODO: 结构体和数组
        Operand op1=trans_Exp(root->child);
        Operand op2=trans_Exp(root->child->next->next);
        InterCodes code=new_intercode(IR_ASSIGN);
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
        code0->code->u.assign.left=tmp;
        code0->code->u.assign.right=new_operand(IR_CONSTANT);
        code0->code->u.assign.right->u.value=0;
        insert_code(code0);
        trans_Cond(root,lable1,lable2);
        InterCodes code2=new_intercode(IR_LABEL);
        code2->code->u.unaryop.unary=lable1;
        insert_code(code2);
        code2=new_intercode(IR_ASSIGN);
        code2->code->u.assign.left=tmp;
        code2->code->u.assign.right=new_operand(IR_CONSTANT);
        code2->code->u.assign.right->u.value=1;
        insert_code(code2);
        code2=new_intercode(IR_LABEL);
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
        code0->code->u.assign.left=tmp;
        code0->code->u.assign.right=new_operand(IR_CONSTANT);
        code0->code->u.assign.right->u.value=0;
        insert_code(code0);
        trans_Cond(root,lable1,lable2);
        InterCodes code2=new_intercode(IR_LABEL);
        code2->code->u.unaryop.unary=lable1;
        insert_code(code2);
        code2=new_intercode(IR_ASSIGN);
        code2->code->u.assign.left=tmp;
        code2->code->u.assign.right=new_operand(IR_CONSTANT);
        code2->code->u.assign.right->u.value=1;
        insert_code(code2);
        code2=new_intercode(IR_LABEL);
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
        code0->code->u.assign.left=tmp;
        code0->code->u.assign.right=new_operand(IR_CONSTANT);
        code0->code->u.assign.right->u.value=0;
        insert_code(code0);
        trans_Cond(root,lable1,lable2);
        InterCodes code2=new_intercode(IR_LABEL);
        code2->code->u.unaryop.unary=lable1;
        insert_code(code2);
        code2=new_intercode(IR_ASSIGN);
        code2->code->u.assign.left=tmp;
        code2->code->u.assign.right=new_operand(IR_CONSTANT);
        code2->code->u.assign.right->u.value=1;
        insert_code(code2);
        code2=new_intercode(IR_LABEL);
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
        else{debug("error colculate\n");assert(0);}
        node->code->u.binop.op1=t1;
        node->code->u.binop.op2=t2;
        node->code->u.binop.result=t3;
        insert_code(node);
        return t3;
    }
    else if(gencheck(root,3,"Exp","DOT","ID")){
        debug("Exp -> Exp DOT ID\n");
        Operand head=trans_Exp(root->child);
        head->access-=1;
        Operand size=new_operand(IR_CONSTANT);
        FieldList field=((Type)root->child->type)->u.structure;
        while(field&&strcmp(field->name,root->child->next->next->info_char)){
            size->u.value+=getsize(field->type);
            field=field->tail;
        }
        InterCodes node=new_intercode(IR_ADD);
        node->code->u.binop.op1=head;
        node->code->u.binop.op2=size;
        Operand pos=new_tmpop();
        node->code->u.binop.result=pos;
        insert_code(node);
        Operand ret=new_operand(IR_TMPOP);
        ret->access=IR_POINT;
        ret->u.tmpno=pos->u.tmpno;
        return ret;
    }
    else if(gencheck(root,4,"Exp","LB","Exp","RB")){
        debug("Exp -> Exp LB Exp RB\n");
        Operand head=trans_Exp(root->child);
        head->access-=1;
        Operand pos=trans_Exp(root->child->next->next);
        Operand size=new_operand(IR_CONSTANT);
        size->u.value=getsize(((Type)root->child->type)->u.array.elem);
        InterCodes node=new_intercode(IR_MUL);
        node->code->u.binop.op1=pos;
        node->code->u.binop.op2=size;
        pos=new_tmpop();
        node->code->u.binop.result=pos;
        insert_code(node);

        node=new_intercode(IR_ADD);
        node->code->u.binop.op1=head;
        node->code->u.binop.op2=pos;
        pos=new_tmpop();
        node->code->u.binop.result=pos;
        insert_code(node);
        Operand ret=new_operand(IR_TMPOP);
        ret->access=IR_POINT;
        ret->u.tmpno=pos->u.tmpno;
        return ret;
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
            node->code->u.unaryop.unary=op;
            insert_code(node);
            return op;
        }
        else{
            Operand op=new_tmpop();
            InterCodes node=new_intercode(IR_CALL);
            node->code->u.assign.left=op;
            node->code->u.assign.right=new_operand(IR_FUNCNAME);
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
            node->code->u.unaryop.unary=op;
            insert_code(node);
            Operand tmp=new_operand(IR_CONSTANT);
            tmp->u.value=0;
            return tmp;
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
            node->code->u.assign.left=op;
            node->code->u.assign.right=new_operand(IR_FUNCNAME);
            node->code->u.assign.right->u.funcname=root->child->info_char;
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
    if(gencheck(root,1,"Exp")){
        debug("Args -> Exp\n");
        Operand op=trans_Exp(root->child);
        if(((Type)root->child->type)->kind!=BASIC){
            op->access-=1;
        }
        node->code->u.unaryop.unary=op;
        node->next=NULL;
        return node;
    }
    else if(gencheck(root,3,"Exp","COMMA","Args")){
        debug("Args -> Exp COMMA Args\n");
        Operand op=trans_Exp(root->child);
        if(((Type)root->child->type)->kind!=BASIC){
            op->access-=1;
        }
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
        Operand t1=trans_Exp(root->child);//code1
        Operand t2=trans_Exp(root->child->next->next);//code2
        Operand relop=get_relop(root->child->next->info_char);
        InterCodes node1=new_intercode(IR_IFGOTO);
        node1->code->u.gotop.op1=t1;
        node1->code->u.gotop.op2=t2;
        node1->code->u.gotop.lable=label1;
        node1->code->u.gotop.relop=relop;
        insert_code(node1);//code3
        node1=new_intercode(IR_GOTO);
        node1->code->u.unaryop.unary=label2;
        insert_code(node1);//GOTO label_false
    }
    else if(gencheck(root,2,"NOT","Exp")){
        trans_Cond(root->child->next,label2,label1);
    }
    else if(gencheck(root,3,"Exp","AND","Exp")){
        Operand label3=new_lable();
        trans_Cond(root->child,label3,label2);//code1
        InterCodes node=new_intercode(IR_LABEL);
        node->code->u.unaryop.unary=label3;
        insert_code(node);//LABEL label3
        trans_Cond(root->child->next->next,label1,label2);//code2

    }
    else if(gencheck(root,3,"Exp","OR","Exp")){
        Operand label3=new_lable();
        trans_Cond(root->child,label1,label3);//code1
        InterCodes node=new_intercode(IR_LABEL);
        node->code->u.unaryop.unary=label3;
        insert_code(node);//LABEL label3
        trans_Cond(root->child->next->next,label1,label2);//code2
    }
    else {
        Operand t1=trans_Exp(root);//code1
        InterCodes code2=new_intercode(IR_IFGOTO);
        code2->code->u.gotop.op1=t1;
        code2->code->u.gotop.relop=new_operand(IR_RELOP);
        code2->code->u.gotop.relop->u.relopid="!=";
        code2->code->u.gotop.op2=new_operand(IR_VARIABLE);
        code2->code->u.gotop.op2->u.value=0;
        code2->code->u.gotop.lable=label1;
        insert_code(code2);//code2
        code2=new_intercode(IR_GOTO);
        code2->code->u.unaryop.unary=label2;
        insert_code(code2);//GOTO label_false;
    }
}