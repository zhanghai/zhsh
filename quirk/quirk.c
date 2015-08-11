//
// Copyright (c) 2015 zh
// All rights reserved.
//

#include "quirk.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "echo.h"
#include "env.h"
#include "help.h"
#include "ls.h"

const char *BUILTINS[] = {
        "cd",
        "exit",
        "export",
        "logout",
        "unset",
        // Quirk builtin begin.
        "clr",
        "dir",
        "echo",
        "environ",
        "help",
        "quit",
        // Quirk builtin end.
        NULL
};

void clr_exec_func(char **argv) {
    printf("\033[H\033[2J");
    exit(EXIT_SUCCESS);
}

void dir_exec_func(char **argv) {
    exit(ls((int) strarr_len(argv), argv));
}

void environ_exec_func(char **argv) {
    exit(env());
}

void echo_exec_func(char **argv) {
    exit(echo(argv));
}

void help_exec_func(char **argv) {
    exit(help());
}

void exec_builtin_quirk(char **argv, void **fdmaps, intarr_t *fds_to_close, bool wait) {
    if (strcmp(argv[0], "clr") == 0) {
        // clr.
        exec_fork(fdmaps, fds_to_close, wait, clr_exec_func, argv);
        return;
    } else if (strcmp(argv[0], "dir") == 0) {
        // dir.
        exec_fork(fdmaps, fds_to_close, wait, dir_exec_func, argv);
        return;
    } else if (strcmp(argv[0], "echo") == 0) {
        // echo.
        exec_fork(fdmaps, fds_to_close, wait, echo_exec_func, argv);
        return;
    } else if (strcmp(argv[0], "environ") == 0) {
        // environ.
        exec_fork(fdmaps, fds_to_close, wait, environ_exec_func, argv);
        return;
    } else if (strcmp(argv[0], "help") == 0) {
        // help.
        exec_fork(fdmaps, fds_to_close, wait, help_exec_func, argv);
        return;
    } else if (strcmp(argv[0], "quit") == 0) {
        // quit.
        exit(EXIT_SUCCESS);
    } else {
        print_err_msg(argv[0], "no matching builtin found");
        exit_status = ZHSH_EXIT_INTERNAL_FAILURE;
    }

    // Close file descriptors for those without exec_fork (otherwise they should call return above).
    close_fds(fds_to_close);
}
