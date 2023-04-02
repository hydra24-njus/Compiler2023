#include "symbol_table.h"

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
    struct SymbolNode_ *node=malloc(sizeof(struct SymbolNode_));
    node->next=NULL;
    if(hashtable[index]==NULL){
        hashtable[index]=node;
    }
    else{
        node->next=hashtable[index];
        hashtable[index]=node;
    }
}
Type query_symbol(char *name){
    int index=hash(name);
    sNode ret=NULL;
    for(sNode i=hashtable[index]->next;i;i=i->next){
        if(!strcmp(name,i->name)){
            ret=i;
        }
    }
    if(ret){
        return ret->type;
    }
    return NULL;
}
void print_table(){
    for(int i=0;i<TABLE_SIZE;i++){
        printf("%d: ",i);
        if(hashtable[i]!=NULL){
            sNode node=hashtable[i]->next;
            while(node!=NULL){
                printf("{name:}");
                node=node->next;
            }
        }
        printf("\n");
    }
}