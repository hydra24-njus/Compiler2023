#include <stdio.h>
#include "debug.h"
#include "asm.h"
extern InterCodes ir_head,ir_tail;
int _offset=0;
stack shead=NULL;
//对于每个函数 进入时先扫描一遍函数体，创建相应大小的内存空间
//退出时释放整个栈
//注意数组或结构体变量的offset是内存中较小的一端
void push(int kind,int id,int size){
    stack node=malloc(sizeof(struct Stack_));
    memset(node,0,sizeof(struct Stack_));
    node->kind=kind;
    node->id=id;
    _offset+=size;
    node->offset=_offset;
    if(shead==NULL){
        shead=node;
    }
    else{
        node->next=shead;
        shead=node;
    }
}
void pop(){
    while(shead!=NULL){
        stack tmp=shead;
        shead=shead->next;
        free(tmp);
    }
    _offset=0;
}
int find_pos(int kind,int id){
    stack tmp=shead;
    while(tmp!=NULL){
        if(tmp->kind==kind&&tmp->id==id)
            return tmp->offset;
        tmp=tmp->next;
    }
    return -1;
}
void print(){
    stack tmp=shead;
    while(tmp!=NULL){
        switch(tmp->kind){
            case IR_VARIABLE:printf("v%d:%d\n",tmp->id,tmp->offset);break;
            case IR_TMPOP:printf("t%d:%d\n",tmp->id,tmp->offset);break;
        }
        tmp=tmp->next;
    }
}
void check_and_push(Operand op){
    if(op->kind==IR_CONSTANT)return;
    if(op->kind!=IR_VARIABLE&&op->kind!=IR_TMPOP){
        printf("%d\n",op->kind);
        assert(0);
    }
    int kind=op->kind;
    int id=op->u.vid;
    if(find_pos(kind,id)==-1){
        push(kind,id,4);
    }
}
void print_asm(FILE* fp){
    init_regs();
    init_data(fp);
    init_io(fp);
    trans_codes(fp,ir_head);
}

