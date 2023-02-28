%option yylineno
%{
/*定义第三部分需要用到的变量*/
#include "syntax.tab.h"
int yycolumn=1;
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
\r|\n       {yycolumn=1;printf("\n");}
"//"        {char c=input();
             while(c&&(c!='\n'))c=input();
             //TODO:yycolumn;
            }
"/*"        {char c1=input(),c2=input();
             while(c2&&!(c1=='*'&&c2=='/')){
                c1=c2;c2=input();
             }
             if(!c2){
               //TODO:未匹配到 "*/"
               //printf("");
             }
             //TODO:yycolumn;
            }
{SEMI}      {return SEMI;}
{COMMA}     {return SEMI;}
{ASSIGNOP}  {return SEMI;}
{RELOP}     {return SEMI;}
{PLUS}      {return SEMI;}
{MINUS}     {return SEMI;}
{STAR}      {return SEMI;}
{DIV}       {return SEMI;}
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
{INT}       {return SEMI;}
{FLOAT}     {return SEMI;}
{ID}        {return ID;}
.           {printf("error!Non-matched patten.%s\n",yytext);}
%%