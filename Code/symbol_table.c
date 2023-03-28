#include "symbol_table.h"

struct SymbolNode_* hashtable[TABLE_SIZE];

void symboltable_init(){
    for(int i=0;i<TABLE_SIZE;i++){
        hashtable[i]=NULL;
    }
}