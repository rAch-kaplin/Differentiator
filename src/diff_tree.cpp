#include <stdio.h>
#include <assert.h>
#include <cmath>
#include <stdlib.h>

#include "file_read.h"
#include "tree_func.h"
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

void ReplaceWithOne(Node **node);

bool IsNum(const Node *node);

const double EPSILON = 1e-10;
const size_t MAX_VARS = 10;

//#define _LOG(b)    NewNode(FUNC, NodeValue {.func = (LN)}, nullptr, b)

Variable* GetVarsTable() //TODO struct
{
    static Variable vars_table[MAX_VARS] = {};
    return vars_table;
}

size_t LookupVar(Variable *vars_table, const char* name, size_t len_name)
{
    assert(vars_table && name);

    size_t cur = 0;
    while (vars_table[cur].name && cur < MAX_VARS)
    {
        if (strncmp(name, vars_table[cur].name, len_name) == 0)
        {
            break;
        }

        cur++;
    }

    return cur;
}

size_t AddVartable(Variable *vars_table, const char* name, size_t len_name)
{
    assert(vars_table && name);

    size_t pos = LookupVar(vars_table, name, len_name);
    if (vars_table[pos].name == nullptr)
    {
        vars_table[pos].name = strdup(name);
        vars_table[pos].len_name = len_name;
    }

    return pos;
}

bool CheckVars(Node* node)
{
    if (!node) return false;
    if (node->type == VAR) return true;
    if (node->type == NUM) return false;
    if (node->type == OP || node->type == FUNC)
        return CheckVars(node->left) || CheckVars(node->right);
    return false;
}


bool CompareDoubles(double value1, double value2)
{
    return fabs(value1 - value2) < EPSILON;;
}

