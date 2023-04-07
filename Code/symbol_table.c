#include "symbol_table.h"
#include "debug.h"

struct SymbolNode_ *hashtable[TABLE_SIZE];
struct SymbolNode_ *structable[STRUCT_SIZE];
struct FunctionList_ *functable;
struct ScopeList_ *scopelist;
unsigned _depth;

int hash(char *name,int size){
    unsigned int val=0,i;
    for(;*name;++name){
        val=(val<<2)+*name;
        if(i=val&(~size))val=(val^(i>>12))&size;
    }
    return val;
}

//初始化用
void symboltable_init(){
    static struct SymbolNode_ tab[TABLE_SIZE];
    for(int i=0;i<TABLE_SIZE;i++){
        hashtable[i]=&(tab[i]);
    }
    static struct SymbolNode_ structab[STRUCT_SIZE];
    for(int i=0;i<STRUCT_SIZE;i++){
        structable[i]=&(structab[i]);
    }
    static struct FunctionList_ functab;
    functable=&functab;
    static struct ScopeList_ slist;
    scopelist=&slist;
}

//全局符号表
void insert_node(Type type,char *name,int deep,ScopeList scope){
    int index=hash(name,TABLE_SIZE);
    struct SymbolNode_ *node=malloc(sizeof(struct SymbolNode_));
    node->name=name;
    node->next=NULL;
    node->type=type;
    node->depth=deep;
    node->next=hashtable[index]->next;
    hashtable[index]->next=node;
    if(scope!=NULL){
        node->tail=scope->tail;
        scope->tail=node;
    }
}

Type query_symbol(char *name,int type,int deep){
    //type=0:只查当前层 是否重定义
    //type=1:查本层及更浅的层
    int index=hash(name,TABLE_SIZE);
    sNode ret=NULL;
    if(type==0){
        for(sNode i=hashtable[index]->next;i;i=i->next){
            if(strcmp(name,i->name)==0){
                if(i->depth==deep){
                    ret=i;
                    break;
                }
            }
        }
        if(ret){
            return ret->type;
        }
    }
    else if(type==1){
        for(sNode i=hashtable[index]->next;i;i=i->next){
            if(strcmp(name,i->name)==0){
                if(i->depth<=deep){
                    ret=i;
                    break;
                }
            }
        }
        if(ret){
            return ret->type;
        }
    }
    return NULL;
}

void delete_node(struct SymbolNode_ *node){
    int index=hash(node->name,TABLE_SIZE);
    struct SymbolNode_ *tmp=hashtable[index];
    while(tmp->next!=node){
        tmp=tmp->next;
    }
    tmp->next=node->next;
    free(node);
}

//结构体内部作用域的符号表
void insert_node_struct(Type type,char *name){
    int index=hash(name,STRUCT_SIZE);
    struct SymbolNode_ *node=malloc(sizeof(struct SymbolNode_));
    node->name=name;
    node->next=NULL;
    node->type=type;
    node->next=structable[index]->next;
    structable[index]->next=node;

}

Type query_symbol_struct(char *name){
    int index=hash(name,STRUCT_SIZE);
    sNode ret=NULL;
    for(sNode i=structable[index]->next;i;i=i->next){
        if(strcmp(name,i->name)==0){
            ret=i;
            break;
        }
    }
    if(ret){
        return ret->type;
    }
    return NULL;
}

void delete_struct_node(char *name){
    int index=hash(name,STRUCT_SIZE);
    sNode ret=NULL;
    sNode prev=structable[index];
    for(sNode i=structable[index]->next;i;i=i->next){
        if(strcmp(name,i->name)==0){
            ret=i;//最深层的
            break;
        }
        prev=i;
    }
    if(ret){
        prev->next=ret->next;
        free(ret);
    }
    return ;
}

void delete_struct_table(){
    for(int i=0;i<STRUCT_SIZE;i++){
        if(structable[i]->next==NULL)
            continue;
        struct SymbolNode_ *node=structable[i]->next;
        structable[i]->next=NULL;
        while(node!=NULL){
            struct SymbolNode_ *tmp=node;
            node=node->next;
            free(tmp);
        }
    }
}

//函数定义的全局链表
void insert_function(int lineno,char *name){
    struct FunctionList_ *node=malloc(sizeof(struct FunctionList_));
    node->lineno=lineno;
    node->name=name;
    node->next=functable->next;
    functable->next=node;
    return;
}

void delete_function(char *name){
    struct FunctionList_ *prev=functable;
    struct FunctionList_ *ret=functable->next;
    while(ret!=NULL){
        if(strcmp(ret->name,name)==0){
            break;
        }
        prev=ret;
        ret=ret->next;
    }
    prev->next=ret->next;
    ret->next=NULL;
    free(ret);
}

void delete_functable(){
    struct FunctionList_ *tmp=functable->next;
    while(tmp!=NULL){
        functable->next=tmp->next;
        free(tmp);
        tmp=functable->next;
    }
    functable->next=NULL;
}

//作用域控制
struct ScopeList_ *enter_new_scope(){
    struct ScopeList_ *ret=malloc(sizeof(struct ScopeList_));
    ret->next=scopelist->next;
    scopelist->next=ret;
    return ret;
}

struct ScopeList_ *exit_cur_scope(){
    struct ScopeList_ *ret=scopelist->next;
    struct SymbolNode_ *tmp=ret->tail;
    struct SymbolNode_ *out=ret->tail;
    while(ret->tail!=NULL){
        tmp=ret->tail;
        ret->tail=tmp->tail;
        delete_node(tmp);
    }
    scopelist->next=ret->next;
    free(ret);
    return scopelist->next;
}


//一些调试信息
char *tpname[5]={"basic","array","structure","function"};
char *basicname[3]={"int","float"};

void print_table(){
    for(int i=0;i<TABLE_SIZE;i++){
        debug("%d: ",i);
        if(hashtable[i]!=NULL){
            sNode node=hashtable[i]->next;
            while(node!=NULL){
                debug("{%s %s",node->type!=NULL?tpname[node->type->kind]:"unknowtype",node->name);
                if(node->type&&node->type->kind==FUNCTION_T){
                    FieldList tmp=node->type->u.function.paramlist;
                    debug("(");
                    while(tmp!=NULL){
                        debug("%s %s",tpname[tmp->type->kind],tmp->name);
                        tmp=tmp->tail;
                        if(tmp!=NULL)debug(", ");
                    }
                    debug(")");
                    debug("returntype:%s",tpname[node->type->u.function.returntype->kind]);
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
    if(A==NULL&&B==NULL)return 1;
    else if(A==NULL||B==NULL)return 0;
    else if(A->kind!=B->kind){
        return 0;
    }
    else if(A->kind==BASIC){
        if(A->u.basic!=B->u.basic){
            return 0;
        }
    }
    else if(A->kind==ARRAY){
        return typecheck(A->u.array.elem,B->u.array.elem);
    }
    else if(A->kind==STRUCTURE){
        FieldList tmpa=A->u.structure,tmpb=B->u.structure;
        while(tmpa!=NULL&&tmpb!=NULL){
            if(!typecheck(tmpa->type,tmpb->type)){
                return 0;
            }
            tmpa=tmpa->tail;
            tmpb=tmpb->tail;
        }
        if(tmpa!=NULL||tmpb!=NULL){
            return 0;
        }
    }
    else if(A->kind==FUNCTION_T){
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