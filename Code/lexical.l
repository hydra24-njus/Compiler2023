%{
/*定义第三部分需要用到的变量*/
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
INT         0|[1-9][0-9]*
FLOAT       {digit}+"."{digit}+
ID          {letter}({digit}|{letter})*
%%
{space}     {/*Do nothing*/;}
\r|\n       {yycolumn=1;printf("\n");}
"//"        {char c=input();while(c!='\n')c=input();}
{SEMI}      {printf(";");}
{COMMA}     {printf(",");}
{ASSIGNOP}  {printf("=");}
{RELOP}     {printf("RELOP");}
{PLUS}      {printf("+");}
{MINUS}     {printf("-");}
{STAR}      {printf("*");}
{DIV}       {printf("/");}
{AND}       {printf("&&");}
{OR}        {printf("||");}
{DOT}       {printf(".");}
{NOT}       {printf("!");}
{TYPE}      {printf(" TYPE ");}
{LP}        {printf("(");}
{RP}        {printf(")");}
{LB}        {printf("[");}
{RB}        {printf("]");}
{LC}        {printf("{");}
{RC}        {printf("}");}
{STRUCT}    {printf(" STRUCT ");}
{RETURN}    {printf(" RETURN ");}
{IF}        {printf(" IF ");}
{ELSE}      {printf(" ELSE ");}
{WHILE}     {printf(" WHILE ");}
{INT}       {printf(" (INT,%s) ",yytext);}
{FLOAT}     {printf(" (FLOAT,%s) ",yytext);}
{ID}        {printf(" (ID,%s) ",yytext);}
.           {printf("error!Non-matched patten.%s\n",yytext);}
%%