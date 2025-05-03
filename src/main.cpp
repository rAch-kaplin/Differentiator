#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "file_read.h"
#include "tree_func.h"
#include "diff_tree.h"
#include "logger.h"
#include "lexical_analysis.h"
#include "syntaxis_analysis.h"
#include "arg_parser.h"
#include "colors.h"
#include "TeX_dump.h"

const size_t MAX_VARS = 10;

//   (3 * x^2) ^ (1/2) + sin ( ln ( cos (x^2) ) )$
// ch(x + 1) + arctg((x^2 + 1) / x) + sin(x) * ln(x + 2) + 1 / (x^2 + 2)$

void GenerateTeXReport(Node* node_G, const char* file_tex)
{
    TeX tex = {};
    WriteToTexStart(node_G, file_tex, &tex);

    _WRITE_NODE_TEX(tex.buffer_TeX, &(tex.cur_len), "\\section*{Original expression}\n\n");
    WriteExpressionToTeX(node_G, tex.buffer_TeX, &(tex.cur_len));

    Node *diff_node = CopyTree(node_G);
    Node *diff_result = Diff(diff_node, "x");
    FixTree(diff_result);

    _WRITE_NODE_TEX(tex.buffer_TeX, &(tex.cur_len), "\\section*{After differentiation}\n\n");
    WriteExpressionToTeX(diff_result, tex.buffer_TeX, &(tex.cur_len));

    Simplifications(&diff_result);
    _WRITE_NODE_TEX(tex.buffer_TeX, &(tex.cur_len), "\\section*{After simplification}\n\n");
    WriteExpressionToTeX(diff_result, tex.buffer_TeX, &(tex.cur_len));

    size_t num_vars = 0;
    Node** partials = DiffAll(node_G, &num_vars);

    if (partials)
    {
        _WRITE_NODE_TEX(tex.buffer_TeX, &(tex.cur_len), "\\section*{Partial derivatives}\n\n");

        Variable* vars_table = GetVarsTable();
        for (size_t i = 0; i < num_vars; i++)
        {
           _WRITE_NODE_TEX(tex.buffer_TeX, &(tex.cur_len),
                    "\\subsection*{$\\frac{\\partial f}{\\partial %s}$}\n\n", vars_table[i].name);
            Simplifications(&partials[i]);
            WriteExpressionToTeX(partials[i], tex.buffer_TeX, &(tex.cur_len));
            FreeTree(&partials[i]);
        }
        free(partials);
    }

    WriteToTexEnd(node_G, file_tex, &tex);

    FreeTree(&diff_node);
    FreeTree(&diff_result);
}


int main(int argc, const char* argv[]) //TODO not const
{
    printf(GREEN "\nStart main! ============================================================================\n\n" RESET);

    clock_t start = clock();

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

    const char* file_tex = options[OUTPUT].argument;
    if (file_expr == nullptr)
    {
        fprintf(stderr, RED "Can't open file_tex, please input -i *.txt\n" RESET);
        return 1;
    }

    Node *node_G = ReadExpression(file_expr);
    if (node_G == nullptr)
    {
        fprintf(stderr, "ERROR! check log file\n");
    }

    TreeDumpDot2(node_G);

    GenerateTeXReport(node_G, file_tex);


    FreeTree(&node_G);
    FreeVarsTable();

    LoggerDeinit();

    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf(MAGENTA "Execution time: %f seconds\n\n" RESET, time_spent);

    printf(GREEN "End main! ==============================================================================\n\n" RESET);
}
