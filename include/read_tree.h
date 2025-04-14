#ifndef _READ_TREE
#define _READ_TREE

#include "diff_tree.h"

size_t GetSizeFile(FILE *name_base);
char *ReadFileToBuffer(const char *name_base, size_t *file_size);
void ParseMathExpr(Node **node, char **buffer, Node *parent);
void FreeTree(Node** node);

#endif //_READ_TREE
