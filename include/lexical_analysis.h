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

typedef struct Function
{
    Func func;
    const char* name;
} Function;

static const Function func[] =
{
    {SIN, "sin"},
    {COS, "cos"},
    {LOG,  "ln"}
};

const size_t size_of_func = sizeof(func) / sizeof(func[0]);

typedef struct Lexeme
{
    LexemeType type;

    union
    {
        double num;
        Op op;
        Func func;
    } value;
} Lexeme;

Lexeme* StringToLexemes(const char *str);
void PrintLexemes(const Lexeme *lexeme_array);
Lexeme* InitLexemeArray(const char* file_expr);
void DeinitLexemes(Lexeme* lexeme_array);

#endif // _LEXICAL_ANALYSIS
