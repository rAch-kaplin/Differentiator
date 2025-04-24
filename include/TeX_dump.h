#ifndef _TeX_DUMP
#define _TeX_DUMP

#include "tree_func.h"
#include "diff_tree.h"

typedef struct
{
    char *buffer_TeX;
    int cur_len;
} TeX;

void WriteToTexStart(Node *root, const char* filename_tex, TeX *tex);
void WriteToTexEnd  (Node *root, const char* filename_tex, TeX *tex);

#endif // _TeX_DUMP
