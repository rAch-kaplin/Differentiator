#include <stdio.h>
#include <assert.h>
#include <cmath>

#include "read_tree.h"
#include "diff_tree.h"

const double EPSILON = 1e-10;

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

