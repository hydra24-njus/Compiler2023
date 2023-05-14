#include "optimize.h"
#include <assert.h>
int *_label_table=NULL;
int *_is_leader=NULL;
InterCodes *_L=NULL;
int _ir_cnt=0;
extern struct InterCodes_ *ir_head,*ir_tail;
int find_max_label(){
    int label_max=0;
    for(struct InterCodes_ *i=ir_head;i!=NULL;i=i->next){
        if(i->code->kind==IR_LABEL){
            label_max=label_max>i->code->u.unaryop.unary->u.lableno?label_max:i->code->u.unaryop.unary->u.lableno;
        }
    }
    return label_max;
}
void get_label_table(){
    int lmax=find_max_label();
    _label_table=malloc(sizeof(int)*(lmax+1));
    memset(_label_table,0,sizeof(int)*(lmax+1));
    int ir_cnt=0;
    for(struct InterCodes_ *i=ir_head;i!=NULL;i=i->next){
        if(i->code->kind==IR_LABEL){
            int j=i->code->u.unaryop.unary->u.lableno;
            _label_table[j]=ir_cnt;
        }
        ir_cnt++;
        _ir_cnt++;
    }
}
void build_basic_blocks(){
    _is_leader=malloc(sizeof(int)*(_ir_cnt+1));
    memset(_is_leader,0,sizeof(int)*(_ir_cnt+1));
    _is_leader[0]=1;
    int ir_cnt=1;
    int k=1;
    for(struct InterCodes_ *i=ir_head->next;i!=NULL;i=i->next){
        if(i->code->kind==IR_GOTO){
            _is_leader[_label_table[i->code->u.unaryop.unary->u.lableno]]=1;
            _is_leader[ir_cnt+1]=1;
            k+=2;
        }
        else if(i->code->kind==IR_IFGOTO){
            _is_leader[_label_table[i->code->u.gotop.lable->u.lableno]]=1;
            _is_leader[ir_cnt+1]=1;
            k+=2;
        }
        ir_cnt++;
    }
    ir_cnt=0;
    _L=malloc(sizeof(InterCodes)*(k+1));
    memset(_L,NULL,sizeof(InterCodes)*(k+1));
    k=0;
    for(struct InterCodes_ *i=ir_head;i!=NULL;i=i->next){
        if(_is_leader[ir_cnt]==1){
            _L[k]=i;
            k++;
        }
        ir_cnt++;
    }
    free(_is_leader);
    free(_label_table);
}


extern void print_op(FILE *fp,Operand op);
void opt_local_exp_true(InterCodes start,InterCodes end){
    InterCodes tmp=start;
    FILE *fp=stdout;
    printf("\n");
    while(tmp!=end){
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

void opt_local_exp(){
    int cnt=0;
    InterCodes start=ir_head;
    InterCodes end=ir_head;
    while(_L[cnt]!=NULL){
        start=_L[cnt];
        end=_L[cnt+1];
        opt_local_exp_true(start,end);
        cnt++;
    }
}

void _build_bb(){
    get_label_table();
    build_basic_blocks();
    opt_local_exp();
}