#ifndef _DIIF_TREE
#define _DIIF_TREE

#include "read_tree.h"

const double Global_x = 2.0;

enum CodeError //TODO 0x...
{
    OK = 0,
    INVALID_ARGUMENT,
    INVALID_FORMAT,
    MEM_ALLOC_FAIL,
    FILE_NOT_OPEN,
    INVALID_OPERATION,
    INVALID_NODE_TYPE,
};

enum NodeType
{
    OP,
    NUM,
    VAR,
};

enum Op
{
    ADD, SUB, MUL, DIV, //POW, SIN, COS, TG, CTG, LN, LOG, SQRT, SH, CH, TH, CTH,
};

typedef struct Operation
{
    Op op;
    const char* symbol;
} Operation;

union NodeValue
{
    double num;
    Op op;
    int var;
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
    {DIV, "/"}
};

Node* NewNode(NodeType type, NodeValue value, Node* left, Node* right);
double Eval(Node *node);
Node* Diff(Node *node);
Node* CopyTree(Node *root);

CodeError TreeDumpDot(Node* root);
CodeError TreeDumpDot2(Node* root);

#endif //_DIIF_TREE
