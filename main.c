#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <readline/readline.h>
#include <sys/wait.h>

#include "util.h"

#define TOKEN_DELIMITERS " \t\n\v\f\r"

#define EXIT_INTERNAL_FAILURE -1
#define EXIT_BUILTIN_FAILURE -2

int exit_status;

bool exec_builtin(char **tokens) {
    if (strcmp(tokens[0], "exit") == 0) {
        // exit.
        exit(EXIT_SUCCESS);
    } else if (strcmp(tokens[0], "cd") == 0) {
        // cd.
        // Try to cd to $HOME when no argument is given.
        char *dir = tokens[1] ? tokens[1] : getenv("HOME");
        if (!dir) {
            print_err_msg("cd", "HOME not set");
            exit_status = EXIT_BUILTIN_FAILURE;
        } else {
            chdir(dir);
            if (errno) {
                print_err("chdir");
                exit_status = EXIT_BUILTIN_FAILURE;
            }
        }
    } else if (strcmp(tokens[0], "export") == 0) {
        // export.
        // Iterate through each name-value pair.
        for (char **token_i = &tokens[1], *pair; (pair = *token_i); ++token_i) {
            // name=value is delimited by the first =, and no space is allowed around it, so it is within this token.
            char *delimiter = strchr(pair, '=');
            if (!delimiter) {
                // If = is not found, simply ignore the "pair".
                continue;
            }
            char *name = strndup(pair, delimiter - pair);
            char *value = delimiter + 1;
            // Don't use putenv(); it removes the environment variable if no = is found, contrary to common behavior of
            // export. And setenv() is required in POSIX while putenv() is not.
            // See also http://stackoverflow.com/questions/5873029/questions-about-putenv-and-setenv .
            setenv(name, value, true);
            free(name);
            if (errno) {
                print_err("setenv");
                exit_status = EXIT_BUILTIN_FAILURE;
                // Fail fast.
                break;
            }
        }
    } else if (strcmp(tokens[0], "unset") == 0) {
        // unset.
        // Iterate through each name-value pair.
        for (char **token_i = &tokens[1], *name; (name = *token_i); ++token_i) {
            unsetenv(name);
            if (errno) {
                print_err("unsetenv");
                exit_status = EXIT_BUILTIN_FAILURE;
                // Fail fast.
                break;
            }
        }
    } else {
        return false;
    }
    return true;
}

void exec_sys(char **tokens, int **fdmaps, bool wait) {

    // fork
    pid_t cpid = fork();
    if (errno) {
        print_err("fork");
        exit_status = EXIT_INTERNAL_FAILURE;
        return;
    }
    if (!cpid) {
        // Child process
        // I/O redirection with dup2
        for (int **fdmap_i = fdmaps, *fdmap; (fdmap = *fdmap_i); ++fdmap_i) {
            dup2(fdmap[1], fdmap[0]);
            if (errno) {
                print_err("dup2");
                exit(EXIT_INTERNAL_FAILURE);
            }
        }
        // exec
        execvp(tokens[0], tokens);
        if (errno) {
            print_err(tokens[0]);
            exit(EXIT_INTERNAL_FAILURE);
        }
    }

    if (!wait) {
        return;
    }
    // Wait for child process and set exit status.
    int status;
    waitpid(cpid, &status, WUNTRACED | WCONTINUED);
    if (errno) {
        print_err("waitpid");
        exit_status = EXIT_INTERNAL_FAILURE;
        return;
    }
    exit_status = WEXITSTATUS(status);
    if (WIFSIGNALED(status)) {
        int signal = WTERMSIG(status);
        if (WCOREDUMP(status)) {
            fprintf(stderr, "Core dumped by signal %d", signal);
        } else {
            fprintf(stderr, "Killed by signal %d", signal);
        }
    } else if (WIFSTOPPED(status)) {
        int signal = WSTOPSIG(status);
        fprintf(stderr, "Stopped by signal %d\n", signal);
    }
}

void exec_cmd(char *cmd, bool wait) {

    // TODO: IO redirection.

    // Tokenize.
    // Make a copy so that the original command is not tempered by strtok().
    cmd = strdup(cmd);
    if (errno) {
        print_err("strdup");
        exit_status = EXIT_INTERNAL_FAILURE;
        return;
    }
    char **tokens = NULL;
    for (size_t argc = 0; ; ++argc) {
        tokens = realloc(tokens, (argc + 1) * sizeof(tokens[0]));
        if (errno) {
            print_err("realloc");
            exit_status = EXIT_INTERNAL_FAILURE;
            return;
        }
        if (argc == 0) {
            tokens[argc] = strtok(cmd, TOKEN_DELIMITERS);
        } else {
            tokens[argc] = strtok(NULL, TOKEN_DELIMITERS);
        }
        if (!tokens[argc]) {
            // tokens has been null terminated now.
            break;
        }
    }

    if (tokens[0]) {
        // tokens is not empty, execute the command.
        // Default to EXIT_SUCCESS.
        exit_status = EXIT_SUCCESS;
        if (!exec_builtin(tokens)) {
            exec_sys(tokens, wait);
        }
    }
    free(tokens);
    free(cmd);
}

void exec_line(char *line) {
    // Make a copy because we are going to tamper the string.
    line = strdup(line);
    if (errno) {
        print_err("strdup");
        exit_status = EXIT_INTERNAL_FAILURE;
        return;
    }
    char *command = line;
    // Execute commands in background.
    while (true) {
        char *command_end = strchr(command, '&');
        if (!command_end) {
            // & not found, it's the last command or empty.
            break;
        }
        command_end[0] = '\0';
        exec_cmd(command, false);
        command = command_end + 1;
    }
    if (command[0]) {
        // Execute the last command with wait.
        exec_cmd(command, true);
    }
    free(line);
}

void loop() {
    char *prompt = "zhsh $ ";
    char *line = readline(prompt);
    if (!line) {
        // EOF encountered with empty line, exit this shell.
        fprintf(stderr, "exit\n");
        exit(EXIT_SUCCESS);
    }
    exec_line(line);
    free(line);
}

int main(int argc, char *argv[]) {
    while (true) {
        loop();
    }
    // Never gets here.
    return EXIT_FAILURE;
}
