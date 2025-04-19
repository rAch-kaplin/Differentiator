#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "diff_tree.h"
#include "read_tree.h"
#include "logger.h"
#include "lexical_analysis.h"
#include "syntaxis_analysis.h"

const size_t BUFFER_SIZE = 10;

Node* ReadExpression(const char *file_expr)
{
    Lexeme *lexeme_array = InitLexemeArray(file_expr);
    if (lexeme_array == nullptr)
    {
        LOG(LOGL_ERROR, "Lexeme_array was not allocated");
        return nullptr;
    }
    PrintLexemes(lexeme_array);

    size_t cur = 0;
    Node *node_G = GetG(lexeme_array, &cur);
    DeinitLexemes(lexeme_array);

    return node_G;
}

Node* NewNode(NodeType type, NodeValue value, Node* left, Node* right)
{
    Node* node = (Node*)calloc(1, sizeof(Node));
    if (!node) return nullptr;

    node->type = type;
    node->value = value;

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

    Node* node = (Node*)calloc(1, sizeof(Node));
    if (!node)
        return MEM_ALLOC_FAIL;

    NodeType type = DetectNodeType(data);
    NodeValue value = {};

    switch (type)
    {
        case OP:
        {
            bool found = false;
            for (size_t i = 0; i < sizeof(operations) / sizeof(operations[0]); i++)
            {
                if (strcmp(data, operations[i].symbol) == 0)
                {
                    value.op = operations[i].op;
                    found = true;
                    break;
                }
            }
            if (!found) return INVALID_OPERATION;
            break;
        }

        case VAR:
        {
            value.var = AddVartable(GetVarsTable(), data, strlen(data));
            break; //FIXME errors overflow
        }

        case NUM:
            value.num = atof(data);
            break;

        case FUNC:
            break; //TODO

        default:
            return INVALID_NODE_TYPE;
    }

    node->type = type;
    node->value = value;
    node->left = nullptr;
    node->right = nullptr;
    node->parent = parent;

    *dest = node;

    return OK;
}

void FreeTree(Node** node)
{
    if (! node || ! *node) return;

    FreeTree(&(*node)->left);
    FreeTree(&(*node)->right);
    free(*node);
    *node = nullptr;
}

// void Dtor(Node **node, Variable *var)
// {
//     FreeTree(node);
//     free(var->name);
// }

size_t GetSizeFile(FILE *name_base)
{
    assert(name_base);

    if (fseek(name_base, 0, SEEK_END) != 0)
    {
        LOG(LOGL_ERROR, "fseek() Error!");
        return 0;
    }
    size_t file_size = (size_t)ftell(name_base);
    rewind(name_base);

    return file_size;
}

char* ReadFileToBuffer(const char *expr, size_t *file_size)
{
    FILE *file_expr = fopen(expr, "r");
    if (file_expr == nullptr)
    {
        LOG(LOGL_ERROR, "Can't open file: %s", file_expr);
        return nullptr;
    }

    *file_size = GetSizeFile(file_expr);
    char *buffer = (char*)calloc(*file_size + 1, sizeof(char));
    if (buffer == nullptr)
    {
        LOG(LOGL_ERROR, "Memory allocation failed");
        fclose(file_expr);
        return nullptr;
    }

    size_t read = fread(buffer, sizeof(char), *file_size, file_expr);
    if (read != *file_size)
    {
        free(buffer);
        fclose(file_expr);
        return nullptr;
    }

    fclose(file_expr);

    return buffer;
}

NodeType DetectNodeType(const char *str)
{
    if (strlen(str) == 1)
    {
        char ch = str[0];

        for (size_t i = 0; i < size_of_opertations; i++)
        {
            if (strcmp(str, operations[i].symbol) == 0)
                return OP;
        }

        if (isalpha(ch))
            return VAR;
    }

    bool is_number = true;
    for (size_t i = 0; i < strlen(str); i++)
    {
        if (!isdigit(str[i]) && !(i == 0 && str[i] == '-'))
        {
            is_number = false;
            break; //FIXME
        }
    }

    return is_number ? NUM : VAR;
}

void SkipSpaces(char **buffer)
{
    while (isspace(**buffer)) (*buffer)++;
}

void ParseMathExpr(Node **node, char **buffer, Node *parent)
{
    assert(node != nullptr);
    assert(buffer != nullptr);
    if (*buffer == nullptr) return;

    SkipSpaces(buffer);

    if (**buffer != '(')
    {
        LOG(LOGL_ERROR, "Expected '(' at the beginning");
        return;
    }

    (*buffer)++;
    SkipSpaces(buffer);

    char value_buf[BUFFER_SIZE] = "";
    LOG(LOGL_DEBUG, "value_buf p=%p", value_buf);
    int offset = 0;
    if (sscanf(*buffer, "%[^ )]%n", value_buf, &offset) != 1)
    {
        LOG(LOGL_ERROR, "Invalid content inside parentheses");
        return;
    }

    *buffer += offset;
    SkipSpaces(buffer);

    NodeType type = DetectNodeType(value_buf);
    CodeError err = CreateNode(node, value_buf, parent);
    if (err != OK) return;

    if (type == NUM || type == VAR)
    {
        if (**buffer != ')')
        {
            LOG(LOGL_ERROR, "Expected ')' after leaf");
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
        LOG(LOGL_ERROR, "Expected ')' after operator subtree");
        FreeTree(node);
        return;
    }

    (*buffer)++;
}


