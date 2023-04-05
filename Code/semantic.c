#include "semantic.h"
#include "debug.h"


static struct Type_ type_int={
    .kind=BASIC,
    .u.basic=INT
};
static struct Type_ type_float={
    .kind=BASIC,
    .u.basic=FLOAT
};
static int senmatic_error=0;
static char* error_msg[20]={
    "",//0 占位
    "Undefined variable",//1
    "Undefined function",//2
    "Redefined variable",//3
    "Redefined function",//4
    "Type mismatched for assignment",//5
    "The left-hand side of an assignment must be a variable",//6
    "Type mismatched for operands",//7
    "Type mismatched for return",//8
    "Function is not applicable for arguments",//9
    "variable is not an array",//10
    "variable is not a function",//11
    "constant is not an integer",//12
    "Illegal use of \".\"",//13
    "Non-existent field",//14
    "Redefined field",//15
    "Duplicated name",//16
    "Undefined structure",//17
    "Undefined function",//18
    "Inconsistent declaration of function"//19
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
        ExtDef_analyse(child1);
        ExtDefList_analyse(child2);
    }
    else{
        debug("ExtDefList -> ExtDef ExtDefList(empty)\n");
        ExtDef_analyse(child1);
    }
}

void ExtDef_analyse(Node *root){
    Node *child1=NULL,*child2=NULL,*child3=NULL;
    Type type=NULL;
    child1=root->child;
    if(child1->next)child2=child1->next;
    if(child2&&child1->next)child3=child2->next;
    if(child1&&child2&&child3){
        if(strcmp(child1->info_char,"Specifier")==0&&strcmp(child2->info_char,"ExtDecList")==0&&strcmp(child3->info_char,"SEMI")==0){
            debug("ExtDef -> Specifier ExtDecList SEMI\n");
            type=Specifier_analyse(child1);
            ExtDecList_analyse(child2,type);
        }
        else if(strcmp(child1->info_char,"Specifier")==0&&strcmp(child2->info_char,"FunDec")==0&&strcmp(child3->info_char,"CompSt")==0){
            debug("ExtDef -> Specifier FunDec CompSt\n");
            //TODO:进入新的作用域
            type=Specifier_analyse(child1);
            FunDec_analyse(child2,type,1);
            CompSt_analyse(child3);
        }
        else if(strcmp(child1->info_char,"Specifier")==0&&strcmp(child2->info_char,"FunDec")==0&&strcmp(child3->info_char,"SEMI")==0){
            debug("ExtDef -> Specifier FunDec SEMI\n");
            //TODO:进入新的作用域
            type=Specifier_analyse(child1);
            FunDec_analyse(child2,type,0);
        }
        else{
            debug("error in ExtDef_analyse\n");
            exit(0);
        }
    }
    else if(child1&&child2){
        debug("ExtDef -> Specifier SEMI\n");
        type=Specifier_analyse(child1);
        //TODO:添加进符号表
    }
    else{
        debug("error in ExtDef_analyse\n");
        exit(0);
    }
}

void ExtDecList_analyse(Node *node,Type type){
    Node *child1=node->child,*child2=NULL,*child3=NULL;
    if(child1->next){
        child2=child1->next;
    }
    if(child2&&child2->next){
        child3=child2->next;
    }
    if(child1&&child2&&child3){
        debug("ExtDecList -> VarDec COMMA ExtDecList\n");
        FieldList field=VarDec_analyse(child1,type);
        if(query_symbol(field->name)!=NULL){
            //TODO:报错
        }
        else{
            insert_node(field->type,field->name);
            free(field);
        }
        ExtDecList_analyse(child3,type);
    }
    else if(child1){
        debug("ExtDecList -> VarDec\n");
        FieldList field=VarDec_analyse(child1,type);
        if(query_symbol(field->name)!=NULL){
            //TODO:报错
        }
        else{
            insert_node(field->type,field->name);
            free(field);
        }
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
            debug("should not reach here 1\n");
            exit(1);
        }
    }
    else{
        type=malloc(sizeof(struct Type_));
        type->kind=STRUCTURE;
        //TODO: 结构体(可能有匿名)
    }
    return type;
}

