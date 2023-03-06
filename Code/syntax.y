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
    |           VarDec error                {if(check(@1.first_line))printf("Error type B at line %d: global variable error\n",@1.first_line);}
    ;
// 7.1.3 Specifiers
Specifier:      TYPE                        {}
    |           StructSpecifier             {}
    ;
StructSpecifier:STRUCT OptTag LC DefList RC {}
    |           STRUCT Tag                  {}
    |           STRUCT error                {if(check(@1.first_line))printf("Error type B at line %d: Illegal identifier\n",@1.first_line);}
    ;
OptTag:         ID                          {}
    |                                       {}
    ;
Tag:            ID                          {}
    ;
// 7.1.4 Declarators
VarDec:         ID                          {}
    |           VarDec LB INT RB            {}
    |           VarDec LB error RB          {if(check(@1.first_line))printf("Error type B at line %d: Definition of an array only accepts an INT dimension\n",@1.first_line);}
    ;
FunDec:         ID LP VarList RP            {}
    |           ID LP RP                    {}
    |           ID LP error RP              {if(check(@2.first_line))printf("Error type B at line %d: Error in Fundec\n",@2.first_line);}
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
    |           Stmt Specifier error SEMI StmtList  {if(check(@2.first_line))printf("Error type B at line %d: Illegal declaration\n",@2.first_line);}
//    |           error StmtList              {if(check(@1.first_line))printf("Error type B at line %d: Expected \";\"\n",@1.first_line);}
    ;
Stmt:           Exp SEMI                    {}
    |           CompSt                      {}
    |           RETURN Exp SEMI             {}
    |           IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {}
    |           IF LP Exp error Stmt %prec LOWER_THAN_ELSE {if(check(@3.first_line))printf("Error type B at line %d: Expected \")\"\n",@3.first_line);}
    |           IF LP Exp RP Stmt ELSE Stmt {}
    |           IF LP Exp error Stmt ELSE Stmt {if(check(@3.first_line))printf("Error type B at line %d: Expected \")\"\n",@3.first_line);}
    |           WHILE LP Exp RP Stmt        {}
    |           WHILE LP Exp error          {if(check(@1.first_line))printf("Error type B at line %d: Expected \")\"\n",@1.first_line);}
    |           Exp COMMA error SEMI        {if(check(@1.first_line))printf("Error type B at line %d: Unexpected \",\"\n",@1.first_line);}
    |           Exp error                   {if(check(@1.first_line))printf("Error type B at line %d: missing \";\"\n",@1.first_line);}
    |           RETURN Exp error            {if(check(@1.first_line))printf("Error type B at line %d: missing \";\"\n",@1.first_line);}
    |           error SEMI                  {if(check(@1.first_line))printf("Error type B at line %d: stmt error\n",@1.first_line);}
    ;
// 7.1.6 Local Definitions
DefList:        Def DefList                 {}
    |                                       {}
    ;
Def:            Specifier DecList SEMI      {}
    |           Specifier error SEMI        {if(check(@1.first_line))printf("Error type B at line %d: Wrong variable\n",@1.first_line);}
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
    |           Exp RELOP error             {if(check(@1.first_line))printf("Error type B at line %d: Unexpected op\n",@1.first_line);}
    |           Exp PLUS Exp                {}
    |           Exp MINUS Exp               {}
    |           Exp STAR Exp                {}
    |           Exp DIV Exp                 {}
    |           LP Exp RP                   {}
    |           LP error RP                 {if(check(@1.first_line))printf("Error type B at line %d: Error Exp in\"()\"\n",@1.first_line);}
    |           MINUS Exp %prec NEG         {}
    |           NOT Exp                     {}
    |           ID LP Args RP               {}
    |           ID LP error RP              {if(check(@1.first_line))printf("Error type B at line %d: Error function\n",@1.first_line);}
    |           ID LP RP                    {}
    |           Exp LB Exp RB               {}
    |           Exp LB error RB             {if(check(@1.first_line))printf("Error type B at line %d: Error dimension\n",@1.first_line);}
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
    
    //fprintf(stderr, "Error type B at line %d: \"%s\"\n",yylineno);
    return 0;
}