#include "semantic.h"
#include "debug.h"
#include <stdarg.h>


static struct Type_ type_int={
    .kind=BASIC,
    .u.basic=INT
};
static struct Type_ type_float={
    .kind=BASIC,
    .u.basic=FLOAT
};
int semantic_error=0;
static char* error_msg[20]={
    "unhandled errors",//0 eg. if(0.5)Stmt
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

int gencheck(Node *root,int cnt,...){
    va_list ap;
    va_start(ap,cnt);
    Node *child=root->child;
    char* name=NULL;
    for(int i=0;i<cnt;i++){
        name=va_arg(ap,char*);
        if(child==NULL){
            va_end(ap);
            return 0;
        }
        if(strcmp("OP",name)==0){
            child=child->next;
            continue;
        }
        else if(strcmp("ID",name)==0){
            if(child->node_type!=lexid){
                va_end(ap);
                return 0;//拒绝
            }
        }
        else if(strcmp("TYPE",name)==0){
            if(child->node_type!=lextype){
                va_end(ap);
                return 0;//拒绝
            }
        }
        else if(strcmp("INT",name)==0){
            if(child->node_type!=lexint){
                va_end(ap);
                return 0;//拒绝
            }
        }
        else if(strcmp("FLOAT",name)==0){
            if(child->node_type!=lexfloat){
                va_end(ap);
                return 0;//拒绝
            }
        }
        else{
            if(strcmp(name,child->info_char)!=0){
                va_end(ap);
                return 0;
            }
        }
        child=child->next;
    }
    va_end(ap);
    if(child==NULL)
        return 1;//接受
    return 0;
}

void error_output(int no,int line,char* msg){
    printf("Error type %d at Line %d: %s \"%s\".\n",no,line,error_msg[no],msg);
    semantic_error++;
}

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
    if(gencheck(root,2,"ExtDef","ExtDefList")){
        debug("ExtDefList -> ExtDef ExtDefList\n");
        ExtDef_analyse(child1);
        ExtDefList_analyse(child2);
    }
    else if(gencheck(root,1,"ExtDef")){
        debug("ExtDefList -> ExtDef ExtDefList(empty)\n");
        ExtDef_analyse(child1);
    }
}

void ExtDef_analyse(Node *root){
    Type type=NULL;
    if(gencheck(root,3,"Specifier","ExtDecList","SEMI")){
        debug("ExtDef -> Specifier ExtDecList SEMI\n");
        type=Specifier_analyse(root->child);
        ExtDecList_analyse(root->child->next,type);
    }
    else if(gencheck(root,3,"Specifier","FunDec","CompSt")){
        debug("ExtDef -> Specifier FunDec CompSt\n");
        //TODO:进入新的作用域
        type=Specifier_analyse(root->child);
        FunDec_analyse(root->child->next,type,1);
        CompSt_analyse(root->child->next->next);
    }
    else if(gencheck(root,3,"Specifier","FunDec","SEMI")){
        debug("ExtDef -> Specifier FunDec SEMI\n");
        //TODO:进入新的作用域
        type=Specifier_analyse(root->child);
        FunDec_analyse(root->child->next,type,0);
    }
    else if(gencheck(root,2,"Specifier","SEMI")){
        debug("ExtDef -> Specifier SEMI\n");
        type=Specifier_analyse(root->child);
        //TODO:添加进符号表
    }
    else{
        debug("error in ExtDef_analyse\n");
        exit(1);
    }
}

void ExtDecList_analyse(Node *node,Type type){
    if(gencheck(node,3,"VarDec","COMMA","ExtDecList")){
        debug("ExtDecList -> VarDec COMMA \n");
        FieldList field=VarDec_analyse(node->child,type);
        if(query_symbol(field->name)!=NULL){
            //TODO:报错
	    error_output(3,node->lineno,field->name);
        }
        else{
            insert_node(field->type,field->name);
            free(field);
        }
        ExtDecList_analyse(node->child->next->next,type);
    }
    else if(gencheck(node,1,"VarDec")){
        debug("ExtDecList -> VarDec\n");
        FieldList field=VarDec_analyse(node->child,type);
        if(query_symbol(field->name)!=NULL){
            //TODO:报错
	    error_output(3,node->lineno,field->name);
        }
        else{
            insert_node(field->type,field->name);
            free(field);
        }
    }
    else{
        debug("error in ExtDecList_analyse\n");
        exit(1);
    }
}

