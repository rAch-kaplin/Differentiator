#include <stdio.h>
#include <assert.h>

#include "logger.h"
#include "diff_tree.h"
#include "tree_func.h"
#include "TeX_dump.h"

const size_t TeX_buffer_size = 4048;

#define _WRITE_NODE_TEX(buffer_TeX, cur_len, ...)                                         \
    do {                                                                                  \
        if ((size_t)*(cur_len) < TeX_buffer_size) {                                       \
            int _written = snprintf((buffer_TeX) + *(cur_len),                            \
                                    (TeX_buffer_size) - (size_t)*(cur_len), __VA_ARGS__); \
            if (_written > 0) *(cur_len) += _written;                                     \
        }                                                                                 \
    } while (0)

void WriteExpressionToTeX(Node *root, char *buffer_TeX, int *cur_len);

static void WriteNode                  (Node *node, char *buffer_TeX, int *cur_len);
static void HandleOperationsToTeX      (Node *node, char *buffer_TeX, int *cur_len);
static void HandleFuncToTeX            (Node *node, char *buffer_TeX, int *cur_len);
static void HandleNumToTeX             (Node *node, char *buffer_TeX, int *cur_len);
static void HandleVarToTeX             (Node *node, char *buffer_TeX, int *cur_len);
static int  GetOpPriority              (Op op);

void WriteToTexStart(Node *root, const char* filename_tex, TeX *tex)
{
    assert(root);
    assert(filename_tex);

    char *buffer_TeX = (char*)calloc(TeX_buffer_size, sizeof(char));
    if (buffer_TeX == nullptr)
    {
        LOG(LOGL_ERROR, "Memory was not allocated");
        return;
    }
    tex->buffer_TeX = buffer_TeX;

    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\\documentclass{article}\n");
    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\\begin{document}\n");
    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\n");

    WriteExpressionToTeX(root, buffer_TeX, &(tex->cur_len));

}

void WriteToTexEnd(Node *root, const char* filename_tex, TeX *tex)
{
    assert(root);
    assert(filename_tex);

    _WRITE_NODE_TEX(tex->buffer_TeX, &(tex->cur_len), "\n\n");
    _WRITE_NODE_TEX(tex->buffer_TeX, &(tex->cur_len), "\\end{document}\n");

    FILE* tex_file = fopen(filename_tex, "w");
    if (tex_file == nullptr)
    {
        LOG(LOGL_ERROR, "Failed to open file %s for writing", filename_tex);
        free(tex->buffer_TeX);
        return;
    }

    size_t written = fwrite(tex->buffer_TeX, sizeof(char), (size_t)(tex->cur_len), tex_file);
    if (written != (size_t)(tex->cur_len))
    {
        LOG(LOGL_ERROR, "Failed to write full buffer to file");
    }

    fclose(tex_file);
    free(tex->buffer_TeX);

    LOG(LOGL_INFO, "Successfully wrote TeX expression to %s", filename_tex);
}

void WriteExpressionToTeX(Node *root, char *buffer_TeX, int *cur_len)
{
    assert(root);

    _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\[");
    WriteNode(root, buffer_TeX, cur_len);
    _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\]");
    _WRITE_NODE_TEX(buffer_TeX, cur_len, "\n\n");

}

void WriteNode(Node *node, char *buffer_TeX, int *cur_len)
{
    assert(node);
    assert(buffer_TeX);
    assert(cur_len);

    switch (node->type)
    {
        case OP:
            HandleOperationsToTeX(node, buffer_TeX, cur_len);
            break;

        case FUNC:
            HandleFuncToTeX(node, buffer_TeX, cur_len);
            break;

        case NUM:
            HandleNumToTeX(node, buffer_TeX, cur_len);
            break;

        case VAR:
            HandleVarToTeX(node, buffer_TeX, cur_len);
            break;

        default:
            LOG(LOGL_ERROR, "UNKNOW TYPE");
            break;
    }
}

