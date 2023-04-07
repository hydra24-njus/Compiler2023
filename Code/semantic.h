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
void FunDec_analyse(Node *node,Type type,int def,ScopeList scope);
FieldList VarList_analyse(Node *node,FieldList head,ScopeList scope);
FieldList ParamDec_analyse(Node *node);
void CompSt_analyse(Node *node,Type type,ScopeList scope);
void StmtList_analyse(Node *node,Type type,ScopeList scope);
void Stmt_analyse(Node *node,Type type,ScopeList scope);
void DefList_analyse(Node *node,ScopeList scope);
void Def_analyse(Node *node,ScopeList scope);
void DecList_analyse(Node *node,Type type,ScopeList scope);
void Dec_analyse(Node *node,Type type,ScopeList scope);
Type Exp_analyse(Node *node);
int  Args_analyse(Node *node,FieldList field);

FieldList DefList_struct_analyse(Node *node);
FieldList Def_struct_analyse(Node *node);
FieldList DecList_struct_analyse(Node *node,Type type);
FieldList Dec_struct_analyse(Node *node,Type type);
#endif