#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <assert.h>

#include "read_tree.h"

const size_t size_op = 32;

int GenerateGraph(Node* node, char* buffer, int* buffer_len, const size_t BUFFER_SIZE);
const char* GetNodeLabel(const Node* node);
const char* GetNodeColor(const Node* node);
int GenerateGraph2(Node* node, char* buffer, int* buffer_len, const size_t BUFFER_SIZE);

const char* GetNodeLabel(const Node* node)
{
    static char label[size_op];

    switch (node->type)
    {
        case NUM:
            snprintf(label, sizeof(label), "%lg", node->value.num);
            break;
        case VAR:
            snprintf(label, sizeof(label), "%c", node->value.var);
            break;
        case OP:
        {
            const char* op_symbol = NULL;
            for (size_t i = 0; i < sizeof(operations)/sizeof(operations[0]); i++)
            {
                if (operations[i].op == node->value.op)
                {
                    op_symbol = operations[i].symbol;
                    break;
                }
            }
            snprintf(label, sizeof(label), "%s", op_symbol ? op_symbol : "?");
            break;
        }
        default:
        {
            snprintf(label, sizeof(label), "UNKNOWN");
            break;
        }
    }

    return label;
}

CodeError TreeDumpDot(Node* root)
{
    static int dump_counter = 0;
    const size_t PNG_NAME_SIZE = 64;
    const size_t BUFFER_SIZE = 32768;

    char* buffer = (char*)calloc(BUFFER_SIZE, sizeof(char));
    if (buffer == nullptr)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return MEM_ALLOC_FAIL;
    }

    int buffer_len = 0;
    buffer_len += snprintf(buffer + buffer_len, BUFFER_SIZE - (size_t)buffer_len,
                           "digraph G {\n"
                           "\trankdir = HR;\n"
                           "\tbgcolor=\"#fcf0d2\";\n");

    GenerateGraph(root, buffer, &buffer_len, BUFFER_SIZE);

    buffer_len += snprintf(buffer + buffer_len, BUFFER_SIZE - (size_t)buffer_len, "}\n");

    FILE* file = fopen("dump.dot", "w+");
    if (!file)
    {
        fprintf(stderr, "Cannot open dot file\n");
        free(buffer);
        return FILE_NOT_OPEN;
    }

    fprintf(file, "%s", buffer);
    fclose(file);
    free(buffer);

    char png_filename[PNG_NAME_SIZE];
    snprintf(png_filename, sizeof(png_filename), "dump_%d.png", dump_counter++);

    char command[256];
    snprintf(command, sizeof(command), "dot -Tpng dump.dot -o %s", png_filename);
    system(command);

    return OK;
}

int GenerateGraph(Node* node, char* buffer, int* buffer_len, const size_t BUFFER_SIZE)
{
    if (!node) return 0;

    const char* label = GetNodeLabel(node);

    *buffer_len += snprintf(buffer + *buffer_len, BUFFER_SIZE - (size_t)*buffer_len,
        "\t             node%p [shape=plaintext; style=filled; color=\"#fcf0d2\"; label = <\n"
        "\t\t                     <table border=\"0\" cellborder=\"1\" cellspacing=\"0\" cellpadding=\"6\" bgcolor=\"#bfb58f\" color=\"#4d3d03\">\n"
        "\t\t\t                   <tr><td align='center' colspan='2'><FONT COLOR='#3a3a3a'><b>Node: %p</b></FONT></td></tr>\n"
        "\t\t\t                   <tr><td align='center' colspan='2'><FONT COLOR='#3a3a3a'><b>Type: %s</b></FONT></td></tr>\n"
        "\t\t\t                   <tr><td align='center' colspan='2'><FONT COLOR='#ba6504'>Val: <b>%s</b></FONT></td></tr>\n"
        "\t\t\t                   <tr>\n"
        "\t\t\t\t                     <td WIDTH='150' PORT='left' align='center'><FONT COLOR='#006400'><b>Left: %p</b></FONT></td>\n"
        "\t\t\t\t                     <td WIDTH='150' PORT='right' align='center'><FONT COLOR='#8b0000'><b>Right: %p</b></FONT></td>\n"
        "\t\t\t                   </tr>\n"
        "\t\t                     </table> >];\n",
        node, node, (node->type == OP ? "OP" : (node->type == NUM ? "NUM" : "VAR")), label, node->left, node->right);

    if (node->left)
    {
        *buffer_len += GenerateGraph(node->left, buffer, buffer_len, BUFFER_SIZE);
        *buffer_len += snprintf(buffer + *buffer_len, BUFFER_SIZE - (size_t)*buffer_len,
                                "\tnode%p:left -> node%p [color=\"#006400\" style=bold; weight=1000];\n",
                                node, node->left);
    }

    if (node->right)
    {
        *buffer_len += GenerateGraph(node->right, buffer, buffer_len, BUFFER_SIZE);
        *buffer_len += snprintf(buffer + *buffer_len, BUFFER_SIZE - (size_t)*buffer_len,
                                "\tnode%p:right -> node%p [color=\"#8b0000\" style=bold; weight=1000];\n",
                                node, node->right);
    }

    return 0;
}

