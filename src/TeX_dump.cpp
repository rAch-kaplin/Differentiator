#include <stdio.h>
#include <assert.h>
#include <cmath>
#include <math.h>

#include "logger.h"
#include "diff_tree.h"
#include "tree_func.h"
#include "TeX_dump.h"

static void WriteNode                  (Node *node, char *buffer_TeX, int *cur_len);
static void HandleOperationsToTeX      (Node *node, char *buffer_TeX, int *cur_len);
static void HandleFuncToTeX            (Node *node, char *buffer_TeX, int *cur_len);
static void HandleNumToTeX             (Node *node, char *buffer_TeX, int *cur_len);
static void HandleVarToTeX             (Node *node, char *buffer_TeX, int *cur_len);
static int  GetOpPriority              (Op op);

static void WriteFullDifferential   (Node **partials, size_t num_vars, TeX *tex);
static void WritePartialDerivatives (Node **partials, size_t num_vars, TeX *tex);

const int Global_x = 0;

void GenerateTeXReport(Node* node_G, const char* file_tex)
{
    TeX tex = {};

    WriteToTexStart(node_G, file_tex, &tex);

    _WRITE_NODE_TEX(tex.buffer_TeX, &(tex.cur_len), "\\section*{Original expression}\n\n");
    WriteExpressionToTeX(node_G, tex.buffer_TeX, &(tex.cur_len));

    Node *diff_node = CopyTree(node_G);
    if (diff_node == nullptr)
    {
        LOG(LOGL_ERROR, "diff_node = (nil)"); //FIXME
        return;
    }

    size_t num_vars = 0;
    Node** partials = DiffAll(diff_node, &num_vars);
    if (!partials)
    {
        FreeTree(&diff_node);
        return;
    }

    _WRITE_NODE_TEX(tex.buffer_TeX, &(tex.cur_len), "\\subsection*{The full differential of the expression}\n\n");
    WriteFullDifferential(partials, num_vars, &tex);

    _WRITE_NODE_TEX(tex.buffer_TeX, &(tex.cur_len), "\\section*{Partial derivatives}\n\n");
    WritePartialDerivatives(partials, num_vars, &tex);

    WriteToTexEnd(node_G, file_tex, &tex);

    for (size_t i = 0; i < num_vars; ++i)
        FreeTree(&partials[i]);
    free(partials);
    FreeTree(&diff_node);
}

void WriteFullDifferential(Node **partials, size_t num_vars, TeX *tex)
{
    Variable* vars_table = GetVarsTable();
    _WRITE_NODE_TEX(tex->buffer_TeX, &(tex->cur_len), "\\[\n\\mathrm{d}f = ");

    for (size_t i = 0; i < num_vars; i++)
    {
        TreeDumpDot2(partials[i]);
        Simplifications(&partials[i]);
        TreeDumpDot2(partials[i]);
        _WRITE_NODE_TEX(tex->buffer_TeX, &(tex->cur_len), "\\left[");
        WriteExpressionToTeX2(partials[i], tex->buffer_TeX, &(tex->cur_len));
        _WRITE_NODE_TEX(tex->buffer_TeX, &(tex->cur_len), "\\right]\\mathrm{d}%s", vars_table[i].name);

        if (i + 1 < num_vars)
            _WRITE_NODE_TEX(tex->buffer_TeX, &(tex->cur_len), " + ");
    }

    _WRITE_NODE_TEX(tex->buffer_TeX, &(tex->cur_len), "\\]\n");
}

void WritePartialDerivatives(Node **partials, size_t num_vars, TeX *tex)
{
    Variable* vars_table = GetVarsTable();

    _WRITE_NODE_TEX(tex->buffer_TeX, &(tex->cur_len),
    "\\begin{itemize}\n");

    for (size_t i = 0; i < num_vars; i++)
    {
        Simplifications(&partials[i]);

        _WRITE_NODE_TEX(tex->buffer_TeX, &(tex->cur_len),
            "\\item Partial derivative of по $%s$:\n", vars_table[i].name);

        _WRITE_NODE_TEX(tex->buffer_TeX, &(tex->cur_len),
            "\\[\n\\frac{\\partial f}{\\partial %s} = ", vars_table[i].name);

        WriteExpressionToTeX2(partials[i], tex->buffer_TeX, &(tex->cur_len));

        _WRITE_NODE_TEX(tex->buffer_TeX, &(tex->cur_len), "\n\\]");
    }

    _WRITE_NODE_TEX(tex->buffer_TeX, &(tex->cur_len),
        "\\end{itemize}\n");
}

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

    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\\documentclass[a4paper,12pt]{article}\n");
    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\\usepackage[a4paper,top=1.3cm,bottom=2cm,left=1.5cm,right=1.5cm,marginparwidth=0.75cm]{geometry}\n");
    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\\usepackage{setspace}\n");
    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\\usepackage{cmap}\n");
    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\\usepackage{mathtext}\n");
    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\\usepackage[T2A]{fontenc}\n");
    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\\usepackage[utf8]{inputenc}\n");
    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\\usepackage[english,russian]{babel}\n");
    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\\usepackage{multirow}\n");
    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\\usepackage{graphicx}\n");
    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\\usepackage{wrapfig}\n");
    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\\usepackage{tabularx}\n");
    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\\usepackage{float}\n");
    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\\usepackage{longtable}\n");
    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\\usepackage{hyperref}\n");
    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\\hypersetup{colorlinks=true,urlcolor=blue}\n");
    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\\usepackage[rgb]{xcolor}\n");
    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\\usepackage{amsmath,amsfonts,amssymb,amsthm}\n");
    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\\usepackage{icomma}\n");
    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\\usepackage{euscript}\n");
    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\\usepackage{mathrsfs}\n");
    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\\setlength{\\parindent}{1cm}\n");

    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\n\\title{\\textbf{Math Differentiator}}\n");
    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\\date{}\n");
    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\\author{\\text{Kaplin Artyom, B01-402}}\n\n");

    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\\begin{document}\n");
    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\\maketitle\n\n");

    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "\\section*{Annotation}\n");
    _WRITE_NODE_TEX(buffer_TeX, &(tex->cur_len), "This is auto-generated by Differentiator document. Don't try to change .tex file directly.\n\n");
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

