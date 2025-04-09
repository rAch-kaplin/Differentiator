

enum CodeError
{
    OK = 0,
    INVALID_ARGUMENT,
    INVALID_FORMAT,
    MEM_ALLOC_FAIL,
    FILE_NOT_OPEN,
};

enum NodeType
{
    OP,
    NUM,
    VAR
};

typedef struct Node
{
    NodeType type;

    double value;

    struct Node* left;
    struct Node* right;
    struct Node* parent;
} Node;

size_t GetSizeFile(FILE *name_base);
char *ReadFileToBuffer(const char *name_base, size_t *file_size);
void ParseMathExpr(Node **node, char **buffer, Node *parent);
void FreeTree(Node **root);
CodeError TreeDumpDot(Node* root);
CodeError TreeDumpDot2(Node* root);
