#include <stdio.h>
#include <assert.h>
#include <cmath>

#include "read_tree.h"
#include "diff_tree.h"
#include "diff_DSL.h"
#include "logger.h"

bool ConstFolding(Node *node);
bool RemoveNeutralElems(Node **node);
void Replace(Node **node, Node *son);
bool AddOptimisation(Node **node);
bool SubOptimisation(Node **node);
bool MulOptimisation(Node **node);
bool DivOptimisation(Node **node);
bool PowOptimisation(Node **node);
bool CompareDoubles(double value1, double value2);

#define ReplaceWithOne()                  \
do {                                      \
    (*node)->type = NUM;                  \
    (*node)->value.num = 1.0;             \
                                          \
    FreeTree(&(*node)->left);             \
    FreeTree(&(*node)->right);            \
                                          \
    (*node)->left  = nullptr;             \
    (*node)->right = nullptr;             \
} while (0)

const double EPSILON = 1e-10;

bool CheckVars(Node* node)
{
    return node && node->type == VAR;
}

bool CompareDoubles(double value1, double value2)
{
    return (fabs(value1) - fabs(value2) < EPSILON);
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

void Optimize(Node **node)
{
    assert(node && *node);

    bool changed = false;

    do
    {
        changed = false;

        changed |= ConstFolding(*node);

        changed |= RemoveNeutralElems(node);

    } while (changed);
}


bool ConstFolding(Node *node)
{
    assert(node);
    bool changed = false;

    if (node->left)
        changed |= ConstFolding(node->left);
    if (node->right)
        changed |= ConstFolding(node->right);

    if (node->type == OP && !CheckVars(node))
    {
        node->value.num = Eval(node);
        node->type = NUM;

        FreeTree(&node->left);
        FreeTree(&node->right);
        node->left = nullptr;
        node->right = nullptr;

        changed = true;
    }

    return changed;
}

bool RemoveNeutralElems(Node **node)
{
    assert(node && *node);

    if ((*node)->type != OP) return false;

    switch ((*node)->value.op)
    {
        case ADD: return AddOptimisation(node);
        case SUB: return SubOptimisation(node);
        case MUL: return MulOptimisation(node);
        case DIV: return DivOptimisation(node);
        case POW: return PowOptimisation(node);
        default:  return false;
    }
}

void Replace(Node **node, Node *son)
{
    assert(node && *node);

    Node *temp = *node;
    *node = son;
    temp->left = temp->right = nullptr;
    FreeTree(&temp);
}

bool AddOptimisation(Node **node)
{
    assert(node && *node);

    if ((*node)->left->type == NUM && CompareDoubles((*node)->left->value.num, 0))
    {
        Replace(node, (*node)->right);
        return true;
    }
    else if ((*node)->right->type == NUM && CompareDoubles((*node)->right->value.num, 0))
    {
        Replace(node, (*node)->left);
        return true;
    }

    return false;
}

bool SubOptimisation(Node **node)
{
    assert(node && *node);

    if ((*node)->right->type == NUM && CompareDoubles((*node)->right->value.num, 0))
    {
        Replace(node, (*node)->left);
        return true;
    }

    return false;
}

bool MulOptimisation(Node **node)
{
    assert(node && *node);

    if ((*node)->left->type == NUM)
    {
        if (CompareDoubles((*node)->left->value.num, 1))
        {
            Replace(node, (*node)->right);
            return true;
        }
        else if (CompareDoubles((*node)->left->value.num, 0))
        {
            Replace(node, (*node)->left);
            return true;
        }
    }
    else if ((*node)->right->type == NUM)
    {
        if (CompareDoubles((*node)->right->value.num, 1))
        {
            Replace(node, (*node)->left);
            return true;
        }
        else if (CompareDoubles((*node)->right->value.num, 0))
        {
            Replace(node, (*node)->right);
            return true;
        }
    }

    return false;
}

bool DivOptimisation(Node **node)
{
    assert(node && *node);

    if ((*node)->right->type == NUM)
    {
        if (CompareDoubles((*node)->right->value.num, 1))
        {
            Replace(node, (*node)->left);
            return true;
        }
        else if ((*node)->left->type == NUM && CompareDoubles((*node)->right->value.num, (*node)->left->value.num))
        {
            ReplaceWithOne();
            return true;
        }
    }

    return false;
}

bool PowOptimisation(Node **node)
{
    assert(node && *node);

    if ((*node)->left->type == NUM)
    {
        if (CompareDoubles((*node)->left->value.num, 0) || CompareDoubles((*node)->left->value.num, 1))
        {
            Replace(node, (*node)->left);
            return true;
        }
    }

    if ((*node)->right->type == NUM)
    {
        if (CompareDoubles((*node)->right->value.num, 0))
        {
            ReplaceWithOne();
            return true;
        }
        else if (CompareDoubles((*node)->right->value.num, 1))
        {
            Replace(node, (*node)->left);
            return true;
        }
    }

    return false;
}

#undef ReplaceWithOne
