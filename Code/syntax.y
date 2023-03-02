%locations
%{
#include <stdio.h>
#include "lex.yy.c"
extern int Synerror;
%}
/* declared types */
%union {
int type_int;
float type_float;
char* type_char;
}

%token <type_int> INT
%token <type_float> FLOAT
%token <type_char> ID
%token SEMI
%token COMMA
%token ASSIGNOP
%token RELOP
%token PLUS MINUS STAR DIV
%token AND OR NOT
%token DOT
%token TYPE
%token LP RP LB RB LC RC
%token STRUCT
%token RETURN IF ELSE WHILE


/* declared non-terminals */
%type Program ExtDefList ExtDef ExtDecList
%type Specifier StructSpecifier 
%type <type_char> OptTag 
%type Tag
%type VarDec FunDec VarList ParamDec
%type DefList Def Dec DecList
%type Exp Args


%right ASSIGN
%left ADD SUB
%left MUL DIV
%left LP RP


%%
// 7.1.2 High-level Definitions
Program:        ExtDefList                  {}
    ;
ExtDefList:     ExtDef ExtDefList           {}
    |                                       {}
    ;
ExtDef:         Specifier ExtDecList SEMI   {}
    |           Specifier SEMI              {}
    |           Specifier FunDec CompSt     {}
    ;
ExtDecList:     VarDec                      {}
    |           VarDec COMMA ExtDecList     {}
    |           VarDec error                {fprintf(stderr,"global variable error\n");}
    ;
// 7.1.3 Specifiers
Specifier:      TYPE                        {}
    |           StructSpecifier             {}
    ;
StructSpecifier:STRUCT OptTag LC DefList RC {}
    |           STRUCT Tag                  {}
    |           STRUCT error OptTag LC DefList RC        {fprintf(stderr,"Illegal identifier\n");}
    |           STRUCT error SEMI           {fprintf(stderr,"syntax error\n");}
    ;
OptTag:         ID                          {}
    |                                       {}
    ;
Tag:            ID                          {}
    ;
// 7.1.4 Declarators
VarDec:         ID                          {}
    |           VarDec LB INT RB            {}
    |           VarDec LB error RB          {fprintf(stderr,"Definition of an array only accepts an INT dimension\n");}
    ;
FunDec:         ID LP VarList RP            {}
    |           ID LP RP                    {}
    ;
VarList:        ParamDec COMMA VarList      {}
    |           ParamDec                    {}
    ;
ParamDec:       Specifier VarDec            {}
    ;
// 7.1.5 Statments
CompSt:         LC DefList StmtList RC      {}
    |           LC DefList StmtList error RC{fprintf(stderr,"Illegal declaration\n");}
    ;
StmtList:       Stmt StmtList               {}
    |                                       {}
    ;
Stmt:           Exp SEMI                    {}
    |           CompSt                      {}
    |           RETURN Exp SEMI             {}
    |           IF LP Exp RP Stmt           {}
    |           IF LP Exp RP Stmt ELSE Stmt {}
    |           WHILE LP Exp RP Stmt        {}
    |           Exp COMMA error SEMI        {fprintf(stderr,"Unexpected \",\"\n");}
    |           Exp error                   {fprintf(stderr,"missing \";\"\n");}
    ;
// 7.1.6 Local Definitions
DefList:        Def DefList                 {}
    |                                       {}
    ;
Def:            Specifier DecList SEMI      {}
    ;
DecList:        Dec                         {}
    |           Dec COMMA DecList           {}
    ;
Dec:            VarDec                      {}
    |           VarDec ASSIGNOP Exp         {}
    ;
// 7.1.7 Expressions
Exp:            Exp ASSIGNOP Exp            {}
    |           Exp AND Exp                 {}
    |           Exp OR Exp                  {}
    |           Exp RELOP Exp               {}
    |           Exp PLUS Exp                {}
    |           Exp MINUS Exp               {}
    |           Exp STAR Exp                {}
    |           Exp DIV Exp                 {}
    |           LP Exp RP                   {}
    |           MINUS Exp                   {}
    |           NOT Exp                     {}
    |           ID LP Args RP               {}
    |           ID LP RP                    {}
    |           Exp LB Exp RB               {}
    |           Exp LB error RB             {fprintf(stderr,"Error dimension\n");}
    |           Exp DOT ID                  {}
    |           ID                          {}
    |           INT                         {}
    |           FLOAT                       {}
    ;
Args:           Exp COMMA Args              {}
    |           Exp                         {}
%%

int yyerror(char* msg){
    Synerror=1;
    fprintf(stderr, "Error type B at line %d: ",yylineno);
    return 0;
}