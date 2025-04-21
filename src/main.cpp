#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "read_tree.h"
#include "diff_tree.h"
#include "logger.h"
#include "lexical_analysis.h"
#include "syntaxis_analysis.h"
#include "arg_parser.h"
#include "colors.h"

int main(int argc, const char* argv[]) //TODO not const
{
    const char* log_file = "logfile.log";
    LoggerInit(LOGL_DEBUG, log_file, DEFAULT_MODE);

    ArgOption options[] = { {"-i",  "--input", true, nullptr, false},
                            {"-o", "--output", true, nullptr, false} };

    if (ParseArguments(argc, argv, options, sizeof(options) / sizeof(options[0])) != PARSE_OK)
    {
        fprintf(stderr, "ParseArg error\n");
        return 1;
    }

    const char* file_expr = options[INPUT].argument;
    if (file_expr == nullptr)
    {
        fprintf(stderr, RED "Can't open file_expr, please input -i *.txt\n" RESET);
        return 1;
    }

    const char *file_expr2 = "expr2.txt";

    Node *node_G = ReadExpression(file_expr2);
    TreeDumpDot2(node_G);
    //printf("%lg\n", Eval(node_G));
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

    FreeTree(&root);

    LoggerDeinit();
    printf("End main!\n");
}

