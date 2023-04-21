#include "semantic.h"
#include "debug.h"
#include "ir.h"
#include <assert.h>

struct Type_ type_int={
    .kind=BASIC,
    .u.basic=INT
};
struct Type_ type_float={
    .kind=BASIC,
    .u.basic=FLOAT
};
const char* readname="read";
const char* writename="write";
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
    "Type mismatched for return, should be",//8
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
static char* type_name[5]={"int","float","array","struct","function"};
extern struct FunctionList_ *functable;
int _depth=0;
int _struct_depth=0;

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
        else{
            if(strcmp(name,child->node_info)!=0){
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

void funcheck(){
    struct FunctionList_ *node=functable->next;
    while(node!=NULL){
        error_output(18,node->lineno,node->name);
        node=node->next;
    }
}

__attribute__((constructor)) int io_init(){
    //printf("io init\n\n");
    Type readfun=malloc(sizeof(struct Type_));
    memset(readfun,0,sizeof(struct Type_));
    readfun->kind=FUNCTION_T;
    readfun->u.function.returntype=&type_int;
    readfun->u.function.paramscnt=0;
    readfun->u.function.isdef=1;
    readfun->u.function.paramlist=NULL;
    insert_node(readfun,readname,0,FUNCTION,NULL);

    Type writefun=malloc(sizeof(struct Type_));
    memset(writefun,0,sizeof(struct Type_));
    writefun->kind=FUNCTION_T;
    writefun->u.function.returntype=&type_int;
    writefun->u.function.paramscnt=1;
    writefun->u.function.isdef=1;
    FieldList param=malloc(sizeof(struct FieldList_));
    memset(param,0,sizeof(struct FieldList_));
    param->name=NULL;
    param->type=&type_int;
    param->tail=NULL;
    writefun->u.function.paramlist=param;
    insert_node(writefun,writename,0,FUNCTION,NULL);
    return 0;
}

int semantic(Node *root){
    if(root==NULL){
        return 1;
    }
    if(strcmp(root->node_info,"Program")!=0){
        return 1;
    }
    Program_analyse(root);
    funcheck();
    delete_functable();
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
        ScopeList scope1=enter_new_scope();
        _depth+=1;
            FunDec_analyse(root->child->next,type,1,scope1);
            CompSt_analyse(root->child->next->next,type,scope1);
        _depth-=1;
        exit_cur_scope();
    }
    else if(gencheck(root,3,"Specifier","FunDec","SEMI")){
        debug("ExtDef -> Specifier FunDec SEMI\n");
        //TODO:进入新的作用域
        type=Specifier_analyse(root->child);
        ScopeList scope=enter_new_scope();
        _depth+=1;
            FunDec_analyse(root->child->next,type,0,scope);
        _depth-=1;
        exit_cur_scope();
    }
    else if(gencheck(root,2,"Specifier","SEMI")){
        debug("ExtDef -> Specifier SEMI\n");
        type=Specifier_analyse(root->child);
    }
    else{
        debug("error in ExtDef_analyse\n");
        assert(0);
    }
}

void ExtDecList_analyse(Node *node,Type type){
    if(gencheck(node,3,"VarDec","COMMA","ExtDecList")){
        debug("ExtDecList -> VarDec COMMA ExtDecList\n");
        FieldList field=VarDec_analyse(node->child,type);
        if(query_symbol(field->name,0,0)!=NULL){
            //TODO:报错
	        error_output(3,node->lineno,field->name);
        }
        else{
            insert_node(field->type,field->name,0,VARIABLE,NULL);
            free(field);
        }
        ExtDecList_analyse(node->child->next->next,type);
    }
    else if(gencheck(node,1,"VarDec")){
        debug("ExtDecList -> VarDec\n");
        FieldList field=VarDec_analyse(node->child,type);
        if(query_symbol(field->name,0,0)!=NULL){
            //TODO:报错
	        error_output(3,node->lineno,field->name);
        }
        else{
            insert_node(field->type,field->name,0,VARIABLE,NULL);
            free(field);
        }
    }
    else{
        debug("error in ExtDecList_analyse\n");
        assert(0);
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
            assert(0);
        }
    }
    else if(gencheck(node,1,"StructSpecifier")){
        debug("Specifier -> StructSpecifier\n");
        type=StructSpecifier_analyse(node->child);
    }
    else{
        debug("Error int Specifier_analyse\n");
    }
    return type;
}

