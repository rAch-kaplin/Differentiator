#ifndef _READ_TREE
#define _READ_TREE

const double Global_x = 2.0;

enum CodeError
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
    ADD, SUB, MUL, DIV,
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

size_t GetSizeFile(FILE *name_base);
char *ReadFileToBuffer(const char *name_base, size_t *file_size);
void ParseMathExpr(Node **node, char **buffer, Node *parent);
void FreeTree(Node *root);
CodeError TreeDumpDot(Node* root);
CodeError TreeDumpDot2(Node* root);

#endif //_READ_TREE
