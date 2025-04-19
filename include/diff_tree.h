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
    SIN, COS, LOG,
};

typedef struct Operation
{
    Op op;
    const char* symbol;
} Operation;

typedef struct Variable
{
    const char *name;
    size_t len_name;
} Variable;

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

static const Operation operations[] =
{
    {ADD, "+"},
    {SUB, "-"},
    {MUL, "*"},
    {DIV, "/"},
    {POW, "^"},

};

const size_t size_of_opertations = sizeof(operations) / sizeof(operations[0]);

Node* NewNode(NodeType type, NodeValue value, Node* left, Node* right);
double Eval(Node *node);
Node* Diff(Node *node);
Node* CopyTree(Node *root);

CodeError TreeDumpDot(Node* root);
CodeError TreeDumpDot2(Node* root);

bool CheckVars(Node* node);

Variable* GetVarsTable();
size_t LookupVar(Variable *vars_table, const char* name, size_t len_name);
size_t AddVartable(Variable *vars_table, const char* name, size_t len_name);

#endif //_DIFF_TREE
