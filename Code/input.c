#include <assert.h>
#include "input.h"

struct Operand_ *handle_var(char *t){
    char* var=t;
    struct Operand_ *ret=malloc(sizeof(struct Operand_));
    memset(ret,0,sizeof(struct Operand_));
    if(var[0]=='#'){
        ret->kind=IR_CONSTANT;
        int x=0;
        sscanf(var,"#%d",&x);
        ret->u.value=x;
        return ret;
    }
    if(var[0]=='*'){
        ret->access=IR_POINT;
        var++;
    }
    else if(var[0]=='&'){
        ret->access=IR_ADDR;
        var++;
    }
    if(var[0]=='t'){
        ret->kind=IR_TMPOP;
        var++;
    }
    else if(var[0]=='v'){
        ret->kind=IR_VARIABLE;
        var++;
    }
    else assert(0);
    int x=0;
    sscanf(var,"%d",&x);
    ret->u.vid=x;
    return ret;
}
struct Operand_ *handle_fun(char *t){
    struct Operand_ *ret=malloc(sizeof(struct Operand_));
    memset(ret,0,sizeof(struct Operand_));
    ret->kind=IR_FUNCNAME;
    ret->u.funcname=malloc((strlen(t)+1)*sizeof(char));
    strcpy(ret->u.funcname,t);
    return ret;
}
struct Operand_ *handle_label(int x){
    struct Operand_ *ret=malloc(sizeof(struct Operand_));
    memset(ret,0,sizeof(struct Operand_));
    ret->kind=IR_LABELOP;
    ret->u.lableno=x;
    return ret;
}
struct Operand_ *handle_relop(char *t){
    struct Operand_ *ret=malloc(sizeof(struct Operand_));
    memset(ret,0,sizeof(struct Operand_));
    ret->kind=IR_RELOP;
    ret->u.relopid=malloc((strlen(t)+1)*sizeof(char));
    strcpy(ret->u.relopid,t);
    return ret;
}
struct InterCodes_ *input(FILE *fp){
    char buf[1024];
    char arg[4][32];
    int n1,n2,n3;

    while(fgets(buf,sizeof(buf),fp)!=NULL){
        memset(arg,'\0',sizeof(char)*32*4);
        n1=0,n2=0,n3=0;
        if(strncmp(buf,"LABEL",5)==0){
            sscanf(buf,"LABEL label%d :",&n1);
            InterCodes node=new_intercode(IR_LABEL);
            node->code->u.unaryop.unary=handle_label(n1);
            insert_code(node);
        }
        else if(strncmp(buf,"FUNCTION",8)==0){
            sscanf(buf,"FUNCTION %s :",arg[0]);
            InterCodes node=new_intercode(IR_FUNCTION);
            node->code->u.unaryop.unary=handle_fun(arg[0]);
            insert_code(node);
        }
        else if(strncmp(buf,"GOTO",4)==0){
            sscanf(buf,"GOTO label%d",&n1);
            InterCodes node=new_intercode(IR_GOTO);
            node->code->u.unaryop.unary=handle_label(n1);
            insert_code(node);
        }
        else if(strncmp(buf,"IF",2)==0){
            sscanf(buf,"IF %s %s %s GOTO label%d",arg[0],arg[1],arg[2],&n1);
            InterCodes node=new_intercode(IR_IFGOTO);
            node->code->u.gotop.op1=handle_var(arg[0]);
            node->code->u.gotop.relop=handle_relop(arg[1]);
            node->code->u.gotop.op2=handle_var(arg[2]);
            node->code->u.gotop.lable=handle_label(n1);
            insert_code(node);
        }
        else if(strncmp(buf,"RETURN",6)==0){
            sscanf(buf,"RETURN %s",arg[0]);
            InterCodes node=new_intercode(IR_RETURN);
            node->code->u.unaryop.unary=handle_var(arg[0]);
            insert_code(node);
        }
        else if(strncmp(buf,"DEC",3)==0){
            sscanf(buf,"DEC %s %d",arg[0],&n1);
            InterCodes node=new_intercode(IR_DEC);
            node->code->u.assign.left=handle_var(arg[0]);
            node->code->u.assign.right=malloc(sizeof(struct Operand_));
            memset(node->code->u.assign.right,0,sizeof(struct Operand_));
            node->code->u.assign.right->kind=IR_CONSTANT;
            node->code->u.assign.right->u.value=n1;
            insert_code(node);
        }
        else if(strncmp(buf,"ARG",3)==0){
            sscanf(buf,"ARG %s",arg[0]);
            InterCodes node=new_intercode(IR_ARG);
            node->code->u.unaryop.unary=handle_var(arg[0]);
            insert_code(node);
        }
        else if(strncmp(buf,"PARAM",5)==0){
            sscanf(buf,"PARAM %s",arg[0]);
            InterCodes node=new_intercode(IR_PARAM);
            node->code->u.unaryop.unary=handle_var(arg[0]);
            insert_code(node);
        }
        else if(strncmp(buf,"READ",4)==0){
            sscanf(buf,"READ %s",arg[0]);
            InterCodes node=new_intercode(IR_READ);
            node->code->u.unaryop.unary=handle_var(arg[0]);
            insert_code(node);
        }
        else if(strncmp(buf,"WRITE",5)==0){
            sscanf(buf,"WRITE %s",arg[0]);
            InterCodes node=new_intercode(IR_WRITE);
            node->code->u.unaryop.unary=handle_var(arg[0]);
            insert_code(node);
        }
        else{
            //这里是第一个元素是变量的情况
            sscanf(buf,"%s := %s %s %s",arg[0],arg[1],arg[2],arg[3]);
            if(strncmp(arg[1],"CALL",4)==0){
                InterCodes node=new_intercode(IR_CALL);
                node->code->u.assign.left=handle_var(arg[0]);
                node->code->u.assign.right=handle_fun(arg[2]);
                insert_code(node);
            }
            else{
                if(arg[2][0]=='\0'){
                    InterCodes node=new_intercode(IR_ASSIGN);
                    node->code->u.assign.left=handle_var(arg[0]);
                    node->code->u.assign.right=handle_var(arg[1]);
                    insert_code(node);
                }
                else{
                    InterCodes node=new_intercode(IR_ADD);
                    node->code->u.binop.result=handle_var(arg[0]);
                    node->code->u.binop.op1=handle_var(arg[1]);
                    node->code->u.binop.op2=handle_var(arg[3]);
                    switch(arg[2][0]){
                        case '+':node->code->kind=IR_ADD;break;
                        case '-':node->code->kind=IR_SUB;break;
                        case '*':node->code->kind=IR_MUL;break;
                        case '/':node->code->kind=IR_DIV;break;
                        default:assert(0);break;
                    }
                    insert_code(node);
                }
            }
        }
    }
    return NULL;
}