void StructSpecifier_analyse(Node *node){return;}

void OptTag_analyse(Node *node){return;}

void Tag_analyse(Node *node){return;}

FieldList VarDec_analyse(Node *node,Type type){
    Node *child1=node->child;
    if(child1->node_type==lexid){
        debug("VarDec -> ID\n");
        FieldList field=malloc(sizeof(struct FieldList_));
        field->name=child1->info_char;
        field->type=type;
        return field;
    }
    else{
        debug("VarDec -> VarDec LB INT RB\n");
        Node *child2=child1->next->next;
        //TODO:
    }
    return NULL;
}

void FunDec_analyse(Node *node,Type type,int def){
    //def=0:只声明
    //def=1:定义
    Node *id=node->child;
    Node *varlist=id->next->next;

    Type functype=malloc(sizeof(struct Type_));
    functype->kind=FUNCTION_T;
    functype->u.function.returntype=type;

    //TODO:判断是否定义和声明

    if(strcmp(varlist->info_char,"VarList")==0){
        debug("FunDec -> ID LP VarList RP\n");
        //TODO:创建一个field
        FieldList field=VarList_analyse(varlist,NULL);
        int cnt=0;
        FieldList tmp=field;
        while(tmp!=NULL){
            cnt++;
            tmp=tmp->tail;
        }
        functype->u.function.paramscnt=cnt;
        functype->u.function.paramlist=field;
        functype->u.function.isdef=def;
        Type qtype=query_symbol(id->info_char);
        if(qtype==NULL){
            //未定义 or 声明的函数
            insert_node(functype,id->info_char);
        }
        else{
            if(def&&qtype->u.function.isdef){
                printf("Error type %d at line %d: %s\n",4,node->lineno,error_msg[4]);
            }
            else if(typecheck(functype,qtype)==0){
                printf("Error type %d at line %d: %s\n",19,node->lineno,error_msg[19]);
            }
        }
    }
    else{
        debug("FunDec -> ID LP RP\n");
        functype->u.function.paramscnt=0;
        functype->u.function.paramlist=NULL;
        functype->u.function.isdef=def;
        Type qtype=query_symbol(id->info_char);
        if(qtype==NULL){
            //未定义 or 声明的函数
            insert_node(functype,id->info_char);
        }
        else{
            if(def&&qtype->u.function.isdef){
                printf("Error type %d at line %d: %s\n",4,node->lineno,error_msg[4]);
            }
            else if(typecheck(functype,qtype)==0){
                printf("Error type %d at line %d: %s\n",19,node->lineno,error_msg[19]);
            }
        }
    }
    return;
}

FieldList VarList_analyse(Node *node,FieldList head){
    Node *paramdec=node->child;
    Node *varlist=NULL;
    if(paramdec->next!=NULL){
        varlist=paramdec->next->next;
    }
    if(varlist==NULL){
        debug("VarList -> ParamDec\n");
        FieldList field=ParamDec_analyse(paramdec);
        if(head!=NULL){
            head->tail=field;
        }
        return field;
    }
    else{
        debug("VarList -> ParamDec COMMA VarList\n");
        FieldList field=ParamDec_analyse(paramdec);
        if(head!=NULL){
            head->tail=field;
        }
        VarList_analyse(varlist,field);
        return field;
    }
    return NULL;
}

FieldList ParamDec_analyse(Node *node){
    debug("ParamDec -> Specifier VarDec\n");
    Type type=Specifier_analyse(node->child);
    FieldList field=VarDec_analyse(node->child->next,type);
    return field;
}

