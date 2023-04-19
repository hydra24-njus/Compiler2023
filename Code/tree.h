#ifndef __TREE_H__
#define __TREE_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
    /*
*   每个词素作为一个节点
*   每个语法单元作为一个节点
*   
    */

typedef struct node{
    struct node* child;
    struct node* next;
    enum{synunit=1,lexid,lextype,lexint,lexfloat,lexother} node_type;//当前节点类型；
    int lineno;
    char *node_info;
    void *type;
    union 
    {
        int   info_int;
        float info_float;
        char  *info_char;
    };
}Node;


// 创建一个节点，包括
Node *creat_node(unsigned node_type,int lineno,int ii,float if_,char *ic,char *ni);
void build_tree(Node *father,Node *child);
void print_tree(Node *root,int deep);
#endif