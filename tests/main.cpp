#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "read_tree.h"
#include "diff_tree.h"
#include "logger.h"

int EvalTests(char* argv[]);

int main(int argc, char* argv[])
{
    if (argc == 4 && strcmp(argv[1], "eval") == 0)
    {
        EvalTests(argv);
    }
}

int EvalTests(char* argv[])
{
    const char* input_path = argv[2];
    const char* output_path = argv[3];

        size_t file_size = 0;
        char *buffer = ReadFileToBuffer(input_path, &file_size);
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
            fprintf(stderr, "Failed to read tree from %s\n", input_path);
            return 1;
        }

        double result = Eval(root);

        FILE* fout = fopen(output_path, "w");
        if (!fout)
        {
            fprintf(stderr, "Can't open output file %s\n", output_path);
            return 2;
        }

        fprintf(fout, "%.10lf\n", result);
        fclose(fout);
        FreeTree(&root);
        return 0;
}
