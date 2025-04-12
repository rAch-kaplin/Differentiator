#include <stdio.h>
#include <assert.h>
#include <cmath>

#include "read_tree.h"
#include "diff_tree.h"
#include "diff_DSL.h"
#include "logger.h"

const double EPSILON = 1e-10;

bool CheckVars(Node* node)
{
    return node && node->type == VAR;
}

double Eval(Node *node)
{
    assert(node);

    if (node->type == NUM) return node->value.num;
    if (node->type == VAR) return Global_x;
    if (node->type == OP)
    {
        #define DEF_OPER(oper, eval_rule, ...) case oper: return eval_rule;
        switch (node->value.op)
        {
            #include "diff_rules_DSL.h"

            default:
                LOG(LOGL_ERROR,"Error: Unknown operation");
                return NAN;
        }
        #undef DEF_OPER
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
        #define DEF_OPER(oper, eval_rule, diff_rule, ...) case oper: diff_rule;
        switch (node->value.op)
        {
            #include "diff_rules_DSL.h"

            default:
                LOG(LOGL_ERROR,"Error: Unknown operation");
                return nullptr;
        }
        #undef DEF_OPER
    }

    return nullptr;
}

