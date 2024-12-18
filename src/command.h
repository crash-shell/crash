#ifndef COMMAND_H_
    #define COMMAND_H_
    #include "cr_string.h"
    #include <stdbool.h>

bool command_execute(args_t *command, char **env, char **path)
__attribute__((nonnull(1, 2)));

args_t command_parse_args(char *buff);
#endif
