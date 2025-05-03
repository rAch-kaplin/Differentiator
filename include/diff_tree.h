#ifndef _DIFF_TREE
#define _DIFF_TREE

#include <stdint.h>

const double Global_x = 2.0;

enum CodeError
{
    OK                  = 0x00,
    INVALID_ARGUMENT    = 0x01,
    INVALID_FORMAT      = 0x02,
    MEM_ALLOC_FAIL      = 0x03,
    FILE_NOT_OPEN       = 0x04,
    INVALID_OPERATION   = 0x05,
    INVALID_NODE_TYPE   = 0x06,
};

/**************************ENUMS**************************************************************************/

enum NodeType
{
    OP,
    NUM,
    VAR,
    FUNC,
};

enum Op
{
    ADD, SUB, MUL, DIV, POW,
};

enum Func
{
    SIN,
    COS,
    LN,
    //EXP,
    TG,
    CTG,
    SH,
    CH,
    TH,
    CTH,
    ARCSIN,
    ARCCOS,
    ARCTG,
    ARCCTG,
    ARCSH,
    ARCCH,
    ARCTH,
    ARCCTH,

    UNKNOW
};

/*************************NODE******************************************************************************/

union NodeValue
{
    double num;
    Op op;
    Func func;
    size_t var;
};

typedef struct Node
{
    NodeType type;

    NodeValue value;

    struct Node* left;
    struct Node* right;
    struct Node* parent;
} Node;

/*************************VARIABLES******************************************************************************/

typedef struct Variable
{
    char *name;
    size_t len_name;
} Variable;

/*************************OPERATIONS******************************************************************************/

typedef struct Operation
{
    Op op;
    const char* symbol;
} Operation;

static const Operation operations[] =
{
    {ADD, "+"},
    {SUB, "-"},
    {MUL, "*"},
    {DIV, "/"},
    {POW, "^"},

};

const size_t size_of_operations = sizeof(operations) / sizeof(operations[0]);

/*************************FUNCS******************************************************************************/

typedef struct Function
{
    Func func;
    const char* name;
} Function;

const Function func[] =
{
    {SIN,    "sin"},
    {COS,    "cos"},
    {LN,     "ln"},
    //{EXP,    "exp"},
    {TG,     "tg"},
    {CTG,    "ctg"},
    {SH,     "sh"},
    {CH,     "ch"},
    {TH,     "th"},
    {CTH,    "cth"},
    {ARCSIN, "arcsin"},
    {ARCCOS, "arccos"},
    {ARCTG,  "arctg"},
    {ARCCTG, "arcctg"},
    {ARCSH,  "arcsh"},
    {ARCCH,  "arcch"},
    {ARCTH,  "arcth"},
    {ARCCTH, "arccth"}
};

const size_t size_of_func = sizeof(func) / sizeof(func[0]);

/***********************************************************************************************************/

double Eval(Node *node);
Node* Diff    (Node *node, const char* diff_var);
Node** DiffAll(Node* expr, size_t* num_vars_out);

Node* CopyTree(Node *root);
bool CheckVars(Node* node);

Variable* GetVarsTable();
size_t LookupVar(Variable *vars_table, const char* name, size_t len_name);
size_t AddVartable(Variable *vars_table, const char* name, size_t len_name);
void FreeVarsTable();

void Simplifications(Node **node);
void FixTree(Node* node);

#endif //_DIFF_TREE
