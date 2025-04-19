#ifndef _DIFF_RULES_DCL
#define _DIFF_RULES_DCL

#ifdef DEF_OPER

DEF_OPER(ADD,
    _ELEFT + _ERIGHT,
    { return _ADD (dL, dR); },
    '+')

DEF_OPER(SUB,
    _ELEFT - _ERIGHT,
    { return _SUB (dL, dR); },
    '-')

DEF_OPER(MUL,
    _ELEFT * _ERIGHT,
    { return _ADD (_MUL (dL, CR), _MUL (CL, dR)); },
    '*')

DEF_OPER(DIV,
    _ELEFT / _ERIGHT,
    { return _DIV (_SUB (_MUL (dL, CR), _MUL(CL, dR)), _MUL(CR, CR)); },
    '/')

DEF_OPER(POW,
    pow(_ELEFT, _ERIGHT),
    {
        bool var_in_base   = CheckVars(node->left);
        bool var_in_degree = CheckVars(node->right);

        if (var_in_base && var_in_degree)
        {
            return _MUL ( _ADD ( _MUL ( dR, _LOG(CL) ), _DIV ( _MUL (dL, CR), CL ) ), _POW (CL, CR) );
        }

        if (var_in_base)
        {
            return _MUL(dL, _MUL(CR, _POW(CL, _SUB(CR, _NUM(1)))));
        }

        if (var_in_degree)
        {
            return _MUL(_POW(CL, CR), _MUL(dR, _LOG(CL)));
        }

        return nullptr;
    },

    '^')
#endif //DEF_OPER

#ifdef DEF_FUNC
DEF_FUNC(SIN,
    sin(_ERIGHT),
    { return _MUL (_COS(CR), dR); },
    "sin")

DEF_FUNC(COS,
    cos(_ERIGHT),
    { return _MUL (_SIN (_MUL ( _NUM(-1), CR)), dR); },
    "cos")

DEF_FUNC(LOG,
    log(_ERIGHT),
    { return _DIV (dR, CR); },
    "ln")

#endif //DEF_FUNC

#endif ///DIFF_RULES_FUNC