void CompSt_analyse(Node *node){
    Node *lc=node->child;
    Node *deflist=lc->next;
    Node *child=deflist->next;
    if(child==NULL){
        debug("Compst -> LC DefList(empty) StmtList(empty) RC\n");
        // Do nothing
    }
    else if(strcmp(deflist->info_char,"DefList")==0){
        if(child!=NULL&&strcmp(child->info_char,"StmtList")==0){
            debug("Compst -> LC DefList StmtList RC\n");
            DefList_analyse(deflist);
            StmtList_analyse(child);
        }
        else{
            debug("Compst -> LC DefList StmtList(empty) RC\n");
            DefList_analyse(deflist);
        }
    }
    else if(strcmp(deflist->info_char,"StmtList")==0){
        debug("CompSt -> LC DefList(empty) StmtList RC\n");
        StmtList_analyse(deflist);
    }
    return;
}

void StmtList_analyse(Node *node){
    Node *stmt=node->child;
    Node *stmtlist=stmt->next;
    if(stmtlist==NULL){
        debug("StmtList -> STMT StmtList(empty)\n");
        Stmt_analyse(stmt);
    }
    else{
        debug("STMT -> STMT StmtList\n");
        Stmt_analyse(stmt);
        StmtList_analyse(stmtlist);
    }
    return;
}

void Stmt_analyse(Node *node){
    Node *child1=node->child;
    Node *child2=NULL;
    Node *child3=NULL;
    Node *child4=NULL;
    if(strcmp(child1->info_char,"CompSt")==0){
        debug("Stmt -> CompSt\n");
        //TODO:进入新作用域并分析
    }
    else if(strcmp(child1->info_char,"Exp")==0){
        debug("Stmt -> Exp SEMI\n");
        //TODO:检查Exp是否错误
        Exp_analyse(child1);
    }
    else if(strcmp(child1->info_char,"RETURN")==0){
        debug("Stmt -> RETURN Exp SEMI\n");
        child2=child1->next;
        //TODO:检查Exp是否错误
        Type etype=Exp_analyse(child2);
        //TODO:检查返回值类型是否正确
    }
    else if(strcmp(child1->info_char,"IF")==0){
        child2=child1->next->next;//Exp
        child3=child2->next->next;//Stmt
        child4=child3->next;//NULL or ELSE
        if(child4!=NULL){
            child4=child4->next;//Stmt;
            debug("Stmt -> IF LP Exp RP Stmt ELSE Stmt\n");
            //TODO:检查Exp错误
        }
        else{
            debug("Stmt -> IF LP Exp RP Stmt\n");
        }
    }
    else if(strcmp(child1->info_char,"WHILE")==0){
        debug("Stmt -> WHILE LP Exp RP Stmt\n");
        //TODO:检查Exp错误
    }
    else{
        debug("should not reach here 2\n");
        exit(1);
    }
    return;
}

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
    Type type=Specifier_analyse(specifier);
    DecList_analyse(declist,type);
    return;
}

void DecList_analyse(Node *node,Type type){
    Node *dec=node->child;
    Node *comma=dec->next;
    Node *declist=NULL;
    if(comma==NULL){
        debug("DecList -> Dec\n");
        Dec_analyse(dec,type);
    }
    else{
        declist=comma->next;
        debug("DecList -> Dec COMMA DecList\n");
        Dec_analyse(dec,type);
        DecList_analyse(declist,type);
    }
    return;
}

void Dec_analyse(Node *node,Type type){
    Node *vardec=node->child;
    Node *assignop=vardec->next;
    Node *exp=NULL;
    if(assignop==NULL){
        debug("Dec -> VarDec\n");
        FieldList field=VarDec_analyse(vardec,type);
        if(query_symbol(field->name)!=NULL){
            //TODO:报错
        }
        else{
            insert_node(field->type,field->name);
            free(field);
        }
    }
    else{
        debug("Dec -> VarDec ASSIGNOP EXP\n");
        exp=assignop->next;
        FieldList field=VarDec_analyse(vardec,type);
        Type type=Exp_analyse(exp);
        if(type!=field->type){
            //TODO:报错
            debug("should not reach here 3\n");
            exit(1);
        }
        if(query_symbol(field->name)!=NULL){
            //TODO:报错
            debug("Error type %d at line %d: %s",3,node->lineno,error_msg[3]);
        }
        else{
            insert_node(field->type,field->name);
            free(field);
        }
    }
    return;
}

