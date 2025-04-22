#ifndef _LEXICAL_ANALYSIS
#define _LEXICAL_ANALYSIS

#include "diff_tree.h"
#include "diff_rules_DSL.h"

typedef enum LexemeType
{
    LEX_NUM,
    LEX_OP,
    LEX_VAR,
    LEX_FUNC,
    LEX_LBRACKET,
    LEX_RBRACKET,
    LEX_END
} LexemeType;

typedef struct Lexeme
{
    LexemeType type;

    union
    {
        double num;
        Op op;
        Func func;
        size_t var;
    } value;
} Lexeme;

Lexeme* StringToLexemes(const char *str);
void PrintLexemes(const Lexeme *lexeme_array, size_t lexeme_count);
Lexeme* InitLexemeArray(const char* file_expr, size_t *lexeme_count);
void DeinitLexemes(Lexeme* lexeme_array);

#endif // _LEXICAL_ANALYSIS
