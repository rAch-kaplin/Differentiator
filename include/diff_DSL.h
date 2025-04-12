#define _ELEFT  Eval(node -> left)
#define _ERIGHT Eval(node -> right)

#define _NUM(n) NewNode(NUM, (NodeValue){.num = (n)}, nullptr, nullptr)
#define _VAR(x) NewNode(VAR, (NodeValue){.var = (x)}, nullptr, nullptr)

#define OP_VALUE(oper) NodeValue {.op = (oper)}

#define _ADD(a, b) NewNode(OP, OP_VALUE(ADD), a, b)
#define _SUB(a, b) NewNode(OP, OP_VALUE(SUB), a, b)
#define _MUL(a, b) NewNode(OP, OP_VALUE(MUL), a, b)
#define _DIV(a, b) NewNode(OP, OP_VALUE(DIV), a, b)
#define _COS(b)    NewNode(OP, OP_VALUE(COS), nullptr, b)
#define _SIN(b)    NewNode(OP, OP_VALUE(SIN), nullptr, b)
#define _LOG(b)    NewNode(OP, OP_VALUE(LOG), nullptr, b)
#define _POW(a, b) NewNode(OP, OP_VALUE(POW), a, b)

#define dL  Diff(node->left)
#define dR  Diff(node->right)
#define CL  CopyTree(node->left)
#define CR  CopyTree(node->right)
