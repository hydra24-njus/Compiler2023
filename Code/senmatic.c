#include "semantic.h"
#define __DEBUG__
#ifdef __DEBUG__
#define debug(format,...) printf(format,##__VA_ARGS__)
#else
#define debug(format,...)
#endif

static struct Type_ type_int={
    .kind=BASIC,
    .u.basic=INT
};
static struct Type_ type_float={
    .kind=BASIC,
    .u.basic=FLOAT
};

int semantic(Node *root){
    if(root==NULL){
        return 1;
    }
    if(strcmp(root->info_char,"Program")!=0){
        return 1;
    }
    symboltable_init();
    Program_analyse(root);
    return 0;
}

void Program_analyse(Node *root){
    debug("Program -> ExtDefList\n");
    ExtDefList_analyse(root->child);
}

void ExtDefList_analyse(Node *root){
    Node *child1=root->child;
    Node *child2=child1->next;
    if(child2!=NULL){
        debug("ExtDefList -> ExtDef ExtDefList\n");
        ExtDefList_analyse(child2);
    }
    else{
        debug("ExtDefList -> ExtDef (empty)\n");
    }
    ExtDef_analyse(child1);
}

void ExtDef_analyse(Node *root){
    Node *child1=NULL,*child2=NULL,*child3=NULL;
    child1=root->child;
    if(child1->next)child2=child1->next;
    if(child2&&child1->next)child3=child2->next;
    if(child1&&child2&&child3){
        if(strcmp(child1->info_char,"Specifier")==0&&strcmp(child2->info_char,"ExtDecList")==0&&strcmp(child3->info_char,"SEMI")==0){
            debug("ExtDef -> Specifier ExtDecList SEMI\n");
            Specifier_analyse(child1);
            ExtDecList_analyse(child2);
        }
        else if(strcmp(child1->info_char,"Specifier")==0&&strcmp(child2->info_char,"FunDec")==0&&strcmp(child3->info_char,"CompSt")==0){
            debug("ExtDef -> Specifier FunDec CompSt\n");
            Specifier_analyse(child1);
            FunDec_analyse(child2);
            CompSt_analyse(child3);
        }
        else if(strcmp(child1->info_char,"Specifier")==0&&strcmp(child2->info_char,"FunDec")==0&&strcmp(child3->info_char,"SEMI")==0){
            debug("ExtDef -> Specifier FunDec SEMI\n");
            Specifier_analyse(child1);
            FunDec_analyse(child2);
        }
        else{
            debug("error in ExtDef_analyse\n");
            exit(0);
        }
    }
    else if(child1&&child2){
        debug("ExtDef -> Specifier SEMI\n");
        Specifier_analyse(child1);
    }
    else{
        debug("error in ExtDef_analyse\n");
        exit(0);
    }
}

void ExtDecList_analyse(Node *node){
    Node *child1=node->child,*child2=NULL,*child3=NULL;
    if(child1->next){
        child2=child1->next;
    }
    if(child2&&child2->next){
        child3=child2->next;
    }
    if(child1&&child2&&child3){
        debug("ExtDecList -> VarDec COMMA ExtDecList\n");
        VarDec_analyse(child1);
        ExtDecList_analyse(child3);
    }
    else if(child1){
        debug("VarDec");
    }
    else{
        debug("error in ExtDecList_analyse\n");
        exit(0);
    }
}

Type Specifier_analyse(Node *node){
    Type type=NULL;
    Node *child1=node->child;
    if(child1->node_type==lextype){
        debug("Specifier -> TYPE\n");
        if(strcmp(child1->info_char,"int")==0){
            type=&type_int;
        }
        else if(strcmp(child1->info_char,"float")==0){
            type=&type_float;
        }
        else{
            debug("%s %d\n",child1->info_char,strcmp(child1->info_char,"FLOAT"));
        }
    }
    else{
        type=malloc(sizeof(struct Type_));
        type->kind=STRUCTURE;
        //TODO: 结构体
    }
    return type;
}
void StructSpecifier_analyse(Node *node){return;}
void OptTag_analyse(Node *node){return;}
void Tag_analyse(Node *node){return;}
void VarDec_analyse(Node *node){return;}
void FunDec_analyse(Node *node){return;}
void VarList_analyse(Node *node){return;}
void ParamDec_analyse(Node *node){return;}

void CompSt_analyse(Node *node){
    Node *lc=node->child;
    Node *deflist=lc->next;
    Node *child=deflist->next;
    //todo:进入新的作用域
    if(strcmp(child->info_char,"StmtList")==0){
        debug("CompSt -> LC DefList StmtList RC\n");
        DefList_analyse(deflist);
        StmtList_analyse(child);
    }
    else{
        debug("CompSt -> LC DefList StmtList(empty) RC\n");
        DefList_analyse(deflist);
    }
    //todo:退出这个作用域
    return;
}

void StmtList_analyse(Node *node){
    Node *stmt=node->child;
    Node *stmtlist=stmt->next;
    if(stmtlist==NULL){
        debug("STMT -> STMT empty\n");
        Stmt_analyse(stmt);
    }
    else{
        debug("STMT -> STMT StmtList\n");
        Stmt_analyse(stmt);
        StmtList_analyse(stmtlist);
    }
    return;
}

void Stmt_analyse(Node *node){return;}

void DefList_analyse(Node *node){
    Node *def=node->child;
    Node *deflist=def->next;
    if(deflist==NULL){
        debug("DefList -> Def DefList(empty)\n");
        Def_analyse(def);
    }
    else{
        debug("DefList -> Def DefList\n");
        Def_analyse(def);
        DefList_analyse(deflist);
    }
    return;
}

void Def_analyse(Node *node){
    Node *specifier=node->child;
    Node *declist=specifier->next;
    debug("Def -> Specifier DecList SEMI\n");
    Specifier_analyse(specifier);
    DecList_analyse(declist);
    return;
}

void DecList_analyse(Node *node){
    Node *dec=node->child;
    Node *comma=dec->next;
    Node *declist=NULL;
    if(comma==NULL){
        debug("DecList -> Dec\n");
        Dec_analyse(dec);
    }
    else{
        declist=comma->next;
        debug("DecList -> Dec COMMA DecList\n");
        Dec_analyse(dec);
        DecList_analyse(declist);
    }
    return;
}

void Dec_analyse(Node *node){
    Node *vardec=node->child;
    Node *assignop=vardec->next;
    Node *exp=NULL;
    if(assignop==NULL){
        debug("Dec -> VarDec\n");
    }
    else{
        debug("Dec -> VarDec ASSIGNOP EXP\n");
        exp=assignop->next;
    }
    return;
}

void Exp_analyse(Node *node){return;}
void Args_analyse(Node *node){return;}