Type Specifier_analyse(Node *node){
    Type type=NULL;
    if(gencheck(node,1,"TYPE")){
        debug("Specifier -> TYPE\n");
        if(strcmp(node->child->info_char,"int")==0){
            type=&type_int;
        }
        else if(strcmp(node->child->info_char,"float")==0){
            type=&type_float;
        }
        else{
            debug("should not reach here(Specifier basic type)\n");
            exit(1);
        }
    }
    else{
        debug("Error int Specifier_analyse\n");
        //TODO: 结构体(可能有匿名)
    }
    return type;
}

void StructSpecifier_analyse(Node *node){return;}

void OptTag_analyse(Node *node){return;}

void Tag_analyse(Node *node){return;}

FieldList VarDec_analyse(Node *node,Type type){
    if(gencheck(node,1,"ID")){
        debug("VarDec -> ID\n");
        FieldList field=malloc(sizeof(struct FieldList_));
        field->name=node->child->info_char;
        field->type=type;
        return field;
    }
    else if(gencheck(node,4,"VarDec","LB","INT","RB")){
        debug("VarDec -> VarDec LB INT RB\n");
        Node *child2=node->child->next->next;
        //TODO:
    }
    else{
        debug("error int VarDec_analyse\n");
        exit(1);
    }
    return NULL;
}

void FunDec_analyse(Node *node,Type type,int def){
    //def=0:只声明
    //def=1:定义
    Type functype=malloc(sizeof(struct Type_));
    functype->kind=FUNCTION_T;
    functype->u.function.returntype=type;

    //TODO:判断是否定义和声明
    if(gencheck(node,4,"ID","LP","VarList","RP")){
        debug("FunDec -> ID LP VarList RP\n");
        //TODO:创建一个field
        FieldList field=VarList_analyse(node->child->next->next,NULL);
        int cnt=0;
        FieldList tmp=field;
        while(tmp!=NULL){
            cnt++;
            tmp=tmp->tail;
        }
        functype->u.function.paramscnt=cnt;
        functype->u.function.paramlist=field;
        functype->u.function.isdef=def;
        Type qtype=query_symbol(node->child->info_char);
        if(qtype==NULL){
            //未定义 or 声明的函数
            insert_node(functype,node->child->info_char);
        }
        else{
            if(def&&qtype->u.function.isdef){
                error_output(4,node->lineno,node->child->info_char);
            }
            else if(typecheck(functype,qtype)==0){
                error_output(19,node->lineno,node->child->info_char);

            }
        }
    }
    else if(gencheck(node,3,"ID","LP","RP")){
        debug("FunDec -> ID LP RP\n");
        functype->u.function.paramscnt=0;
        functype->u.function.paramlist=NULL;
        functype->u.function.isdef=def;
        Type qtype=query_symbol(node->child->info_char);
        if(qtype==NULL){
            //未定义 or 声明的函数
            insert_node(functype,node->child->info_char);
        }
        else{
            if(def&&qtype->u.function.isdef){
                error_output(4,node->lineno,node->child->info_char);
            }
            else if(typecheck(functype,qtype)==0){
                error_output(19,node->lineno,node->child->info_char);

            }
        }
    }
    else{
        debug("error in _analyse\n");
    }
    return;
}

FieldList VarList_analyse(Node *node,FieldList head){
    if(gencheck(node,1,"ParamDec")){
        debug("VarList -> ParamDec\n");
        FieldList field=ParamDec_analyse(node->child);
        if(head!=NULL){
            head->tail=field;
        }
        return field;
    }
    else if(gencheck(node,3,"ParamDec","COMMA","VarList")){
        debug("VarList -> ParamDec COMMA VarList\n");
        FieldList field=ParamDec_analyse(node->child);
        if(head!=NULL){
            head->tail=field;
        }
        VarList_analyse(node->child->next->next,field);
        return field;
    }
    else{
        debug("error in VarList_analyse\n");
    }
    return NULL;
}

FieldList ParamDec_analyse(Node *node){
    if(gencheck(node,2,"Specifier","VarDec")){
        debug("ParamDec -> Specifier VarDec\n");
        Type type=Specifier_analyse(node->child);
        FieldList field=VarDec_analyse(node->child->next,type);
        return field;
    }
    else{
        debug("error in ParamDec_analyse\n");
    }
    return NULL;
}

