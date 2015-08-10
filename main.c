#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

#include <readline/readline.h>

#include "line_parser.h"
#include "line_syntax.h"
#include "parser.h"
#include "util.h"

static int exit_status;

#define ZHSH_EXIT_INTERNAL_FAILURE -1
#define ZHSH_EXIT_PARSER_FAILURE -2
#define ZHSH_EXIT_BUILTIN_FAILURE -3

// FIXME: errno resume.

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
            exit_status = ZHSH_EXIT_BUILTIN_FAILURE;
        } else {
            chdir(dir);
            if (errno) {
                print_err("chdir");
                exit_status = ZHSH_EXIT_BUILTIN_FAILURE;
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
                exit_status = ZHSH_EXIT_BUILTIN_FAILURE;
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
                exit_status = ZHSH_EXIT_BUILTIN_FAILURE;
                // Fail fast.
                break;
            }
        }
    } else {
        return false;
    }
    return true;
}

void exec_sys(char **tokens, void **fdmaps, bool wait) {

    // fork
    pid_t cpid = fork();
    if (errno) {
        print_err("fork");
        exit_status = ZHSH_EXIT_INTERNAL_FAILURE;
        return;
    }
    if (!cpid) {
        // Child process
        // I/O redirection with dup2, fdmap[0] will be the same as fdmap[1].
        for (void **fdmap_i = fdmaps, *fdmap_; (fdmap_ = *fdmap_i); ++fdmap_i) {
            int *fdmap = (int *)fdmap_;
            dup2(fdmap[1], fdmap[0]);
            if (errno) {
                print_err("dup2");
                exit(ZHSH_EXIT_INTERNAL_FAILURE);
            }
        }
        // exec
        execvp(tokens[0], tokens);
        if (errno) {
            print_err(tokens[0]);
            exit(ZHSH_EXIT_INTERNAL_FAILURE);
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
        exit_status = ZHSH_EXIT_INTERNAL_FAILURE;
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

void exec_cmd(cmd_t *cmd, bool wait) {

    // Build file descriptor map from redir_t.
    void **fdmaps = ptrarr_alloc();
    if (errno) {
        print_err("ptrarr_alloc");
        exit_status = ZHSH_EXIT_INTERNAL_FAILURE;
        return;
    }
    for (void **redir_i = cmd->redirs, *redir_; (redir_ = *redir_i); ++redir_i) {
        redir_t *redir = (redir_t *) redir_;
        // fdmap[0] will be the same as fdmap[1]
        int *fdmap = malloc(2 * sizeof(int));
        fdmap[0] = redir->left_fd;
        fdmap[1] = -1;
        switch (redir->type) {
            case REDIRECT_INPUT_FROM_FILE:
                fdmap[1] = open(redir->right_file, O_RDONLY | O_CLOEXEC);
                if (errno) {
                    print_err("open");
                    exit_status = ZHSH_EXIT_INTERNAL_FAILURE;
                    // FIXME: Should free fds opened by this operation.
                    ptrarr_free(fdmaps, free);
                    return;
                }
                break;
            case REDIRECT_INPUT_FROM_FILE_DESCRIPTOR:
                fdmap[1] = redir->right_fd;
                break;
            case REDIRECT_OUTPUT_TO_FILE:
                fdmap[1] = open(redir->right_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
                if (errno) {
                    print_err("open");
                    exit_status = ZHSH_EXIT_INTERNAL_FAILURE;
                    ptrarr_free(fdmaps, free);
                    return;
                }
                break;
            case REDIRECT_OUTPUT_TO_FILE_DESCRIPTOR:
                fdmap[1] = redir->right_fd;
                break;
            case REDIRECT_OUTPUT_APPEND_TO_FILE:
                fdmap[1] = open(redir->right_file, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU | S_IRWXG | S_IRWXO);
                if (errno) {
                    print_err("open");
                    exit_status = ZHSH_EXIT_INTERNAL_FAILURE;
                    ptrarr_free(fdmaps, free);
                    return;
                }
                break;
            case REDIRECT_OUTPUT_APPEND_TO_FILE_DESCRIPTOR:
                fdmap[1] = redir->right_fd;
                break;
            default:
                errno = EINVAL;
                print_err("redir->type");
                exit_status = ZHSH_EXIT_INTERNAL_FAILURE;
                ptrarr_free(fdmaps, free);
                return;
        }
        ptrarr_append(&fdmaps, fdmap);
    }

    // Execute command.
    // Default exit status to EXIT_SUCCESS.
    exit_status = EXIT_SUCCESS;
    if (!exec_builtin(cmd->args)) {
        exec_sys(cmd->args, fdmaps, wait);
    }
    // FIXME: Should close the fds opened by this function.
}

void exec_line(char *line) {

    // No-op if the line is empty.
    // TODO.

    // Parse the line.
    cmd_list_t *cmd_list = parse_line(line);
    if (errno) {
        print_err("parser");
        exit_status = ZHSH_EXIT_PARSER_FAILURE;
        return;
    }

    // Execute the line.
    for (size_t i = 0; ; ++i) {

        cmd_t *cmd = cmd_list->cmds[i];
        if (!cmd) {
            break;
        }
        int op = -1;
        if (i < cmd_list->op_len) {
            op = cmd_list->ops[i];
        }

        // TODO: Pipe.
        //bool pipe = op == PIPE;
        bool wait = op != BACKGROUND;
        bool break_on_success = op == OR;
        bool break_on_failure = op == AND;

        exec_cmd(cmd, wait);
        if (errno) {
            // TODO
        }
        if (exit_status == EXIT_SUCCESS) {
            if (break_on_success) {
                break;
            }
        } else {
            if (break_on_failure) {
                break;
            }
        }
    }

    cmd_list_free(cmd_list);
}

void rep() {
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
        errno = 0;
        rep();
    }
    // Never gets here.
    return EXIT_FAILURE;
}
