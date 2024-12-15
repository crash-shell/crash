#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "command.h"

static
int ensure_args_capacity(args_t *command)
{
    char **ptr = NULL;

    if (command->count == command->cap - 1) {
        ptr = realloc(command->args, command->cap << 1);
        if (ptr == NULL)
            return 0;
        command->args = ptr;
    }
    for (size_t i = command->count; i < command->cap; i++)
        command->args[i] = NULL;
    return command->cap << 1;
}

args_t command_parse_args(char *buff)
{
    args_t command = { 0, .cap = 15, .count = 0 };

    command.args = malloc(sizeof *command.args * command.cap);
    if (command.args == NULL)
        return (args_t){ .args = NULL };
    for (size_t i = command.count; i < command.cap; i++)
        command.args[i] = NULL;
    for (char *tok = strtok(buff, " "); tok != NULL; tok = strtok(NULL, " ")) {
        command.cap = ensure_args_capacity(&command);
        if (!command.cap)
            return free(command.args), (args_t){ NULL, 0, 0 };
        command.args[command.count] = tok;
        command.count++;
    }
    return command;
}

static
void command_handler_errors(char *name)
{
    switch (errno) {
        case ENOENT:
            dprintf(STDERR_FILENO, "%s: Command not found.\n", name);
            break;
        default:
            dprintf(STDERR_FILENO, "execve: Unknown error\n");
    }
}

bool command_execute(args_t *command, char **env, char **path)
{
    int status;
    pid_t pid = fork();

    if (pid == 0) {
        if (command->args == NULL)
            return false;
        if (execve(command->args[0], command->args, NULL) == -1)
            command_handler_errors(command->args[0]);
    } else if (pid > 0) {
        waitpid(pid, &status, 0);
        return true;
    }
    return false;
}
