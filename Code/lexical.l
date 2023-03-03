
%option yylineno
%{
/*定义第三部分需要用到的变量*/
#include "syntax.tab.h"
#include <stdlib.h>
#include <stdio.h>
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
             while(c2&&!(c1=='*'&&c2=='/')){
                if(c2=='\r'||c2=='\n')yycolumn=1;
                else yycolumn++;
                c1=c2;c2=input();
             }
             if(!c2){
               //TODO:未匹配到 "*/"
               Lexerror=1;
               fprintf(stderr,"Error type A at line %d: unterminated comment\n", lineno_now);
             }
             else yycolumn++;
             //貌似没有必要维护 yycolumn
            }
{SEMI}      {return SEMI;}
{COMMA}     {return COMMA;}
{ASSIGNOP}  {return ASSIGNOP;}
{RELOP}     {return RELOP;}
{PLUS}      {return PLUS;}
{MINUS}     {return MINUS;}
{STAR}      {return STAR;}
{DIV}       {return DIV;}
{AND}       {return AND;}
{OR}        {return OR;}
{DOT}       {return DOT;}
{NOT}       {return NOT;}
{TYPE}      {return TYPE;}
{LP}        {return LP;}
{RP}        {return RP;}
{LB}        {return LB;}
{RB}        {return RB;}
{LC}        {return LC;}
{RC}        {return RC;}
{STRUCT}    {return STRUCT;}
{RETURN}    {return RETURN;}
{IF}        {return IF;}
{ELSE}      {return ELSE;}
{WHILE}     {return WHILE;}
{INT}       {yylval.type_int=strtol(yytext,NULL,0);return INT;}//a good function strtol(const char* nptr,char** endptr,int base);endptr为NULL则转换完整字符串、base为0则根据形式决定进制
{INT_E}     {yylval.type_int=0;Lexerror=1;fprintf(stderr,"Error type A at Line %d: Illegal INT: \'%s\'\n",yylineno, yytext);return INT;}
{FLOAT}     {yylval.type_float=atof(yytext);return FLOAT;}
{FLOAT_E}   {yylval.type_float=0;Lexerror=1;fprintf(stderr,"Error type A at Line %d: Illegal FLOAT: \'%s\'\n",yylineno, yytext);return FLOAT;}
{ID}        {yylval.type_char=yytext;return ID;}
.           {Lexerror=1;fprintf(stderr,"Error type A at Line %d: Mysterious characters \'%s\'\n",yylineno, yytext);}

%%
