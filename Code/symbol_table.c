#include "symbol_table.h"

struct SymbolNode_* hashtable[TABLE_SIZE];

int hash(char *name){
    unsigned int val=0,i;
    for(;*name;++name){
        val=(val<<2)+*name;
        if(i=val&~TABLE_SIZE)val==(val^(i>>12))&TABLE_SIZE;
    }
    return val;
}

void symboltable_init(){
    for(int i=0;i<TABLE_SIZE;i++){
        hashtable[i]=NULL;
    }
}

void insert_node(Type type,char *name){
    int index=hash(name);
    struct SymbolNode_ *node=malloc(sizeof(struct SymbolNode_));
    node->next=NULL;
    node->behind=NULL;
    node->field.type=type;
    node->field.name=name;
    if(hashtable[index]==NULL){
        hashtable[index]=node;
    }
    else{
        node->next=hashtable[index];
        hashtable[index]=node;
    }
}
int query_symbol(char *name){
    return 0;
}
void print_table(){
    for(int i=0;i<TABLE_SIZE;i++){
        printf("%d: ",i);
        if(hashtable[i]!=NULL){
            sNode node=hashtable[i];
            while(node!=NULL){
                printf("{name:%s}",node->field.name);
            }
        }
        printf("\n");
    }
}