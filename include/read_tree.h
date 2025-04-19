#ifndef _READ_TREE
#define _READ_TREE

#include "diff_tree.h"

size_t GetSizeFile(FILE *name_base);
char *ReadFileToBuffer(const char *name_base, size_t *file_size);
void ParseMathExpr(Node **node, char **buffer, Node *parent);
void FreeTree(Node** node);
CodeError CreateNode(Node **dest, const char *data, Node *parent);
NodeType DetectNodeType(const char *str);
void SkipSpaces(char **buffer);
Node* ReadExpression(const char *file_expr);

#endif //_READ_TREE
