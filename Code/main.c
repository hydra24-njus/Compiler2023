#include<stdio.h>

extern int yylineno;
extern int yyparse();
extern void yyrestart(FILE*);

int main(int argc,char** argv){
    if(argc<=1)return 1;
    FILE *f=fopen(argv[1],"r");
    if(!f){
        perror(argv[1]);
        return 1;
    }
    yyrestart(f);//将flex输入文件的指针设为f，并指向文件开头。
    yyparse();//对输入文件进行分析
    return 0;
}