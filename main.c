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
            print_error_message("cd", "HOME not set");
            exit_status = EXIT_BUILTIN_FAILURE;
        } else {
            chdir(dir);
            if (errno) {
                print_error("chdir");
                exit_status = EXIT_BUILTIN_FAILURE;
            }
        }
    } else if (strcmp(tokens[0], "export") == 0) {
        // export.
        // Iterate through each name-value pair.
        for (size_t i = 1; ; ++i) {
            char *pair = tokens[i];
            if (!pair) {
                break;
            }
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
                print_error("setenv");
                exit_status = EXIT_BUILTIN_FAILURE;
                // Fail fast.
                break;
            }
        }
    } else if (strcmp(tokens[0], "unset") == 0) {
        // unset.
        // Iterate through each name-value pair.
        for (size_t i = 1; ; ++i) {
            char *name = tokens[i];
            if (!name) {
                break;
            }
            unsetenv(name);
            if (errno) {
                print_error("unsetenv");
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

void exec_system(char **tokens, bool wait) {

    // fork() and execvp().
    pid_t cpid = fork();
    if (errno) {
        print_error("fork");
        exit_status = EXIT_INTERNAL_FAILURE;
        return;
    }
    if (!cpid) {
        // Child process
        execvp(tokens[0], tokens);
        if (errno) {
            print_error(tokens[0]);
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
        print_error("waitpid");
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

void exec_command(char *command, bool wait) {

    // TODO: IO redirection.

    // Tokenize.
    // Make a copy so that the original command is not tempered by strtok().
    command = strdup(command);
    if (errno) {
        print_error("strdup");
        exit_status = EXIT_INTERNAL_FAILURE;
        return;
    }
    char **tokens = NULL;
    for (size_t argc = 0; ; ++argc) {
        tokens = realloc(tokens, (argc + 1) * sizeof(tokens[0]));
        if (!tokens) {
            print_error("realloc");
            exit_status = EXIT_INTERNAL_FAILURE;
            return;
        }
        if (argc == 0) {
            tokens[argc] = strtok(command, TOKEN_DELIMITERS);
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
            exec_system(tokens, wait);
        }
    }
    free(tokens);
    free(command);
}

void exec_line(char *line) {
    // Make a copy because we are going temper the string.
    line = strdup(line);
    if (errno) {
        print_error("strdup");
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
        exec_command(command, false);
        command = command_end + 1;
    }
    if (command[0]) {
        // Execute the last command with wait.
        exec_command(command, true);
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
