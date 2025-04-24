#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <assert.h>

#include "file_read.h"
#include "logger.h"
#include "lexical_analysis.h"
#include "tree_func.h"

const size_t size_op = 32;
const size_t PNG_NAME_SIZE = 64;
const size_t BUFFER_SIZE = 327680;
const size_t command_buf = 512;

const char* GetNodeLabel(const Node* node);
const char* GetNodeColor(const Node* node);
int GenerateGraph(Node* node, char* buffer, int* buffer_len);
int GenerateGraph2(Node* node, char* buffer, int* buffer_len);

const char* GetNodeLabel(const Node* node)
{
    static char label[size_op] = "";

    switch (node->type)
    {
        case NUM:
            snprintf(label, sizeof(label), "%lg", node->value.num);
            break;
        case VAR:
        {
            Variable* vars_table = GetVarsTable();
            size_t var_index = node->value.var;

            if (vars_table[var_index].name != nullptr)
            {
                LOG(LOGL_DEBUG, "GetNOdeLabel(): p=%p", vars_table[var_index].name);
                snprintf(label, sizeof(label), "%s", vars_table[var_index].name);
            }
            else
            {
                snprintf(label, sizeof(label), "var[%zu]", var_index);
            }
            break;
        }
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
        case FUNC:
        {
            const char* func_name = nullptr;

            for (size_t i = 0; i < sizeof(func) / sizeof(func[0]); i++)
            {
                if (func[i].func == node->value.func)
                {
                    func_name = func[i].name;
                    break;
                }
            }

            snprintf(label, sizeof(label), "%s", func_name ? func_name : "Unknow");
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

const char* GetNodeColor(const Node* node)
{
    assert(node != nullptr);

    switch (node->type)
    {
        case NUM:    return "#ff9a8d";
        case VAR:    return "#7e8aab";
        case OP:     return "#aed6dc";
        case FUNC:   return "#809fb0";
        default:     return "#ffffff";
    }
}

CodeError TreeDumpDot(Node* root)
{
    static int dump_counter = 0;

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
                           "\tbgcolor=\"#ebf7fa\";\n");

    GenerateGraph(root, buffer, &buffer_len);

    buffer_len += snprintf(buffer + buffer_len, BUFFER_SIZE - (size_t)buffer_len, "}\n");

    FILE* file = fopen("graphiz/dot/dump.dot", "w+");
    if (!file)
    {
        fprintf(stderr, "Cannot open dot file\n");
        free(buffer);
        return FILE_NOT_OPEN;
    }

    fprintf(file, "%s", buffer);
    fclose(file);
    free(buffer);

    system("mkdir -p graphiz/img graphiz/dot");

    char png_name[PNG_NAME_SIZE] = {};
    snprintf(png_name, sizeof(png_name), "graphiz/img/dump_%d.png", dump_counter++);

    char command[command_buf] = {};
    snprintf(command, sizeof(command), "dot -Tpng graphiz/dot/dump.dot -o %s", png_name);
    system(command);

    return OK;
}

int GenerateGraph(Node* node, char* buffer, int* buffer_len)
{
    if (!node) return 0;

    const char* label = GetNodeLabel(node);
    const char* color = GetNodeColor(node);

    *buffer_len += snprintf(buffer + *buffer_len, BUFFER_SIZE - (size_t)*buffer_len,
        "\t             node%p [shape=plaintext; style=filled; label = <\n"
        "\t\t                     <table border=\"0\" cellborder=\"1\" cellspacing=\"0\" cellpadding=\"6\" bgcolor=\"%s\" color=\"#4d3d03\">\n"
        "\t\t\t                   <tr><td align='center' colspan='2'><FONT COLOR='#3a3a3a'><b>Node: %p</b></FONT></td></tr>\n"
        "\t\t\t                   <tr><td align='center' colspan='2'><FONT COLOR='#3a3a3a'><b>Parent: %p</b></FONT></td></tr>\n"
        "\t\t\t                   <tr><td align='center' colspan='2'><FONT COLOR='#3a3a3a'><b>Type: %s</b></FONT></td></tr>\n"
        "\t\t\t                   <tr><td align='center' colspan='2'><FONT COLOR='#3a3a3a'><b>%s</b></FONT></td></tr>\n"
        "\t\t\t                   <tr>\n"
        "\t\t\t\t                     <td WIDTH='150' PORT='left' align='center'><FONT COLOR='#3a3a3a'><b>Left: %p</b></FONT></td>\n"
        "\t\t\t\t                     <td WIDTH='150' PORT='right' align='center'><FONT COLOR='#3a3a3a'><b>Right: %p</b></FONT></td>\n"
        "\t\t\t                   </tr>\n"
        "\t\t                     </table> >];\n",
        node, color, node, node->parent, (node->type == OP ? "OP" : (node->type == NUM ? "NUM" : "VAR")), label, node->left, node->right);

    if (node->left)
    {
        *buffer_len += GenerateGraph(node->left, buffer, buffer_len);
        *buffer_len += snprintf(buffer + *buffer_len, BUFFER_SIZE - (size_t)*buffer_len,
                                "\tnode%p:left -> node%p [color=\"%s\" style=bold; weight=1000];\n",
                                node, node->left, color);
    }

    if (node->right)
    {
        *buffer_len += GenerateGraph(node->right, buffer, buffer_len);
        *buffer_len += snprintf(buffer + *buffer_len, BUFFER_SIZE - (size_t)*buffer_len,
                                "\tnode%p:right -> node%p [color=\"%s\" style=bold; weight=1000];\n",
                                node, node->right, color);
    }

    return 0;
}

int GenerateGraph2(Node* node, char* buffer, int* buffer_len)
{
    if (!node) return 0;

    const char* label = GetNodeLabel(node);
    const char* color = GetNodeColor(node);

    *buffer_len += snprintf(buffer + *buffer_len, BUFFER_SIZE - (size_t)*buffer_len,
        "\tnode%p [shape=\"Mrecord\"; style=filled; color=\"%s\"; label = \"%s\" ];\n",
        node, color, label);

    if (node->left)
    {
        *buffer_len += GenerateGraph2(node->left, buffer, buffer_len);
        *buffer_len += snprintf(buffer + *buffer_len, BUFFER_SIZE - (size_t)*buffer_len,
            "\tnode%p -> node%p [color=\"%s\"; style=bold;  weight=1000;];\n",
            node, node->left, color);
    }

    if (node->right)
    {
        *buffer_len += GenerateGraph2(node->right, buffer, buffer_len);
        *buffer_len += snprintf(buffer + *buffer_len, BUFFER_SIZE - (size_t)*buffer_len,
            "\tnode%p -> node%p [color=\"%s\"; style=bold; weight=1000;];\n",
            node, node->right, color);
    }

    return 0;
}

CodeError TreeDumpDot2(Node* root)
{
    static int dump_counter = 0;
    char* buffer = (char*)calloc(BUFFER_SIZE, sizeof(char));
    if (!buffer) return MEM_ALLOC_FAIL;

    int buffer_len = snprintf(buffer, BUFFER_SIZE,
        "digraph G {\n"
        "\trankdir=HR;\n"
        "\tbgcolor=\"#d8e3eb\";\n"
        "\tnode [fontname=\"Arial\", fontsize=12];\n");

    GenerateGraph2(root, buffer, &buffer_len);
    buffer_len += snprintf(buffer + buffer_len, BUFFER_SIZE - (size_t)buffer_len, "}\n");


    FILE* file = fopen("graphiz/dot/dump2.dot", "w+");
    if (!file)
    {
        fprintf(stderr, "Cannot open dot file\n");
        free(buffer);
        return FILE_NOT_OPEN;
    }

    fprintf(file, "%s", buffer);
    fclose(file);
    free(buffer);

    system("mkdir -p graphiz/img graphiz/dot");

    char png_name[PNG_NAME_SIZE] = {};
    snprintf(png_name, sizeof(png_name), "graphiz/img/dump2_%d.png", dump_counter++);

    char command[command_buf] = {};
    snprintf(command, sizeof(command), "dot -Tpng graphiz/dot/dump2.dot -o %s", png_name);
    system(command);

    return OK;
}