Type StructSpecifier_analyse(Node *node){
    //should return Type
    if(gencheck(node,4,"STRUCT","LC","DefList","RC")){
        debug("StructSpecifier -> STRUCT OptTag(empty) LC DefList RC\n");
        //匿名结构体
        _struct_depth++;
        FieldList field=DefList_struct_analyse(node->child->next->next);
        _struct_depth--;
        Type type=malloc(sizeof(struct Type_));memset(type,0,sizeof(struct Type_));
        type->kind=STRUCTURE;
        type->u.structure=field;
        while(field!=NULL){
            delete_struct_table(field->name);
            field=field->tail;
        }
        char *noname=malloc(4*sizeof(char));memset(noname,0,4*sizeof(char));
        static int noname_cnt=0;
        sprintf(noname,"%d",noname_cnt++);
        insert_node(type,noname,0,STRUCT,NULL);
        return type;
    }
    else if(gencheck(node,5,"STRUCT","OptTag","LC","DefList","RC")){
        debug("StructSpecifier -> STRUCT OptTag LC DefList RC\n");
        char *optag=node->child->next->child->info_char;
        if(query_symbol(optag,0,0)!=NULL){
            error_output(16,node->child->next->lineno,optag);
            return NULL;
        }
        _struct_depth++;
        FieldList field=DefList_struct_analyse(node->child->next->next->next);
        _struct_depth--;
        Type type=malloc(sizeof(struct Type_));memset(type,0,sizeof(struct Type_));
        type->kind=STRUCTURE;
        type->u.structure=field;
        while(field!=NULL){
            delete_struct_table(field->name);
            field=field->tail;
        }
        if(query_symbol(optag,0,0)!=NULL){
            error_output(16,node->child->next->lineno,optag);
        }
        insert_node(type,optag,0,STRUCT,NULL);
        return type;
    }
    else if(gencheck(node,2,"STRUCT","Tag")){
        debug("StructSpecifier -> Struct Tag\n");
        char *tag=node->child->next->child->info_char;
        Type type=query_symbol(tag,1,0);
        if(type!=NULL){
            return type;
        }
        else{
            error_output(17,node->child->next->lineno,tag);
        }
    }
    else if(gencheck(node,4,"STRUCT","OptTag","LC","RC")){
        debug("StructSpecifier -> Struct OptTag LC RC\n");
        char *optag=node->child->next->child->info_char;
        if(query_symbol(optag,0,0)!=NULL){
            error_output(16,node->child->next->lineno,optag);
            return NULL;
        }
        Type type=malloc(sizeof(struct Type_));memset(type,0,sizeof(struct Type_));
        type->kind=STRUCTURE;
        type->u.structure=NULL;
        insert_node(type,optag,0,STRUCT,NULL);
        return type;
    }
    else if(gencheck(node,3,"STRUCT","LC","RC")){
        debug("StructSpecifier -> STRUCT LC RC\n");
        //匿名结构体
        FieldList field=NULL;
        Type type=malloc(sizeof(struct Type_));memset(type,0,sizeof(struct Type_));
        type->kind=STRUCTURE;
        type->u.structure=field;
        char noname[4];
        static int noname_cnt=0;
        sprintf(noname,"%d",noname_cnt++);
        insert_node(type,noname,0,STRUCT,NULL);
        return type;
    }
    else{
        debug("error in StructSpecifier_analyse(%d)\n",node->lineno);
        assert(0);
    }
    return NULL;
}

FieldList DefList_struct_analyse(Node *node){
    if(gencheck(node,1,"Def")){
        debug("DefList -> Def DefList(empty) (in struct)\n");
        return Def_struct_analyse(node->child);
    }
    else if(gencheck(node,2,"Def","DefList")){
        debug("DefList -> Def DefList (in struct)\n");
        FieldList head=Def_struct_analyse(node->child);
        FieldList tmp=head;
        while(tmp->tail!=NULL){
            tmp=tmp->tail;
        }
        tmp->tail=DefList_struct_analyse(node->child->next);
        return head;
    }
    else{
        debug("error in DefList_struct_analyse (in struct)\n");
    }
    return NULL;
}

