#ifndef ZHSH_LINE_SYNTAX_H
#define ZHSH_LINE_SYNTAX_H

typedef struct {
    int left_fd;
    int type;
    union {
        int *fd;
        char *file;
    } right;
} redir_t;

redir_t *redir_malloc() {
    redir_t *redir = malloc(sizeof(redir_t));
    redir->right.file = NULL;
}

void redir_free(redir_t *redir) {
    free(redir->right.file);
    free(redir);
}

typedef struct {
    char **args;
    redir_t **redirs;
} cmd_t;

cmd_t *cmd_malloc() {
    cmd_t *cmd = malloc(sizeof(cmd_t));
    cmd->args = NULL;
    cmd->redirs = NULL;
}

void cmd_free(cmd_t *cmd) {
    // TODO: Free args array content.
    free(cmd->args);
    // TODO: Free redirs array content.
    free(cmd->redirs);
    free(cmd);
}

#endif //ZHSH_LINE_SYNTAX_H
