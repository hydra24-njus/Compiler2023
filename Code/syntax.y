%locations
%{
#include <stdio.h>
#include "lex.yy.c"

extern int Synerror;
extern Node *root;
unsigned _error[100];
int cnt=0;
unsigned check(int lineno){
    for(int i=0;i<cnt;i++)
        if(_error[i]==lineno)
            return 0;
    _error[cnt++]=lineno;
    return 1;
}
#ifdef _DEBUG_
#define YYDEBUG 1
int yydebug=1;
#endif
%}

%union{
    struct node* type_node;
}
%token <type_node> INT
%token <type_node> FLOAT
%token <type_node> ID
%token <type_node> SEMI
%token <type_node> COMMA
%token <type_node> ASSIGNOP
%token <type_node> RELOP
%token <type_node> PLUS MINUS STAR DIV
%token <type_node> AND OR NOT
%token <type_node> DOT
%token <type_node> TYPE
%token <type_node> LP RP LB RB LC RC
%token <type_node> STRUCT
%token <type_node> RETURN IF ELSE WHILE


/* declared non-terminals */
%type <type_node> Program ExtDefList ExtDef ExtDecList
%type <type_node> Specifier StructSpecifier 
%type <type_node> OptTag 
%type <type_node> Tag
%type <type_node> VarDec FunDec VarList ParamDec
%type <type_node> CompSt StmtList Stmt
%type <type_node> DefList Def Dec DecList
%type <type_node> Exp Args


%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT NEG
%left LP RP LB RB LC RC DOT


%%
// 7.1.2 High-level Definitions
Program:        ExtDefList                  {$$=creat_node(synunit,@$.first_line,0,0,"Program");root=$$;build_tree($$,$1);}
    ;
ExtDefList:     ExtDef ExtDefList           {$$=creat_node(synunit,@$.first_line,0,0,"ExtDefList");build_tree($$,$2);build_tree($$,$1);}
    |                                       {$$=NULL;}
    ;
