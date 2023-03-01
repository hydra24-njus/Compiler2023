
%option yylineno
%{
/*定义第三部分需要用到的变量*/
#include "syntax.tab.h"
#include <stdlib.h>
#include <stdio.h>
int yycolumn=1;
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
INT_8       0[1-7][0-7]*
INT_16      0[xX][0-9a-fA-F]+
INT         {INT_8}|{INT_10}|{INT_16}
FLOAT       {digit}+"."{digit}+
ID          {letter}({digit}|{letter})*
%%
{space}     {/*Do nothing*/;}
\r|\n       {yycolumn=1;}
"//"        {char c=input();
             while(c&&(c!='\n'))c=input();
             //TODO:yycolumn;
            }
"/*"        {int lineno_now=yylineno;char c1=input(),c2=input();
             while(c2&&!(c1=='*'&&c2=='/')){
                if(c2=='\r'||c2=='\n')yycolumn=1;
                else yycolumn++;
                c1=c2;c2=input();
             }
             if(!c2){
               //TODO:未匹配到 "*/"
               printf("Error type A at line %d: unterminated comment\n", lineno_now);
             }
             else yycolumn++;
             //貌似没有必要维护 yycolumn
            }
{SEMI}      {return SEMI;}
{COMMA}     {return SEMI;}
{ASSIGNOP}  {return SEMI;}
{RELOP}     {return SEMI;}
{PLUS}      {return PLUS;}
{MINUS}     {return MINUS;}
{STAR}      {return STAR;}
{DIV}       {return DIV;}
{AND}       {return SEMI;}
{OR}        {return SEMI;}
{DOT}       {return SEMI;}
{NOT}       {return SEMI;}
{TYPE}      {return SEMI;}
{LP}        {return SEMI;}
{RP}        {return SEMI;}
{LB}        {return SEMI;}
{RB}        {return SEMI;}
{LC}        {return SEMI;}
{RC}        {return SEMI;}
{STRUCT}    {return SEMI;}
{RETURN}    {return SEMI;}
{IF}        {return SEMI;}
{ELSE}      {return SEMI;}
{WHILE}     {return SEMI;}
{INT}       {printf("line:%d\tcolumn:%d\n",yylloc.first_line,yylloc.first_column);yylval.type_int=strtol(yytext,NULL,0);return INT;}//a good function strtol(const char* nptr,char** endptr,int base);endptr为NULL则转换完整字符串、base为0则根据形式决定进制
{FLOAT}     {yylval.type_float=atof(yytext);return FLOAT;}
{ID}        {return ID;}
.           {printf("Error type A at Line %d: Mysterious characters \'%s\'\n",yylineno, yytext);}

%%