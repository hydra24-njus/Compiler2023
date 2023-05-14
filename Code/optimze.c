#include "optimize.h"
#include <assert.h>
int *_label_table=NULL;//label的数组 标号为i的label在ir的第(_label_table[i]+1)行
int *_is_leader=NULL;//指示某行是否为基本块头，第i行若是基本块开头则_is_leader[i]==1
InterCodes *_L=NULL;//基本块指针。若ir有k个基本块，_L长度为k+1.其中_L[i]指向第i个基本块头部
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
            i->ishead=1;
            k++;
        }
        ir_cnt++;
    }
    free(_is_leader);
    free(_label_table);
}

int eq_operand(Operand a,Operand b){
    if(a->kind!=b->kind)return 0;
    if(a->access!=b->access)return 0;
    if(a->is_addr!=b->is_addr)return 0;
    if(a->u.vid!=b->u.vid)return 0;
    return 1;
}

int common_subexp(InterCodes start,InterCodes end){

}

extern void print_op(FILE *fp,Operand op);
void opt_local_true(InterCodes start,InterCodes end){
    //TODO:建立有向图并优化
    
}

void opt_local(){
    int cnt=0;
    InterCodes start=ir_head;
    InterCodes end=ir_head;
    while(_L[cnt]!=NULL){
        start=_L[cnt];
        end=_L[cnt+1];
        opt_local_true(start,end);
        cnt++;
    }
}

void _build_bb(){
    get_label_table();
    build_basic_blocks();
    opt_local();
}