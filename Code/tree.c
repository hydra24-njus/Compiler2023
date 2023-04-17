#include "tree.h"
Node *creat_node(unsigned node_type,int lineno,int ii,float if_,char *ic,char *ni){
    Node *new_node=malloc(sizeof(Node));
    new_node->child     =      NULL;
    new_node->next      =      NULL;
    new_node->node_type = node_type;
    new_node->lineno    =    lineno;
    char *nstr=NULL;
    char *nstr2=NULL;
    nstr2=malloc((strlen(ni)+1)*sizeof(char));
    strcpy(nstr2,ni);
    new_node->node_info=nstr2;
    if(ic!=NULL){
        nstr=malloc((strlen(ic) + 1) * sizeof(char));
        strcpy(nstr,ic);
    }
    switch (node_type)
    {
    case lexint:
        new_node->info_int=ii;
        break;
    case lexfloat:
        new_node->info_float=if_;
        break;
    case lexid:case lextype:case synunit:case lexother:
        new_node->info_char=nstr;
        break;
    default:
        break;
    }
    
    return new_node;
}
void build_tree(Node *father,Node *child){
    if(father&&child){
        child->next=father->child;
        father->child=child;
    }
    return ;
}
void print_tree(Node *root,int deep){
    if(root==NULL)return;
    if(root->node_type==synunit){
        for(int i=0;i<deep;i++)printf("  ");
        printf("%s (%d)\n", root->info_char, root->lineno);
    }
    else if(root->node_type==lexint){
        for(int i=0;i<deep;i++)printf("  ");
        printf("INT: %d\n", root->info_int);
    }
    else if(root->node_type==lexfloat){
        for(int i=0;i<deep;i++)printf("  ");
        printf("FLOAT: %f\n", root->info_float);
    }
    else if(root->node_type==lexid){
        for(int i=0;i<deep;i++)printf("  ");
        printf("ID: %s\n", root->info_char);
    }
    else if(root->node_type==lextype){
        for(int i=0;i<deep;i++)printf("  ");
        printf("TYPE: %s\n", root->info_char);
    }
    else if(root->node_type==lexother){
        for(int i=0;i<deep;i++)printf("  ");
        printf("%s\n", root->info_char);
    }
    print_tree(root->child,deep+1);
    print_tree(root->next,deep);
    return ;
}