ExtDef:         Specifier ExtDecList SEMI   {$$=creat_node(synunit,@$.first_line,0,0,"ExtDef");build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    |           Specifier SEMI              {$$=creat_node(synunit,@$.first_line,0,0,"ExtDef");build_tree($$,$2);build_tree($$,$1);}
    |           Specifier FunDec CompSt     {$$=creat_node(synunit,@$.first_line,0,0,"ExtDef");build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    |           Specifier FunDec SEMI       {$$=creat_node(synunit,@$.first_line,0,0,"ExtDef");build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    |           error SEMI                  {$$=NULL;if(check(@1.first_line))printf("Error type B at line %d: \n",@1.first_line);}
    ;
ExtDecList:     VarDec                      {$$=creat_node(synunit,@$.first_line,0,0,"ExtDecList");build_tree($$,$1);}
    |           VarDec COMMA ExtDecList     {$$=creat_node(synunit,@$.first_line,0,0,"ExtDecList");build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    |           VarDec error                {$$=NULL;if(check(@1.first_line))printf("Error type B at line %d: global variable error\n",@1.first_line);}
    ;
// 7.1.3 Specifiers
Specifier:      TYPE                        {$$=creat_node(synunit,@$.first_line,0,0,"Specifier");build_tree($$,$1);}
    |           StructSpecifier             {$$=creat_node(synunit,@$.first_line,0,0,"Specifier");build_tree($$,$1);}
    ;
StructSpecifier:STRUCT OptTag LC DefList RC {$$=creat_node(synunit,@$.first_line,0,0,"StructSpecifier");build_tree($$,$5);build_tree($$,$4);build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    |           STRUCT Tag                  {$$=creat_node(synunit,@$.first_line,0,0,"StructSpecifier");build_tree($$,$2);build_tree($$,$1);}
    |           STRUCT error                {$$=NULL;if(check(@1.first_line))printf("Error type B at line %d: Illegal identifier\n",@1.first_line);}
    ;
OptTag:         ID                          {$$=creat_node(synunit,@$.first_line,0,0,"OptTag");build_tree($$,$1);}
    |                                       {$$=NULL;}
    ;
Tag:            ID                          {$$=creat_node(synunit,@$.first_line,0,0,"Tag");build_tree($$,$1);}
    ;
// 7.1.4 Declarators
VarDec:         ID                          {$$=creat_node(synunit,@$.first_line,0,0,"VarDec");build_tree($$,$1);}
    |           VarDec LB INT RB            {$$=creat_node(synunit,@$.first_line,0,0,"VarDec");build_tree($$,$4);build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    |           VarDec LB error RB          {$$=NULL;if(check(@1.first_line))printf("Error type B at line %d: Definition of an array only accepts an INT dimension\n",@1.first_line);}
    ;
FunDec:         ID LP VarList RP            {$$=creat_node(synunit,@$.first_line,0,0,"FunDec");build_tree($$,$4);build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    |           ID LP RP                    {$$=creat_node(synunit,@$.first_line,0,0,"FunDec");build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    |           ID LP error RP              {$$=NULL;if(check(@2.first_line))printf("Error type B at line %d: Error in Fundec\n",@2.first_line);}
    ;
VarList:        ParamDec COMMA VarList      {$$=creat_node(synunit,@$.first_line,0,0,"VarList");build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    |           ParamDec                    {$$=creat_node(synunit,@$.first_line,0,0,"VarList");build_tree($$,$1);}
    ;
ParamDec:       Specifier VarDec            {$$=creat_node(synunit,@$.first_line,0,0,"ParamDec");build_tree($$,$2);build_tree($$,$1);}
    ;
// 7.1.5 Statments
CompSt:         LC DefList StmtList RC      {$$=creat_node(synunit,@$.first_line,0,0,"CompSt");build_tree($$,$4);build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    ;
StmtList:       Stmt StmtList               {$$=creat_node(synunit,@$.first_line,0,0,"StmtList");build_tree($$,$2);build_tree($$,$1);}
    |                                       {$$=NULL;}
    |           Stmt Specifier error SEMI StmtList  {$$=NULL;if(check(@2.first_line))printf("Error type B at line %d: Illegal declaration\n",@2.first_line);}
//    |           error StmtList              {if(check(@1.first_line))printf("Error type B at line %d: Expected \";\"\n",@1.first_line);}
    ;
Stmt:           Exp SEMI                    {$$=creat_node(synunit,@$.first_line,0,0,"Stmt");build_tree($$,$2);build_tree($$,$1);}
    |           CompSt                      {$$=creat_node(synunit,@$.first_line,0,0,"Stmt");build_tree($$,$1);}
    |           RETURN Exp SEMI             {$$=creat_node(synunit,@$.first_line,0,0,"Stmt");build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    |           IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {$$=creat_node(synunit,@$.first_line,0,0,"Stmt");build_tree($$,$5);build_tree($$,$4);build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    |           IF LP Exp error Stmt %prec LOWER_THAN_ELSE {if(check(@3.first_line))printf("Error type B at line %d: Expected \")\"\n",@3.first_line);}
    |           IF LP Exp RP Stmt ELSE Stmt {$$=creat_node(synunit,@$.first_line,0,0,"Stmt");build_tree($$,$7);build_tree($$,$6);build_tree($$,$5);build_tree($$,$4);build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    |           IF LP Exp error Stmt ELSE Stmt {$$=NULL;if(check(@3.first_line))printf("Error type B at line %d: Expected \")\"\n",@3.first_line);}
    |           WHILE LP Exp RP Stmt        {$$=creat_node(synunit,@$.first_line,0,0,"Stmt");build_tree($$,$5);build_tree($$,$4);build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    |           WHILE LP Exp error          {$$=NULL;if(check(@1.first_line))printf("Error type B at line %d: Expected \")\"\n",@1.first_line);}
    |           Exp COMMA error SEMI        {$$=NULL;if(check(@1.first_line))printf("Error type B at line %d: Unexpected \",\"\n",@1.first_line);}
    |           Exp error                   {$$=NULL;if(check(@1.first_line))printf("Error type B at line %d: missing \";\"\n",@1.first_line);}
    |           RETURN Exp error            {$$=NULL;if(check(@1.first_line))printf("Error type B at line %d: missing \";\"\n",@1.first_line);}
    |           error SEMI                  {$$=NULL;if(check(@1.first_line))printf("Error type B at line %d: stmt error\n",@1.first_line);}
    ;
// 7.1.6 Local Definitions
DefList:        Def DefList                 {$$=creat_node(synunit,@$.first_line,0,0,"DefList");build_tree($$,$2);build_tree($$,$1);}
    |                                       {$$=NULL;}
    ;
Def:            Specifier DecList SEMI      {$$=creat_node(synunit,@$.first_line,0,0,"Def");build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    |           Specifier error SEMI        {$$=NULL;if(check(@1.first_line))printf("Error type B at line %d: Wrong variable\n",@1.first_line);}
    ;
DecList:        Dec                         {$$=creat_node(synunit,@$.first_line,0,0,"DecList");build_tree($$,$1);}
    |           Dec COMMA DecList           {$$=creat_node(synunit,@$.first_line,0,0,"DecList");build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    ;
Dec:            VarDec                      {$$=creat_node(synunit,@$.first_line,0,0,"Dec");build_tree($$,$1);}
    |           VarDec ASSIGNOP Exp         {$$=creat_node(synunit,@$.first_line,0,0,"Dec");build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    ;
// 7.1.7 Expressions
Exp:            Exp ASSIGNOP Exp            {$$=creat_node(synunit,@$.first_line,0,0,"Exp");build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    |           Exp AND Exp                 {$$=creat_node(synunit,@$.first_line,0,0,"Exp");build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    |           Exp OR Exp                  {$$=creat_node(synunit,@$.first_line,0,0,"Exp");build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    |           Exp RELOP Exp               {$$=creat_node(synunit,@$.first_line,0,0,"Exp");build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    |           Exp PLUS Exp                {$$=creat_node(synunit,@$.first_line,0,0,"Exp");build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    |           Exp MINUS Exp               {$$=creat_node(synunit,@$.first_line,0,0,"Exp");build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    |           Exp STAR Exp                {$$=creat_node(synunit,@$.first_line,0,0,"Exp");build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    |           Exp DIV Exp                 {$$=creat_node(synunit,@$.first_line,0,0,"Exp");build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    |           EExp error                  {$$=NULL;if(check(@1.first_line))printf("Error type B at line %d: Unexpected op\n",@1.first_line);}
    |           LP Exp RP                   {$$=creat_node(synunit,@$.first_line,0,0,"Exp");build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    |           LP error RP                 {$$=NULL;if(check(@1.first_line))printf("Error type B at line %d: Error Exp in\"()\"\n",@1.first_line);}
    |           MINUS Exp %prec NEG         {$$=creat_node(synunit,@$.first_line,0,0,"Exp");build_tree($$,$2);build_tree($$,$1);}
    |           NOT Exp                     {$$=creat_node(synunit,@$.first_line,0,0,"Exp");build_tree($$,$2);build_tree($$,$1);}
    |           ID LP Args RP               {$$=creat_node(synunit,@$.first_line,0,0,"Exp");build_tree($$,$4);build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    |           ID LP error RP              {$$=NULL;if(check(@1.first_line))printf("Error type B at line %d: Error function\n",@1.first_line);}
    |           ID LP RP                    {$$=creat_node(synunit,@$.first_line,0,0,"Exp");build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    |           Exp LB Exp RB               {$$=creat_node(synunit,@$.first_line,0,0,"Exp");build_tree($$,$4);build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    |           Exp LB error RB             {$$=NULL;if(check(@1.first_line))printf("Error type B at line %d: Error dimension\n",@1.first_line);}
    |           Exp DOT ID                  {$$=creat_node(synunit,@$.first_line,0,0,"Exp");build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    |           ID                          {$$=creat_node(synunit,@$.first_line,0,0,"Exp");build_tree($$,$1);}
    |           INT                         {$$=creat_node(synunit,@$.first_line,0,0,"Exp");build_tree($$,$1);}
    |           FLOAT                       {$$=creat_node(synunit,@$.first_line,0,0,"Exp");build_tree($$,$1);}
    ;
EExp:           Exp ASSIGNOP                {}
    |           Exp AND                     {}
    |           Exp OR                      {}
    |           Exp RELOP                   {}
    |           Exp PLUS                    {}
    |           Exp MINUS                   {}
    |           Exp STAR                    {}
    |           Exp DIV                     {}
    ;
Args:           Exp COMMA Args              {$$=creat_node(synunit,@$.first_line,0,0,"Args");build_tree($$,$3);build_tree($$,$2);build_tree($$,$1);}
    |           Exp                         {$$=creat_node(synunit,@$.first_line,0,0,"Args");build_tree($$,$1);}
%%
int yyerror(char* msg){
    Synerror=1;
    
    fprintf(stderr, "Error type B at line %d: (from yyerror) %s\n",yylineno,msg);
    return 0;
}