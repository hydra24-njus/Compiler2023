#include "ir.h"
#include "debug.h"
#include "semantic.h"

static int _var_cnt=0;
static int _tmp_cnt=0;

__attribute__((constructor)) void ir_init(){
    //printf("ir init\n\n");
    return;
}

void trans_FunDec(Node *root){

}

void trans_Stmt(Node *root){

}

void trans_Exp(Node *root){
    if(gencheck(root,1,"INT")){
        debug("Exp -> INT\n");
    }
    else if(gencheck(root,1,"FLOAT")){
        debug("Exp -> FLOAT\n");
    }
    else if(gencheck(root,1,"ID")){
        debug("Exp -> ID\n");
    }
    else if(gencheck(root,2,"MINUS","Exp")){
        debug("Exp -> MINUS Exp\n");
    }
    else if(gencheck(root,2,"NOT","Exp")){
        debug("Exp -> NOT Exp\n");
    }
    else if(gencheck(root,3,"Exp","ASSIGNOP","Exp")){
        debug("Exp -> Exp ASSIGNOP Exp\n");
    }
    else if(gencheck(root,3,"Exp","RELOP","Exp")){
        debug("Exp -> Exp RELOP Exp\n");
    }
    else if(gencheck(root,3,"Exp","OP","Exp")){
        debug("Exp -> Exp OP Exp\n");
    }
    else if(gencheck(root,3,"Exp","DOT","ID")){
        debug("Exp -> Exp DOT ID\n");
    }
    else if(gencheck(root,4,"Exp","LB","Exp","RB")){
        debug("Exp -> Exp LB Exp RB\n");
    }
    else if(gencheck(root,3,"LP","Exp","RP")){
        debug("Exp -> LP Exp RP\n");
    }
    else if(gencheck(root,3,"ID","LP","RP")){
        debug("Exp -> ID LP RP\n");
    }
    else if(gencheck(root,4,"ID","LP","Args","RP")){
        debug("Exp -> ID LP Args RP\n");
    }
    else{
        debug("error in trans_Exp\n");
    }
    return;
}
