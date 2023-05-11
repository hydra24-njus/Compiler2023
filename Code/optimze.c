#include "optimize.h"

int *_label_table=NULL;
int *_is_leader=NULL;
int *_L=NULL;
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
    _L=malloc(sizeof(int)*(k+1));
    memset(_L,-1,sizeof(int)*(k+1));
    k=0;
    for(struct InterCodes_ *i=ir_head;i!=NULL;i=i->next){
        if(_is_leader[ir_cnt]==1){
            _L[k]=ir_cnt;
            k++;
        }
        ir_cnt++;
    }
    free(_is_leader);
    free(_label_table);
}
void print_bb(){
    int i=0;
    while(_L[i]!=-1){
        printf("%d\n",_L[i]+1);
        i++;
    }
}
void _build_bb(){
    get_label_table();
    build_basic_blocks();
    print_bb();
}