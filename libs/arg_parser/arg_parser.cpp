#include <assert.h>
#include <string.h>

#include "arg_parser.h"
#include <string.h>
#include <stdio.h>

ParseError ParseArguments(int argc, const char* argv[], ArgOption options[], size_t options_count)
{
    //TODO processing func
    for (int i = 1; i < argc; i++)
    {
        bool option_found = false;

        for (size_t j = 0; j < options_count; j++)
        {
            if ((options[j].short_name && strcmp(argv[i], options[j].short_name) == 0) ||
                (options[j].long_name  && strcmp(argv[i], options[j].long_name)  == 0))
            {
                options[j].is_present = true;
                option_found = true;

                if (options[j].requires_arg)
                {
                    if (i + 1 >= argc)
                    {
                        fprintf(stderr, "Error: option %s need arg!\n", argv[i]);
                        options[j].is_present = false;
                        return PARSE_ERROR;
                    }
                    options[j].argument = argv[++i];
                }
                break;
            }
        }

        if (!option_found)
        {
            fprintf(stderr, "Warning: unknow arg %s\n", argv[i]);
        }
    }

    return PARSE_OK;
}

