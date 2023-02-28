%{

#include "lex.yy.c"

%}

%token INT
%token ID
%token SEMI
%%

Id:     ID      {printf("id");}

%%

yyerror(char* msg){
    fprintf(stderr, "Error type B at line %d: %s\n", yylineno,msg);
}