void CompSt_analyse(Node *node){
    if(gencheck(node,2,"LC","RC")){
        debug("Compst -> LC DefList(empty) StmtList(empty) RC\n");
        // Do nothing
    }
    else if(gencheck(node,3,"LC","DefList","RC")){
        debug("Compst -> LC DefList StmtList(empty) RC\n");
        DefList_analyse(node->child->next->next);
    }
    else if(gencheck(node,3,"LC","StmtList","RC")){
        debug("CompSt -> LC DefList(empty) StmtList RC\n");
        StmtList_analyse(node->child->next);
    }
    else if(gencheck(node,4,"LC","DefList","StmtList","RC")){
        debug("Compst -> LC DefList StmtList RC\n");
        DefList_analyse(node->child->next);
        StmtList_analyse(node->child->next->next);
    }
    else{
        debug("error in CompSt_analyse\n");
    }
    return;
}

void StmtList_analyse(Node *node){
    if(gencheck(node,1,"Stmt")){
        debug("StmtList -> Stmt StmtList(empty)\n");
        Stmt_analyse(node->child);
    }
    else if(gencheck(node,2,"Stmt","StmtList")){
        debug("StmtList -> Stmt StmtList\n");
        Stmt_analyse(node->child);
        StmtList_analyse(node->child->next);
    }
    else{
        debug("error in StmtList_analyse\n");
    }
    return;
}

void Stmt_analyse(Node *node){
    if(gencheck(node,1,"CompSt")){
        debug("Stmt -> CompSt\n");
        //TODO:进入新作用域并分析
        CompSt_analyse(node->child);
    }
    else if(gencheck(node,2,"Exp","SEMI")){
        debug("Stmt -> Exp SEMI\n");
        //TODO:检查Exp是否错误
        Exp_analyse(node->child);
    }
    else if(gencheck(node,3,"RETURN","Exp","SEMI")){
        debug("Stmt -> RETURN Exp SEMI\n");
        //TODO:检查Exp是否错误
        Type etype=Exp_analyse(node->child->next);
        //TODO:检查返回值类型是否正确
    }
    else if(gencheck(node,5,"IF","LP","Exp","RP","Stmt")){
        debug("Stmt -> IF LP Exp RP Stmt\n");
        Type etype=Exp_analyse(node->child->next->next);
        if(!typecheck(etype,&type_int)){
            //TODO:if中变量类型错误
            error_output(0,node->lineno,"");
        }
        Stmt_analyse(node->child->next->next->next->next);
    }
    else if(gencheck(node,7,"IF","LP","Exp","RP","Stmt","ELSE","Stmt")){
        debug("Stmt -> IF LP Exp RP Stmt ELSE Stmt\n");
        Type etype=Exp_analyse(node->child->next->next);
        if(!typecheck(etype,&type_int)){
            //TODO:if中变量类型错误
            error_output(0,node->lineno,"");

        }
        Stmt_analyse(node->child->next->next->next->next);
        Stmt_analyse(node->child->next->next->next->next->next->next);
    }
    else if(gencheck(node,5,"WHILE","LP","Exp","RP","Stmt")){
        debug("Stmt -> WHILE LP Exp RP Stmt\n");
        //TODO:检查Exp错误
        Type etype=Exp_analyse(node->child->next->next);
        if(!typecheck(etype,&type_int)){
            //TODO:while中变量类型错误
            error_output(0,node->lineno,"");

        }
        Stmt_analyse(node->child->next->next->next->next);
    }
    else{
        debug("error in Stmt_analyse\n");
    }
    return;
}

void DefList_analyse(Node *node){
    if(gencheck(node,1,"Def")){
        debug("DefList -> Def DefList(empty)\n");
        Def_analyse(node->child);
    }
    else if(gencheck(node,2,"Def","DefList")){
        debug("DefList -> Def DefList\n");
        Def_analyse(node->child);
        DefList_analyse(node->child->next);
    }
    else{
        debug("error in DefList_analyse\n");
    }
    return;
}

void Def_analyse(Node *node){
    if(gencheck(node,3,"Specifier","DecList","SEMI")){
        debug("Def -> Specifier DecList SEMI\n");
        Type type=Specifier_analyse(node->child);
        DecList_analyse(node->child->next,type);
    }
    else{
        debug("error in Def_analyse\n");
    }
    return;
}

