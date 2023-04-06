#ifndef __SEMANTIC_H__
#define __SEMANTIC_H__

#include "symbol_table.h"
#include "tree.h"


int semantic(Node *root);
void Program_analyse(Node *root);
void ExtDefList_analyse(Node *root);
void ExtDef_analyse(Node *root);
void ExtDecList_analyse(Node *node,Type type);
Type Specifier_analyse(Node *node);
Type StructSpecifier_analyse(Node *node);
FieldList VarDec_analyse(Node *node,Type type);
void FunDec_analyse(Node *node,Type type,int def);
FieldList VarList_analyse(Node *node,FieldList head);
FieldList ParamDec_analyse(Node *node);
void CompSt_analyse(Node *node);
void StmtList_analyse(Node *node);
void Stmt_analyse(Node *node);
void DefList_analyse(Node *node);
void Def_analyse(Node *node);
void DecList_analyse(Node *node,Type type);
void Dec_analyse(Node *node,Type type);
Type Exp_analyse(Node *node);
int  Args_analyse(Node *node,FieldList field);

FieldList DefList_struct_analyse(Node *node);

#endif