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
%type <type_int> Exp Args


%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT NEG
%left LP RP LB RB DOT

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

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
    ;
StmtList:       Stmt StmtList               {}
    |                                       {}
    ;
Stmt:           Exp SEMI                    {}
    |           CompSt                      {}
    |           RETURN Exp SEMI             {}
    |           IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {}
    |           IF LP Exp RP Stmt ELSE Stmt {}
    |           WHILE LP Exp RP Stmt        {}
    |           WHILE LP Exp error          {fprintf(stderr,"Expected \")\"\n");}
    |           Exp COMMA error SEMI        {fprintf(stderr,"Unexpected \",\"\n");}
    |           Exp error                   {fprintf(stderr,"line:%d missing \";\"\n",@1.first_line);}
    ;
// 7.1.6 Local Definitions
DefList:        Def DefList                 {}
    |                                       {}
    |           error DefList               {fprintf(stderr,"Illegal declaration\n");}
    ;
Def:            Specifier DecList SEMI      {}
    |           Specifier error SEMI        {fprintf(stderr,"Expected variable\n");}
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
    |           Exp RELOP error             {fprintf(stderr,"Unexpected op\n");}
    |           Exp PLUS Exp                {}
    |           Exp MINUS Exp               {}
    |           Exp STAR Exp                {}
    |           Exp DIV Exp                 {}
    |           LP Exp RP                   {}
    |           MINUS Exp %prec NEG         {}
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