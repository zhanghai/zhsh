//
// Copyright (c) 2015 zh
// All rights reserved.
//

#include "line_syntax.h"

#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

redir_t *redir_alloc() {
    redir_t *redir = malloc(sizeof(redir_t));
    if (errno) {
        return NULL;
    }
    redir->right_file = NULL;
    return redir;
}

void redir_free(redir_t *redir) {
    if (redir == NULL) {
        return;
    }
    free(redir->right_file);
    free(redir);
}

int redir_parse_fd(const char *redir_token) {
    char *strtol_end;
    long fd_l = strtol(redir_token, &strtol_end, 10);
    if (!errno) {
        if (strtol_end == redir_token) {
            errno = EINVAL;
        } else if (fd_l > INT_MAX || fd_l < INT_MIN) {
            errno = ERANGE;
        }
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
    if (cmd == NULL) {
        return;
    }
    strarr_free(cmd->args);
    ptrarr_free(cmd->redirs, cmd_free_redir_func);
    free(cmd);
}

cmd_list_t *cmd_list_alloc() {
    cmd_list_t *cmd_list = malloc(sizeof(cmd_list_t));
    if (errno) {
        return NULL;
    }
    cmd_list->cmds = ptrarr_alloc();
    intarr_init(&(cmd_list->ops));
    return cmd_list;
}

void cmd_list_free_cmd_func(void *cmd) {
    cmd_free((cmd_t *) cmd);
}

void cmd_list_free(cmd_list_t *cmd_list) {
    if (cmd_list == NULL) {
        return;
    }
    ptrarr_free(cmd_list->cmds, cmd_list_free_cmd_func);
    intarr_fin(&(cmd_list->ops));
    free(cmd_list);
}