FieldList Def_struct_analyse(Node *node){
    if(gencheck(node,3,"Specifier","DecList","SEMI")){
        debug("Def -> Specifier DecList SEMI (in struct)\n");
        Type type=Specifier_analyse(node->child);
        return DecList_struct_analyse(node->child->next,type);
    }
    else{
        debug("error in Def_struct_analyse (in struct)\n");
    }
    return NULL;
}

FieldList DecList_struct_analyse(Node *node,Type type){
    if(gencheck(node,1,"Dec")){
        debug("DecList -> Dec (in struct)\n");
        return Dec_struct_analyse(node->child,type);
    }
    else if(gencheck(node,3,"Dec","COMMA","DecList")){
        debug("DecList -> Dec COMMA DecList (in struct)\n");
        FieldList head=Dec_struct_analyse(node->child,type);
        head->tail=DecList_struct_analyse(node->child->next->next,type);
        return head;
    }
    else{
        debug("error in DecList_struct_analyse (in struct)\n");
    }
    return NULL;
}

FieldList Dec_struct_analyse(Node *node,Type type){
    if(gencheck(node,1,"VarDec")){
        debug("Dec -> VarDec (in struct)\n");
        FieldList field=VarDec_analyse(node->child,type);
        if(query_symbol_struct(field->name,_struct_depth)!=NULL){
            //TODO:改成查本结构体的表
            error_output(15,node->lineno,field->name);
        }
        else{
            insert_node_struct(field->type,field->name,_struct_depth);
        }
        return field;
    }
    else if(gencheck(node,3,"VarDec","ASSIGNOP","Exp")){
        debug("Dec -> VarDec ASSIGNOP Exp (in struct)\n");
        //报错：结构体不能赋值
        error_output(15,node->child->next->lineno,"assignop");
        //注意 不能return NULL
        FieldList field=VarDec_analyse(node->child,type);
        Type type=Exp_analyse(node->child->next->next);
        if(!typecheck(type,field->type)){
            //TODO:报错
            debug("should not reach here 4\n");
            assert(0);
        }
        if(query_symbol_struct(field->name,_struct_depth)!=NULL){
            error_output(15,node->child->lineno,field->name);
        }
        else{
            insert_node_struct(field->type,field->name,_struct_depth);
        }
        return field;
    }
    else{
        debug("error in Dec_struct_analyse (in struct)\n");
    }
    return NULL;
}

FieldList VarDec_analyse(Node *node,Type type){
    if(gencheck(node,1,"ID")){
        debug("VarDec -> ID\n");
        FieldList field=malloc(sizeof(struct FieldList_));memset(field,0,sizeof(struct FieldList_));
        memset(field,0,sizeof(struct FieldList_));
        field->name=node->child->info_char;
        field->type=type;
        return field;
    }
    else if(gencheck(node,4,"VarDec","LB","INT","RB")){
        debug("VarDec -> VarDec LB INT RB\n");
        FieldList field=VarDec_analyse(node->child,type);

        Type type=malloc(sizeof(struct Type_));memset(type,0,sizeof(struct Type_));
        memset(type,0,sizeof(struct Type_));
        type->kind=ARRAY;
        type->u.array.size=node->child->next->next->info_int;

        if(field->type->kind!=ARRAY){
            type->u.array.elem=field->type;
            field->type=type;
        }
        else{
            Type elemprev=field->type;
            while(elemprev->u.array.elem->kind==ARRAY){
                elemprev=elemprev->u.array.elem;
            }
            type->u.array.elem=elemprev->u.array.elem;
            elemprev->u.array.elem=type;
        }

        return field;
    }
    else{
        debug("error int VarDec_analyse\n");
        assert(0);
    }
    return NULL;
}

