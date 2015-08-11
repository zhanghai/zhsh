#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

#include <readline/history.h>
#include <readline/readline.h>

#include "line_parser.h"
#include "line_syntax.h"
#include "parser.h"
#include "util.h"

static int exit_status;

#define ZHSH_EXIT_INTERNAL_FAILURE -1
#define ZHSH_EXIT_PARSER_FAILURE -2
#define ZHSH_EXIT_BUILTIN_FAILURE -3

void init() {
    // Set SHELL.
    char *shell = readlink_malloc("/proc/self/exe");
    if (errno) {
        print_err("readlink_malloc");
    } else {
        setenv("SHELL", shell, true);
        if (errno) {
            print_err("setenv");
        }
        free(shell);
    }
    errno = 0;
}

char *get_prompt_and_set_title() {

    // Basic information.
    char *euname = geteuname();
    if (errno) {
        print_err("geteuname");
        errno = 0;
    }
    char *hostname = gethostname_malloc();
    if (errno) {
        print_err("gethostname_malloc");
        errno = 0;
    }
    char *cwd = getcwd_malloc();
    if (errno) {
        print_err("getcwd_malloc");
        errno = 0;
    }

    // Set title.
    printf("\033]2;%s@%s:%s\007", euname, hostname, cwd);

    // Build prompt.
// RL_PROMPT_START_IGNORE = '\001', and RL_PROMPT_END_IGNORE = '\002'.
#define ESC_BOLD_GREEN "\001\033[01;32m\002"
#define ESC_BOLD_BLUE "\001\033[01;34m\002"
#define ESC_BOLD_RED "\001\033[01;31m\002"
#define ESC_RESET "\001\033[00m\002"

    char *exit_status_face = exit_status != EXIT_SUCCESS ? " " ESC_BOLD_RED ":(" ESC_BOLD_BLUE : "";

    char *prompt;
    if (geteuid() == 0) {
        prompt = sprintf_malloc(ESC_BOLD_RED "%s" ESC_BOLD_BLUE " %s%s #" ESC_RESET " ", hostname, cwd,
                                exit_status_face);
    } else {
        prompt = sprintf_malloc(ESC_BOLD_GREEN "%s@%s" ESC_BOLD_BLUE " %s%s $" ESC_RESET " ", euname, hostname, cwd,
                                exit_status_face);
    }

#undef ESC_BOLD_GREEN
#undef ESC_BOLD_BLUE
#undef ESC_BOLD_RED
#undef ESC_RESET

    free(hostname);
    free(cwd);

    return prompt;
}

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
            } else {
                char *old_pwd = getenv("PWD");
                if (errno) {
                    print_err("getenv");
                    exit_status = ZHSH_EXIT_BUILTIN_FAILURE;
                } else {
                    setenv("OLDPWD", old_pwd, true);
                    if (errno) {
                        print_err("getenv");
                        exit_status = ZHSH_EXIT_BUILTIN_FAILURE;
                        // But still we want to set PWD correctly.
                    }
                }
                int old_errno = errno;
                errno = 0;
                setenv("PWD", dir, true);
                if (errno) {
                    print_err("setenv");
                    exit_status = ZHSH_EXIT_BUILTIN_FAILURE;
                } else {
                    errno = old_errno;
                }
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
            // setenv() also makes copy of name and value passed in, contrary to putenv().
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

void close_fds(intarr_t *fds) {
    for (size_t i = 0; i < fds->len; ++i) {
        // Debug
        //fprintf(stderr, "close(%d)\n", fds->arr[i]);
        close(fds->arr[i]);
    }
}

void exec_sys(char **tokens, void **fdmaps, intarr_t *fds_to_close, bool wait) {

    // Fork
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
            // Debug
            //fprintf(stderr, "dup2(%d, %d)\n", fdmap[1], fdmap[0]);
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

    // Close file descriptors before we wait.
    close_fds(fds_to_close);
    if (errno) {
        print_err("close_fds");
        exit_status = ZHSH_EXIT_INTERNAL_FAILURE;
        return;
    }

    if (!wait) {
        // FIXME: Should kill zombies.
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

typedef struct {
    bool redir_stdin;
    int redir_stdin_fd;
    bool redir_stdout;
    int redir_stdout_fd;
} pipe_redir_t;

void exec_cmd(cmd_t *cmd, pipe_redir_t pipe_redir, intarr_t *fds_to_close, bool wait) {

    // Build file descriptor map from pipe_redir_t and redir_t.
    void **fdmaps = ptrarr_alloc();
    if (errno) {
        print_err("ptrarr_alloc");
        exit_status = ZHSH_EXIT_INTERNAL_FAILURE;
        close_fds(fds_to_close);
        return;
    }
    // pipe_redir_t
    if (pipe_redir.redir_stdin) {
        // fdmap[0] will be the same as fdmap[1]
        int *fdmap = malloc(2 * sizeof(int));
        if (errno) {
            print_err("malloc");
            exit_status = ZHSH_EXIT_INTERNAL_FAILURE;
            close_fds(fds_to_close);
            ptrarr_free(fdmaps, free);
            return;
        }
        fdmap[0] = STDIN_FILENO;
        fdmap[1] = pipe_redir.redir_stdin_fd;
        ptrarr_append(&fdmaps, fdmap);
        if (errno) {
            print_err("ptrarr_append");
            exit_status = ZHSH_EXIT_INTERNAL_FAILURE;
            close_fds(fds_to_close);
            ptrarr_free(fdmaps, free);
            return;
        }
    }
    if (pipe_redir.redir_stdout) {
        // fdmap[0] will be the same as fdmap[1]
        int *fdmap = malloc(2 * sizeof(int));
        if (errno) {
            print_err("malloc");
            exit_status = ZHSH_EXIT_INTERNAL_FAILURE;
            close_fds(fds_to_close);
            ptrarr_free(fdmaps, free);
            return;
        }
        fdmap[0] = STDOUT_FILENO;
        fdmap[1] = pipe_redir.redir_stdout_fd;
        ptrarr_append(&fdmaps, fdmap);
        if (errno) {
            print_err("ptrarr_append");
            exit_status = ZHSH_EXIT_INTERNAL_FAILURE;
            close_fds(fds_to_close);
            ptrarr_free(fdmaps, free);
            return;
        }
    }
    // redir_t
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
                    close_fds(fds_to_close);
                    ptrarr_free(fdmaps, free);
                    return;
                }
                intarr_append(fds_to_close, fdmap[1]);
                break;
            case REDIRECT_INPUT_FROM_FILE_DESCRIPTOR:
                fdmap[1] = redir->right_fd;
                break;
            case REDIRECT_OUTPUT_TO_FILE:
                fdmap[1] = open(redir->right_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
                if (errno) {
                    print_err("open");
                    exit_status = ZHSH_EXIT_INTERNAL_FAILURE;
                    close_fds(fds_to_close);
                    ptrarr_free(fdmaps, free);
                    return;
                }
                intarr_append(fds_to_close, fdmap[1]);
                break;
            case REDIRECT_OUTPUT_TO_FILE_DESCRIPTOR:
                fdmap[1] = redir->right_fd;
                break;
            case REDIRECT_OUTPUT_APPEND_TO_FILE:
                fdmap[1] = open(redir->right_file, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU | S_IRWXG | S_IRWXO);
                if (errno) {
                    print_err("open");
                    exit_status = ZHSH_EXIT_INTERNAL_FAILURE;
                    close_fds(fds_to_close);
                    ptrarr_free(fdmaps, free);
                    return;
                }
                intarr_append(fds_to_close, fdmap[1]);
                break;
            case REDIRECT_OUTPUT_APPEND_TO_FILE_DESCRIPTOR:
                fdmap[1] = redir->right_fd;
                break;
            default:
                errno = EINVAL;
                print_err("redir->type");
                exit_status = ZHSH_EXIT_INTERNAL_FAILURE;
                close_fds(fds_to_close);
                ptrarr_free(fdmaps, free);
                return;
        }
        ptrarr_append(&fdmaps, fdmap);
        if (errno) {
            print_err("ptrarr_append");
            exit_status = ZHSH_EXIT_INTERNAL_FAILURE;
            close_fds(fds_to_close);
            ptrarr_free(fdmaps, free);
            return;
        }
    }

    // Execute command.
    // Default exit status to EXIT_SUCCESS.
    exit_status = EXIT_SUCCESS;
    if (!exec_builtin(cmd->args)) {
        exec_sys(cmd->args, fdmaps, fds_to_close, wait);
    }

    ptrarr_free(fdmaps, free);
}

void exec_line(char *line) {

    // Parse the line.
    cmd_list_t *cmd_list = parse_line(line);
    if (errno) {
        print_err("parser");
        exit_status = ZHSH_EXIT_PARSER_FAILURE;
        return;
    }

    // Execute the line.
    bool pipe_redir_next_stdin = false;
    int pipe_redir_next_stdin_fd = -1;
    for (size_t i = 0; ; ++i) {

        cmd_t *cmd = cmd_list->cmds[i];
        if (!cmd) {
            break;
        }
        int op = -1;
        if (i < cmd_list->ops.len) {
            op = cmd_list->ops.arr[i];
        }

        // Handle operator.
        bool do_pipe_redir = op == PIPE;
        bool wait = op != BACKGROUND && !do_pipe_redir;
        bool stop_on_success = op == OR;
        bool stop_on_failure = op == AND;

        // Handle piping.
        int pipe_fds[2] = {-1, -1};
        if (do_pipe_redir) {
            pipe(pipe_fds);
            if (errno) {
                print_err("pipe");
                exit_status = ZHSH_EXIT_INTERNAL_FAILURE;
                break;
            }
            // Debug
            //fprintf(stderr, "pipe(%d, %d)\n", pipe_fds[0], pipe_fds[1]);
            fcntl(pipe_fds[0], F_SETFD, FD_CLOEXEC);
            if (errno) {
                print_err("fcntl");
                exit_status = ZHSH_EXIT_INTERNAL_FAILURE;
                break;
            }
            fcntl(pipe_fds[1], F_SETFD, FD_CLOEXEC);
            if (errno) {
                print_err("fcntl");
                exit_status = ZHSH_EXIT_INTERNAL_FAILURE;
                break;
            }
        }
        pipe_redir_t pipe_redir = {pipe_redir_next_stdin, pipe_redir_next_stdin_fd, do_pipe_redir, pipe_fds[1]};

        // Init fds_to_close.
        intarr_t fds_to_close;
        intarr_init(&fds_to_close);
        if (pipe_redir.redir_stdin) {
            intarr_append(&fds_to_close, pipe_redir.redir_stdin_fd);
        }
        if (pipe_redir.redir_stdout) {
            intarr_append(&fds_to_close, pipe_redir.redir_stdout_fd);
        }

        exec_cmd(cmd, pipe_redir, &fds_to_close, wait);

        intarr_fin(&fds_to_close);

        if (errno) {
            // Error occurred and printed in exec_cmd, simply break out this loop and we will return.
            break;
        }
        if (exit_status == EXIT_SUCCESS) {
            if (stop_on_success) {
                break;
            }
        } else {
            if (stop_on_failure) {
                break;
            }
        }

        pipe_redir_next_stdin = do_pipe_redir;
        pipe_redir_next_stdin_fd = pipe_fds[0];
    }

    cmd_list_free(cmd_list);
}

void rep() {
    char *prompt = get_prompt_and_set_title();
    char *line = readline(prompt);
    free(prompt);
    if (!line) {
        // EOF encountered with empty line, exit this shell.
        fprintf(stderr, "exit\n");
        exit(EXIT_SUCCESS);
    }
    // No-op if the line is empty.
    if (strspn(line, " \t\v\f") != strlen(line)) {
        exec_line(line);
        add_history(line);
    }
    free(line);
    errno = 0;
}

int main(int argc, char *argv[]) {
    init();
    while (true) {
        rep();
    }
    // Never gets here.
    return EXIT_FAILURE;
}
