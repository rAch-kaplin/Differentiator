#ifndef _SYNTAXIS_ANALYSIS
#define _SYNTAXIS_ANALYSIS

#include "diff_tree.h"
#include "lexical_analysis.h"

Node* GetG(Lexeme *lexeme_array, size_t *cur);
Node* GetE(Lexeme *lexeme_array, size_t *cur);
Node* GetT(Lexeme *lexeme_array, size_t *cur);
Node* GetD(Lexeme *lexeme_array, size_t *cur);
Node* GetP(Lexeme *lexeme_array, size_t *cur);

#endif // _SYNTAXIS_ANALYSIS
