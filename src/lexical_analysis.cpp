#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "lexical_analysis.h"
#include "logger.h"
#include "read_tree.h"

const size_t lexeme_array_size = 100;
bool GetOperation(Lexeme *lexeme_array, size_t lexeme_ip, const char **cur);

void SkipSpaces(const char **buffer)
{
    while (isspace(**buffer)) (*buffer)++;
}

Lexeme* StringToLexemes(const char *str)
{
    Lexeme *lexeme_array = (Lexeme*)calloc(lexeme_array_size, sizeof(Lexeme));
    if (lexeme_array == nullptr)
    {
        LOG(LOGL_ERROR, "lexeme_array memory was not allocated");
        return nullptr;
    }

    const char *cur = str;
    const char *end = strchr(str, '\0');

    size_t lexeme_ip = 0;

    while (cur < end)
    {
        SkipSpaces(&cur);
        LOG(LOGL_DEBUG, "curr: %c ---> {%d}, ip = [%d]", *cur, *cur, lexeme_ip);

        if (*cur == '(')
        {
            lexeme_array[lexeme_ip].type = LEX_LBRACKET;
            lexeme_array[lexeme_ip].value.num = '(';
            cur++;
            lexeme_ip++;
            continue;
        }
        else if (*cur == ')')
        {
            lexeme_array[lexeme_ip].type = LEX_RBRACKET;
            lexeme_array[lexeme_ip].value.num = ')';
            cur++;
            lexeme_ip++;
            continue;
        }
        else if (*cur == '$')
        {
            lexeme_array[lexeme_ip].type = LEX_END;
            lexeme_array[lexeme_ip].value.num = '$';
            cur++;
            lexeme_ip++;
            continue;
        }
        else if (isdigit(*cur))
        {
            lexeme_array[lexeme_ip].type = LEX_NUM;
            char *end_num = nullptr;
            lexeme_array[lexeme_ip].value.num = strtod(cur, &end_num);
            cur = (const char*)end_num;
            lexeme_ip++;
            continue;
        }
        else if (isalpha(*cur))
        {
            continue; //FIXME
        }
        else if (GetOperation(lexeme_array, lexeme_ip, &cur))
        {
            lexeme_ip++;
            continue;
        }
    }
    return lexeme_array;
}


bool GetOperation(Lexeme *lexeme_array, size_t lexeme_ip, const char **cur)
{
    assert(lexeme_array);
    assert(cur && *cur);

    char ch = **cur;
    char symbol[2] = {ch, '\0'};

    for (size_t i = 0; i < sizeof(operations) / sizeof(operations[0]); i++)
    {
        if (strcmp(symbol, operations[i].symbol) == 0)
        {
            lexeme_array[lexeme_ip].type = LEX_OP;
            lexeme_array[lexeme_ip].value.op = operations[i].op;

            (*cur)++;
            return true;
        }
    }

    return false;
}

void PrintLexemes(const Lexeme *lexeme_array)
{
    assert(lexeme_array);

    printf("Lexemes:\n");

    for (size_t i = 0; i < 15; i++)
    {
        const Lexeme *lex = &lexeme_array[i];

        switch (lex->type)
        {
            case LEX_NUM:
                printf("  [%zu] NUM: %lf\n", i, lex->value.num);
                break;

            case LEX_OP:
                for (size_t j = 0; j < size_of_opertations; j++)
                {
                    if (operations[j].op == lex->value.op)
                    {
                        printf("  [%zu] OP: '%s'\n", i, operations[j].symbol);
                        break;
                    }
                }
                break;

            case LEX_VAR:
                printf("  [%zu] VAR index: %zu\n", i, (size_t)lex->value.num);
                break;

            case LEX_LBRACKET:
                printf("  [%zu] LBRACKET: '('\n", i);
                break;

            case LEX_RBRACKET:
                printf("  [%zu] RBRACKET: ')'\n", i);
                break;

            case LEX_END:
                printf("  [%zu] END\n", i);
                break;

            default:
                printf("  [%zu] UNKNOWN type: %d\n", i, lex->type);
                break;
        }
    }
}

Lexeme* InitLexemeArray(const char* file_expr)
{
    assert(file_expr);

    size_t file_size = 0;
    char *expr_buffer = ReadFileToBuffer(file_expr, &file_size);
    if (!expr_buffer)
    {
        fprintf(stderr, "Failed to read input file.\n");
        return nullptr;
    }

    Lexeme *lexeme_array = StringToLexemes(expr_buffer);
    free(expr_buffer);

    return lexeme_array;
}

void DeinitLexemes(Lexeme* lexeme_array)
{
    if (!lexeme_array) return;

    free(lexeme_array);
}

