#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "read_tree.h"

int main()
{
    const char* file_expr = "expr.txt";

    size_t file_size = 0;
    char *buffer = ReadFileToBuffer(file_expr, &file_size);
    if (!buffer)
    {
        fprintf(stderr, "Failed to read input file.\n");
        return 1;
    }

    char *ptr = buffer;
    Node *root = NULL;

    ParseMathExpr(&root, &ptr, NULL);
    free(buffer);

    if (!root)
    {
        fprintf(stderr, "Failed to parse expression.\n");
        return 1;
    }

    printf("Expression parsed successfully.\n");

    //TreeDumpDot(root);
    TreeDumpDot2(root);

    FreeTree(&root);

    printf("End main!\n");
}

