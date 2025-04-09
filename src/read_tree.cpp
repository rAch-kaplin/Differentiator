#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "read_tree.h"

const size_t BUFFER_SIZE = 10;

Node* NewNode(NodeType type, double content, Node* left, Node* right);
NodeType DetectNodeType(const char *str);
CodeError CreateNode(Node **dest, const char *data, Node *parent);

Node* NewNode(NodeType type, double content, Node* left, Node* right)
{
    Node* node = (Node*)calloc(1, sizeof(Node));
    if (!node) return nullptr;

    node->type = type;
    node->value = content;

    node->left = left;
    node->right = right;

    if (left)  left->parent  = node;
    if (right) right->parent = node;

    return node;
}

CodeError CreateNode(Node **dest, const char *data, Node *parent)
{
    if (dest == nullptr || data == nullptr)
        return INVALID_ARGUMENT;

    NodeType type = DetectNodeType(data);
    int content = 0;

    switch (type)
    {
        case OP:
        case VAR:
            content = (int)data[0];
            break;
        case NUM:
            content = atoi(data);
            break;
        default:
            break;
    }

    Node* node = NewNode(type, content, nullptr, nullptr);
    if (!node)
        return MEM_ALLOC_FAIL;

    node->parent = parent;
    *dest = node;
    return OK;
}

void FreeTree(Node **root)
{
    if (root == nullptr || *root == nullptr)
        return;

    FreeTree(&((*root)->left));
    FreeTree(&((*root)->right));

    free(*root);
    *root = nullptr;
}

size_t GetSizeFile(FILE *name_base)
{
    fseek(name_base, 0, SEEK_END);
    size_t file_size = (size_t)ftell(name_base);
    rewind(name_base);

    return file_size;
}

char *ReadFileToBuffer(const char *name_base, size_t *file_size)
{
    FILE *base_file = fopen(name_base, "r");
    if (base_file == nullptr)
    {
        //LOG(LOGL_ERROR, "Can't open file: %s", name_base);
        return nullptr;
    }

    *file_size = GetSizeFile(base_file);
    char *buffer = (char*)calloc(*file_size + 1, sizeof(char));
    if (buffer == nullptr)
    {
        //LOG(LOGL_ERROR, "Memory allocation failed");
        fclose(base_file);
        return nullptr;
    }

    size_t read = fread(buffer, sizeof(char), *file_size, base_file);
    if (read != *file_size)
    {
        free(buffer);
        fclose(base_file);
        return nullptr;
    }

    fclose(base_file);

    return buffer;
}

NodeType DetectNodeType(const char *str)
{
    if (str == nullptr) return VAR;

    if (strlen(str) == 1)
    {
        char ch = str[0];

        if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '^')
            return OP;

        if (isalpha(ch))
            return VAR;
    }

    int is_number = 1;
    for (size_t i = 0; i < strlen(str); i++)
    {
        if (!isdigit(str[i]) && !(i == 0 && str[i] == '-'))
        {
            is_number = 0;
            break;
        }
    }

    return is_number ? NUM : VAR;
}

void ParseMathExpr(Node **node, char **buffer, Node *parent)
{
    assert(node != nullptr);
    assert(buffer != nullptr);
    if (*buffer == nullptr) return;

    while (isspace(**buffer)) (*buffer)++;

    if (**buffer != '(')
    {
        fprintf(stderr, "Expected '(' at the beginning\n");
        return;
    }

    (*buffer)++;
    while (isspace(**buffer)) (*buffer)++;

    char value_buf[BUFFER_SIZE] = "";
    int offset = 0;
    if (sscanf(*buffer, "%[^ )]%n", value_buf, &offset) != 1)
    {
        fprintf(stderr, "Invalid content inside parentheses\n");
        return;
    }

    *buffer += offset;
    while (isspace(**buffer)) (*buffer)++;

    NodeType type = DetectNodeType(value_buf);
    CodeError err = CreateNode(node, value_buf, parent);
    if (err != OK) return;

    if (type == NUM || type == VAR)
    {
        if (**buffer != ')')
        {
            fprintf(stderr, "Expected ')' after leaf\n");
            return;
        }

        (*buffer)++;
        return;
    }

    ParseMathExpr(&((*node)->left), buffer, *node);
    ParseMathExpr(&((*node)->right), buffer, *node);

    while (isspace(**buffer)) (*buffer)++;

    if (**buffer != ')')
    {
        fprintf(stderr, "Expected ')' after operator subtree\n");
        FreeTree(node);
        return;
    }

    (*buffer)++;
}


