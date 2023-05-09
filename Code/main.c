#include<stdio.h>
#include "debug.h"
#include "semantic.h"
#include "ir.h"
#include "asm.h"

extern int yylineno;
extern int yyparse();
extern void yyrestart(FILE*);
int Lexerror=0,Synerror=0;
extern int semantic_error;
Node *root=NULL;
int main(int argc,char** argv){
    if(argc<=1)return 1;
    FILE *f=fopen(argv[1],"r");
    if(!f){
        perror(argv[1]);
        return 1;
    }
    
    FILE *fp=fopen(argv[2],"wt+");
    if(!fp){
        perror(argv[2]);
        return 1;
    }
    yyrestart(f);//将flex输入文件的指针设为f，并指向文件开头。
    yyparse();//对输入文件进行分析
    debug("----------lexical and syntax pass----------\n");
    if(Lexerror||Synerror){
        return 0;//有词法 or 语法错误，直接退出
    }
    if(semantic(root)){
        debug("error in semantic.\n");
        return 1;//there must be something error
    }
    debug("---------------semantic pass---------------\n");
    if(semantic_error!=0){
        debug("has semantic error!\n");
        return 1;
    }
    //print_ir(stdout);
    print_asm(fp);
    return 0;
}