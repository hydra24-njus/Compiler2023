#include "optimize.h"
#include <assert.h>
int *_label_table=NULL;//label的数组 标号为i的label在ir的第(_label_table[i]+1)行
int *_is_leader=NULL;//指示某行是否为基本块头，第i行若是基本块开头则_is_leader[i]==1
InterCodes *_L=NULL;//基本块指针。若ir有k个基本块，_L长度为k+1.其中_L[i]指向第i个基本块头部
int _ir_cnt=0;
extern struct InterCodes_ *ir_head,*ir_tail;
struct BB_List_ bblist;
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
    _L=malloc(sizeof(InterCodes)*(k+2));
    memset(_L,NULL,sizeof(InterCodes)*(k+2));
    k=0;
    for(struct InterCodes_ *i=ir_head;i!=NULL;i=i->next){
        if(_is_leader[ir_cnt]==1){
            _L[k]=i;
            i->ishead=1;
            k++;
        }
        ir_cnt++;
    }
    free(_is_leader);
    free(_label_table);
    bblist.bb_cnt=k;
    bblist.array=malloc(sizeof(struct BasicBlock_)*k);
    k=0;
    while(_L[k]!=NULL){
        bblist.array[k].start=_L[k];
        bblist.array[k].end=_L[k+1];
        bblist.array[k].dead=0;
        k++;
    }
}

extern void print_op(FILE *fp,Operand op);
void print_bb(){
    FILE *fp=stdout;
    for(int i=0;i<bblist.bb_cnt;i++){
        InterCodes tmp=bblist.array[i].start;
        printf("\t\tbb%d\n",i);
        while(tmp!=bblist.array[i].end){
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
}
//比较两个operand是否相等，flag为0不比较version
int eq_operand(Operand a,Operand b,int flag){
    if(a==NULL && b==NULL)return 1;
    else if(a==NULL||b==NULL)return 0;
    else if(flag==1&&a->version!=b->version)return 0;
    if(a->kind!=b->kind)return 0;
    if(a->access!=b->access)return 0;
    if(a->is_addr!=b->is_addr)return 0;
    if(a->u.vid!=b->u.vid)return 0;
    return 1;
}

struct  DGAnode_ *DGA_search(Operand op,struct DGAnodelist_ list,int flag){
    struct DGAnode_ *ret=NULL;
    int ver=0;
    for(int i=0;i<list.DGA_cnt;i++){
        if(eq_operand(op,list.array[i]->op,flag)&&list.array[i]->op->version>=ver){
            ret=list.array[i];
            ver=ret->op->version;
        }
    }
    return ret;
}
void addDGAnode(struct DGAnode_ *node,struct DGAnodelist_ list){
    if(list.array==NULL){
        list.capacity=64;
        list.DGA_cnt=0;
        list.array=malloc(sizeof(struct DGAnode_*)*64);
    }
    else if(list.DGA_cnt==list.capacity){
        struct DGAnode_ ** tmp=malloc(sizeof(struct DGAnode_*)*list.capacity*2);
        memcpy(tmp,list.array,sizeof(struct DGAnode_*)*list.capacity);
        list.capacity*=2;
        free(list.array);
        list.array=tmp;
    }
    list.array[list.DGA_cnt++]=node;
    return;
}
struct DGAnode_ buildDGANode(Operand op){
    
}



int common_subexp(InterCodes start,InterCodes end){

}

void _build_bblist(){
    get_label_table();
    build_basic_blocks();
    //print_bb();
}