double Eval(Node *node)
{
    assert(node);

    LOG(LOGL_DEBUG, "Eval node");
    if (node->type == NUM) return node->value.num;
    if (node->type == VAR) return Global_x;
    if (node->type == OP)
    {

    /**************************************************************************************************************************
        In this project you can see the use of DSL rules. The project uses this as an experiment
        (for training purposes). It is better not to use it here, because not so many conditions need to be checked,
        also it is used in one file. In this case you should not do that, keep another file with rules.
    ****************************************************************************************************************************/

        #define DEF_OPER(oper, eval_rule, ...) case oper: return eval_rule;
        switch (node->value.op)
        {
            // #include "diff_rules_DSL.h"
            case ADD: return _ELEFT + _ERIGHT;
            case SUB: return _ELEFT - _ERIGHT;
            case MUL: return _ELEFT * _ERIGHT;
            case DIV: return _ELEFT / _ERIGHT;
            case POW: return pow(_ELEFT, _ERIGHT);

            default:
                LOG(LOGL_ERROR,"Error: Unknown operation");
                return NAN;
        }
        #undef DEF_OPER
    }
    if (node->type == FUNC)
    {
        #define DEF_FUNC(func, eval_rule, ...) case func: return eval_rule;
        switch (node->value.func)
        {
            // #include "diff_rules_DSL.h"

            case SIN:
                return sin(_ERIGHT);
            case COS:
                return cos(_ERIGHT);
            case LN:
                return log(_ERIGHT);


            default:
                LOG(LOGL_ERROR,"Error: Unknown operation");
                return NAN;
        }
        #undef DEF_FUNC
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

    LOG(LOGL_DEBUG, "Diff node");
    if (node->type == NUM) return _NUM(0);
    LOG(LOGL_ERROR,"NUM: %d", node->value.num);

    if (node->type == VAR) return _NUM(1);
    LOG(LOGL_ERROR,"VAR: %d", node->value.var);

    if (node->type == OP)
    {
        #define DEF_OPER(oper, eval_rule, diff_rule, ...) case oper: diff_rule;

        LOG(LOGL_ERROR,"OP: %d", node->value.op);
        switch (node->value.op)
        {
            // #include "diff_rules_DSL.h"

            case ADD:
                return _ADD(dL, dR);
            case SUB:
                return _SUB(dL, dR);
            case MUL:
                return _ADD(_MUL(dL, CR), _MUL(CL, dR));
            case DIV:
                return _DIV(_SUB(_MUL(dL, CR), _MUL(CL, dR)), _MUL(CR, CR));
            case POW:
            {
                bool var_in_base   = CheckVars(node->left);
                bool var_in_degree = CheckVars(node->right);

                if (var_in_base && var_in_degree)
                {
                    return _MUL(_ADD( _MUL(dR, _LOG(CL)),_DIV(_MUL(dL, CR), CL)), _POW(CL, CR));
                }
                if (var_in_base)
                {
                    return _MUL(dL,  _MUL(CR, _POW(CL, _SUB(CR, _NUM(1)))));
                }
                if (var_in_degree)
                {
                    return _MUL( _POW(CL, CR),  _MUL(dR, _LOG(CL)));
                }

                return nullptr;
            }

            default:
                LOG(LOGL_ERROR,"Error: Unknown operation %d", node->value.op);
                return nullptr;
        }
        #undef DEF_OPER
    }
    if (node->type == FUNC)
    {
        #define DEF_FUNC(func, eval_rule, diff_rule, ...) case func: diff_rule

        LOG(LOGL_ERROR,"FUNC: %d", node->value.func);
        switch (node->value.func)
        {
            // #include "diff_rules_DSL.h"

            case SIN:
                return _MUL(_COS(CR), dR);
            case COS:
                return _MUL(_MUL(_NUM(-1), _SIN(CR)), dR);
            case LN:
                return _DIV(dR, CR);

            default:
                LOG(LOGL_ERROR,"Error: Unknown func");
                return nullptr;
        }
        #undef DEF_FUNC
    }

    return nullptr;
}

void Optimize(Node **node) //TODO Rename
{
    assert(node && *node);

    bool changed = false;

    do
    {
        changed |= ConstFolding(*node);
        changed |= RemoveNeutralElems(node);

    } while (!changed);
}

bool ConstFolding(Node *node)
{
    assert(node);
    bool changed = false;
    LOG(LOGL_DEBUG, "Const folding()");
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

    bool changed = false;

    if ((*node)->left)
        changed |= RemoveNeutralElems(&(*node)->left);
    if ((*node)->right)
        changed |= RemoveNeutralElems(&(*node)->right);

    if ((*node)->type != OP) return changed;

    LOG(LOGL_DEBUG, "RemoveNeutralElems()");
    switch ((*node)->value.op)
    {
        case ADD: changed |= AddOptimisation(node); break;
        case SUB: changed |= SubOptimisation(node); break;
        case MUL: changed |= MulOptimisation(node); break;
        case DIV: changed |= DivOptimisation(node); break;
        case POW: changed |= PowOptimisation(node); break;

        default: break;
    }

    return changed;
}

void Replace(Node **node, Node *son)
{
    assert(node && *node);

    Node *temp = *node;

    if (son == temp->left) temp->left = nullptr;
    if (son == temp->right) temp->right = nullptr;

    *node = son;
    FreeTree(&temp);
}

bool IsNum(const Node *node)
{
    return node != nullptr && node->type == NUM;
}

bool AddOptimisation(Node **node)
{
    assert(node && *node);

    if (IsNum((*node)->left) && CompareDoubles((*node)->left->value.num, 0))
    {
        Replace(node, (*node)->right);
        return true;
    }
    else if (IsNum((*node)->right) && CompareDoubles((*node)->right->value.num, 0))
    {
        Replace(node, (*node)->left);
        return true;
    }

    return false;
}

bool SubOptimisation(Node **node)
{
    assert(node && *node);

    if (IsNum((*node)->right)  && CompareDoubles((*node)->right->value.num, 0))
    {
        Replace(node, (*node)->left);
        return true;
    }

    return false;
}

bool MulOptimisation(Node **node)
{
    assert(node && *node);

    LOG(LOGL_DEBUG, "MulOptimisation()");
    if (IsNum((*node)->left))
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
    else if (IsNum((*node)->right) )
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

    if (IsNum((*node)->right) )
    {
        if (CompareDoubles((*node)->right->value.num, 1))
        {
            Replace(node, (*node)->left);
            return true;
        }
        else if (IsNum((*node)->left) && CompareDoubles((*node)->right->value.num, (*node)->left->value.num))
        {
            ReplaceWithOne(node);
            return true;
        }
    }

    return false;
}

bool PowOptimisation(Node **node)
{
    assert(node && *node);

    if (IsNum((*node)->left))
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
            ReplaceWithOne(node);
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

void ReplaceWithOne(Node **node)
{
    assert(node);

    (*node)->type = NUM;
    (*node)->value.num = 1.0;

    FreeTree(&(*node)->left);
    FreeTree(&(*node)->right);

    (*node)->left  = nullptr;
    (*node)->right = nullptr;
}

