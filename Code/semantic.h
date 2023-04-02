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
void StructSpecifier_analyse(Node *node);
void OptTag_analyse(Node *node);
void Tag_analyse(Node *node);
void VarDec_analyse(Node *node);
void FunDec_analyse(Node *node,Type type);
void VarList_analyse(Node *node);
void ParamDec_analyse(Node *node);
void CompSt_analyse(Node *node);
void StmtList_analyse(Node *node);
void Stmt_analyse(Node *node);
void DefList_analyse(Node *node);
void Def_analyse(Node *node);
void DecList_analyse(Node *node);
void Dec_analyse(Node *node);
void Exp_analyse(Node *node);
void Args_analyse(Node *node);
#endif