const char* GetNodeColor(const Node* node)
{
    assert(node != nullptr);

    switch (node->type)
    {
        case NUM:    return "#ff9a8d";
        case VAR:    return "#4a536b";
        case OP:     return "#aed6dc";
        default:     return "#ffffff";
    }
}

int GenerateGraph2(Node* node, char* buffer, int* buffer_len, const size_t BUFFER_SIZE)
{
    if (!node) return 0;

    const char* label = GetNodeLabel(node);
    const char* color = GetNodeColor(node);

    *buffer_len += snprintf(buffer + *buffer_len, BUFFER_SIZE - (size_t)*buffer_len,
        "\tnode%p [shape=\"Mrecord\"; style=filled; color=\"%s\"; label = \"%s\" ];\n",
        node, color, label);

    if (node->left)
    {
        *buffer_len += GenerateGraph2(node->left, buffer, buffer_len, BUFFER_SIZE);
        *buffer_len += snprintf(buffer + *buffer_len, BUFFER_SIZE - (size_t)*buffer_len,
            "\tnode%p -> node%p [style=bold; weight=1000;];\n",
            node, node->left);
    }

    if (node->right)
    {
        *buffer_len += GenerateGraph2(node->right, buffer, buffer_len, BUFFER_SIZE);
        *buffer_len += snprintf(buffer + *buffer_len, BUFFER_SIZE - (size_t)*buffer_len,
            "\tnode%p -> node%p [style=bold; weight=1000;];\n",
            node, node->right);
    }

    return 0;
}

CodeError TreeDumpDot2(Node* root)
{
    static int dump_counter = 0;
    const size_t BUFFER_SIZE = 32768;
    char* buffer = (char*)calloc(BUFFER_SIZE, sizeof(char));
    if (!buffer) return MEM_ALLOC_FAIL;

    int buffer_len = snprintf(buffer, BUFFER_SIZE,
        "digraph G {\n"
        "\trankdir=HR;\n"
        "\tbgcolor=\"#ebf7fa\";\n"
        "\tnode [fontname=\"Arial\", fontsize=12];\n");

    GenerateGraph2(root, buffer, &buffer_len, BUFFER_SIZE);
    buffer_len += snprintf(buffer + buffer_len, BUFFER_SIZE - (size_t)buffer_len, "}\n");


    FILE* file = fopen("dump2.dot", "w+");
    if (!file)
    {
        fprintf(stderr, "Cannot open dot file\n");
        free(buffer);
        return FILE_NOT_OPEN;
    }

    fprintf(file, "%s", buffer);
    fclose(file);
    free(buffer);

    char png_name[64];
    snprintf(png_name, sizeof(png_name), "dump2_%d.png", dump_counter++);

    char command[128];
    snprintf(command, sizeof(command), "dot -Tpng dump2.dot -o %s", png_name);
    system(command);

    return OK;
}