Type Exp_analyse(Node *node){
    Node *child1=node->child;
    if(child1->node_type==lexint){
        debug("Exp -> INT\n");
        return &type_int;
    }
    else if(child1->node_type==lexfloat){
        debug("Exp -> FLOAT\n");
        return &type_float;
    }
    else if(child1->node_type==lexid&&child1->next==NULL){
        debug("Exp -> ID\n");
        Type type=query_symbol(child1->info_char);
        if(type==NULL){
            //TODO:报错
        }
        else{
            return type;
        }
    }
    else if(strcmp(child1->info_char,"MINUS")==0){
        debug("Exp -> MINUS Exp\n");
        return Exp_analyse(child1->next);
    }
    else if(strcmp(child1->info_char,"NOT")==0){
        debug("Exp -> NOT Exp\n");
        return Exp_analyse(child1->next);
    }
    else if(strcmp(child1->info_char,"Exp")==0){
        if(strcmp(child1->next->next->info_char,"Exp")==0&&child1->next->next->next==NULL){
            debug("Exp -> Exp OP Exp\n");
            Type type1=Exp_analyse(child1);
            Type type2=Exp_analyse(child1->next->next);
            if(typecheck(type1,type2)){
                return type1;
            }
            else{
                //报错
            }
        }
        if(child1->next->next->node_type==lexid){
            debug("Exp -> Exp DOT ID\n");
            Type type1=Exp_analyse(child1);
            //TODO:找到ID的类型并返回
        }
        if(strcmp(child1->next->next->info_char,"Exp")==0){
            debug("Exp -> Exp LB Exp RB\n");
            Type type1=Exp_analyse(child1);
            Type type2=Exp_analyse(child1->next->next);
            if(typecheck(type2,&type_int)){
                return type1->u.array.elem;
            }
            else{
                //报错
            }
        }
    }
    else if(strcmp(child1->info_char,"LP")==0){
        debug("Exp -> LP Exp RP\n");
        return Exp_analyse(child1->next);
    }
    else if(child1->node_type==lexid&&child1->next!=NULL){
        if(strcmp(child1->next->next->info_char,"RP")){
            debug("Exp -> ID LP RP\n");
            //TODO:函数调用
            Type type=query_symbol(child1->info_char);
            if(type->u.function.paramscnt==0){
                return type->u.function.returntype;
            }
            else{
                //TODO:函数调用的参数数量不符
                printf("Error %d at line %d: %s\n",9,node->lineno,error_msg[9]);
            }
        }
        else if(strcmp(child1->next->next->info_char,"Args")){
            debug("Exp -> ID LP Args RP\n");
            //TODO:函数调用
            Type type=query_symbol(child1->info_char);
            FieldList head=type->u.function.paramlist;
            if(Args_analyse(child1->next->next,head)==0){
                return type->u.function.returntype;
            }
            else{
                //TODO:函数调用的参数数量/类型不符
                printf("Error %d at line %d: %s\n",9,node->lineno,error_msg[9]);
            }
        }
    }
    else{
        debug("should not reach here 4\n");
        exit(1);
    }
    return NULL;
}

int  Args_analyse(Node *node,FieldList field){
    Node *exp=node->child;
    Node *args=NULL;
    if(exp->next==NULL){
        debug("Args -> Exp\n");
        if(field->tail!=NULL){
            //参数数量不符
            return 1;
        }
        Type type=Exp_analyse(exp);
        if(field&&typecheck(type,field->type)){
            return 0;
        }
        else{
            return 1;
        }
    }
    else{
        debug("Args -> Exp COMMA Args\n");
        args=exp->next->next;
        Type type=Exp_analyse(exp);
        if(typecheck(type,field->type)&&field->tail!=NULL){
            return Args_analyse(args,field->tail);
        }
        else{
            return 1;
        }
    }
    return 0;
}