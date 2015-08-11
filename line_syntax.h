//
// Copyright (c) 2015 zh
// All rights reserved.
//

#ifndef ZHSH_LINE_SYNTAX_H
#define ZHSH_LINE_SYNTAX_H

#include <stddef.h>

#include "util.h"

typedef struct {
    int left_fd;
    int type;
    // We are not using a union here, because we need to know whether we need to free file. Binding to the meaning of
    // type does not seem like a good idea.
    int right_fd;
    char *right_file;
} redir_t;

redir_t * redir_alloc();

void redir_free(redir_t *redir);

int redir_parse_fd(const char *redir_token);

int redir_parse_left_fd(const char *redir_token, size_t redir_len, int def_fd);

typedef struct {
    char **args;
    void **redirs;
} cmd_t;

cmd_t * cmd_alloc();

void cmd_free_redir_func(void *redir);

void cmd_free(cmd_t *cmd);

typedef struct {
    void **cmds;
    intarr_t ops;
} cmd_list_t;

cmd_list_t * cmd_list_alloc();

void cmd_list_append_op(cmd_list_t *cmd_list, int op);

void cmd_list_free_cmd_func(void *cmd);

void cmd_list_free(cmd_list_t *cmd_list);

#endif //ZHSH_LINE_SYNTAX_H
