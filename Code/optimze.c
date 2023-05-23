#include "optimize.h"
#include <assert.h>
int *_label_table=NULL;//label的数组 标号为i的label在ir的第(_label_table[i]+1)行
int _ir_cnt=0;
extern struct InterCodes_ *ir_head,*ir_tail;
struct Global_BBlist_ gbblist;
extern void print_op(FILE *fp,Operand op);
int find_max_label(InterCodes start){
    int label_max=0;
    for(struct InterCodes_ *i=start;i!=NULL;i=i->next){
        if(i->code->kind==IR_FUNCTION&&i!=start)break;
        if(i->code->kind==IR_LABEL){
            label_max=label_max>i->code->u.unaryop.unary->u.lableno?label_max:i->code->u.unaryop.unary->u.lableno;
        }
    }
    return label_max;
}
void get_label_table(InterCodes start){
    int lmax=find_max_label(start);
    _label_table=malloc(sizeof(int)*(lmax+1));
    memset(_label_table,0,sizeof(int)*(lmax+1));
    int ir_cnt=0;
    _ir_cnt=0;
    for(struct InterCodes_ *i=start;i!=NULL;i=i->next){
        if(i->code->kind==IR_FUNCTION&&i!=start)break;
        if(i->code->kind==IR_LABEL){
            int j=i->code->u.unaryop.unary->u.lableno;
            _label_table[j]=ir_cnt;
        }
        ir_cnt++;
        _ir_cnt++;
    }
}
InterCodes build_basic_blocks(InterCodes start,struct BB_List_ *bblist){
    get_label_table(start);
    int *_is_leader=malloc(sizeof(int)*(_ir_cnt+1));
    memset(_is_leader,0,sizeof(int)*(_ir_cnt+1));
    _is_leader[0]=1;
    int ir_cnt=1;
    int k=1;
    InterCodes ret=NULL;
    for(struct InterCodes_ *i=start->next;i!=NULL;i=i->next){
        if(i->code->kind==IR_FUNCTION){
            ret=i;
            break;
        }
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
        else if(i->code->kind==IR_RETURN){
            _is_leader[ir_cnt+1]=1;
            k+=1;
        }
        ir_cnt++;
    }
    ir_cnt=0;
    InterCodes *_L=malloc(sizeof(InterCodes)*(k+2));
    memset(_L,NULL,sizeof(InterCodes)*(k+2));
    k=0;
    for(struct InterCodes_ *i=start;i!=NULL;i=i->next){
        if(i==ret)break;
        if(_is_leader[ir_cnt]==1){
            _L[k]=i;
            k++;
        }
        ir_cnt++;
    }
    free(_is_leader);
    free(_label_table);
    bblist->bb_cnt=k;
    bblist->array=malloc(sizeof(struct BasicBlock_)*k);
    memset(bblist->array,0,sizeof(struct BasicBlock_)*k);
    k=0;
    while(_L[k]!=NULL){
        bblist->array[k].start=_L[k];
        bblist->array[k].end=_L[k+1];
        bblist->array[k].dead=0;
        k++;
    }
    bblist->array[bblist->bb_cnt-1].end=ret;
    free(_L);
    return ret;
}
extern int _func_cnt;
void build_bb_global(){
    gbblist.gbb_cnt=_func_cnt;
    gbblist.bblist=malloc(sizeof(struct BB_List_)*(_func_cnt+1));
    InterCodes start=ir_head;
    for(int i=0;i<_func_cnt;i++){
        start=build_basic_blocks(start,&(gbblist.bblist[i]));
    }
}
extern void print_op(FILE *fp,Operand op);
void print_bb(FILE *fp){
    for(int j=0;j<gbblist.gbb_cnt;j++){
        struct BB_List_ *bblist=&(gbblist.bblist[j]);
        fprintf(fp,"\t\t#function%d\n",j);
        for(int i=0;i<bblist->bb_cnt;i++){
            fprintf(fp,"\t\t#bb%d\n",i);
            InterCodes tmp=bblist->array[i].start;
            while(tmp!=bblist->array[i].end){
                if(tmp->dead==1&&tmp->code->kind!=IR_READ){tmp=tmp->next;continue;}
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
        //printf("\n");
    }
}
//比较两个operand是否相等，flag为0不比较version
int eq_operand(Operand a,Operand b,int flag){
    if(a==NULL && b==NULL)return 1;
    if(a==NULL||b==NULL)return 0;
    if(flag==1&&a->version!=b->version)return 0;
    if(a->kind!=b->kind)return 0;
    if(a->u.vid!=b->u.vid)return 0;
    if(flag==2)return 1;
    if(a->access!=b->access)return 0;
    if(a->is_addr!=b->is_addr)return 0;
    return 1;
}

struct  DAGnode_ *DAG_search(Operand op,struct DAGnodelist_ *list,int flag){
    struct DAGnode_ *ret=NULL;
    int ver=0;
    for(int i=0;i<list->DAG_cnt;i++){
        if(eq_operand(op,list->array[i]->op,flag)&&list->array[i]->op->version>=ver){
            ret=list->array[i];
            ver=ret->op->version;//返回图中最新的一个
            if(flag==1)break;
        }
    }
    return ret;
}
void addDAGnode(struct DAGnode_ *node,struct DAGnodelist_ *list){
    if(list->array==NULL){
        list->capacity=64;
        list->DAG_cnt=0;
        list->array=malloc(sizeof(struct DAGnode_*)*64);
    }
    else if(list->DAG_cnt==list->capacity){
        struct DAGnode_ ** tmp=malloc(sizeof(struct DAGnode_*)*list->capacity*2);
        memcpy(tmp,list->array,sizeof(struct DAGnode_*)*list->capacity);
        list->capacity*=2;
        free(list->array);
        list->array=tmp;
    }
    list->array[list->DAG_cnt++]=node;
    return;
}
struct DAGnode_ *buildDAGnode_leaf(Operand op){
    struct DAGnode_ *node=malloc(sizeof(struct DAGnode_));
    node->kind=DAG_LEAF;
    node->op=op;
    node->op->version=0;
    return node;
}
void creatDAGnode(InterCodes code,struct DAGnodelist_ *list){
    if(code->code->kind==IR_ASSIGN){
        Operand left=code->code->u.assign.left;
        Operand right=code->code->u.assign.right;
        if(left->access==IR_POINT||right->access==IR_POINT||right->access==IR_ADDR){
            //如果是变量是指针，无法判断变量指向的内容版本
            //直接跳过该句ir
            //注意：如果其右值是重复表达式，会在之后的优化中被替换
            return;
        }
        struct DAGnode_ *cur_node=DAG_search(left,list,0);
        struct DAGnode_ *new_node=malloc(sizeof(struct DAGnode_));
        new_node->kind=DAG_ASSIGN;
        new_node->op=left;
        if(cur_node==NULL){
            left->version=0;
        }
        else{
            left->version=cur_node->op->version+1;
        }
        struct DAGnode_ *child=DAG_search(right,list,0);//获取当前最新的节点（右值一定是版本号最大的一个
        if(child==NULL){
            child=buildDAGnode_leaf(right);
            addDAGnode(child,list);
        }
        else{
            right->version=child->op->version;
        }
        new_node->child[0]=child;
        addDAGnode(new_node,list);
    }
    else if(code->code->kind>=11&&code->code->kind<=14){//加减乘除
        Operand result=code->code->u.binop.result;
        Operand op1=code->code->u.binop.op1;
        Operand op2=code->code->u.binop.op2;
        if(result->access==IR_POINT||
           op1->access==IR_POINT||op1->access==IR_ADDR||
           op2->access==IR_POINT||op2->access==IR_ADDR)
            return;
        struct DAGnode_ *newnode=malloc(sizeof(struct DAGnode_));
        newnode->kind=code->code->kind-10;//DAG_PLUS=1,IR_PLUS=11
        struct DAGnode_ *curnode=DAG_search(result,list,0);
        if(curnode==NULL){
            result->version=0;
        }
        else{
            result->version=curnode->op->version+1;
        }
        newnode->op=result;
        struct DAGnode_ *child1=DAG_search(op1,list,0),*child2=DAG_search(op2,list,0);
        if(child1==NULL){
            child1=buildDAGnode_leaf(op1);
            addDAGnode(child1,list);
        }
        else{
            op1->version=child1->op->version;
        }
        if(child2==NULL){
            child2=buildDAGnode_leaf(op2);
            addDAGnode(child2,list);
        }
        else{
            op2->version=child2->op->version;
        }
        newnode->child[0]=child1;
        newnode->child[1]=child2;
        addDAGnode(newnode,list);
    }
    else if(code->code->kind==IR_IFGOTO){
        Operand op1=code->code->u.gotop.op1;
        Operand op2=code->code->u.gotop.op2;
        struct DAGnode_ *node=NULL;
        node=DAG_search(op1,list,0);
        if(node==NULL)op1->version=0;
        else op1->version=node->op->version;
        node=NULL;
        node=DAG_search(op2,list,0);
        if(node==NULL)op2->version=0;
        else op2->version=node->op->version;
    }
    else if(code->code->kind==IR_ARG||code->code->kind==IR_WRITE||code->code->kind==IR_RETURN){
        Operand op=code->code->u.unaryop.unary;
        struct DAGnode_ *node=DAG_search(op,list,0);
        if(node==NULL)op->version=0;
        else op->version=node->op->version;
    }
    else if(code->code->kind==IR_READ){
        Operand op=code->code->u.unaryop.unary;
        struct DAGnode_ *node=DAG_search(op,list,0);
        struct DAGnode_ *newnode=malloc(sizeof(struct DAGnode_));
        newnode->kind=DAG_READ;
        newnode->op=op;
        if(node==NULL)newnode->op->version=0;
        else newnode->op->version=node->op->version+1;
        addDAGnode(newnode,list);
    }

}
Operand createConstOp(int x){
    Operand ret=malloc(sizeof(struct Operand_));
    ret->kind=IR_CONSTANT;
    ret->u.value=x;
    ret->version=0;
    return ret;
}
void replaceOperand_true(Operand src,Operand dst){
    src->kind=dst->kind;
    src->version=dst->version;
    src->u.vid=dst->u.vid;
}
int replaceOperand(Operand src,Operand dst,struct BasicBlock_ *bb){
    int flag=0;
    InterCodes code=bb->start;
    while(code!=bb->end){
        if(code->dead==1){code=code->next;continue;}
        switch(code->code->kind){
            case IR_ASSIGN:{
                Operand left=code->code->u.assign.left;
                Operand right=code->code->u.assign.right;
                if(eq_operand(right,src,1)){
                    flag++;
                    replaceOperand_true(right,dst);
                }
            }break;
            case IR_ADD:
            case IR_SUB:
            case IR_MUL:
            case IR_DIV:{
                Operand op1=code->code->u.binop.op1;
                Operand op2=code->code->u.binop.op2;
                Operand result=code->code->u.binop.result;
                if(eq_operand(op1,src,1)){
                    flag++;
                    replaceOperand_true(op1,dst);
                }
                if(eq_operand(op2,src,1)){
                    flag++;
                    replaceOperand_true(op2,dst);
                }
            }break;
            case IR_ARG:
            case IR_READ:
            case IR_RETURN:{
                Operand op=code->code->u.unaryop.unary;
                if(eq_operand(op,src,1)){
                    flag++;
                    replaceOperand_true(op,dst);
                }
            }break;
            case IR_IFGOTO:{
                Operand op1=code->code->u.gotop.op1;
                Operand op2=code->code->u.gotop.op2;
                if(eq_operand(op1,src,1)){
                    flag++;
                    replaceOperand_true(op1,dst);
                }
                if(eq_operand(op2,src,1)){
                    flag++;
                    replaceOperand_true(op2,dst);
                }
            }break;
        }
        code=code->next;
    }
    return flag;
}
void DAG_debugger(struct DAGnodelist_ *list){
    for(int i=0;i<list->DAG_cnt;i++){
        printf("%d: ",i);
        print_op(stdout,list->array[i]->op);
        printf("(ver:%d)\n",list->array[i]->op->version);
    }
}
struct DAGnodelist_ *buildDAG(struct BasicBlock_ *bb){
    InterCodes code=bb->start;
    struct DAGnodelist_ *list=malloc(sizeof(struct DAGnodelist_));
    list->array=NULL;
    list->DAG_cnt=0;
    list->capacity=0;
    while(code!=bb->end){
        if(code->dead==1){code=code->next;continue;}
        creatDAGnode(code,list);
        code=code->next;
    }
    return list;
}
void unbuildDAG(struct DAGnodelist_ *list){
    free(list->array);
    free(list);
    return;
}
int deleteSubExp(struct BasicBlock_ *bb){
    int flag=0;
    struct DAGnodelist_ *list=buildDAG(bb);
    //DAG_debugger(list);
    for(int i=list->DAG_cnt-1;i>=0;i--){
        if(list->array[i]->kind>=1&&list->array[i]->kind<=4){
            for(int j=0;j<i;j++){
                if(list->array[j]->kind!=list->array[i]->kind)continue;
                Operand src1=list->array[i]->child[0]->op;
                Operand src2=list->array[i]->child[1]->op;
                Operand dst1=list->array[j]->child[0]->op;
                Operand dst2=list->array[j]->child[1]->op;
                if(eq_operand(src1,dst1,1)&&eq_operand(src2,dst2,1)){
                    flag=1;
                    list->array[i]->kind = DAG_ASSIGN;
                    list->array[i]->child[0] = list->array[j];
                    list->array[i]->child[1] = NULL;
                    replaceOperand(list->array[i]->op,list->array[j]->op,bb);
                    break;
                }
                else if((list->array[i]->kind==DAG_PLUS||list->array[i]->kind==DAG_MUL)){
                    if(eq_operand(src1,dst2,1)&&eq_operand(src2,dst1,1)){
                        flag=1;
                        list->array[i]->kind = DAG_ASSIGN;
                        list->array[i]->child[0] = list->array[j];
                        list->array[i]->child[1] = NULL;
                        replaceOperand(list->array[i]->op,list->array[j]->op,bb);
                        break;
                    }
                }
            }
        }
    }
    //print_bb();
    return flag;
}
int foldConstant(struct BasicBlock_ *bb){
    struct DAGnodelist_ *list=buildDAG(bb);
    //DAG_debugger(list);
    int flag=0;
    InterCodes tmp=bb->start;
    while(tmp!=bb->end){
        if(tmp->dead==1){tmp=tmp->next;continue;}
        if(tmp->code->kind==IR_ASSIGN){
            Operand left=tmp->code->u.assign.left;
            struct Operand_ temp;
            Operand right=tmp->code->u.assign.right;
            if(right->kind==IR_CONSTANT){
                if(left->access==IR_NOMAL){
                    memcpy(&temp,left,sizeof(struct Operand_));
                    flag=replaceOperand(left,right,bb);
                    memcpy(left,&temp,sizeof(struct Operand_));
                }
            }
        }
        tmp=tmp->next;
    }
    tmp=bb->start;
    while(tmp!=bb->end){
        if(tmp->code->kind>=11&&tmp->code->kind<=14){
            Operand result=tmp->code->u.binop.result;
            Operand op1=tmp->code->u.binop.op1;
            Operand op2=tmp->code->u.binop.op2;
            if(op1->kind==IR_CONSTANT&&op2->kind==IR_CONSTANT){
                flag=1;
                tmp->code->u.assign.left=result;
                switch(tmp->code->kind){
                    case IR_ADD:tmp->code->u.assign.right=createConstOp(op1->u.value+op2->u.value);break;
                    case IR_SUB:tmp->code->u.assign.right=createConstOp(op1->u.value-op2->u.value);break;
                    case IR_MUL:tmp->code->u.assign.right=createConstOp(op1->u.value*op2->u.value);break;
                    case IR_DIV:tmp->code->u.assign.right=createConstOp(op1->u.value/op2->u.value);break;
                }
                tmp->code->kind=IR_ASSIGN;
                
            }
        }
        tmp=tmp->next;
    }
    //print_bb();
    return flag;
}
int ifalive(Operand op,InterCodes start,struct BasicBlock_ *bb){
    if(op->access==IR_POINT)return 1;
    int flag=0;
    InterCodes tmp=start->next;
    while(tmp!=bb->end){
        if(tmp->dead==1){tmp=tmp->next;continue;}
        if(tmp->code->kind==IR_ASSIGN){
            if(eq_operand(tmp->code->u.assign.left,op,2)){
                if(tmp->code->u.assign.left->access==IR_POINT)
                    if(op->access!=IR_POINT)return 1;
                    else return flag;
                else return flag;
            }
            if(eq_operand(tmp->code->u.assign.right,op,2)){return 1;}
        }
        else if(tmp->code->kind==IR_CALL){
            if(eq_operand(tmp->code->u.unaryop.unary,op,2)){
                if(tmp->code->u.unaryop.unary->access==IR_POINT)return 1;
                else return flag;
            }
        }
        else if(tmp->code->kind==IR_READ){
            if(eq_operand(tmp->code->u.unaryop.unary,op,2)){
                if(tmp->code->u.unaryop.unary->access==IR_POINT)return 1;
                else return flag;
            }
        }
        else if(tmp->code->kind>=11&&tmp->code->kind<=14){
            if(eq_operand(tmp->code->u.binop.result,op,2)){
                if(tmp->code->u.binop.result->access==IR_POINT)return 1;
                else return flag;
            }
            if(eq_operand(tmp->code->u.binop.op1,op,2)){return 1;}
            if(eq_operand(tmp->code->u.binop.op2,op,2)){return 1;}
        }
        else if(tmp->code->kind==IR_IFGOTO){
            if(eq_operand(tmp->code->u.gotop.op1,op,2)){return 1;}
            if(eq_operand(tmp->code->u.gotop.op2,op,2)){return 1;}
        }
        else if(tmp->code->kind==IR_RETURN){
            if(eq_operand(tmp->code->u.unaryop.unary,op,2)){return 1;}
        }
        else if(tmp->code->kind==IR_ARG){
            if(eq_operand(tmp->code->u.unaryop.unary,op,2)){return 1;}
        }
        else if(tmp->code->kind==IR_WRITE){
            if(eq_operand(tmp->code->u.unaryop.unary,op,2)){return 1;}
        }
        tmp=tmp->next;
    }
    if(op->kind==IR_TMPOP&&(bb->end==NULL||bb->end->code->kind==IR_FUNCTION))return 0;
    return 1;
}
int removeDeadCode(struct BasicBlock_ *bb){
    int flag=0;
    InterCodes tmp=bb->start;
    while(tmp!=bb->end){
        if(tmp->dead==1){tmp=tmp->next;continue;}
        Operand op=NULL;
        if(tmp->code->kind==IR_ASSIGN){
            op=tmp->code->u.assign.left;
        }
        else if(tmp->code->kind==IR_CALL){
            op=tmp->code->u.unaryop.unary;
        }
        else if(tmp->code->kind==IR_READ){
            op=tmp->code->u.unaryop.unary;
        }
        else if(tmp->code->kind>=11&&tmp->code->kind<=14){
            op=tmp->code->u.binop.result;
        }
        if(op!=NULL){
            if(op->access!=IR_NOMAL){tmp=tmp->next;continue;}
            int flag1=ifalive(op,tmp,bb);
            if(flag1==0){
                flag=1;
                tmp->dead=1;
            }
        }
        tmp=tmp->next;
    }
    return flag;
}
void localoptimize(){
    for(int j=0;j<gbblist.gbb_cnt;j++){
        struct BB_List_ *bblist=&(gbblist.bblist[j]);
        for(int i=0;i<bblist->bb_cnt;i++){
            int flag1=deleteSubExp(&(bblist->array[i]));
            int flag2=foldConstant(&(bblist->array[i]));
            int flag3=removeDeadCode(&(bblist->array[i]));
            int cnt=0;
            while(flag1||flag2||flag3){
                flag1=0;
                flag1=deleteSubExp(&(bblist->array[i]));
                flag2=foldConstant(&(bblist->array[i]));
                flag3=removeDeadCode(&(bblist->array[i]));
                fprintf(stderr,"%d%d%d\n",flag1,flag2,flag3);
                cnt++;
                if(cnt>15)break;
            }
        }
    }
}
void addpre(struct BasicBlock_ *bb,int i){
    if(bb->pre==NULL){
        bb->pre_capacity=8;
        bb->precnt=0;
        bb->pre=malloc(sizeof(int)*8);
    }
    else if(bb->pre_capacity==bb->precnt){
        int *npre=malloc(sizeof(int)*bb->pre_capacity*2);
        memcpy(npre,bb->pre,sizeof(int)*bb->pre_capacity);
        bb->pre_capacity*=2;
        free(bb->pre);
        bb->pre=npre;
    }
    bb->pre[bb->precnt++]=i;
    return;
}
void build_CFG_true(struct BB_List_ *bblist){
    for(int i=0;i<bblist->bb_cnt;i++){
        bblist->array[i].next[0]=-1;
        bblist->array[i].next[1]=-1;
        InterCodes end=bblist->array[i].end;
        if(end==NULL)end=ir_tail;
        else end=end->prev;
        if(end->code->kind==IR_RETURN){//return 没有后继
            continue;
        }
        else if(end->code->kind==IR_GOTO){
            int labelno=end->code->u.unaryop.unary->u.lableno;
            for(int j=0;j<bblist->bb_cnt;j++){
                if(bblist->array[j].start->code->kind==IR_LABEL&&bblist->array[j].start->code->u.unaryop.unary->u.lableno==labelno){
                    addpre(&(bblist->array[j]),i);
                    bblist->array[i].next[0]=j;
                }
            }
        }
        else if(end->code->kind==IR_IFGOTO){
            int labelno=end->code->u.gotop.lable->u.lableno;
            for(int j=0;j<bblist->bb_cnt;j++){
                if(bblist->array[j].start->code->kind==IR_LABEL&&bblist->array[j].start->code->u.unaryop.unary->u.lableno==labelno){
                    addpre(&(bblist->array[j]),i);
                    bblist->array[i].next[0]=j;
                }
            }
            if(i+1==bblist->bb_cnt){
                bblist->array[i].next[0]=-1;
            }
            else{
                bblist->array[i].next[0]=i+1;
                addpre(&(bblist->array[i+1]),i);
            }
        }
        else {
            if(i+1==bblist->bb_cnt){
                bblist->array[i].next[0]=-1;
            }
            else{
                bblist->array[i].next[0]=i+1;
                addpre(&(bblist->array[i+1]),i);
            }
        }
    }
}
void CFG_debugger(struct BB_List_ *bblist){
    for(int i=0;i<bblist->bb_cnt;i++){
        printf("%d:",i);
        /*//通过后继遍历图
        printf("%d ",bblist->array[i].next[0]);
        if(bblist->array[i].next[1]!=-1)
            printf("%d ",bblist->array[i].next[1]);
        printf("\n");*/
        for(int j=0;j<bblist->array[i].precnt;j++){
            printf("%d ",bblist->array[i].pre[j]);
        }
        printf("\n");
    }
}
void build_CFG(){
    for(int i=0;i<gbblist.gbb_cnt;i++){
        build_CFG_true(&(gbblist.bblist[i]));
        CFG_debugger(&(gbblist.bblist[i]));
    }
}

void globaloptimize(){
    build_CFG();
}
void optimize(FILE *fp){
    build_bb_global();
    localoptimize();
    globaloptimize();
    print_bb(fp);
}