void FunDec_analyse(Node *node,Type type,int def,ScopeList scope){
    //def=0:只声明
    //def=1:定义
    Type functype=malloc(sizeof(struct Type_));memset(functype,0,sizeof(struct Type_));
    functype->kind=FUNCTION_T;
    functype->u.function.returntype=type;

    if(gencheck(node,4,"ID","LP","VarList","RP")){
        debug("FunDec -> ID LP VarList RP\n");
        FieldList field=VarList_analyse(node->child->next->next,NULL,scope);
        int cnt=0;
        FieldList tmp=field;
        while(tmp!=NULL){
            cnt++;
            if(query_symbol(tmp->name,0,_depth)==NULL){
                insert_node_asaddr(tmp->type,tmp->name,_depth,VARIABLE,scope);
            }
            else{
                error_output(3,node->child->next->next->lineno,tmp->name);
            }
            tmp=tmp->tail;
        }
        functype->u.function.paramscnt=cnt;
        functype->u.function.paramlist=field;
        functype->u.function.isdef=def;
        Type qtype=query_symbol(node->child->info_char,1,0);
        if(qtype==NULL){
            //未定义 or 声明的函数
            insert_node(functype,node->child->info_char,0,FUNCTION,NULL);
            Type tp=query_symbol(node->child->info_char,0,0);
            tp->u.function.isdef=def;
            if(def==0){
                //插入到未声明的链表中
                insert_function(node->child->lineno,node->child->info_char);
            }
        }
        else{
            if(def&&qtype->u.function.isdef){
                error_output(4,node->lineno,node->child->info_char);
            }
            else if(typecheck(functype,qtype)==0){
                error_output(19,node->lineno,node->child->info_char);
            }
            else if(def==1){
                qtype->u.function.isdef=1;
                delete_function(node->child->info_char);
            }
        }
    }
    else if(gencheck(node,3,"ID","LP","RP")){
        debug("FunDec -> ID LP RP\n");
        functype->u.function.paramscnt=0;
        functype->u.function.paramlist=NULL;
        functype->u.function.isdef=def;
        Type qtype=query_symbol(node->child->info_char,1,0);
        if(qtype==NULL){
            //未定义 or 声明的函数
            insert_node(functype,node->child->info_char,0,FUNCTION,NULL);
            Type tp=query_symbol(node->child->info_char,0,0);
            tp->u.function.isdef=def;
            if(def==0){
                //插入到未声明的链表中
                insert_function(node->child->lineno,node->child->info_char);
            }
        }
        else{
            if(def&&qtype->u.function.isdef){
                error_output(4,node->lineno,node->child->info_char);
            }
            else if(typecheck(functype,qtype)==0){
                error_output(19,node->lineno,node->child->info_char);
            }
            else if(def==1){
                qtype->u.function.isdef=1;
                delete_function(node->child->info_char);
            }
        }
    }
    else{
        debug("error in _analyse\n");
    }
    if(def)trans_FunDec(node);
    return;
}