void trans_one_code(FILE *fp,InterCodes cur){
    switch(cur->code->kind){
        case IR_FUNCTION:{
            fprintf(fp,"\n%s:\n",cur->code->u.unaryop.unary->u.funcname);
            fprintf(fp,"  move $fp, $sp\n");
            
            if(cur->next->code->kind==IR_PARAM){
                int param_cnt=0;
                InterCodes tmp=cur->next;
                while(tmp!=NULL&&tmp->code->kind==IR_PARAM){
                    param_cnt++;
                    tmp=tmp->next;
                }
                fprintf(fp,"  addi $fp, %d\n",4*param_cnt);
                fprintf(fp,"  addi $sp, %d\n",4*param_cnt);
                InterCodes tmp2=tmp->prev;
                while(1){
                    if(tmp2==cur)break;
                    Operand x=tmp2->code->u.unaryop.unary;
                    push(x->kind,x->u.vid,4);
                    tmp2=tmp2->prev;
                }
            }

            InterCodes tmp=cur->next;
            while(tmp!=NULL&&tmp->code->kind!=IR_FUNCTION){
                switch(tmp->code->kind){
                    case IR_DEC:{
                        int size=tmp->code->u.assign.right->u.value;
                        Operand op=tmp->code->u.assign.left;
                        if(op->kind!=IR_VARIABLE&&op->kind!=IR_TMPOP)
                            assert(0);
                        int kind=op->kind;
                        int id=op->u.vid;
                        if(find_pos(kind,id)==-1)
                            push(kind,id,size);
                    }
                        break;
                    case IR_RETURN:
                    case IR_READ:
                    case IR_WRITE:
                    case IR_ARG:
                        check_and_push(tmp->code->u.unaryop.unary);
                        break;
                    case IR_CALL:
                        check_and_push(tmp->code->u.assign.left);
                        break;
                    case IR_ASSIGN:
                        check_and_push(tmp->code->u.assign.left);
                        check_and_push(tmp->code->u.assign.right);
                        break;
                    case IR_ADD:
                    case IR_SUB:
                    case IR_MUL:
                    case IR_DIV:
                        check_and_push(tmp->code->u.binop.op1);
                        check_and_push(tmp->code->u.binop.op2);
                        check_and_push(tmp->code->u.binop.result);
                        break;
                    case IR_IFGOTO:
                        check_and_push(tmp->code->u.gotop.op1);
                        check_and_push(tmp->code->u.gotop.op2);
                        break;
                }
                tmp=tmp->next;
            }
            //print();
            _offset+=8;
            fprintf(fp,"  addi $sp, $sp, -%d\n",_offset);
            fprintf(fp,"  sw $fp, 0($sp)\n");
            fprintf(fp,"  sw $ra, 4($sp)\n");
        }
            break;
        case IR_LABEL:{
            fprintf(fp,"label%d:\n",cur->code->u.unaryop.unary->u.lableno);
        }
            break;
        case IR_ASSIGN:{
            if(cur->code->u.assign.right->kind==IR_CONSTANT){
                Operand op1=cur->code->u.assign.left;
                Operand op2=cur->code->u.assign.right;
                int pos=find_pos(op1->kind,op1->u.vid);
                fprintf(fp,"  li $t0, %d\n",op2->u.value);
                fprintf(fp,"  sw $t0, -%d($fp)\n",pos);
            }
            else{
                Operand op1=cur->code->u.assign.left;
                Operand op2=cur->code->u.assign.right;
                int pos1=find_pos(op1->kind,op1->u.vid);
                int pos2=find_pos(op2->kind,op2->u.vid);
                fprintf(fp,"  lw $t0, -%d($fp)\n",pos2);
                fprintf(fp,"  sw $t0, -%d($fp)\n",pos1);
            }
        }
            break;
        case IR_ADD:{
            Operand op1=cur->code->u.binop.op1;//t0
            Operand op2=cur->code->u.binop.op2;//t1
            Operand result=cur->code->u.binop.result;//t2
            if(op1->kind==IR_CONSTANT&&op2->kind==IR_CONSTANT){
                int rpos=find_pos(result->kind,result->u.vid);
                fprintf(fp,"  li $t0, %d\n",op1->u.value+op2->u.value);
                fprintf(fp,"  sw $t0, -%d($fp)\n",rpos);
            }
            else if(op1->kind==IR_CONSTANT){
                int pos=find_pos(op2->kind,op2->u.vid);
                int rpos=find_pos(result->kind,result->u.vid);
                fprintf(fp,"  lw $t1, -%d($fp)\n",pos);
                fprintf(fp,"  addi $t2, $t1, %d\n",op1->u.value);
                fprintf(fp,"  sw $t2, -%d($fp)\n",rpos);
            }
            else if(op2->kind==IR_CONSTANT){
                int pos=find_pos(op1->kind,op1->u.vid);
                int rpos=find_pos(result->kind,result->u.vid);
                fprintf(fp,"  lw $t1, -%d($fp)\n",pos);
                fprintf(fp,"  addi $t2, $t1, %d\n",op2->u.value);
                fprintf(fp,"  sw $t2, -%d($fp)\n",rpos);
            }
            else{
                int pos1=find_pos(op1->kind,op1->u.vid);
                int pos2=find_pos(op2->kind,op2->u.vid);
                int rpos=find_pos(result->kind,result->u.vid);
                fprintf(fp,"  lw $t0, -%d($fp)\n",pos1);
                fprintf(fp,"  lw $t1, -%d($fp)\n",pos2);
                fprintf(fp,"  add $t2, $t0, $t1\n");
                fprintf(fp,"  sw $t2, -%d($fp)\n",rpos);
            }
        }
            break;
        case IR_SUB:{
            Operand op1=cur->code->u.binop.op1;//t0
            Operand op2=cur->code->u.binop.op2;//t1
            Operand result=cur->code->u.binop.result;//t2
            if(op1->kind==IR_CONSTANT&&op2->kind==IR_CONSTANT){
                int rpos=find_pos(result->kind,result->u.vid);
                fprintf(fp,"  li $t0, %d\n",op1->u.value-op2->u.value);
                fprintf(fp,"  sw $t0, -%d($fp)\n",rpos);
            }
            else if(op1->kind==IR_CONSTANT){
                int pos=find_pos(op2->kind,op2->u.vid);
                int rpos=find_pos(result->kind,result->u.vid);
                fprintf(fp,"  li $t0, %d\n",op1->u.value);
                fprintf(fp,"  lw $t1, -%d($fp)\n",pos);
                fprintf(fp,"  sub $t2, $t0, $t1\n");
                fprintf(fp,"  sw $t2, -%d($fp)\n",rpos);
            }
            else if(op2->kind==IR_CONSTANT){
                int pos=find_pos(op1->kind,op1->u.vid);
                int rpos=find_pos(result->kind,result->u.vid);
                fprintf(fp,"  lw $t1, -%d($fp)\n",pos);
                fprintf(fp,"  addi $t2, $t1, -%d\n",op2->u.value);
                fprintf(fp,"  sw $t2, -%d($fp)\n",rpos);
            }
            else{
                int pos1=find_pos(op1->kind,op1->u.vid);
                int pos2=find_pos(op2->kind,op2->u.vid);
                int rpos=find_pos(result->kind,result->u.vid);
                fprintf(fp,"  lw $t0, -%d($fp)\n",pos1);
                fprintf(fp,"  lw $t1, -%d($fp)\n",pos2);
                fprintf(fp,"  sub $t2, $t0, $t1\n");
                fprintf(fp,"  sw $t2, -%d($fp)\n",rpos);
            }
        }
            break;
        case IR_MUL:{
            Operand op1=cur->code->u.binop.op1;
            Operand op2=cur->code->u.binop.op2;
            Operand result=cur->code->u.binop.result;
            if(op1->kind==IR_CONSTANT){
                fprintf(fp,"  li $t0, %d\n",op1->u.value);
            }
            else{
                int pos=find_pos(op1->kind,op1->u.vid);
                fprintf(fp," lw $t0, -%d($fp)\n",pos);
            }
            if(op2->kind==IR_CONSTANT){
                fprintf(fp,"  li $t1, %d\n",op2->u.value);
            }
            else{
                int pos=find_pos(op2->kind,op2->u.vid);
                fprintf(fp," lw $t1, -%d($fp)\n",pos);
            }
            int rpos=find_pos(result->kind,result->u.vid);
            fprintf(fp,"  mul $t2, $t0, $t1\n");
            fprintf(fp,"  sw $t2, -%d($fp)\n",rpos);
        }
            break;
        case IR_DIV:{
            Operand op1=cur->code->u.binop.op1;
            Operand op2=cur->code->u.binop.op2;
            Operand result=cur->code->u.binop.result;
            if(op1->kind==IR_CONSTANT){
                fprintf(fp,"  li $t0, %d\n",op1->u.value);
            }
            else{
                int pos=find_pos(op1->kind,op1->u.vid);
                fprintf(fp," lw $t0, -%d($fp)\n",pos);
            }
            if(op2->kind==IR_CONSTANT){
                fprintf(fp,"  li $t1, %d\n",op2->u.value);
            }
            else{
                int pos=find_pos(op2->kind,op2->u.vid);
                fprintf(fp," lw $t1, -%d($fp)\n",pos);
            }
            int rpos=find_pos(result->kind,result->u.vid);
            fprintf(fp,"  div $t0, $t1\n");
            fprintf(fp,"  mflo $t2\n");
            fprintf(fp,"  sw $t2, -%d($fp)\n",rpos);
        }
            break;
        case IR_GOTO:
            fprintf(fp,"  j label%d\n",cur->code->u.unaryop.unary->u.lableno);
            break;
        case IR_CALL:{
            Operand x=cur->code->u.assign.left;
            int pos=find_pos(x->kind,x->u.vid);
            Operand f=cur->code->u.assign.right;
            fprintf(fp,"  jal %s\n",f->u.funcname);
            fprintf(fp,"  move $sp, $fp\n");
            fprintf(fp,"  lw $fp, 0($sp)\n");
            fprintf(fp,"  lw $ra, 4($sp)\n");
            fprintf(fp,"  sw $v0, -%d($fp)\n",pos);
        }
            break;//TODO
        case IR_RETURN:{
                Operand op=cur->code->u.unaryop.unary;
                if(op->kind==IR_CONSTANT){
                    fprintf(fp,"  li $v0, %d\n",op->u.value);
                }
                else{
                    int pos=find_pos(op->kind,op->u.vid);
                    fprintf(fp,"  lw $v0, -%d($fp)\n",pos);
                }
                fprintf(fp,"  jr $ra\n");
            }
            break;
        case IR_IFGOTO:{
            Operand x=cur->code->u.gotop.op1;//t0
            Operand y=cur->code->u.gotop.op2;//t1
            int z=cur->code->u.gotop.lable->u.lableno;
            char *relop=cur->code->u.gotop.relop->u.relopid;
            if(x->kind==IR_CONSTANT){
                fprintf(fp,"  li $t0, %d\n",x->u.value);
            }
            else{
                int pos=find_pos(x->kind,x->u.vid);
                fprintf(fp," lw $t0, -%d($fp)\n",pos);
            }
            if(y->kind==IR_CONSTANT){
                fprintf(fp,"  li $t1, %d\n",y->u.value);
            }
            else{
                int pos=find_pos(y->kind,y->u.vid);
                fprintf(fp," lw $t1, -%d($fp)\n",pos);
            }
            if(strcmp(relop,"==")==0){
                fprintf(fp,"  beq $t0, $t1, label%d\n",z);
            }
            else if(strcmp(relop,"!=")==0){
                fprintf(fp,"  bne $t0, $t1, label%d\n",z);
            }
            else if(strcmp(relop,">")==0){
                fprintf(fp,"  bgt $t0, $t1, label%d\n",z);
            }
            else if(strcmp(relop,"<")==0){
                fprintf(fp,"  blt $t0, $t1, label%d\n",z);
            }
            else if(strcmp(relop,">=")==0){
                fprintf(fp,"  bge $t0, $t1, label%d\n",z);
            }
            else if(strcmp(relop,"<=")==0){
                fprintf(fp,"  ble $t0, $t1, label%d\n",z);
            }
        }
            break;//TODO
        case IR_READ:{
            Operand x=cur->code->u.unaryop.unary;
            int pos=find_pos(x->kind,x->u.vid);
            fprintf(fp,"  jal read\n");
            fprintf(fp,"  move $sp, $fp\n");
            fprintf(fp,"  lw $fp, 0($sp)\n");
            fprintf(fp,"  lw $ra, 4($sp)\n");
            fprintf(fp,"  sw $v0, -%d($fp)\n",pos);
        }
            break;
        case IR_WRITE:{
            Operand x=cur->code->u.unaryop.unary;
            if(x->kind==IR_CONSTANT){
                fprintf(fp,"  li $a0, %d\n",x->u.value);
            }
            else{
                int pos=find_pos(x->kind,x->u.vid);
                fprintf(fp,"  lw $a0, -%d($fp)\n",pos);
            }
            fprintf(fp,"  jal write\n");
            fprintf(fp,"  move $sp, $fp\n");
            fprintf(fp,"  lw $fp, 0($sp)\n");
            fprintf(fp,"  lw $ra, 4($sp)\n");
        }
            break;
        case IR_ARG:{
            Operand x=cur->code->u.unaryop.unary;
            int pos=find_pos(x->kind,x->u.vid);
            fprintf(fp,"  addi $sp, -4\n");
            fprintf(fp,"  lw $t0, -%d($fp)\n",pos);
            fprintf(fp,"  sw $t0, 0($sp)\n");
        }
            break;
    }
    return;
}

