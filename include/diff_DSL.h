#ifndef _DIFF_DCL
#define _DIFF_DCL

#include "diff_tree.h"
#include "tree_func.h"

#define _ELEFT  Eval(node -> left)
#define _ERIGHT Eval(node -> right)

#define _NUM(n) NewNode(NUM, (NodeValue){.num = (n)}, nullptr, nullptr)
#define _VAR(x) NewNode(VAR, (NodeValue){.var = (x)}, nullptr, nullptr)

#define OP_VALUE(oper)   NodeValue {.op   = (oper)}

#define _FUNC(func_type, arg) NewNode(FUNC,  (NodeValue){.func = (func_type)}, nullptr, arg)
#define _OP(op, a, b) NewNode(OP, (NodeValue){.op = (op)}, a, b)

#define _ADD(a, b) NewNode(OP, OP_VALUE(ADD), a, b)
#define _SUB(a, b) NewNode(OP, OP_VALUE(SUB), a, b)
#define _MUL(a, b) NewNode(OP, OP_VALUE(MUL), a, b)
#define _DIV(a, b) NewNode(OP, OP_VALUE(DIV), a, b)
#define _POW(a, b) NewNode(OP, OP_VALUE(POW), a, b)
#define _COS(b)    NewNode(FUNC, FUNC_VALUE(COS), nullptr, b)
#define _SIN(b)    NewNode(FUNC, FUNC_VALUE(SIN), nullptr, b)
#define _LOG(b)    NewNode(FUNC, FUNC_VALUE(LOG), nullptr, b)

#define dL  Diff(node->left)
#define dR  Diff(node->right)
#define CL  CopyTree(node->left)
#define CR  CopyTree(node->right)

#endif //DIFF_DCL
