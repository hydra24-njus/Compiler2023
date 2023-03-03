%locations
%{
#include <stdio.h>
#include "lex.yy.c"
extern int Synerror;
unsigned _error[100];
int cnt=0;
unsigned check(int lineno){
    for(int i=0;i<cnt;i++)
        if(_error[i]==lineno)
            return 0;
    _error[cnt++]=lineno;
    return 1;
}
//#define YYDEBUG 1
//int yydebug=1;
#define CERROR(msg)\
        Synerror=1;\
        if(check(yylineno))\
            //printf("Error type B at line %d: %s",yylineno,msg);
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
    |           VarDec error                {CERROR("global variable error\n");}
    ;
// 7.1.3 Specifiers
Specifier:      TYPE                        {}
    |           StructSpecifier             {}
    ;
StructSpecifier:STRUCT OptTag LC DefList RC {}
    |           STRUCT Tag                  {}
    |           STRUCT error                {CERROR("Illegal identifier\n");}
    ;
OptTag:         ID                          {}
    |                                       {}
    ;
Tag:            ID                          {}
    ;
// 7.1.4 Declarators
VarDec:         ID                          {}
    |           VarDec LB INT RB            {}
    |           VarDec LB error RB          {CERROR("Definition of an array only accepts an INT dimension\n");}
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
    |           Stmt Specifier error SEMI StmtList  {CERROR("Illegal declaration\n");}
    ;
Stmt:           Exp SEMI                    {}
    |           CompSt                      {}
    |           RETURN Exp SEMI             {}
    |           IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {}
    |           IF LP Exp RP Stmt ELSE Stmt {}
    |           WHILE LP Exp RP Stmt        {}
    |           WHILE LP Exp error          {CERROR("Expected \")\"\n");}
    |           Exp COMMA error SEMI        {CERROR("Unexpected \",\"\n");}
    |           Exp error                   {CERROR("missing \";\"\n");}
    ;
// 7.1.6 Local Definitions
DefList:        Def DefList                 {}
    |                                       {}
    ;
Def:            Specifier DecList SEMI      {}
    |           Specifier error SEMI        {CERROR("Wrong variable\n");}
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
    |           Exp RELOP error             {CERROR("Unexpected op\n");}
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
    |           Exp LB error RB             {CERROR("Error dimension\n");}
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
    fprintf(stderr, "Error at line %d: \"%s\"\n",yylineno,yytext);
    return 0;
}