%include {
    #include "line_parser.h"

    #include <assert.h>
    #include <errno.h>
    #include <stdbool.h>
    #include <stdlib.h>
    #include <string.h>

    #include <unistd.h>

    #include "line_syntax.h"
    #include "util.h"

    #define BREAK_IF_ERRNO \
        if (errno) { \
            break; \
        }
}

%name LineParser

%token_type { char * }
%token_destructor {
    free($$);
}

%type redirection { redir_t * }
%destructor redirection {
    redir_free($$);
}
%type command { cmd_t * }
%destructor command {
    cmd_free($$);
}
%type commandList { cmd_list_t * }
%destructor commandList {
    cmd_list_free($$);
}
%type start { void * }
%destructor start {}

%extra_argument { cmd_list_t **cmd_list_p }

%parse_failure {
    errno = EINVAL;
}

%start_symbol start

redirection(REDIR) ::= REDIRECT_INPUT_FROM_FILE(RIFF) ARGUMENT(ARG) . {
    BREAK_IF_ERRNO
    REDIR = redir_alloc();
    BREAK_IF_ERRNO
    REDIR->left_fd = redir_parse_left_fd(RIFF, 1, STDIN_FILENO);
    REDIR->type = REDIRECT_INPUT_FROM_FILE;
    REDIR->right_file = ARG;
}
redirection(REDIR) ::= REDIRECT_INPUT_FROM_FILE_DESCRIPTOR(RIFFD) ARGUMENT(ARG) . {
    BREAK_IF_ERRNO
    REDIR = redir_alloc();
    BREAK_IF_ERRNO
    REDIR->left_fd = redir_parse_left_fd(RIFFD, 2, STDIN_FILENO);
    REDIR->type = REDIRECT_INPUT_FROM_FILE_DESCRIPTOR;
    REDIR->right_fd = redir_parse_fd(ARG);
    free(ARG);
}
redirection(REDIR) ::= REDIRECT_OUTPUT_TO_FILE(ROTF) ARGUMENT(ARG) . {
    BREAK_IF_ERRNO
    REDIR = redir_alloc();
    BREAK_IF_ERRNO
    REDIR->left_fd = redir_parse_left_fd(ROTF, 1, STDOUT_FILENO);
    REDIR->type = REDIRECT_OUTPUT_TO_FILE;
    REDIR->right_file = ARG;
}
redirection(REDIR) ::= REDIRECT_OUTPUT_TO_FILE_DESCRIPTOR(ROTFD) ARGUMENT(ARG) . {
    BREAK_IF_ERRNO
    REDIR = redir_alloc();
    BREAK_IF_ERRNO
    REDIR->left_fd = redir_parse_left_fd(ROTFD, 2, STDOUT_FILENO);
    REDIR->type = REDIRECT_OUTPUT_TO_FILE_DESCRIPTOR;
    REDIR->right_fd = redir_parse_fd(ARG);
    free(ARG);
}
redirection(REDIR) ::= REDIRECT_OUTPUT_APPEND_TO_FILE(ROATF) ARGUMENT(ARG) . {
    BREAK_IF_ERRNO
    REDIR = redir_alloc();
    BREAK_IF_ERRNO
    REDIR->left_fd = redir_parse_left_fd(ROATF, 2, STDOUT_FILENO);
    REDIR->type = REDIRECT_OUTPUT_APPEND_TO_FILE;
    REDIR->right_file = ARG;
}

command(CMD) ::= ARGUMENT(ARG) . {
    BREAK_IF_ERRNO
    CMD = cmd_alloc();
    BREAK_IF_ERRNO
    strarr_append(&(CMD->args), ARG);
}
command(CMD) ::= redirection(REDIR) . {
    BREAK_IF_ERRNO
    CMD = cmd_alloc();
    BREAK_IF_ERRNO
    ptrarr_append(&(CMD->redirs), REDIR);
}
command(CMD) ::= command(CMD_) ARGUMENT(ARG) . {
    BREAK_IF_ERRNO
    CMD = CMD_;
    strarr_append(&(CMD->args), ARG);
}
command(CMD) ::= command(CMD_) redirection(REDIR) . {
    BREAK_IF_ERRNO
    CMD = CMD_;
    ptrarr_append(&(CMD->redirs), REDIR);
}

commandList(CMD_LIST) ::= command(CMD) . {
    BREAK_IF_ERRNO
    CMD_LIST = cmd_list_alloc();
    BREAK_IF_ERRNO
    ptrarr_append(&(CMD_LIST->cmds), CMD);
}
commandList(CMD_LIST) ::= commandList(CMD_LIST_) PIPE command(CMD) . {
    BREAK_IF_ERRNO
    CMD_LIST = CMD_LIST_;
    intarr_append(&(CMD_LIST->ops), PIPE);
    ptrarr_append(&(CMD_LIST->cmds), CMD);
}
commandList(CMD_LIST) ::= commandList(CMD_LIST_) OR command(CMD) . {
    BREAK_IF_ERRNO
    CMD_LIST = CMD_LIST_;
    intarr_append(&(CMD_LIST->ops), OR);
    ptrarr_append(&(CMD_LIST->cmds), CMD);
}
commandList(CMD_LIST) ::= commandList(CMD_LIST_) AND command(CMD) . {
    BREAK_IF_ERRNO
    CMD_LIST = CMD_LIST_;
    intarr_append(&(CMD_LIST->ops), AND);
    ptrarr_append(&(CMD_LIST->cmds), CMD);
}
commandList(CMD_LIST) ::= commandList(CMD_LIST_) BACKGROUND command(CMD). {
    BREAK_IF_ERRNO
    CMD_LIST = CMD_LIST_;
    intarr_append(&(CMD_LIST->ops), BACKGROUND);
    ptrarr_append(&(CMD_LIST->cmds), CMD);
}
commandList(CMD_LIST) ::= commandList(CMD_LIST_) BACKGROUND . {
    BREAK_IF_ERRNO
    CMD_LIST = CMD_LIST_;
    intarr_append(&(CMD_LIST->ops), BACKGROUND);
}
commandList(CMD_LIST) ::= commandList(CMD_LIST_) SEMICOLON command(CMD). {
    BREAK_IF_ERRNO
    CMD_LIST = CMD_LIST_;
    intarr_append(&(CMD_LIST->ops), SEMICOLON);
    ptrarr_append(&(CMD_LIST->cmds), CMD);
}
commandList(CMD_LIST) ::= commandList(CMD_LIST_) SEMICOLON . {
    BREAK_IF_ERRNO
    CMD_LIST = CMD_LIST_;
    intarr_append(&(CMD_LIST->ops), SEMICOLON);
}

start ::= commandList(CMD_LIST) . {
    BREAK_IF_ERRNO
    // Save our AST from being freed by Lemon!
    *cmd_list_p = CMD_LIST;
}
