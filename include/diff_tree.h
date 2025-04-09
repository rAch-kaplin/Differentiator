#ifndef _DIIF_TREE
#define _DIIF_TREE

#include "read_tree.h"

Node* NewNode(NodeType type, NodeValue value, Node* left, Node* right);
double Eval(Node *node);
Node* Diff(Node *node);
Node* CopyTree(Node *root);

#endif //_DIIF_TREE
