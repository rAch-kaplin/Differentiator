#ifndef _HARG_PARSER
#define _HARG_PARSER

enum ParseError
{
    PARSE_OK    = 0,
    PARSE_ERROR = 1,
};

enum ArgIndex
{
    INPUT  = 0,
    OUTPUT = 1,
};

typedef struct
{
    const char* short_name;
    const char* long_name;
    bool requires_arg;
    const char* argument;
    bool is_present;
} ArgOption;

ParseError ParseArguments(int argc, const char* argv[], ArgOption options[], size_t options_count);

#endif //_HARG_PARSER