void WriteExpressionToTeX2(Node *root, char *buffer_TeX, int *cur_len)
{
    assert(root);

    WriteNode(root, buffer_TeX, cur_len);
    _WRITE_NODE_TEX(buffer_TeX, cur_len, "\n");
}

static void WriteNode(Node *node, char *buffer_TeX, int *cur_len)
{
    assert(node);
    assert(buffer_TeX);
    assert(cur_len);
    LOG(LOGL_DEBUG, "\nWriteNode: <%p> \n", node);

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

static void HandleOperationsToTeX(Node *node, char *buffer_TeX, int *cur_len)
{
    assert(node);
    assert(buffer_TeX);
    assert(cur_len);

    //FIXME func()
    bool is_in_exponent = (node->parent && node->parent->type == OP && node->parent->value.op == POW);
    bool need_parentheses = (!is_in_exponent && node->parent &&
                           (node->parent->type == OP) &&
                           (GetOpPriority(node->value.op) < GetOpPriority(node->parent->value.op)));

    switch (node->value.op)
    {
        case ADD:
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

        case SUB:
        {
            if (need_parentheses)
            {
                _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\left(");
            }

            WriteNode(node->left, buffer_TeX, cur_len);
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "{-}");

            bool right_is_op = (node->right->type == OP);
            if (right_is_op)
            {
                _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\left(");
            }

            WriteNode(node->right, buffer_TeX, cur_len);

            if (right_is_op)
            {
                _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\right)");
            }

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

            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\frac{");
            WriteNode(node->left, buffer_TeX, cur_len);
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "}{");
            WriteNode(node->right, buffer_TeX, cur_len);
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "}");

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

static int GetOpPriority(Op op)
{
    switch(op)
    {
        case ADD: case SUB: return 1;
        case MUL: case DIV: return 2;
        case POW: return 3;

        default: return 0;
    }
}

static void HandleFuncToTeX(Node *node, char *buffer_TeX, int *cur_len)
{
    assert(node);
    assert(buffer_TeX);
    assert(cur_len);

    switch (node->value.func) //TODO handle FUNC for tex
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
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\ln{\\left(");
            WriteNode(node->right, buffer_TeX, cur_len);
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\right)}");
            break;
        }

        case TG:
        {
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\tan{\\left(");
            WriteNode(node->right, buffer_TeX, cur_len);
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\right)}");
            break;
        }

        case CTG:
        {
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\cot{\\left(");
            WriteNode(node->right, buffer_TeX, cur_len);
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\right)}");
            break;
        }

        case SH:
        {
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\sinh{\\left(");
            WriteNode(node->right, buffer_TeX, cur_len);
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\right)}");
            break;
        }

        case CH:
        {
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\cosh{\\left(");
            WriteNode(node->right, buffer_TeX, cur_len);
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\right)}");
            break;
        }

        case TH:
        {
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\tanh{\\left(");
            WriteNode(node->right, buffer_TeX, cur_len);
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\right)}");
            break;
        }

        case CTH:
        {
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\coth{\\left(");
            WriteNode(node->right, buffer_TeX, cur_len);
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\right)}");
            break;
        }

        case ARCSIN:
        {
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\arcsin{\\left(");
            WriteNode(node->right, buffer_TeX, cur_len);
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\right)}");
            break;
        }

        case ARCCOS:
        {
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\arccos{\\left(");
            WriteNode(node->right, buffer_TeX, cur_len);
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\right)}");
            break;
        }

        case ARCTG:
        {
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\arctan{\\left(");
            WriteNode(node->right, buffer_TeX, cur_len);
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\right)}");
            break;
        }

        case ARCCTG:
        {
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\text{arcctg}{\\left(");
            WriteNode(node->right, buffer_TeX, cur_len);
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\right)}");
            break;
        }

        case ARCSH:
        {
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\text{arcsinh}{\\left(");
            WriteNode(node->right, buffer_TeX, cur_len);
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\right)}");
            break;
        }

        case ARCCH:
        {
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\text{arcch}{\\left(");
            WriteNode(node->right, buffer_TeX, cur_len);
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\right)}");
            break;
        }

        case ARCTH:
        {
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\text{arcth}{\\left(");
            WriteNode(node->right, buffer_TeX, cur_len);
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\right)}");
            break;
        }

        case ARCCTH:
        {
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\text{arccth}{\\left(");
            WriteNode(node->right, buffer_TeX, cur_len);
            _WRITE_NODE_TEX(buffer_TeX, cur_len, "\\right)}");
            break;
        }

        case UNKNOW:
        default:
        {
            LOG(LOGL_ERROR, "UNKNOWN FUNCTION");
            break;
        }
    }
}

static void HandleNumToTeX(Node *node, char *buffer_TeX, int *cur_len)
{
    assert(node);
    assert(buffer_TeX);
    assert(cur_len);

    _WRITE_NODE_TEX(buffer_TeX, cur_len, "{%lg}", node->value.num);
}

static void HandleVarToTeX(Node *node, char *buffer_TeX, int *cur_len)
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


