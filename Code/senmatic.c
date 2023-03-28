#include "semantic.h"

int semantic(Node *root){
    if(root==NULL){
        return 1;
    }
    if(strcmp(root->info_char,"Program")!=0){
        return 1;
    }
    symboltable_init();
    Program_analyse(root);
}

void Program_analyse(Node *root){
    ExtDefList_analyse(root->child);
}

void ExtDefList_analyse(Node *root){
    Node *child1=root->child;
    Node *child2=child1->next;
    ExtDef_analyse(child1);
    if(child2!=NULL){
        ExtDefList_analyse(child2);
    }
}

void ExtDef_analyse(Node *root){

}