void trans_codes(FILE *fp,InterCodes head){
    InterCodes tmp=ir_head;
	while(tmp!=NULL){
		trans_one_code(fp,tmp);
		tmp=tmp->next;
	}
    return;
}


void init_data(FILE* fp){
    fprintf(fp,".data\n");
    fprintf(fp,"_prompt: .asciiz \"Enter an integer:\"\n");
    fprintf(fp,"_ret: .asciiz \"\\n\"\n");
    fprintf(fp,".globl main\n");
}

void init_io(FILE* fp){
    //read
    fprintf(fp,"\n.text\n");
    fprintf(fp,"\nread:\n");
    fprintf(fp,"move $fp, $sp\n");
    fprintf(fp,"li $v0, 4\n");
    fprintf(fp,"la $a0, _prompt\n");
    fprintf(fp,"syscall\n");
    fprintf(fp,"li $v0, 5\n");
    fprintf(fp,"syscall\n");
    fprintf(fp,"jr $ra\n");
    //write
    fprintf(fp,"\nwrite:\n");
    fprintf(fp,"move $fp, $sp\n");
    fprintf(fp,"li $v0, 1\n");
    fprintf(fp,"syscall\n");
    fprintf(fp,"li $v0, 4\n");
    fprintf(fp,"la $a0, _ret\n");
    fprintf(fp,"syscall\n");
    fprintf(fp,"move $v0, $s0\n");
    fprintf(fp,"jr $ra\n");
}

void init_regs(){
    return;
}