#include <stdio.h>
#include <assert.h>
#include <cmath>

#include "read_tree.h"
#include "diff_tree.h"
#include "logger.h"

const double EPSILON = 1e-10;

#define _NUM(n) NewNode(NUM, (NodeValue){.num = (n)}, nullptr, nullptr)

#define OP_VALUE(oper) NodeValue {.op = (oper)}

#define _ADD(a,b) NewNode(OP, OP_VALUE(ADD), a, b)
#define _MUL(a,b) NewNode(OP, OP_VALUE(MUL), a, b)

#define dL  Diff(node->left)
#define dR  Diff(node->right)
#define CL  CopyTree(node->left)
#define CR  CopyTree(node->right)


double Eval(Node *node)
{
    assert(node);

    if (node->type == NUM) return node->value.num;
    if (node->type == VAR) return Global_x;
    if (node->type == OP)
    {
        switch (node->value.op)
        {
            case ADD: return Eval(node->left) + Eval(node->right);
            case SUB: return Eval(node->left) - Eval(node->right);
            case MUL: return Eval(node->left) * Eval(node->right);
            case DIV:
            {
                double right = Eval(node->right);
                if (fabs(right) < EPSILON)
                {
                    LOG(LOGL_ERROR, "Error: Division by zero");
                    return NAN;
                }
                return Eval(node->left) / right;
            }

            default:
                LOG(LOGL_ERROR,"Error: Unknown operation");
                return NAN;
        }
    }

    LOG(LOGL_ERROR, "Error: Unknown node type");
    return NAN;
}

Node* CopyTree(Node *root)
{
    assert(root);

    Node* node = NewNode(root->type, root->value, nullptr, nullptr);

    if (root->left)   node->left  = CopyTree(root->left);
    if (root->right)  node->right = CopyTree(root->right);

    return node;
}

Node* Diff(Node *node)
{
    assert(node);

    if (node->type == NUM) return _NUM(0);
    if (node->type == VAR) return _NUM(1);
    if (node->type == OP)
    {
        switch (node->value.op)
        {
            case ADD: return _ADD(dL, dR);
            case MUL: return _ADD(_MUL(dL, CR), _MUL(CL, dR));

             default: LOG(LOGL_ERROR, "ERROR: Unknown operation for differentiator");
                     break;
        }
    }

    return nullptr;
}
