//
// Copyright (c) 2015 zh
// All rights reserved.
//

#ifndef ZHSH_LINE_SYNTAX_H
#define ZHSH_LINE_SYNTAX_H

#include <stdlib.h>

#include "util.h"

typedef struct {
    int left_fd;
    int type;
    // We are not using a union here, because we need to know whether we need to free file. Binding to the meaning of
    // type does not seem like a good idea.
    int *right_fd;
    char *right_file;
} redir_t;

redir_t *redir_alloc() {
    redir_t *redir = malloc(sizeof(redir_t));
    redir->right_file = NULL;
    return redir;
}

void redir_free(redir_t *redir) {
    free(redir->right_file);
    free(redir);
}

typedef struct {
    char **args;
    void **redirs;
} cmd_t;

cmd_t *cmd_alloc() {
    cmd_t *cmd = malloc(sizeof(cmd_t));
    cmd->args = strarr_alloc();
    cmd->redirs = ptrarr_alloc();
    return cmd;
}

void cmd_free(cmd_t *cmd) {
    strarr_free(cmd->args);
    ptrarr_free(cmd->redirs);
    free(cmd);
}

#endif //ZHSH_LINE_SYNTAX_H