void DecList_analyse(Node *node,Type type){
    if(gencheck(node,1,"Dec")){
        debug("DecList -> Dec\n");
        Dec_analyse(node->child,type);
    }
    else if(gencheck(node,3,"Dec","COMMA","DecList")){
        debug("DecList -> Dec COMMA DecList\n");
        Dec_analyse(node->child,type);
        DecList_analyse(node->child->next->next,type);
    }
    else{
        debug("error in DecList_analyse\n");
    }
    return;
}

void Dec_analyse(Node *node,Type type){
    if(gencheck(node,1,"VarDec")){
        debug("Dec -> VarDec\n");
        FieldList field=VarDec_analyse(node->child,type);
        if(query_symbol(field->name)!=NULL){
            //TODO:报错
            error_output(3,node->lineno,field->name);
        }
        else{
            insert_node(field->type,field->name);
            free(field);
        }
    }
    else if(gencheck(node,3,"VarDec","ASSIGNOP","Exp")){
        debug("Dec -> VarDec ASSIGNOP Exp\n");
        FieldList field=VarDec_analyse(node->child,type);
        Type type=Exp_analyse(node->child->next->next);
        if(!typecheck(type,field->type)){
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
    else{
        debug("error in Dec_analyse\n");
    }
    return;
}

Type Exp_analyse(Node *node){
    Node *child1=node->child;
    if(gencheck(node,1,"INT")){
        debug("Exp -> INT\n");
        return &type_int;
    }
    else if(gencheck(node,1,"FLOAT")){
        debug("Exp -> FLOAT\n");
        return &type_float;
    }
    else if(gencheck(node,1,"ID")){
        debug("Exp -> ID\n");
        Type type=query_symbol(child1->info_char);
        if(type==NULL){
            //TODO:报错
        }
        else{
            return type;
        }
    }
    else if(gencheck(node,2,"MINUS","Exp")){
        debug("Exp -> MINUS Exp\n");
        return Exp_analyse(child1->next);
    }
    else if(gencheck(node,2,"NOT","Exp")){
        debug("Exp -> NOT Exp\n");
        return Exp_analyse(child1->next);
    }
    else if(gencheck(node,3,"Exp","OP","Exp")){
        debug("Exp -> Exp OP Exp\n");
        Type type1=Exp_analyse(child1);
        Type type2=Exp_analyse(child1->next->next);
        if(typecheck(type1,type2)){
            return type1;
        }
        else{
            //TODO:报错
        }
    }
    else if(gencheck(node,3,"Exp","DOT","ID")){
        debug("Exp -> Exp DOT ID\n");
        Type type1=Exp_analyse(child1);
        //TODO:找到ID的类型并返回
    }
    else if(gencheck(node,4,"Exp","LB","Exp","RB")){
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
    else if(gencheck(node,3,"LP","Exp","RP")){
        debug("Exp -> LP Exp RP\n");
        return Exp_analyse(child1->next);
    }
    else if(gencheck(node,3,"ID","LP","RP")){
        debug("Exp -> ID LP RP\n");
        //TODO:函数调用
        Type type=query_symbol(child1->info_char);
        if(type->u.function.paramscnt==0){
            return type->u.function.returntype;
        }
        else{
            //TODO:函数调用的参数数量不符
            error_output(9,node->lineno,child1->info_char);
        }
    }
    else if(gencheck(node,4,"ID","LP","Args","RP")){
        debug("Exp -> ID LP Args RP\n");
        //TODO:函数调用
        Type type=query_symbol(child1->info_char);
        FieldList head=type->u.function.paramlist;
        if(Args_analyse(child1->next->next,head)==0){
            return type->u.function.returntype;
        }
        else{
            //TODO:函数调用的参数数量/类型不符
            error_output(9,node->lineno,child1->info_char);

        }
    }
    else{
        debug("error in Exp_analyse\n");
    }
    return NULL;
}

int  Args_analyse(Node *node,FieldList field){
    Node *exp=node->child;
    Node *args=NULL;
    if(gencheck(node,1,"Exp")){
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
    else if(gencheck(node,3,"Exp","COMMA","Args")){
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
    else{
        debug("error in Args_analyse\n");
    }
    return 0;
}
