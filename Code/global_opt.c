#include "global_opt.h"
extern InterCodes ir_head, ir_tail;
extern struct Global_BBlist_ gbblist;
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
void build_CFG(struct BB_List_ *bblist){
    for(int i=0;i<bblist->bb_cnt;i++){
        bblist->array[i].next[0]=-1;
        bblist->array[i].next[1]=-1;
        InterCodes end=bblist->array[i].end;
        if(end==NULL)end=ir_tail;
        else end=end->prev;
        if(end->code->kind==IR_RETURN){//return 没有后继
            continue;
        }
        else if(end->code->kind==IR_GOTO){//goto 的后继是跳转目标
            int labelno=end->code->u.unaryop.unary->u.lableno;
            for(int j=0;j<bblist->bb_cnt;j++){
                if(bblist->array[j].start->code->kind==IR_LABEL&&bblist->array[j].start->code->u.unaryop.unary->u.lableno==labelno){
                    addpre(&(bblist->array[j]),i);
                    bblist->array[i].next[0]=j;
                }
            }
        }
        else if(end->code->kind==IR_IFGOTO){//ifgoto 的后继是跳转目标和ir的下一个bb
            int labelno=end->code->u.gotop.lable->u.lableno;
            for(int j=0;j<bblist->bb_cnt;j++){
                if(bblist->array[j].start->code->kind==IR_LABEL&&bblist->array[j].start->code->u.unaryop.unary->u.lableno==labelno){
                    addpre(&(bblist->array[j]),i);
                    bblist->array[i].next[0]=j;
                }
            }
            if(i+1==bblist->bb_cnt){
                bblist->array[i].next[1]=-1;
            }
            else{
                bblist->array[i].next[1]=i+1;
                addpre(&(bblist->array[i+1]),i);
            }
        }
        else {//其它情况，后继是ir的下一个bb
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
    printf("\n%s:\n",bblist->array[0].start->code->u.unaryop.unary->u.funcname);
    printf("next graph\n");
    for(int i=0;i<bblist->bb_cnt;i++){
        printf("%d:",i);
        //通过后继遍历图
        if(bblist->array[i].next[0]!=-1){
            printf("%d ",bblist->array[i].next[0]);
            if(bblist->array[i].next[1]!=-1)
                printf("%d ",bblist->array[i].next[1]);
        }
        printf("\n");
    }
    printf("\nprev graph\n");
    for(int i=0;i<bblist->bb_cnt;i++){
        printf("%d:",i);
        //通过前驱遍历
        for(int j=0;j<bblist->array[i].precnt;j++){
            printf("%d ",bblist->array[i].pre[j]);
        }
        printf("\n");
    }
}
static inline void update_minmax(Operand op,int *tmin,int *tmax,int *vmin,int *vmax){
    int flag=op->kind;
    int cur=op->u.value;
    if(flag==IR_VARIABLE){
        if(cur<(*vmin))*vmin=cur;
        if(cur>(*vmax))*vmax=cur;
    }
    else if(flag==IR_TMPOP){
        if(cur<(*tmin))*tmin=cur;
        if(cur>(*tmax))*tmax=cur;
    }
}
void get_v_cnt(struct BB_List_ *bblist,int *tmin,int *tmax,int *vmin,int *vmax){
    for(int i=0;i<bblist->bb_cnt;i++){
        for(InterCodes it=bblist->array[i].start;it!=bblist->array[i].end;it=it->next){
            if(it->dead==1)continue;
            switch(it->code->kind){
                case IR_ADD:
                case IR_SUB:
                case IR_MUL:
                case IR_DIV:{
                    update_minmax(it->code->u.binop.op1,tmin,tmax,vmin,vmax);
                    update_minmax(it->code->u.binop.op2,tmin,tmax,vmin,vmax);
                    update_minmax(it->code->u.binop.result,tmin,tmax,vmin,vmax);
                }break;
                case IR_ASSIGN:{
                    update_minmax(it->code->u.assign.left,tmin,tmax,vmin,vmax);
                    update_minmax(it->code->u.assign.right,tmin,tmax,vmin,vmax);
                }break;
                case IR_CALL:{
                    update_minmax(it->code->u.assign.left,tmin,tmax,vmin,vmax);
                }break;
                case IR_READ:
                case IR_WRITE:
                case IR_ARG:
                case IR_PARAM:
                case IR_RETURN:{
                    update_minmax(it->code->u.unaryop.unary,tmin,tmax,vmin,vmax);
                }break;
                case IR_IFGOTO:{
                    update_minmax(it->code->u.gotop.op1,tmin,tmax,vmin,vmax);
                    update_minmax(it->code->u.gotop.op2,tmin,tmax,vmin,vmax);
                }break;
                case IR_DEC:{
                    update_minmax(it->code->u.assign.left,tmin,tmax,vmin,vmax);
                }break;
            }
        }
    }
    if(*tmin==99999)*tmin=0;
    if(*vmin==99999)*vmin=0;
}
int get_index(Operand x,int tmin,int tmax,int vmin,int vmax){
    if(x->kind==IR_VARIABLE){
        int id=x->u.vid-vmin+(tmax-tmin)+1;
        return id;
    }
    else if(x->kind==IR_TMPOP){
        int id=x->u.tmpno-tmin;
        return id;
    }
    return -1;
}
void topo_sort(struct BB_List_ *bblist,int *list,int i){

    int cnt=0;
    while(list[cnt]!=-1){
        if(list[cnt]==i)return;
        cnt++;
    }

    list[cnt]=i;
    if(bblist->array[i].next[0]!=-1){
        topo_sort(bblist,list,bblist->array[i].next[0]);
        if(bblist->array[i].next[1]!=-1){
            topo_sort(bblist,list,bblist->array[i].next[1]);
        }
    }
}

static int eq_operand(Operand a,Operand b){
    if(a==NULL && b==NULL)return 1;
    if(a==NULL||b==NULL)return 0;
    if(a->kind!=b->kind)return 0;
    if(a->u.vid!=b->u.vid)return 0;
    if(a->access!=b->access)return 0;
    if(a->is_addr!=b->is_addr)return 0;
    return 1;
}
int check_intercodes(InterCodes a,InterCodes b){
    if(a->code->kind!=b->code->kind)return 0;
    if(a->code->kind==IR_ADD||a->code->kind==IR_MUL){
        if(eq_operand(a->code->u.binop.op1,b->code->u.binop.op2)==0
        &&eq_operand(a->code->u.binop.op2,b->code->u.binop.op1)==0
        &&eq_operand(a->code->u.binop.result,b->code->u.binop.result)==0)
        return 1;
    }
    if(eq_operand(a->code->u.binop.result,b->code->u.binop.result)==0)return 0;
    if(eq_operand(a->code->u.binop.op1,b->code->u.binop.op1)==0)return 0;
    if(a->code->kind==IR_ASSIGN)return 1;
    if(eq_operand(a->code->u.binop.op2,b->code->u.binop.op2)==0)return 0;
    return 0;
}
int check_exp(InterCodes a,InterCodes b){
    if(a->code->kind!=b->code->kind)return 0;
    if(a->code->u.binop.result==b->code->u.binop.result)return 0;
    if(a->code->kind==IR_ADD||a->code->kind==IR_MUL){
        if(eq_operand(a->code->u.binop.op1,b->code->u.binop.op2)==1
        &&eq_operand(a->code->u.binop.op2,b->code->u.binop.op1)==1)
        return 1;
    }
    if(eq_operand(a->code->u.binop.op1,b->code->u.binop.op1)==0)return 0;
    if(eq_operand(a->code->u.binop.op2,b->code->u.binop.op2)==0)return 0;
    return 1;
}
int subExp_merge(struct BB_List_ *bblist,struct BasicBlock_ *bb,int tmin,int tmax,int vmin,int vmax){
    int flag=0;
    for(int i=0;i<bb->precnt;i++){
        struct BasicBlock_ *pre=&(bblist->array[bb->pre[i]]);
        for(int j=0;j<tmax-tmin+vmax-vmin+2;j++){
            if(pre->out[j]!=bb->in[j]){
                if(bb->in[j]==0){
                    bb->in[j]=pre->out[j];
                    bb->ivalue[j]=pre->ivalue[j];
                    flag=1;
                }
                else if(bb->in[j]==1){
                    if(pre->out[j]==1){
                        if(check_intercodes(bb->ivalue[j],pre->ivalue[j])==0){
                            bb->in[j]=2;flag=1;
                        }
                    }
                    else if(pre->out[j]==2){
                        bb->in[j]=2;
                        flag=1;
                    }
                }
            }
        }
    }
    return flag;
}
int subExp_gen(struct BasicBlock_ *bb,int tmin,int tmax,int vmin,int vmax){
    int flag=0;
    InterCodes tmp=bb->start;
    while(tmp!=bb->end){
        if(tmp->dead==1){tmp=tmp->next;continue;}
        switch(tmp->code->kind){
                
                case IR_ADD:
                case IR_SUB:
                case IR_MUL:
                case IR_DIV:{
                    Operand res=tmp->code->u.binop.result;
                    Operand op1=tmp->code->u.binop.op1;
                    Operand op2=tmp->code->u.binop.op2;
                    if(res->access==IR_POINT)break;
                    if(bb->out[get_index(res,tmin,tmax,vmin,vmax)]==0){
                        flag=1;
                        for(int i=0;i<tmax-tmin+vmax-vmin+2;i++){
                            if(bb->out[get_index(res,tmin,tmax,vmin,vmax)]!=1)continue;
                            InterCodes tmp2=bb->ivalue[i];
                            if(eq_operand(tmp2->code->u.binop.op1,res)){
                                bb->out[i]=2;
                            }
                            if(eq_operand(tmp2->code->u.binop.op2,res)){
                                bb->out[i]=2;
                            }
                        }
                        bb->out[get_index(res,tmin,tmax,vmin,vmax)]=1;
                        bb->ivalue[get_index(res,tmin,tmax,vmin,vmax)]=tmp;
                    }
                    else if(bb->out[get_index(res,tmin,tmax,vmin,vmax)]==1){
                        if(check_intercodes(tmp,bb->ivalue[get_index(res,tmin,tmax,vmin,vmax)])!=1){
                            bb->out[get_index(res,tmin,tmax,vmin,vmax)]=2;
                            flag=1;
                        }
                    }

                }break;
                case IR_ASSIGN:
                case IR_READ:
                case IR_CALL:{
                    Operand op=tmp->code->u.unaryop.unary;
                    for(int i=0;i<tmax-tmin+vmax-vmin+2;i++){
                            if(bb->out[i]!=1)continue;
                            InterCodes tmp2=bb->ivalue[i];
                            if(eq_operand(tmp2->code->u.binop.op1,op)){
                                flag=1;
                                bb->out[i]=2;
                            }
                            if(eq_operand(tmp2->code->u.binop.op2,op)){
                                flag=1;
                                bb->out[i]=2;
                            }
                            if(eq_operand(tmp2->code->u.binop.result,op)){
                                flag=1;
                                bb->out[i]=2;
                            }
                        }
                }break;
                case IR_WRITE:
                case IR_ARG:break;
                case IR_PARAM:{

                }break;
                case IR_RETURN:
                case IR_IFGOTO:
                case IR_DEC:break;
        }
        tmp=tmp->next;
    }
    return flag;
}
static void replaceOperand_true(Operand src,Operand dst){
    src->kind=dst->kind;
    src->u.vid=dst->u.vid;
}
static void replaceOperand(Operand src,Operand dst,InterCodes start,InterCodes end){
    for(InterCodes i=start->next;i!=end;i=i->next){
        if(i->dead==1)continue;
        if(i->code->kind!=IR_IFGOTO&&eq_operand(i->code->u.assign.left,src))break;
        Operand op1=i->code->u.gotop.op1;
        Operand op2=i->code->u.gotop.op2;
        Operand op3=i->code->u.binop.op2;
        if(op1&&eq_operand(op1,src)){
            replaceOperand_true(op1,dst);
        }
        if(op2&&eq_operand(op2,src)){
            replaceOperand_true(op2,dst);
        }
        if(op3&&eq_operand(op3,src)){
            replaceOperand_true(op3,dst);
        }
    }
}
void deleteSubExp_G(struct BB_List_ *bblist,int tmin,int tmax,int vmin,int vmax){
    int *topolist=malloc(sizeof(int)*bblist->bb_cnt);
    memset(topolist,-1,sizeof(int)*bblist->bb_cnt);
    topo_sort(bblist,topolist,0);
    for(int i=0;i<bblist->bb_cnt;i++){
        int *in=malloc(sizeof(int)*(tmax-tmin+vmax-vmin+2));
        memset(in,0,sizeof(int)*(tmax-tmin+vmax-vmin+2));
        int *out=malloc(sizeof(int)*(tmax-tmin+vmax-vmin+2));
        memset(out,0,sizeof(int)*(tmax-tmin+vmax-vmin+2));
        InterCode *value=malloc(sizeof(InterCodes)*(tmax-tmin+vmax-vmin+2));
        memset(value,0,sizeof(InterCodes)*(tmax-tmin+vmax-vmin+2));
        bblist->array[i].in=in;
        bblist->array[i].out=out;
        bblist->array[i].ivalue=value;
    }
    int flag=1;
    subExp_gen(&(bblist->array[0]),tmin,tmax,vmin,vmax);
    while(flag){
        flag=0;
        for(int i=1;i<bblist->bb_cnt;i++){
            int flag1=subExp_merge(bblist,&(bblist->array[topolist[i]]),tmin,tmax,vmin,vmax);
            int flag2=0;
            if(flag1){
                memcpy(bblist->array[topolist[i]].out,bblist->array[topolist[i]].in,sizeof(int)*(tmax-tmin+vmax-vmin+2));
                flag2=subExp_gen(&(bblist->array[topolist[i]]),tmin,tmax,vmin,vmax);
            }
            flag=flag|flag1|flag2;
        }
    }
    /*for(int i=0;i<bblist->bb_cnt;i++){
        for(int j=0;j<tmax-tmin+vmax-vmin+2;j++){
            printf("%d ",bblist->array[i].in[j]);
        }
        printf("\n");
    }
    printf("\n");*/
    for(int i=0;i<bblist->bb_cnt;i++){
        struct BasicBlock_ *bb=&(bblist->array[i]);
        for(int j=0;j<tmax-tmin+vmax-vmin+2;j++){
            if(bb->in[j]!=1)continue;
            InterCodes dstcode=bb->ivalue[j];
            Operand dst=dstcode->code->u.assign.left;
            for(InterCodes tmp=bb->start;tmp!=bb->end;tmp=tmp->next){
                if(tmp->dead==1)continue;
                if(tmp->code->kind==IR_CALL||tmp->code->kind==IR_READ||tmp->code->kind==IR_ASSIGN){
                    Operand op=tmp->code->u.assign.left;
                    if(eq_operand(op,dst)==1)break;
                }
                else if(tmp->code->kind>=11&&tmp->code->kind<=14){
                    Operand op=tmp->code->u.binop.result;
                    Operand op1=tmp->code->u.binop.op1;
                    Operand op2=tmp->code->u.binop.op2;
                    if(eq_operand(op,dst)==1)break;
                    if(check_exp(tmp,dstcode)==1){
                        //printf("%s%d ",dst->kind==IR_TMPOP?"t":"v",dst->u.value);
                        //printf("%s%d ",dstcode->code->u.binop.op1->kind==IR_TMPOP?"t":"v",dstcode->code->u.binop.op1->u.value);
                        //printf("%s%d\t",dstcode->code->u.binop.op2->kind==IR_TMPOP?"t":"v",dstcode->code->u.binop.op2->u.value);
                        //printf("%s%d ",op->kind==IR_TMPOP?"t":"v",op->u.value);
                        //printf("%s%d ",op1->kind==IR_TMPOP?"t":"v",op1->u.value);
                        //printf("%s%d\n",op2->kind==IR_TMPOP?"t":"v",op2->u.value);
                        replaceOperand(op,dst,tmp,bb->end);
                    }
                }
                
            }
        }
    }
    for(int i=0;i<bblist->bb_cnt;i++){
        free(bblist->array[i].in);
        free(bblist->array[i].out);
        free(bblist->array[i].ivalue);
    }
    free(topolist);
}

int constant_merge(struct BB_List_ *bblist,struct BasicBlock_ *bb,int tmin,int tmax,int vmin,int vmax){
    int flag=0;
    for(int i=0;i<bb->precnt;i++){
        struct BasicBlock_ *pre=&(bblist->array[bb->pre[i]]);
        for(int j=0;j<tmax-tmin+vmax-vmin+2;j++){
            if(pre->out[j]!=bb->in[j]){
                if(bb->in[j]==0){
                    bb->in[j]=pre->out[j];
                    bb->value[j]=pre->value[j];
                    flag=1;
                }
                else if(bb->in[j]==1){
                    if(pre->out[j]==1){
                        if(bb->value[j]!=pre->value[j]){
                            bb->in[j]=2;flag=1;
                        }
                    }
                    else if(pre->out[j]==2){
                        bb->in[j]=pre->out[j];
                        flag=1;
                    }
                }
            }
        }
    }
    return flag;
}
int constant_gen(struct BasicBlock_ *bb,int tmin,int tmax,int vmin,int vmax){
    int flag=0;
    InterCodes tmp=bb->start;
    while(tmp!=bb->end){
        if(tmp->dead==1){tmp=tmp->next;continue;}
        switch(tmp->code->kind){
                case IR_ASSIGN:{
                    Operand left=tmp->code->u.assign.left;
                    Operand right=tmp->code->u.assign.right;
                    if(left->access==IR_POINT)break;
                    if(right->kind==IR_CONSTANT){
                        if(bb->out[get_index(left,tmin,tmax,vmin,vmax)]==0){
                            flag=1;
                            bb->out[get_index(left,tmin,tmax,vmin,vmax)]=1;
                            bb->value[get_index(left,tmin,tmax,vmin,vmax)]=right->u.value;
                        }
                        else if(bb->out[get_index(left,tmin,tmax,vmin,vmax)]==1){
                            if(bb->value[get_index(left,tmin,tmax,vmin,vmax)]!=right->u.value){
                                bb->out[get_index(left,tmin,tmax,vmin,vmax)]=2;
                                flag=1;
                            }
                        }
                    }
                    else if(right->kind==IR_TMPOP||right->kind==IR_VARIABLE){
                        if(right->access==IR_ADDR||right->access==IR_POINT){
                            if(bb->out[get_index(left,tmin,tmax,vmin,vmax)]!=2){
                                bb->out[get_index(left,tmin,tmax,vmin,vmax)]=2;flag=1;
                            }
                        }
                        else{
                            if(bb->out[get_index(right,tmin,tmax,vmin,vmax)]==1){
                                if(bb->out[get_index(left,tmin,tmax,vmin,vmax)]==0){
                                    flag=1;
                                    bb->out[get_index(left,tmin,tmax,vmin,vmax)]=1;
                                    bb->value[get_index(left,tmin,tmax,vmin,vmax)]=bb->out[get_index(right,tmin,tmax,vmin,vmax)];
                                }
                                else if(bb->out[get_index(left,tmin,tmax,vmin,vmax)]==1){
                                    if(bb->value[get_index(left,tmin,tmax,vmin,vmax)]!=bb->value[get_index(right,tmin,tmax,vmin,vmax)]){
                                        bb->out[get_index(left,tmin,tmax,vmin,vmax)]=2;
                                        flag=1;
                                    }
                                }
                            }
                            else if(bb->out[get_index(right,tmin,tmax,vmin,vmax)]==2){
                                if(bb->out[get_index(left,tmin,tmax,vmin,vmax)]!=2){
                                    flag=1;
                                    bb->out[get_index(left,tmin,tmax,vmin,vmax)]=2;
                                }
                            }
                        }
                    }
                }break;
                case IR_ADD:
                case IR_SUB:
                case IR_MUL:
                case IR_DIV:{
                    Operand result=tmp->code->u.binop.result;
                    Operand op1=tmp->code->u.binop.op1;
                    Operand op2=tmp->code->u.binop.op2;
                    if(result->access==IR_POINT)break;
                    if(op1->access==IR_ADDR||op1->access==IR_POINT||op2->access==IR_ADDR||op2->access==IR_POINT){
                        if(bb->out[get_index(result,tmin,tmax,vmin,vmax)]!=2){
                        bb->out[get_index(result,tmin,tmax,vmin,vmax)]=2;flag=1;}
                    }
                    else{
                        if(op1->kind==IR_CONSTANT){
                            if(op2->kind!=IR_CONSTANT){
                                if(bb->out[get_index(op2,tmin,tmax,vmin,vmax)]==0){}
                                else if(bb->out[get_index(op2,tmin,tmax,vmin,vmax)]==1){
                                    int c1=op1->u.value;
                                    int c2=bb->value[get_index(op2,tmin,tmax,vmin,vmax)];
                                    int c3=0;
                                    switch (tmp->code->kind){
                                        case IR_ADD:c3=c1+c2;break;
                                        case IR_SUB:c3=c1-c2;break;
                                        case IR_MUL:c3=c1*c2;break;
                                        case IR_DIV:c3=c1/c2;break;
                                    }
                                    if(bb->out[get_index(result,tmin,tmax,vmin,vmax)]==0){
                                        flag=1;
                                        bb->out[get_index(result,tmin,tmax,vmin,vmax)]=1;
                                        bb->value[get_index(result,tmin,tmax,vmin,vmax)]=c3;
                                    }
                                    else if(bb->out[get_index(result,tmin,tmax,vmin,vmax)]==1){
                                        if(bb->value[get_index(result,tmin,tmax,vmin,vmax)]!=c3){
                                            bb->out[get_index(result,tmin,tmax,vmin,vmax)]=2;
                                            flag=1;
                                        }
                                    }

                                }
                                else if(bb->out[get_index(op2,tmin,tmax,vmin,vmax)]==2){
                                    if(bb->out[get_index(result,tmin,tmax,vmin,vmax)]!=2){
                                        bb->out[get_index(result,tmin,tmax,vmin,vmax)]=2;
                                        flag=1;
                                    }
                                }
                            }
                            else{
                                int c1=op1->u.value;
                                int c2=op2->u.value;
                                int c3=0;
                                switch (tmp->code->kind){
                                    case IR_ADD:c3=c1+c2;break;
                                    case IR_SUB:c3=c1-c2;break;
                                    case IR_MUL:c3=c1*c2;break;
                                    case IR_DIV:c3=c1/c2;break;
                                }
                                if(bb->out[get_index(result,tmin,tmax,vmin,vmax)]==0){
                                    flag=1;
                                    bb->out[get_index(result,tmin,tmax,vmin,vmax)]=1;
                                    bb->value[get_index(result,tmin,tmax,vmin,vmax)]=c3;
                                }
                                else if(bb->out[get_index(result,tmin,tmax,vmin,vmax)]==1){
                                    if(bb->value[get_index(result,tmin,tmax,vmin,vmax)]!=c3){
                                        bb->out[get_index(result,tmin,tmax,vmin,vmax)]=2;
                                        flag=1;
                                    }
                                }
                            }
                        }
                        else if(op2->kind==IR_CONSTANT){
                            if(bb->out[get_index(op1,tmin,tmax,vmin,vmax)]==0){}
                            else if(bb->out[get_index(op1,tmin,tmax,vmin,vmax)]==1){
                                int c1=bb->value[get_index(op1,tmin,tmax,vmin,vmax)];
                                int c2=op2->u.value;
                                int c3=0;
                                switch (tmp->code->kind){
                                    case IR_ADD:c3=c1+c2;break;
                                    case IR_SUB:c3=c1-c2;break;
                                    case IR_MUL:c3=c1*c2;break;
                                    case IR_DIV:c3=c1/c2;break;
                                }
                                if(bb->out[get_index(result,tmin,tmax,vmin,vmax)]==0){
                                    flag=1;
                                    bb->out[get_index(result,tmin,tmax,vmin,vmax)]=1;
                                    bb->value[get_index(result,tmin,tmax,vmin,vmax)]=c3;
                                }
                                else if(bb->out[get_index(result,tmin,tmax,vmin,vmax)]==1){
                                    if(bb->value[get_index(result,tmin,tmax,vmin,vmax)]!=c3){
                                        bb->out[get_index(result,tmin,tmax,vmin,vmax)]=2;
                                        flag=1;
                                    }
                                }
                            }
                            else if(bb->out[get_index(op1,tmin,tmax,vmin,vmax)]==2){
                                if(bb->out[get_index(result,tmin,tmax,vmin,vmax)]!=2){
                                    bb->out[get_index(result,tmin,tmax,vmin,vmax)]=2;
                                    flag=1;
                                }
                            }
                        }
                        else if(bb->out[get_index(op1,tmin,tmax,vmin,vmax)]==0||bb->out[get_index(op2,tmin,tmax,vmin,vmax)]==0){

                        }
                        else if(bb->out[get_index(op1,tmin,tmax,vmin,vmax)]==2||bb->out[get_index(op2,tmin,tmax,vmin,vmax)]==2){
                            if(bb->out[get_index(result,tmin,tmax,vmin,vmax)]!=2){
                                flag=1;bb->out[get_index(result,tmin,tmax,vmin,vmax)]=2;
                            }
                        }
                        else{
                            int c1=bb->value[get_index(op1,tmin,tmax,vmin,vmax)];
                            int c2=bb->value[get_index(op2,tmin,tmax,vmin,vmax)];
                            int c3=0;
                            switch (tmp->code->kind)
                            {
                            case IR_ADD:c3=c1+c2;break;
                            case IR_SUB:c3=c1-c2;break;
                            case IR_MUL:c3=c1*c2;break;
                            case IR_DIV:c3=c1/c2;break;
                            }
                            if(bb->out[get_index(result,tmin,tmax,vmin,vmax)]==0){
                                flag=1;
                                bb->out[get_index(result,tmin,tmax,vmin,vmax)]=1;
                                bb->value[get_index(result,tmin,tmax,vmin,vmax)]=c3;
                            }
                            else if(bb->out[get_index(result,tmin,tmax,vmin,vmax)]==1){
                                if(bb->value[get_index(result,tmin,tmax,vmin,vmax)]!=c3){
                                    bb->out[get_index(result,tmin,tmax,vmin,vmax)]=2;
                                    flag=1;
                                }
                            }
                        }
                    }
                }break;
                case IR_CALL:{
                    Operand op=tmp->code->u.assign.left;
                    if(op->access==IR_POINT)break;
                    if(bb->out[get_index(op,tmin,tmax,vmin,vmax)]!=2){
                        flag=1;
                        bb->out[get_index(op,tmin,tmax,vmin,vmax)]=2;
                    }
                }break;
                case IR_READ:{
                    Operand op=tmp->code->u.unaryop.unary;
                    if(bb->out[get_index(op,tmin,tmax,vmin,vmax)]!=2){
                        flag=1;
                        bb->out[get_index(op,tmin,tmax,vmin,vmax)]=2;
                    }
                }break;
                case IR_WRITE:
                case IR_ARG:break;
                case IR_PARAM:{
                    Operand op=tmp->code->u.unaryop.unary;
                    if(bb->out[get_index(op,tmin,tmax,vmin,vmax)]!=2){
                        flag=1;
                        bb->out[get_index(op,tmin,tmax,vmin,vmax)]=2;
                    }
                }break;
                case IR_RETURN:
                case IR_IFGOTO:
                case IR_DEC:break;
        }
        tmp=tmp->next;
    }
    return flag;
}
void replace_const_g(struct BasicBlock_ *bb,int kind,int id,int value){
    InterCodes tmp=bb->start;
    while(tmp!=bb->end){
        if(tmp->dead==1){tmp=tmp->next;continue;}
        if(tmp->code->kind==IR_ASSIGN||tmp->code->kind==IR_CALL){
            Operand op=tmp->code->u.assign.left;
            if(op->access!=IR_POINT&&op->kind==kind&&op->u.vid==id)return;
            if(tmp->code->kind==IR_ASSIGN){
                Operand op1=tmp->code->u.assign.right;
                if(op1->kind==kind&&op1->u.vid==id&&op1->access==IR_NOMAL){
                    Operand nop=malloc(sizeof(struct Operand_));
                    memcpy(nop,op1,sizeof(struct Operand_));
                    nop->kind=IR_CONSTANT;nop->u.value=value;
                    tmp->code->u.assign.right=nop;
                }
            }
        }
        else if(tmp->code->kind==IR_READ){
            Operand op=tmp->code->u.unaryop.unary;
            if(op->access!=IR_POINT&&op->kind==kind&&op->u.vid==id)return;
        }
        else if(tmp->code->kind>=11&&tmp->code->kind<=14){
            Operand op=tmp->code->u.binop.result;
            Operand op1=tmp->code->u.binop.op1;
            Operand op2=tmp->code->u.binop.op2;
            if(op->access!=IR_POINT&&op->kind==kind&&op->u.vid==id)return;
            if(op1->kind==kind&&op1->u.vid==id&&op1->access==IR_NOMAL){
                Operand nop=malloc(sizeof(struct Operand_));
                memcpy(nop,op1,sizeof(struct Operand_));
                nop->kind=IR_CONSTANT;nop->u.value=value;
                tmp->code->u.binop.op1=nop;
            }
            if(op2->kind==kind&&op2->u.vid==id&&op2->access==IR_NOMAL){
                Operand nop=malloc(sizeof(struct Operand_));
                memcpy(nop,op2,sizeof(struct Operand_));
                nop->kind=IR_CONSTANT;nop->u.value=value;
                tmp->code->u.binop.op2=nop;
            }
        }
        tmp=tmp->next;
    }
}
void foldConstant_G(struct BB_List_ *bblist,int tmin,int tmax,int vmin,int vmax){
    int *topolist=malloc(sizeof(int)*bblist->bb_cnt);
    memset(topolist,-1,sizeof(int)*bblist->bb_cnt);
    topo_sort(bblist,topolist,0);
    for(int i=0;i<bblist->bb_cnt;i++){
        int *in=malloc(sizeof(int)*(tmax-tmin+vmax-vmin+2));
        memset(in,0,sizeof(int)*(tmax-tmin+vmax-vmin+2));
        int *out=malloc(sizeof(int)*(tmax-tmin+vmax-vmin+2));
        memset(out,0,sizeof(int)*(tmax-tmin+vmax-vmin+2));
        int *value=malloc(sizeof(int)*(tmax-tmin+vmax-vmin+2));
        memset(value,0,sizeof(int)*(tmax-tmin+vmax-vmin+2));
        bblist->array[i].in=in;
        bblist->array[i].out=out;
        bblist->array[i].value=value;
    }
    int flag=1;
    constant_gen(&(bblist->array[0]),tmin,tmax,vmin,vmax);
    while(flag){
        flag=0;
        for(int i=1;i<bblist->bb_cnt;i++){
            int flag1=constant_merge(bblist,&(bblist->array[topolist[i]]),tmin,tmax,vmin,vmax);
            int flag2=0;
            if(flag1){
                memcpy(bblist->array[topolist[i]].out,bblist->array[topolist[i]].in,sizeof(int)*(tmax-tmin+vmax-vmin+2));
                flag2=constant_gen(&(bblist->array[topolist[i]]),tmin,tmax,vmin,vmax);
            }
            flag=flag|flag1|flag2;
        }
    }
    /*for(int i=0;i<bblist->bb_cnt;i++){
        for(int j=0;j<tmax-tmin+vmax-vmin+2;j++){
            printf("%d ",bblist->array[i].in[j]);
        }
        printf("\n");
    }
    printf("\n");*/
    for(int i=0;i<bblist->bb_cnt;i++){
        for(int j=0;j<tmax-tmin+vmax-vmin+2;j++){
            if(bblist->array[i].in[j]==1){
                int kind=0;
                int id=0;
                if(j<=tmax-tmin){kind=IR_TMPOP;id=tmin+j;}
                else {kind=IR_VARIABLE;id=j-tmax+tmin-1+vmin;}
                replace_const_g(&(bblist->array[i]),kind,id,bblist->array[i].value[j]);
            }
        }
    }
    for(int i=0;i<bblist->bb_cnt;i++){
        free(bblist->array[i].in);
        free(bblist->array[i].out);
        free(bblist->array[i].value);
    }
    free(topolist);
}

void topo_sort_r(struct BB_List_ *bblist,int *list,int i){
    int cnt=0;
    while(list[cnt]!=-1){
        if(list[cnt]==i){
            if(bblist->array[i].next[0]==-1)
                for(int it=0;it<bblist->array[i].precnt;it++){
                    topo_sort_r(bblist,list,bblist->array[i].pre[it]);
                }
            return;
        }
        cnt++;
    }
    list[cnt]=i;
    for(int it=0;it<bblist->array[i].precnt;it++){
        topo_sort_r(bblist,list,bblist->array[i].pre[it]);
    }
}
int deadCode_merge(struct BB_List_ *bblist,struct BasicBlock_ *bb,int tmin,int tmax,int vmin,int vmax){
    int flag=0;
    for(int i=0;i<2;i++){
        if(bb->next[i]==-1)break;
        struct BasicBlock_ *next=&(bblist->array[bb->next[i]]);
        for(int j=0;j<tmax-tmin+vmax-vmin+2;j++){
            if(next->out[j]!=bb->in[j]){
                if(bb->in[j]==0){
                    bb->in[j]=next->out[j];
                    flag=1;
                }
            }
        }
    }
    return flag;
}
int deadCode_gen(struct BasicBlock_ *bb,int tmin,int tmax,int vmin,int vmax){
    int flag=0;
    InterCodes tmp=bb->end;
    if(tmp==NULL)tmp=ir_tail;
    while(tmp!=NULL){
        if(tmp->dead==1){if(tmp==bb->start)break;tmp=tmp->prev;continue;}
        switch(tmp->code->kind){
                case IR_ASSIGN:{
                    Operand left=tmp->code->u.assign.left;
                    Operand right=tmp->code->u.assign.right;
                    if(left->access==IR_POINT){
                        if(bb->out[get_index(left,tmin,tmax,vmin,vmax)]==0){
                            flag=1;
                            bb->out[get_index(left,tmin,tmax,vmin,vmax)]=1;
                        }
                        if(right->kind==IR_TMPOP||right->kind==IR_VARIABLE)
                            if(bb->out[get_index(right,tmin,tmax,vmin,vmax)]==0){
                                flag=1;
                                bb->out[get_index(right,tmin,tmax,vmin,vmax)]=1;
                            }
                    }
                    else{
                        if(right->kind==IR_TMPOP||right->kind==IR_VARIABLE)
                            bb->out[get_index(right,tmin,tmax,vmin,vmax)]=1;
                        bb->out[get_index(left,tmin,tmax,vmin,vmax)]=0;
                    }
                }break;
                case IR_ADD:
                case IR_SUB:
                case IR_MUL:
                case IR_DIV:{
                    Operand result=tmp->code->u.binop.result;
                    Operand op1=tmp->code->u.binop.op1;
                    Operand op2=tmp->code->u.binop.op2;
                    if(result->access==IR_POINT){
                        if(bb->out[get_index(result,tmin,tmax,vmin,vmax)]==0)flag=1;
                        bb->out[get_index(result,tmin,tmax,vmin,vmax)]=1;
                        if(op1->kind==IR_TMPOP||op1->kind==IR_VARIABLE){
                            if(bb->out[get_index(op1,tmin,tmax,vmin,vmax)]==0)flag=1;
                            bb->out[get_index(op1,tmin,tmax,vmin,vmax)]=1;
                        }
                        if(op2->kind==IR_TMPOP||op2->kind==IR_VARIABLE){
                            if(bb->out[get_index(op2,tmin,tmax,vmin,vmax)]==0)flag=1;
                            bb->out[get_index(op2,tmin,tmax,vmin,vmax)]=1;
                        }
                    }
                    else{
                        if(op1->kind==IR_TMPOP||op1->kind==IR_VARIABLE){
                            if(bb->out[get_index(op1,tmin,tmax,vmin,vmax)]==0)flag=1;
                            bb->out[get_index(op1,tmin,tmax,vmin,vmax)]=1;
                        }
                        if(op2->kind==IR_TMPOP||op2->kind==IR_VARIABLE){
                            if(bb->out[get_index(op2,tmin,tmax,vmin,vmax)]==0)flag=1;
                            bb->out[get_index(op2,tmin,tmax,vmin,vmax)]=1;
                        }
                        if(result->kind==IR_TMPOP||result->kind==IR_VARIABLE){
                            if(bb->out[get_index(result,tmin,tmax,vmin,vmax)]==1)flag=1;
                            bb->out[get_index(result,tmin,tmax,vmin,vmax)]=0;
                        }
                    }
                }break;
                case IR_CALL:{
                    Operand op=tmp->code->u.assign.left;
                    if(op->access==IR_POINT){
                        if(bb->out[get_index(op,tmin,tmax,vmin,vmax)]==0)flag=1;
                        bb->out[get_index(op,tmin,tmax,vmin,vmax)]=1;
                    }
                    else{
                        if(bb->out[get_index(op,tmin,tmax,vmin,vmax)]==1)flag=1;
                        bb->out[get_index(op,tmin,tmax,vmin,vmax)]=0;
                    }
                }break;
                case IR_READ:{
                    Operand op=tmp->code->u.unaryop.unary;
                    if(op->access==IR_POINT){
                        if(bb->out[get_index(op,tmin,tmax,vmin,vmax)]==0)flag=1;
                        bb->out[get_index(op,tmin,tmax,vmin,vmax)]=1;
                    }
                    else{
                        if(bb->out[get_index(op,tmin,tmax,vmin,vmax)]==1)flag=1;
                        bb->out[get_index(op,tmin,tmax,vmin,vmax)]=0;
                    }
                }break;
                case IR_WRITE:{
                    Operand op=tmp->code->u.unaryop.unary;
                    if(op->kind==IR_TMPOP||op->kind==IR_VARIABLE){
                        if(bb->out[get_index(op,tmin,tmax,vmin,vmax)]==0)flag=1;
                        bb->out[get_index(op,tmin,tmax,vmin,vmax)]=1;
                    }
                }break;
                case IR_ARG:{
                    Operand op=tmp->code->u.unaryop.unary;
                    if(op->kind==IR_TMPOP||op->kind==IR_VARIABLE){
                        if(bb->out[get_index(op,tmin,tmax,vmin,vmax)]==0)flag=1;
                        bb->out[get_index(op,tmin,tmax,vmin,vmax)]=1;
                    }
                }break;
                case IR_PARAM:break;
                case IR_RETURN:{
                    Operand op=tmp->code->u.unaryop.unary;
                    if(op->kind==IR_TMPOP||op->kind==IR_VARIABLE){
                        if(bb->out[get_index(op,tmin,tmax,vmin,vmax)]==0)flag=1;
                        bb->out[get_index(op,tmin,tmax,vmin,vmax)]=1;
                    }
                }break;
                case IR_IFGOTO:{
                    Operand op1=tmp->code->u.gotop.op1;
                    Operand op2=tmp->code->u.gotop.op2;
                    if(op1->kind==IR_TMPOP||op1->kind==IR_VARIABLE){
                        if(bb->out[get_index(op1,tmin,tmax,vmin,vmax)]==0)flag=1;
                        bb->out[get_index(op1,tmin,tmax,vmin,vmax)]=1;
                    }
                    if(op2->kind==IR_TMPOP||op2->kind==IR_VARIABLE){
                        if(bb->out[get_index(op1,tmin,tmax,vmin,vmax)]==0)flag=1;
                        bb->out[get_index(op2,tmin,tmax,vmin,vmax)]=1;
                    }
                }break;
                case IR_DEC:break;
        }
        if(tmp==bb->start)break;
        tmp=tmp->prev;
    }
    return flag;
}
void replace_deadCode_g(struct BasicBlock_ *bb,int kind,int id){
    InterCodes tmp=bb->end;
    if(tmp==NULL)tmp=ir_tail;
    while(tmp!=NULL){
        if(tmp->dead==1){if(tmp==bb->start)break;tmp=tmp->prev;continue;}
        switch(tmp->code->kind){
            case IR_ASSIGN:{
                Operand op=tmp->code->u.assign.left;
                Operand op2=tmp->code->u.assign.right;
                if(op2->kind==kind&&op2->u.vid==id)return;
                if(op->access==IR_POINT&&op->kind==kind&&op->u.vid==id)return;
                if(op->access!=IR_POINT)
                if(op->kind==kind&&op->u.vid==id){
                    tmp->dead=1;
                    return;
                }
            }break;
            case IR_ADD:case IR_SUB:case IR_MUL:case IR_DIV:{
                Operand op=tmp->code->u.binop.result;
                Operand op1=tmp->code->u.binop.op1;
                Operand op2=tmp->code->u.binop.op2;
                if(op1->kind==kind&&op1->u.vid==id)return;
                if(op2->kind==kind&&op2->u.vid==id)return;
                if(op->access!=IR_POINT)
                if(op->kind==kind&&op->u.vid==id){
                    tmp->dead=1;
                    return;
                }
            }break;
            case IR_CALL:{
                Operand op=tmp->code->u.assign.left;
                if(op->access!=IR_POINT)
                if(op->kind==kind&&op->u.vid==id){
                    tmp->dead=1;
                    return;
                }
            }break;
            case IR_IFGOTO:{
                Operand op1=tmp->code->u.gotop.op1;
                Operand op2=tmp->code->u.gotop.op2;
                if(op1->kind==kind&&op1->u.vid==id)return;
                if(op2->kind==kind&&op2->u.vid==id)return;
            }break;
            case IR_RETURN:{
                Operand op1=tmp->code->u.unaryop.unary;
                if(op1->kind==kind&&op1->u.vid==id)return;
            }break;
            case IR_ARG:{
                Operand op1=tmp->code->u.unaryop.unary;
                if(op1->kind==kind&&op1->u.vid==id)return;
            }break;
            case IR_WRITE:{
                Operand op1=tmp->code->u.unaryop.unary;
                if(op1->kind==kind&&op1->u.vid==id)return;
            }break;
        }
        if(tmp==bb->start)break;
        tmp=tmp->prev;
    }
    
}
void removeDeadCode_G(struct BB_List_ *bblist,int tmin,int tmax,int vmin,int vmax){
    int *topolist=malloc(sizeof(int)*bblist->bb_cnt);
    memset(topolist,-1,sizeof(int)*bblist->bb_cnt);
    int cnt=0;
    for(int i=0;i<bblist->bb_cnt;i++){
        if(bblist->array[i].next[0]==-1){
            topolist[cnt]=i;
            cnt++;
        }
    }
    for(int i=0;i<cnt;i++)
        topo_sort_r(bblist,topolist,topolist[i]);//TODO: check this
    for(int i=0;i<bblist->bb_cnt;i++){
        int *in=malloc(sizeof(int)*(tmax-tmin+vmax-vmin+2));
        memset(in,0,sizeof(int)*(tmax-tmin+vmax-vmin+2));
        int *out=malloc(sizeof(int)*(tmax-tmin+vmax-vmin+2));
        memset(out,0,sizeof(int)*(tmax-tmin+vmax-vmin+2));
        bblist->array[i].in=in;
        bblist->array[i].out=out;
    }
    int flag=1;
    for(int i=0;i<bblist->bb_cnt;i++)deadCode_gen(&(bblist->array[topolist[i]]),tmin,tmax,vmin,vmax);
    while(flag){
        flag=0;
        for(int i=0;i<bblist->bb_cnt;i++){
            int flag1=deadCode_merge(bblist,&(bblist->array[topolist[i]]),tmin,tmax,vmin,vmax);
            int flag2=0;
            if(flag1){
                memcpy(bblist->array[topolist[i]].out,bblist->array[topolist[i]].in,sizeof(int)*(tmax-tmin+vmax-vmin+2));
                flag2=deadCode_gen(&(bblist->array[topolist[i]]),tmin,tmax,vmin,vmax);
            }
            flag=flag|flag1|flag2;
        }
    }
    /*
    for(int i=0;i<bblist->bb_cnt;i++){
        for(int j=0;j<tmax-tmin+vmax-vmin+2;j++){
            printf("%d ",bblist->array[i].in[j]);
        }
        printf("\n");
    }
    printf("\n");*/
    
    for(int i=0;i<bblist->bb_cnt;i++){
        for(int j=0;j<tmax-tmin+vmax-vmin+2;j++){
            if(bblist->array[i].in[j]==0){
                int kind=0;
                int id=0;
                if(j<=tmax-tmin){kind=IR_TMPOP;id=tmin+j;}
                else {kind=IR_VARIABLE;id=j-tmax+tmin-1+vmin;}
                replace_deadCode_g(&(bblist->array[i]),kind,id);
            }
        }
    }
    for(int i=0;i<bblist->bb_cnt;i++){
        free(bblist->array[i].in);
        free(bblist->array[i].out);
    }
    free(topolist);
}

void optimize_G(struct BB_List_ *bblist){
    int tmin=99999,tmax=0,vmin=99999,vmax=0;
    build_CFG(bblist);
    get_v_cnt(bblist,&tmin,&tmax,&vmin,&vmax);
    //CFG_debugger(bblist);
    //printf("%d %d %d %d\n",tmin,tmax,vmin,vmax);
    foldConstant_G(bblist,tmin,tmax,vmin,vmax);
    deleteSubExp_G(bblist,tmin,tmax,vmin,vmax);
    removeDeadCode_G(bblist,tmin,tmax,vmin,vmax);
}