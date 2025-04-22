#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "diff_tree.h"
#include "file_read.h"
#include "logger.h"
#include "lexical_analysis.h"
#include "syntaxis_analysis.h"

const size_t BUFFER_SIZE = 10;

Node* ReadExpression(const char *file_expr)
{
    size_t lexeme_count = 0;
    Lexeme *lexeme_array = InitLexemeArray(file_expr, &lexeme_count);
    if (lexeme_array == nullptr)
    {
        LOG(LOGL_ERROR, "Lexeme_array was not allocated");
        return nullptr;
    }
    PrintLexemes(lexeme_array, lexeme_count);

    size_t cur = 0;
    Node *node_G = GetG(lexeme_array, &cur);

    DeinitLexemes(lexeme_array);

    return node_G;
}


size_t GetSizeFile(FILE *name_base)
{
    assert(name_base);

    if (fseek(name_base, 0, SEEK_END) != 0)
    {
        LOG(LOGL_ERROR, "fseek() Error!");
        return 0;
    }
    size_t file_size = (size_t)ftell(name_base);
    rewind(name_base);

    return file_size;
}

char* ReadFileToBuffer(const char *expr, size_t *file_size)
{
    FILE *file_expr = fopen(expr, "r");
    if (file_expr == nullptr)
    {
        LOG(LOGL_ERROR, "Can't open file: %s", file_expr);
        return nullptr;
    }

    *file_size = GetSizeFile(file_expr);
    char *buffer = (char*)calloc(*file_size + 1, sizeof(char));
    if (buffer == nullptr)
    {
        LOG(LOGL_ERROR, "Memory allocation failed");
        fclose(file_expr);
        return nullptr;
    }

    size_t read = fread(buffer, sizeof(char), *file_size, file_expr);
    if (read != *file_size)
    {
        free(buffer);
        fclose(file_expr);
        return nullptr;
    }

    fclose(file_expr);

    return buffer;
}
