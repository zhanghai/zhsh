//
// Copyright (c) 2015 zh
// All rights reserved.
//

#ifndef ZHSH_SHELL_H
#define ZHSH_SHELL_H

// Interface exposed for quirk.h

#include <stdbool.h>

#include "util.h"

extern int exit_status;

// The same as normal EXIT_FAILURE.
#define ZHSH_EXIT_INTERNAL_FAILURE 1
#define ZHSH_EXIT_PARSER_FAILURE 2
#define ZHSH_EXIT_BUILTIN_FAILURE 3

void close_fds(intarr_t *fds);

typedef void (*exec_func_t)(char **argv);

void exec_fork(void **fdmaps, intarr_t *fds_to_close, bool wait, exec_func_t exec_func, char **argv);

#endif //ZHSH_SHELL_H
