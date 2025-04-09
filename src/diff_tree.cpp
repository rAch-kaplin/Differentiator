#include <stdio.h>
#include <assert.h>
#include <cmath>

#include "read_tree.h"
#include "diff_tree.h"

const double EPSILON = 1e-10;

Node* MakeOpNode(Op op, Node* left, Node* right)
{
    NodeValue val = {};
    val.op = operations[op];
    return NewNode(OP, val, left, right);
}

#define _NUM(n) NewNode(NUM, (NodeValue){.num = (n)}, nullptr, nullptr)
#define _VAR(x) NewNode(VAR, (NodeValue){.var = (x)}, nullptr, nullptr)

#define _ADD(a, b) MakeOpNode(ADD, a, b)
#define _MUL(a, b) MakeOpNode(MUL, a, b)

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
        switch (node->value.op.op)
        {
            case ADD: return Eval(node->left) + Eval(node->right);
            case SUB: return Eval(node->left) - Eval(node->right);
            case MUL: return Eval(node->left) * Eval(node->right);
            case DIV:
            {
                double right = Eval(node->right);
                if (fabs(right) < EPSILON)
                {
                    fprintf(stderr, "Error: Division by zero\n");
                    return NAN;
                }
                return Eval(node->left) / right;
            }

            default:
                fprintf(stderr, "Error: Unknown operation\n");
                return NAN;
        }
    }

    fprintf(stderr, "Error: Unknown node type\n");
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
        switch (node->value.op.op)
        {
            case ADD: return _ADD(dL, dR);
            case MUL: return _ADD(_MUL(dL, CR), _MUL(CL, dR));

             default: fprintf(stderr, "ERROR: Unknown operation for differentiator\n");
                     break;
        }
    }

    return nullptr;
}
