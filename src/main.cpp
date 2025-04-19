#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "read_tree.h"
#include "diff_tree.h"
#include "logger.h"
#include "lexical_analysis.h"
#include "syntaxis_analysis.h"

int main()
{
    const char* log_file = "logfile.log";
    LoggerInit(LOGL_DEBUG, log_file, DEFAULT_MODE);

    const char* file_expr = "expr.txt";
    const char* file_expr2 = "expr2.txt";

    Lexeme *lexeme_array = InitLexemeArray(file_expr2);
    PrintLexemes(lexeme_array);

    size_t cur = 0;
    Node *node_G = GetG(lexeme_array, &cur);
    TreeDumpDot2(node_G);
    DeinitLexemes(lexeme_array);
    FreeTree(&node_G);
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

    TreeDumpDot2(root);
    printf("%lg\n", Eval(root));

    Node *c_node = CopyTree(root);
    Node *d_node = Diff(c_node);
    TreeDumpDot2(d_node);
    TreeDumpDot(d_node);

    FreeTree(&d_node);
    FreeTree(&c_node);
    FreeTree(&root);

    LoggerDeinit();
    printf("End main!\n");
}