FieldList VarList_analyse(Node *node,FieldList head,ScopeList scope){
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
        VarList_analyse(node->child->next->next,field,scope);
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

void CompSt_analyse(Node *node,Type type,ScopeList scope){
    if(gencheck(node,2,"LC","RC")){
        debug("Compst -> LC DefList(empty) StmtList(empty) RC\n");
        // Do nothing
    }
    else if(gencheck(node,3,"LC","DefList","RC")){
        debug("Compst -> LC DefList StmtList(empty) RC\n");
        DefList_analyse(node->child->next,scope);
    }
    else if(gencheck(node,3,"LC","StmtList","RC")){
        debug("CompSt -> LC DefList(empty) StmtList RC\n");
        StmtList_analyse(node->child->next,type,scope);
    }
    else if(gencheck(node,4,"LC","DefList","StmtList","RC")){
        debug("Compst -> LC DefList StmtList RC\n");
        DefList_analyse(node->child->next,scope);
        StmtList_analyse(node->child->next->next,type,scope);
    }
    else{
        debug("error in CompSt_analyse\n");
    }
    return;
}

void StmtList_analyse(Node *node,Type type,ScopeList scope){
    if(gencheck(node,1,"Stmt")){
        debug("StmtList -> Stmt StmtList(empty)\n");
        Stmt_analyse(node->child,type,scope);
    }
    else if(gencheck(node,2,"Stmt","StmtList")){
        debug("StmtList -> Stmt StmtList\n");
        Stmt_analyse(node->child,type,scope);
        StmtList_analyse(node->child->next,type,scope);
    }
    else{
        debug("error in StmtList_analyse\n");
    }
    return;
}

void Stmt_analyse(Node *node,Type type,ScopeList scope){
    if(gencheck(node,1,"CompSt")){
        debug("Stmt -> CompSt\n");
        //TODO:进入新作用域并分析
        ScopeList scope1=enter_new_scope();
        _depth+=1;
            CompSt_analyse(node->child,type,scope1);
        _depth-=1;
        exit_cur_scope();
    }
    else if(gencheck(node,2,"Exp","SEMI")){
        debug("Stmt -> Exp SEMI\n");
        Exp_analyse(node->child);
        //ir.c
        trans_Exp(node->child);
    }
    else if(gencheck(node,3,"RETURN","Exp","SEMI")){
        debug("Stmt -> RETURN Exp SEMI\n");
        Type etype=Exp_analyse(node->child->next);
        if(!typecheck(etype,type)){
            error_output(8,node->child->next->lineno,type_name[type->kind==BASIC?type->u.basic:type->kind+1]);
        }
        //ir.c
        Operand op=trans_Exp(node->child->next);
        InterCodes ret=new_intercode(IR_RETURN);
        ret->code->u.unaryop.unary=op;
        insert_code(ret);
    }
    else if(gencheck(node,5,"IF","LP","Exp","RP","Stmt")){
        debug("Stmt -> IF LP Exp RP Stmt\n");
        Type etype=Exp_analyse(node->child->next->next);
        if(!typecheck(etype,&type_int)){
            //error_output(0,node->lineno,"");
        }
        //ir.c
        Operand lable1=new_lable(),lable2=new_lable();
        trans_Cond(node->child->next->next,lable1,lable2);
        InterCodes code1=new_intercode(IR_LABEL);
        code1->code->u.unaryop.unary=lable1;
        insert_code(code1);

        Stmt_analyse(node->child->next->next->next->next,type,scope);

        //ir.c
        code1=new_intercode(IR_LABEL);
        code1->code->u.unaryop.unary=lable2;
        insert_code(code1);
    }
    else if(gencheck(node,7,"IF","LP","Exp","RP","Stmt","ELSE","Stmt")){
        debug("Stmt -> IF LP Exp RP Stmt ELSE Stmt\n");
        Type etype=Exp_analyse(node->child->next->next);
        if(!typecheck(etype,&type_int)){
            //error_output(0,node->lineno,"");
        }

        //ir.c
        Operand lable1=new_lable(),lable2=new_lable(),lable3=new_lable();
        trans_Cond(node->child->next->next,lable1,lable2);//code1
        InterCodes code1=new_intercode(IR_LABEL);
        code1->code->u.unaryop.unary=lable1;
        insert_code(code1);//LABEL lable1


        Stmt_analyse(node->child->next->next->next->next,type,scope);//code2

        //ir.c
        code1=new_intercode(IR_GOTO);
        code1->code->u.unaryop.unary=lable3;
        insert_code(code1);//GOTO lable3
        code1=new_intercode(IR_LABEL);
        code1->code->u.unaryop.unary=lable2;
        insert_code(code1);//LABEL lable2

        Stmt_analyse(node->child->next->next->next->next->next->next,type,scope);
    
        //ir.c
        code1=new_intercode(IR_LABEL);
        code1->code->u.unaryop.unary=lable3;
        insert_code(code1);//LABEL lable3
    }
    else if(gencheck(node,5,"WHILE","LP","Exp","RP","Stmt")){
        debug("Stmt -> WHILE LP Exp RP Stmt\n");
        Type etype=Exp_analyse(node->child->next->next);
        if(!typecheck(etype,&type_int)){
            //error_output(0,node->lineno,"");
        }

        //ir.c
        Operand lable1=new_lable(),lable2=new_lable(),lable3=new_lable();
        InterCodes code1=new_intercode(IR_LABEL);
        code1->code->u.unaryop.unary=lable1;
        insert_code(code1);//LABEL lable1

        trans_Cond(node->child->next->next,lable2,lable3);//code1
        
        code1=new_intercode(IR_LABEL);
        code1->code->u.unaryop.unary=lable2;
        insert_code(code1);//LABEL lable2

        Stmt_analyse(node->child->next->next->next->next,type,scope);//code2

        //ir.c
        code1=new_intercode(IR_GOTO);
        code1->code->u.unaryop.unary=lable1;
        insert_code(code1);//GOTO lable1

        code1=new_intercode(IR_LABEL);
        code1->code->u.unaryop.unary=lable3;
        insert_code(code1);//LABEL lable3
    }
    else{
        debug("error in Stmt_analyse\n");
    }
    return;
}

void DefList_analyse(Node *node,ScopeList scope){
    if(gencheck(node,1,"Def")){
        debug("DefList -> Def DefList(empty)\n");
        Def_analyse(node->child,scope);
    }
    else if(gencheck(node,2,"Def","DefList")){
        debug("DefList -> Def DefList\n");
        Def_analyse(node->child,scope);
        DefList_analyse(node->child->next,scope);
    }
    else{
        debug("error in DefList_analyse\n");
    }
    return;
}

void Def_analyse(Node *node,ScopeList scope){
    if(gencheck(node,3,"Specifier","DecList","SEMI")){
        debug("Def -> Specifier DecList SEMI\n");
        Type type=Specifier_analyse(node->child);
        DecList_analyse(node->child->next,type,scope);
    }
    else{
        debug("error in Def_analyse\n");
    }
    return;
}

void DecList_analyse(Node *node,Type type,ScopeList scope){
    if(gencheck(node,1,"Dec")){
        debug("DecList -> Dec\n");
        Dec_analyse(node->child,type,scope);
    }
    else if(gencheck(node,3,"Dec","COMMA","DecList")){
        debug("DecList -> Dec COMMA DecList\n");
        Dec_analyse(node->child,type,scope);
        DecList_analyse(node->child->next->next,type,scope);
    }
    else{
        debug("error in DecList_analyse\n");
    }
    return;
}

void Dec_analyse(Node *node,Type type,ScopeList scope){
    if(gencheck(node,1,"VarDec")){
        debug("Dec -> VarDec\n");
        FieldList field=VarDec_analyse(node->child,type);
        if(query_symbol(field->name,0,_depth)!=NULL){
            error_output(3,node->lineno,field->name);
        }
        else{
            insert_node(field->type,field->name,_depth,VARIABLE,scope);

            //ir.c
            Type vartype=field->type;
            int varsize=getsize(vartype);
            if(varsize!=4){
                InterCodes code=new_intercode(IR_DEC);
                code->code->u.assign.left=new_operand(IR_VARIABLE);
                sNode tmpnode=query_node(field->name,0,_depth);
                code->code->u.assign.left->u.vid=tmpnode->vid;
                code->code->u.assign.left->is_addr=tmpnode->is_addr;
                code->code->u.assign.right=new_operand(IR_CONSTANT);
                code->code->u.assign.right->u.value=varsize;
                insert_code(code);
            }

        }
        free(field);
    }
    else if(gencheck(node,3,"VarDec","ASSIGNOP","Exp")){
        debug("Dec -> VarDec ASSIGNOP Exp\n");
        FieldList field=VarDec_analyse(node->child,type);
        char* vname=field->name;
        Type type=Exp_analyse(node->child->next->next);
        if(!typecheck(type,field->type)){
            error_output(5,node->child->lineno,"\40");
        }
        if(query_symbol(field->name,0,_depth)!=NULL){
            error_output(3,node->child->lineno,field->name);
        }
        else{
            insert_node(field->type,field->name,_depth,VARIABLE,scope);
        }
        //ir.c
        Operand t0=trans_Exp(node->child->next->next);
        InterCodes code0=new_intercode(IR_ASSIGN);
        code0->code->u.assign.right=t0;
        code0->code->u.assign.left=new_operand(IR_VARIABLE);
        code0->code->u.assign.left->u.vid=query_node(vname,0,_depth)->vid;
        insert_code(code0);

        free(field);
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
        node->type=&type_int;
        return &type_int;
    }
    else if(gencheck(node,1,"FLOAT")){
        debug("Exp -> FLOAT\n");
        node->type=&type_float;
        return &type_float;
    }
    else if(gencheck(node,1,"ID")){
        debug("Exp -> ID\n");
        Type type=query_symbol(child1->info_char,1,_depth);
        if(type==NULL){
            error_output(1,node->child->lineno,node->child->info_char);
        }
        else{
            node->type=type;
            return type;
        }
    }
    else if(gencheck(node,2,"MINUS","Exp")){
        debug("Exp -> MINUS Exp\n");
        node->type=Exp_analyse(child1->next);
        return node->type;
    }
    else if(gencheck(node,2,"NOT","Exp")){
        debug("Exp -> NOT Exp\n");
        node->type=Exp_analyse(child1->next);
        return node->type;
    }
    else if(gencheck(node,3,"Exp","ASSIGNOP","Exp")){
        debug("Exp -> Exp ASSIGNOP Exp\n");
        Type type1=Exp_analyse(child1);
        Type type2=Exp_analyse(child1->next->next);
        if(gencheck(node->child,1,"ID")){

        }
        else if(gencheck(node->child,4,"Exp","LB","Exp","RB")){

        }
        else if(gencheck(node->child,3,"Exp","DOT","ID")){

        }
        else{
            //左值错误
            error_output(6,node->child->next->lineno,"\40");
        }
        if(typecheck(type1,type2)){
            node->type=type1;
            return type1;
        }
        else{
            error_output(5,node->child->next->lineno,"\40");//TODO:type
        }
    }
    else if(gencheck(node,3,"Exp","RELOP","Exp")){
        debug("Exp -> Exp RELOP Exp\n");
        Type type1=Exp_analyse(child1);
        Type type2=Exp_analyse(child1->next->next);
        if(typecheck(type1,type2)){
        }
        else{
            //TODO:区分赋值和其它运算
            error_output(7,node->child->lineno,"\40");//TODO:type
        }
        node->type=&type_int;
        return &type_int;
    }
    else if(gencheck(node,3,"Exp","OP","Exp")){
        debug("Exp -> Exp OP Exp\n");
        Type type1=Exp_analyse(child1);
        Type type2=Exp_analyse(child1->next->next);
        if(typecheck(type1,type2)){
            node->type=type1;
            return type1;
        }
        else{
            //TODO:区分赋值和其它运算
            error_output(7,node->child->lineno,"\40");//TODO:type
        }
    }
    else if(gencheck(node,3,"Exp","DOT","ID")){
        debug("Exp -> Exp DOT ID\n");
        Type type1=Exp_analyse(child1);
        if(type1==NULL||type1->kind!=STRUCTURE){
            error_output(13,node->child->lineno,"\40");//TODO:ID
            return NULL;
        }
        char *id=child1->next->next->info_char;
        FieldList field=type1->u.structure;
        while(field!=NULL){
            if(strcmp(id,field->name)==0){
                node->type=field->type;
                return field->type;
            }
            field=field->tail;
        }
        error_output(14,node->child->lineno,id);//TODO:type
        return NULL;
    }
    else if(gencheck(node,4,"Exp","LB","Exp","RB")){
        debug("Exp -> Exp LB Exp RB\n");
        Type type1=Exp_analyse(child1);
        Type type2=Exp_analyse(child1->next->next);
        if(type1==NULL||type1->kind!=ARRAY){
            error_output(10,node->child->lineno,"\40");//TODO:
            return NULL;
        }
        if(typecheck(type2,&type_int)){
            node->type=type1->u.array.elem;
            return type1->u.array.elem;
        }
        else{
            //报错
            error_output(12,node->child->lineno,"\40");//TODO:
            node->type=type1->u.array.elem;
            return type1->u.array.elem;
        }
    }
    else if(gencheck(node,3,"LP","Exp","RP")){
        debug("Exp -> LP Exp RP\n");
        node->type=Exp_analyse(child1->next);
        return node->type;
    }
    else if(gencheck(node,3,"ID","LP","RP")){
        debug("Exp -> ID LP RP\n");
        Type type=query_symbol(child1->info_char,1,_depth);
        if(type==NULL){
            error_output(2,node->child->lineno,node->child->info_char);
            return NULL;
        }
        else if(type->kind!=FUNCTION_T){
            error_output(11,node->child->lineno,node->child->info_char);
            return NULL;
        }
        if(type->u.function.paramscnt==0){
            node->type=type->u.function.returntype;
            return node->type;
        }
        else{
            error_output(9,node->lineno,child1->info_char);
            node->type=type->u.function.returntype;
            return node->type;
        }
    }
    else if(gencheck(node,4,"ID","LP","Args","RP")){
        debug("Exp -> ID LP Args RP\n");
        Type type=query_symbol(child1->info_char,1,_depth);
        if(type==NULL){
            error_output(2,node->child->lineno,node->child->info_char);
            return NULL;
        }
        if(type->kind!=FUNCTION_T){
            error_output(11,node->child->lineno,node->child->info_char);
            return NULL;
        }
        FieldList head=type->u.function.paramlist;
        if(Args_analyse(child1->next->next,head)==0){
            node->type=type->u.function.returntype;
            return node->type;
        }
        else{
            error_output(9,node->lineno,child1->info_char);
            node->type=type->u.function.returntype;
            return node->type;
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
