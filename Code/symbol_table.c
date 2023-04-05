#include "symbol_table.h"
#include "debug.h"
static struct SymbolNode_* hashtable[TABLE_SIZE];
static unsigned _depth;
int hash(char *name){
    unsigned int val=0,i;
    for(;*name;++name){
        val=(val<<2)+*name;
        if(i=val&~TABLE_SIZE)val==(val^(i>>12))&TABLE_SIZE;
    }
    return val;
}

void symboltable_init(){
    static struct SymbolNode_ tab[TABLE_SIZE];
    for(int i=0;i<TABLE_SIZE;i++){
        hashtable[i]=&(tab[i]);
    }
}

void insert_node(Type type,char *name){
    if(query_symbol(name)){
        printf("error in insert_node()\n;");
        exit(0);
    }
    int index=hash(name);
    debug("index=%d\n",index);
    struct SymbolNode_ *node=malloc(sizeof(struct SymbolNode_));
    node->name=name;
    node->next=NULL;
    node->type=type;
    if(hashtable[index]->next==NULL){
        hashtable[index]->next=node;
    }
    else{
        node->next=hashtable[index]->next;
        hashtable[index]->next=node;
    }
}
Type query_symbol(char *name){
    int index=hash(name);
    sNode ret=NULL;
    for(sNode i=hashtable[index]->next;i;i=i->next){
        if(strcmp(name,i->name)==0){
            ret=i;//最深层的
        }
    }
    if(ret){
        return ret->type;
    }
    return NULL;
}
char *tpname[5]={"basic","array","structure","function"};
char *basicname[3]={"int","float"};
void print_table(){
    for(int i=0;i<TABLE_SIZE;i++){
        debug("%d: ",i);
        if(hashtable[i]!=NULL){
            sNode node=hashtable[i]->next;
            while(node!=NULL){
                debug("{%s %s",tpname[node->type->kind],node->name);
                if(node->type->kind==FUNCTION_T){
                    FieldList tmp=node->type->u.function.paramlist;
                    debug("(");
                    while(tmp!=NULL){
                        debug("%s %s",tpname[tmp->type->kind],tmp->name);
                        tmp=tmp->tail;
                        if(tmp!=NULL)debug(", ");
                    }
                    debug(")");
                }
                debug("}->");
                node=node->next;
            }
        }
        debug("\n");
    }
}

int typecheck(Type A, Type B){
    //相等返回1，不相等返回0；
    if(A->kind!=B->kind){
        return 0;
    }
    if(A->kind==BASIC){
        if(A->u.basic!=B->u.basic){
            return 0;
        }
    }
    if(A->kind==ARRAY){

    }
    if(A->kind==STRUCTURE){

    }
    if(A->kind==FUNCTION_T){
        if(typecheck(A->u.function.returntype,B->u.function.returntype)==0)
            return 0;
        if(A->u.function.paramscnt!=B->u.function.paramscnt)
            return 0;
        FieldList tmpa=A->u.function.paramlist;
        FieldList tmpb=B->u.function.paramlist;
        while(tmpa!=NULL){
            if(typecheck(tmpa->type,tmpb->type)==0)
                return 0;
            tmpa=tmpa->tail;
            tmpb=tmpb->tail;
        }
    }
    return 1;
}