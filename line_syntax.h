//
// Copyright (c) 2015 zh
// All rights reserved.
//

#ifndef ZHSH_LINE_SYNTAX_H
#define ZHSH_LINE_SYNTAX_H

#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#include "util.h"

typedef struct {
    int left_fd;
    int type;
    // We are not using a union here, because we need to know whether we need to free file. Binding to the meaning of
    // type does not seem like a good idea.
    int right_fd;
    char *right_file;
} redir_t;

redir_t *redir_alloc() {
    redir_t *redir = malloc(sizeof(redir_t));
    if (errno) {
        return NULL;
    }
    redir->right_file = NULL;
    return redir;
}

void redir_free(redir_t *redir) {
    free(redir->right_file);
    free(redir);
}

int redir_parse_fd(const char *redir_token) {
    long fd_l = strtol(redir_token, NULL, 0);
    if (!errno && (fd_l > INT_MAX || fd_l < INT_MIN)) {
        errno = ERANGE;
    }
    if (errno) {
        return -1;
    }
    return (int) fd_l;
}

int redir_parse_left_fd(const char *redir_token, size_t redir_len, int def_fd) {
    if (strlen(redir_token) == redir_len) {
        return def_fd;
    } else {
        return redir_parse_fd(redir_token);
    }
}

typedef struct {
    char **args;
    void **redirs;
} cmd_t;

cmd_t *cmd_alloc() {
    cmd_t *cmd = malloc(sizeof(cmd_t));
    if (errno) {
        return NULL;
    }
    cmd->args = strarr_alloc();
    cmd->redirs = ptrarr_alloc();
    return cmd;
}

void cmd_free_redir_func(void *redir) {
    redir_free((redir_t *) redir);
}

void cmd_free(cmd_t *cmd) {
    strarr_free(cmd->args);
    ptrarr_free(cmd->redirs, cmd_free_redir_func);
    free(cmd);
}

typedef struct {
    void **cmds;
    int *ops;
    size_t op_len;
} cmd_list_t;

cmd_list_t *cmd_list_alloc() {
    cmd_list_t *cmd_list = malloc(sizeof(cmd_list_t));
    if (errno) {
        return NULL;
    }
    cmd_list->cmds = ptrarr_alloc();
    cmd_list->ops = NULL;
    cmd_list->op_len = 0;
    return cmd_list;
}

void cmd_list_append_op(cmd_list_t *cmd_list, int op) {
    cmd_list->ops = realloc(cmd_list->ops, cmd_list->op_len + 1);
    if (errno) {
        return;
    }
    cmd_list->ops[cmd_list->op_len] = op;
    ++(cmd_list->op_len);
}

void cmd_list_free_cmd_func(void *cmd) {
    cmd_free((cmd_t *) cmd);
}

void cmd_list_free(cmd_list_t *cmd_list) {
    ptrarr_free(cmd_list->cmds, cmd_list_free_cmd_func);
    free(cmd_list->ops);
    free(cmd_list);
}

#endif //ZHSH_LINE_SYNTAX_H
