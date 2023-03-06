
%option yylineno
%{
/*定义第三部分需要用到的变量*/
#include "syntax.tab.h"
#include <stdlib.h>
#include <stdio.h>
#include "tree.h"
int yycolumn=1;
extern int Lexerror;
#define YY_USER_ACTION \
  yylloc.first_line = yylloc.last_line = yylineno; \
  yylloc.first_column = yycolumn; \
  yylloc.last_column = yycolumn + yyleng - 1; \
  yycolumn += yyleng;
%}
space       [ \t]
digit       [0-9]
letter      [_a-zA-Z]
SEMI        ";"
COMMA       ","
ASSIGNOP    "="
RELOP       >|<|>=|<=|==|!=
PLUS        "+"
MINUS       "-"
STAR        "*"
DIV         "/"
AND         "&&"
OR          "||"
DOT         "."
NOT         "!"
TYPE        int|float
LP          "("
RP          ")"
LB          "["
RB          "]"
LC          "{"
RC          "}"
STRUCT      struct
RETURN      return
IF          if
ELSE        else
WHILE       while
INT_10      0|[1-9][0-9]*
INT_8       0[0-7]*
INT_8E      (0[1-7][0-7]*[8-9][0-9]*)|(0[8-9][0-9]*)
INT_16      0[xX][0-9a-fA-F]+
INT_16E     (0[xX][0-9a-fA-F][g-zG-Z][0-9a-zA-Z]*)|(0[xX][g-zG-Z][0-9a-zA-Z]*)
INT         {INT_8}|{INT_10}|{INT_16}
INT_E       {INT_8E}|{INT_16E}
FLOAT_N     {digit}+"."{digit}+
FLOAT_S     ([0-9]+\.[0-9]*[eE][+-]?[0-9]+)|([0-9]*\.[0-9]+[eE][+-]?[0-9]+)
FLOAT       {FLOAT_N}|{FLOAT_S}
FLOAT_EN    ("."{digit}+)|({digit}+".")
FLOAT_ES    (([0-9]+\.[0-9]*)|([0-9]*\.[0-9]+))[eE][+-]?([0-9]*\.[0-9]*)?
FLOAT_E     {FLOAT_EN}|{FLOAT_ES}
ID          {letter}({digit}|{letter})*
%%
{space}     {}
\r|\n       {yycolumn=1;}
"//"        {char c=input();
             while(c&&(c!='\n'))c=input();
             //TODO:yycolumn;
            }
"/*"        {int lineno_now=yylineno;char c1=input(),c2=input();
             //printf("%c%c",c1,c2);
             while(c2&&!(c1=='*'&&c2=='/')){
                if(c2=='\r'||c2=='\n')yycolumn=1;
                else yycolumn++;
                //printf("%c",c2);
                c1=c2;c2=input();
             }
             if(!c2){
               //TODO:未匹配到 "*/"
               Lexerror=1;
               fprintf(stdout,"Error type A at line %d: unterminated comment\n", lineno_now);
             }
             else yycolumn++;
             //貌似没有必要维护 yycolumn
            }
{SEMI}      {yylval.type_node=creat_node(NULL,NULL,lexother,0,0,0,"SEMI");return SEMI;}
{COMMA}     {yylval.type_node=creat_node(NULL,NULL,lexother,0,0,0,"COMMA");return COMMA;}
{ASSIGNOP}  {yylval.type_node=creat_node(NULL,NULL,lexother,0,0,0,"ASSIGNOP");return ASSIGNOP;}
{RELOP}     {yylval.type_node=creat_node(NULL,NULL,lexother,0,0,0,"RELOP");return RELOP;}
{PLUS}      {yylval.type_node=creat_node(NULL,NULL,lexother,0,0,0,"PLUS");return PLUS;}
{MINUS}     {yylval.type_node=creat_node(NULL,NULL,lexother,0,0,0,"MINUS");return MINUS;}
{STAR}      {yylval.type_node=creat_node(NULL,NULL,lexother,0,0,0,"STAR");return STAR;}
{DIV}       {yylval.type_node=creat_node(NULL,NULL,lexother,0,0,0,"DIV");return DIV;}
{AND}       {yylval.type_node=creat_node(NULL,NULL,lexother,0,0,0,"AND");return AND;}
{OR}        {yylval.type_node=creat_node(NULL,NULL,lexother,0,0,0,"OR");return OR;}
{DOT}       {yylval.type_node=creat_node(NULL,NULL,lexother,0,0,0,"DOT");return DOT;}
{NOT}       {yylval.type_node=creat_node(NULL,NULL,lexother,0,0,0,"NOT");return NOT;}
{TYPE}      {yylval.type_node=creat_node(NULL,NULL,lextype,0,0,0,yytext);return TYPE;}
{LP}        {yylval.type_node=creat_node(NULL,NULL,lexother,0,0,0,"LP");return LP;}
{RP}        {yylval.type_node=creat_node(NULL,NULL,lexother,0,0,0,"RP");return RP;}
{LB}        {yylval.type_node=creat_node(NULL,NULL,lexother,0,0,0,"LB");return LB;}
{RB}        {yylval.type_node=creat_node(NULL,NULL,lexother,0,0,0,"RB");return RB;}
{LC}        {yylval.type_node=creat_node(NULL,NULL,lexother,0,0,0,"LC");return LC;}
{RC}        {yylval.type_node=creat_node(NULL,NULL,lexother,0,0,0,"RC");return RC;}
{STRUCT}    {yylval.type_node=creat_node(NULL,NULL,lexother,0,0,0,"STRUCT");return STRUCT;}
{RETURN}    {yylval.type_node=creat_node(NULL,NULL,lexother,0,0,0,"RETURN");return RETURN;}
{IF}        {yylval.type_node=creat_node(NULL,NULL,lexother,0,0,0,"IF");return IF;}
{ELSE}      {yylval.type_node=creat_node(NULL,NULL,lexother,0,0,0,"ELSE");return ELSE;}
{WHILE}     {yylval.type_node=creat_node(NULL,NULL,lexother,0,0,0,"WHILE");return WHILE;}
{INT}       {yylval.type_node=creat_node(NULL,NULL,lexint,0,strtol(yytext,NULL,0),0,0);return INT;}//a good function strtol(const char* nptr,char** endptr,int base);endptr为NULL则转换完整字符串、base为0则根据形式决定进制
{INT_E}     {yylval.type_node=creat_node(NULL,NULL,lexint,0,0,0,0);Lexerror=1;fprintf(stdout,"Error type A at Line %d: Illegal INT: \'%s\'\n",yylineno, yytext);return INT;}
{FLOAT}     {yylval.type_node=creat_node(NULL,NULL,lexfloat,0,0,atof(yytext),0);return FLOAT;}
{FLOAT_E}   {yylval.type_node=creat_node(NULL,NULL,lexfloat,0,0,0,0);Lexerror=1;fprintf(stdout,"Error type A at Line %d: Illegal FLOAT: \'%s\'\n",yylineno, yytext);return FLOAT;}
{ID}        {yylval.type_node=creat_node(NULL,NULL,lexid,0,0,0,yytext);return ID;}
.           {Lexerror=1;fprintf(stdout,"Error type A at Line %d: Mysterious characters \'%s\'\n",yylineno, yytext);}

%%
