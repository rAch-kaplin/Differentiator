#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "file_read.h"
#include "tree_func.h"
#include "diff_tree.h"
#include "diff_DSL.h"
#include "logger.h"

static void Replace(Node **node, Node *son);
static void ReplaceWithOne(Node **node);

static bool ConstFolding(Node *node);
static bool RemoveNeutralElems(Node **node);

static bool AddOptimisation(Node **node);
static bool SubOptimisation(Node **node);
static bool MulOptimisation(Node **node);
static bool DivOptimisation(Node **node);
static bool PowOptimisation(Node **node);

static bool CompareDoubles(double value1, double value2);

static bool IsNum(const Node *node);

void FixParents(Node* node);

const double EPSILON = 1e-10;
const size_t MAX_VARS = 10;

void FixParents(Node* node)
{
    assert(node);

    if (node->left)  (node->left)->parent  = node;
    if (node->right) (node->right)->parent = node;
}

void FixTree(Node* node)
{
    LOG(LOGL_DEBUG, "FixParent node: %p", node);
    if (!node) return;

    FixParents(node);

    if (node->left)  FixTree(node->left);
    if (node->right) FixTree(node->right);
}

Variable* GetVarsTable() //TODO struct
{
    static Variable vars_table[MAX_VARS] = {};
    return vars_table;
}

void FreeVarsTable()
{
    Variable* vars_table = GetVarsTable();

    for (size_t i = 0; i < MAX_VARS; i++)
    {
        LOG(LOGL_DEBUG, "free ---> %s ", vars_table[i].name);
        if (vars_table[i].name != nullptr)
        {
            free((vars_table[i].name));
            vars_table[i].name = nullptr;
        }
    }
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
    return fabs(value1 - value2) < EPSILON;
}

double Eval(Node *node)
{
    assert(node);

    LOG(LOGL_DEBUG, "Eval node");
    if (node->type == NUM) return node->value.num;
    if (node->type == VAR) return Global_x;
    if (node->type == OP)
    {
        switch (node->value.op)
        {
            case ADD: return _ELEFT + _ERIGHT;
            case SUB: return _ELEFT - _ERIGHT;
            case MUL: return _ELEFT * _ERIGHT;
            case DIV: return _ELEFT / _ERIGHT;
            case POW: return pow(_ELEFT, _ERIGHT);

            default:
                LOG(LOGL_ERROR,"Error: Unknown operation");
                return NAN;
        }
    }
    if (node->type == FUNC)
    {
        switch (node->value.func)
        {
            case SIN:       return      sin(_ERIGHT);
            case COS:       return      cos(_ERIGHT);
            case LN:        return      log(_ERIGHT);
            // case EXP:       return      exp(_ERIGHT);
            case TG:        return      tan(_ERIGHT);
            case CTG:       return 1.0 / tan(_ERIGHT);
            case SH:        return      sinh(_ERIGHT);
            case CH:        return      cosh(_ERIGHT);
            case TH:        return      tanh(_ERIGHT);
            case CTH:       return 1.0 / tanh(_ERIGHT);
            case ARCSIN:    return      asin(_ERIGHT);
            case ARCCOS:    return      acos(_ERIGHT);
            case ARCTG:     return      atan(_ERIGHT);
            case ARCCTG:    return M_PI_2 - atan(_ERIGHT);
            case ARCSH:     return      asinh(_ERIGHT);
            case ARCCH:     return      acosh(_ERIGHT);
            case ARCTH:     return      atanh(_ERIGHT);
            case ARCCTH:    return      atanh(1.0 / _ERIGHT);

            case UNKNOW:
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
    assert(root != nullptr);

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
        LOG(LOGL_ERROR,"OP: %d", node->value.op);
        switch (node->value.op)
        {
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
    }
    if (node->type == FUNC)
    {
        LOG(LOGL_ERROR,"FUNC: %d", node->value.func);
        switch (node->value.func)
        {
            case SIN:
                return _MUL(_COS(CR), dR);
            case COS:
                return _MUL(_MUL(_NUM(-1), _SIN(CR)), dR);
            case LN:
                return _DIV(dR, CR);
            // case EXP:
            //     return _MUL(_EXP(CR), dR);
            case TG:
                return _DIV(dR, _POW(_COS(CR), _NUM(2)));
            case CTG:
                return _MUL(_NUM(-1), _DIV(dR, _POW(_SIN(CR), _NUM(2))));
            case SH:
                return _MUL(_CH(CR), dR);
            case CH:
                return _MUL(_SH(CR), dR);
            case TH:
                return _DIV(dR, _POW(_CH(CR), _NUM(2)));
            case CTH:
                return _MUL(_NUM(-1), _DIV(dR, _POW(_SH(CR), _NUM(2))));
            case ARCSIN:
                return _DIV(dR, _SQRT(_SUB(_NUM(1), _POW(CR, _NUM(2)))));
            case ARCCOS:
                return _MUL(_NUM(-1), _DIV(dR, _SQRT(_SUB(_NUM(1), _POW(CR, _NUM(2))))));
            case ARCTG:
                return _DIV(dR, _ADD(_POW(CR, _NUM(2)), _NUM(1)));
            case ARCCTG:
                return _MUL(_NUM(-1), _DIV(dR, _ADD(_POW(CR, _NUM(2)), _NUM(1))));
            case ARCSH:
                return _DIV(dR, _SQRT(_ADD(_POW(CR, _NUM(2)), _NUM(1))));
            case ARCCH:
                return _DIV(dR, _SQRT(_SUB(_POW(CR, _NUM(2)), _NUM(1))));
            case ARCTH:
                return _DIV(dR, _SUB(_NUM(1), _POW(CR, _NUM(2))));
            case ARCCTH:
                return _MUL(_NUM(-1), _DIV(dR, _SUB(_NUM(1), _POW(CR, _NUM(2)))));

            case UNKNOW:
            default:
                LOG(LOGL_ERROR,"Error: Unknown func");
                return nullptr;
        }
    }

    return nullptr;
}

void Simplifications(Node **node)
{
    assert(node && *node);

    do
    {

    } while (ConstFolding(*node) | RemoveNeutralElems(node));

    FixTree(*node);
}

static bool ConstFolding(Node *node)
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

static bool RemoveNeutralElems(Node **node)
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

static void Replace(Node **node, Node *son)
{
    assert(node && *node);

    Node *temp = *node;

    if (son == temp->left) temp->left = nullptr;
    if (son == temp->right) temp->right = nullptr;

    *node = son;

    if (son) son->parent = temp->parent;

    FreeTree(&temp);
}

static bool IsNum(const Node *node)
{
    return node != nullptr && node->type == NUM;
}

static bool AddOptimisation(Node **node)
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

static bool SubOptimisation(Node **node)
{
    assert(node && *node);

    if (IsNum((*node)->right)  && CompareDoubles((*node)->right->value.num, 0))
    {
        Replace(node, (*node)->left);
        return true;
    }

    return false;
}

static bool MulOptimisation(Node **node)
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

static bool DivOptimisation(Node **node)
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

static bool PowOptimisation(Node **node)
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

static void ReplaceWithOne(Node **node)
{
    assert(node);

    (*node)->type = NUM;
    (*node)->value.num = 1.0;

    FreeTree(&(*node)->left);
    FreeTree(&(*node)->right);

    (*node)->left  = nullptr;
    (*node)->right = nullptr;
}