void HandleOperationsToTeX(Node *node, char *buffer_TeX, int *cur_len)
{
    assert(node);
    assert(buffer_TeX);
    assert(cur_len);

    bool is_in_exponent = (node->parent && node->parent->type == OP && node->parent->value.op == POW);
    bool need_parentheses = (!is_in_exponent && node->parent &&
                           (node->parent->type == OP) &&
                           (GetOpPriority(node->value.op) < GetOpPriority(node->parent->value.op)));

    switch (node->value.op)
    {
        case ADD:
        case SUB:
        {
            if (need_parentheses)
            {
                _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\left(");
            }

            WriteNode(node->left, buffer_TeX, cur_len);
            _WRITE_NODE_TEX(buffer_TeX, cur_len, node->value.op == ADD ? "{+}" : "{-}");
            WriteNode(node->right, buffer_TeX, cur_len);

            if (need_parentheses)
            {
                _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\right)");
            }
            break;
        }

        case MUL:
        {
            if (need_parentheses)
            {
                _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\left(");
            }

            WriteNode(node->left, buffer_TeX, cur_len);
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "{\\cdot}");
            WriteNode(node->right, buffer_TeX, cur_len);

            if (need_parentheses)
            {
                _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\right)");
            }
            break;
        }

        case DIV:
        {
            if (need_parentheses)
            {
                _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\left(");
            }

            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\frac{");
            WriteNode(node->left, buffer_TeX, cur_len);
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "}{");
            WriteNode(node->right, buffer_TeX, cur_len);
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "}");

            if (need_parentheses)
            {
                _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\right)");
            }
            break;
        }

        case POW:
        {
            bool left_need_paren = (node->left->type == OP) &&
                                  (GetOpPriority(node->left->value.op) < GetOpPriority(POW));

            if (left_need_paren)
            {
                _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\left(");
            }
            WriteNode(node->left, buffer_TeX, cur_len);
            if (left_need_paren)
            {
                _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\right)");
            }

            _WRITE_NODE_TEX(buffer_TeX, cur_len, "^{");
            WriteNode(node->right, buffer_TeX, cur_len);
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "}");

            break;
        }

        default:
        {
            LOG(LOGL_ERROR, "UNKNOW OPERATION");
            break;
        }
    }
}

int GetOpPriority(Op op)
{
    switch(op)
    {
        case ADD: case SUB: return 1;
        case MUL: case DIV: return 2;
        case POW: return 3;

        default: return 0;
    }
}

void HandleFuncToTeX(Node *node, char *buffer_TeX, int *cur_len)
{
    assert(node);
    assert(buffer_TeX);
    assert(cur_len);

    switch (node->value.func)
    {
        case SIN:
        {
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\sin{\\left(");
            WriteNode(node->right, buffer_TeX, cur_len);
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\right)}");
            break;
        }

        case COS:
        {
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\cos{\\left(");
            WriteNode(node->right, buffer_TeX, cur_len);
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\right)}");
            break;
        }

        case LN:
        {
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\ln{");
            WriteNode(node->right, buffer_TeX, cur_len);
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "}");
            break;
        }

        default:
        {
            LOG(LOGL_ERROR, "UNKNOW FUNCTION");
            break;
        }
    }
}

void HandleNumToTeX(Node *node, char *buffer_TeX, int *cur_len)
{
    assert(node);
    assert(buffer_TeX);
    assert(cur_len);

    _WRITE_NODE_TEX(buffer_TeX, cur_len, "{%lg}", node->value.num);
}

void HandleVarToTeX(Node *node, char *buffer_TeX, int *cur_len)
{
    assert(node);
    assert(buffer_TeX);
    assert(cur_len);

    Variable* vars_table = GetVarsTable();
    size_t var_id = node->value.var;

    if (vars_table[var_id].name != nullptr)
    {
        _WRITE_NODE_TEX(buffer_TeX, cur_len, "{%.*s}",
                       (int)vars_table[var_id].len_name,
                       vars_table[var_id].name);
    }

    else
    {
        LOG(LOGL_ERROR, "VAR was not find");
    }
}
