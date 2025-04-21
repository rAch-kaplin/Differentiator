#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "lexical_analysis.h"
#include "logger.h"
#include "file_read.h"
#include "colors.h"

const size_t lexeme_array_size = 1000;
bool GetOperation(Lexeme *lexeme_array, size_t lexeme_count, const char **cur);
void SkipSpaces(const char **buffer);

static void AddLexeme(Lexeme *lexeme_array, size_t *lexeme_count, LexemeType type, double value);
static bool IsNum(const char *cur);
static bool IsFunc(const char *name);
static Func GetFuncType(const char* func_name);

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

    size_t lexeme_count = 0;

    while (cur < end)
    {
        SkipSpaces(&cur);
        LOG(LOGL_DEBUG, "curr: %c ---> {%d}, ip = [%d]", *cur, *cur, lexeme_count);

        if (*cur == '(')
        {
            AddLexeme(lexeme_array, &lexeme_count, LEX_LBRACKET, '(');
            cur++;
            continue;
        }
        else if (*cur == ')')
        {
            AddLexeme(lexeme_array, &lexeme_count, LEX_RBRACKET, ')');
            cur++;
            continue;
        }
        else if (*cur == '$')
        {
            AddLexeme(lexeme_array, &lexeme_count, LEX_END, '&');
            cur++;
            continue;
        }
        else if (IsNum(cur)) //TODO wrapper
        {
            lexeme_array[lexeme_count].type = LEX_NUM;
            char *end_num = nullptr;
            lexeme_array[lexeme_count].value.num = strtod(cur, &end_num);
            cur = (const char*)end_num;
            lexeme_count++;
            continue;
        }

        else if (GetOperation(lexeme_array, lexeme_count, &cur))
        {
            lexeme_count++;
            continue;
        }

        else if (isalpha(*cur))
        {
            const char *start = cur;
            while (isalpha(*cur)) cur++;

            size_t name_len = (size_t)(cur - start);

            char *name = (char*)calloc(name_len  + 1, sizeof(char));
            if (name == nullptr)
            {
                LOG(LOGL_ERROR, "Memory was not allocated");
                return nullptr;
            }
            strncpy(name, start, name_len);
                name[name_len] = '\0';

            if (IsFunc(name))
            {
                lexeme_array[lexeme_count].type = LEX_FUNC;
                lexeme_array[lexeme_count].value.func = GetFuncType(name);
                LOG(LOGL_DEBUG, "FUNC: <%s>", name);
                lexeme_count++;
            }

            else
            {
                Variable* vars_table = GetVarsTable();
                size_t var_pos = AddVartable(vars_table, name, name_len);
                LOG(LOGL_DEBUG, "VAR: <%s>", name);
                lexeme_array[lexeme_count].type = LEX_VAR;
                lexeme_array[lexeme_count].value.var = var_pos;
                lexeme_count++;
            }

            free(name);
            continue; //FIXME
        }
    }
    return lexeme_array;
}

static Func GetFuncType(const char* func_name)
{
    assert(func_name != nullptr);

    for (size_t i = 0; i < size_of_func; i++)
    {
        if (strcmp(func_name, func[i].name) == 0)
        {
            return func[i].func;
        }
    } // FIXME unknow function

}

static bool IsFunc(const char *name)
{
    assert(name);

    const char *functions[] = {"sin", "cos", "tan", "ln"};

    for (size_t i = 0; i < sizeof(functions) / sizeof(functions[0]); i++)
    {
        if (strcmp(name, functions[i]) == 0)
        {
            return true;
        }
    }
    return false;
}

static bool IsNum(const char *cur)
{
    assert(cur);
    return (isdigit(*cur) || (*cur == '-' && isdigit(*(cur + 1))));
}

static void AddLexeme(Lexeme *lexeme_array, size_t *lexeme_count, LexemeType type, double value)
{
    assert(lexeme_array && lexeme_count);

    lexeme_array[*lexeme_count].type = type;
    lexeme_array[*lexeme_count].value.num = value;
    (*lexeme_count)++;
}

// bool GetOperation(Lexeme *lexeme_array, size_t lexeme_count, const char **cur)
// {
//     assert(lexeme_array);
//     assert(cur && *cur);
//
//     char ch = **cur;
//     char symbol[2] = {ch, '\0'}; //FIXME
//
//     for (size_t i = 0; i < sizeof(operations) / sizeof(operations[0]); i++)
//     {
//         if (strcmp(symbol, operations[i].symbol) == 0)
//         {
//             lexeme_array[lexeme_count].type = LEX_OP;
//             lexeme_array[lexeme_count].value.op = operations[i].op;
//
//             (*cur)++;
//             return true;
//         }
//     }
//
//     return false;
// }

bool GetOperation(Lexeme *lexeme_array, size_t lexeme_count, const char **cur)
{
    assert(lexeme_array);
    assert(cur && *cur);

    for (size_t i = 0; i < size_of_operations; i++)
    {
        if (**cur == operations[i].symbol[0])  
        {
            lexeme_array[lexeme_count].type = LEX_OP;
            lexeme_array[lexeme_count].value.op = operations[i].op;

            (*cur)++;
            return true;
        }
    }

    return false;
}

void PrintLexemes(const Lexeme *lexeme_array)
{
    assert(lexeme_array);

    printf(BLUB " //***********************Lexemes***********************// " RESET "\n\n");

    for (size_t i = 0; i < 15; i++) //TODO size
    {
        const Lexeme *lex = &lexeme_array[i];

        printf(GREEN "  [%2zu] " RESET, i);

        switch (lex->type)
        {
            case LEX_NUM:
                printf(CYAN "NUM     " RESET ": " BLUE "%lf" RESET "\n", lex->value.num);
                break;

            case LEX_OP:
                for (size_t j = 0; j < size_of_operations; j++)
                {
                    if (operations[j].op == lex->value.op)
                    {
                        printf(MAGENTA "OP      " RESET ": '" MAGENTA "%s" RESET "'\n", operations[j].symbol);
                        break;
                    }
                }
                break;

            case LEX_FUNC:
                for (size_t j = 0; j < size_of_func; j++)
                {
                    if (func[j].func == lex->value.func)
                    {
                        printf(MAGENTA "FUNC    " RESET ": '" MAGENTA "%s" RESET "'\n", func[j].name);
                        break;
                    }
                }
                break;

            case LEX_VAR:
                printf(GREEN "VAR     " RESET ": index " GREEN "%zu" RESET "\n", lex->value.var);
                break;

            case LEX_LBRACKET:
                printf(YELLOW "LBRACK  " RESET ": '" YELLOW "('" RESET "\n");
                break;

            case LEX_RBRACKET:
                printf(YELLOW "RBRACK  " RESET ": '" YELLOW ")'" RESET "\n");
                break;

            case LEX_END:
                printf(RED "END     " RESET "\n");
                break;

            default:
                printf(REDB " UNKNOWN " RESET " type: %d\n", lex->type);
                break;
        }
    }

    printf("\n");
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
