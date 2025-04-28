#ifndef _HTREE_FUNC
#define _HTREE_FUNC

#include "diff_tree.h"

Node* NewNode(NodeType type, NodeValue value, Node* left, Node* right);
CodeError CreateNode(Node **dest, const char *data, Node *parent);
NodeType DetectNodeType(const char *str);

CodeError TreeDumpDot (Node* root);
CodeError TreeDumpDot2(Node* root);

void FreeTree(Node** node);

#endif // _HTREE_FUNC
