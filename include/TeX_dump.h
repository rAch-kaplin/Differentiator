#ifndef _TeX_DUMP
#define _TeX_DUMP

#include "tree_func.h"
#include "diff_tree.h"

const size_t TeX_buffer_size = 16048;

#define _WRITE_NODE_TEX(buffer_TeX, cur_len, ...)                                         \
    do {                                                                                  \
        if ((size_t)*(cur_len) < TeX_buffer_size) {                                       \
            int _written = snprintf((buffer_TeX) + *(cur_len),                            \
                                    (TeX_buffer_size) - (size_t)*(cur_len), __VA_ARGS__); \
            if (_written > 0) *(cur_len) += _written;                                     \
        }                                                                                 \
    } while (0)

typedef struct
{
    char *buffer_TeX;
    int cur_len;
} TeX;

void WriteToTexStart(Node *root, const char* filename_tex, TeX *tex);
void WriteExpressionToTeX(Node *root, char *buffer_TeX, int *cur_len);
void WriteExpressionToTeX2(Node *root, char *buffer_TeX, int *cur_len);
void WriteToTexEnd  (Node *root, const char* filename_tex, TeX *tex);
void WriteTotalDifferentialTeX(char* tex_buffer, size_t* cur_len, Node** partials, size_t num_vars);
void GenerateTeXReport(Node* node_G, const char* file_tex);

#endif // _TeX_DUMP
