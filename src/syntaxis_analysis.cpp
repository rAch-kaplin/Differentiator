#include <stdio.h>
#include <assert.h>

#include "syntaxis_analysis.h"
#include "lexical_analysis.h"
#include "diff_tree.h"
#include "logger.h"
#include "diff_DSL.h"

Node* GetG(Lexeme *lexeme_array, size_t *cur)
{
    assert(lexeme_array);

    LOG(LOGL_DEBUG, "Logged in func GetG()");
    Node *node = GetE(lexeme_array, cur);
    if (lexeme_array[*cur].type != LEX_END)
    {
        fprintf(stderr, "Not find '$'\n");
        LOG(LOGL_ERROR, "Not find '$'");
        assert(0);
    }
    return node;
}

Node* GetE(Lexeme *lexeme_array, size_t *cur)
{
    assert(lexeme_array);

    LOG(LOGL_DEBUG, "Logged in func GetE()");
    Node *node = GetT(lexeme_array, cur);

    while (lexeme_array[*cur].type == LEX_OP && (lexeme_array[*cur].value.op == ADD || lexeme_array[*cur].value.op == SUB))
    {
        Op op = lexeme_array[*cur].value.op;
        (*cur)++;
        Node *node2 = GetT(lexeme_array, cur);
        if (op == ADD)
        {
            node = _ADD(node, node2);
        }
        else
        {
            node = _SUB(node, node2);
        }
    }
    return node;
}

Node* GetT(Lexeme *lexeme_array, size_t *cur)
{
    assert(lexeme_array);

    LOG(LOGL_DEBUG, "Logged in func GetT()");
    Node *node = GetD(lexeme_array, cur);

    while (lexeme_array[*cur].type == LEX_OP && (lexeme_array[*cur].value.op == MUL || lexeme_array[*cur].value.op == DIV))
    {
        Op op = lexeme_array[*cur].value.op;
        (*cur)++;
        Node *node2 = GetD(lexeme_array, cur);
        if (op == MUL)
        {
            node = _MUL(node, node2);
        }
        else
        {
            node = _DIV(node, node2);
        }
    }
    return node;
}

Node* GetD(Lexeme *lexeme_array, size_t *cur)
{
    assert(lexeme_array);

    LOG(LOGL_DEBUG, "Logged in func GetD()");
    Node *node = GetP(lexeme_array, cur);

    while (lexeme_array[*cur].value.op == POW)
    {
        (*cur)++;
        Node *node2 = GetP(lexeme_array, cur);
        node = _POW(node, node2);
    }
    return node;

}

Node* GetP(Lexeme *lexeme_array, size_t *cur)
{
    assert(lexeme_array);

    LOG(LOGL_DEBUG, "Logged in func GetP()");
    //printf("cur = <%zu>\n", *cur);
    switch (lexeme_array[*cur].type)
    {
        case LEX_NUM:
        {
            return _NUM(lexeme_array[(*cur)++].value.num);
        }

        case LEX_LBRACKET:
        {
            (*cur)++;
            Node *node = GetE(lexeme_array, cur);

            if (lexeme_array[*cur].type != LEX_RBRACKET)
            {
                fprintf(stderr, "Syntax error: expected ')' at position %zu\n", *cur);
                return nullptr;
            }

            (*cur)++;
            return node;
        }


        default:
        {
            fprintf(stderr, "Syntax error: unexpected lexeme type '%d' at position %zu\n",
            lexeme_array[*cur].type, *cur);
            return nullptr;
        }
    }
}
