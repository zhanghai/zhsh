%include {
    #include "line_parser.h"

    #include <assert.h>
    #include <errno.h>
    #include <stdlib.h>
    #include <string.h>

    #include "line_syntax.h"
    #include "util.h"
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
    REDIR = redir_alloc();
    // FIXME: Should add a test on errno here for ENOMEM.
    // errno is set, so on error this will be freed.
    REDIR->left_fd = redir_parse_left_fd(RIFF, 1, 0);
    REDIR->type = REDIRECT_INPUT_FROM_FILE;
    REDIR->right_file = ARG;
}
redirection(REDIR) ::= REDIRECT_INPUT_FROM_FILE_DESCRIPTOR(RIFFD) ARGUMENT(ARG) . {
    REDIR = redir_alloc();
    REDIR->left_fd = redir_parse_left_fd(RIFFD, 2, 0);
    REDIR->type = REDIRECT_INPUT_FROM_FILE_DESCRIPTOR;
    REDIR->right_fd = redir_parse_fd(ARG);
    free(ARG);
}
redirection(REDIR) ::= REDIRECT_OUTPUT_TO_FILE(ROTF) ARGUMENT(ARG) . {
    REDIR = redir_alloc();
    REDIR->left_fd = redir_parse_left_fd(ROTF, 1, 1);
    REDIR->type = REDIRECT_OUTPUT_TO_FILE;
    REDIR->right_file = ARG;
}
redirection(REDIR) ::= REDIRECT_OUTPUT_TO_FILE_DESCRIPTOR(ROTFD) ARGUMENT(ARG) . {
    REDIR = redir_alloc();
    REDIR->left_fd = redir_parse_left_fd(ROTFD, 2, 1);
    REDIR->type = REDIRECT_OUTPUT_TO_FILE_DESCRIPTOR;
    REDIR->right_fd = redir_parse_fd(ARG);
    free(ARG);
}
redirection(REDIR) ::= REDIRECT_OUTPUT_APPEND_TO_FILE(ROATF) ARGUMENT(ARG) . {
    REDIR = redir_alloc();
    REDIR->left_fd = redir_parse_left_fd(ROATF, 2, 1);
    REDIR->type = REDIRECT_OUTPUT_APPEND_TO_FILE;
    REDIR->right_file = ARG;
}
redirection(REDIR) ::= REDIRECT_OUTPUT_APPEND_TO_FILE_DESCRIPTOR(ROATFD) ARGUMENT(ARG) . {
    REDIR = redir_alloc();
    REDIR->left_fd = redir_parse_left_fd(ROATFD, 3, 1);
    REDIR->type = REDIRECT_OUTPUT_APPEND_TO_FILE_DESCRIPTOR;
    REDIR->right_fd = redir_parse_fd(ARG);
    free(ARG);
}

command(CMD) ::= ARGUMENT(ARG) . {
    CMD = cmd_alloc();
    strarr_append(&(CMD->args), ARG);
}
command(CMD) ::= redirection(REDIR) . {
    CMD = cmd_alloc();
    ptrarr_append(&(CMD->redirs), REDIR);
}
command(CMD) ::= command(CMD_) ARGUMENT(ARG) . {
    CMD = CMD_;
    strarr_append(&(CMD->args), ARG);
}
command(CMD) ::= command(CMD_) redirection(REDIR) . {
    CMD = CMD_;
    ptrarr_append(&(CMD->redirs), REDIR);
}

commandList(CMD_LIST) ::= command(CMD) . {
    CMD_LIST = cmd_list_alloc();
    ptrarr_append(&(CMD_LIST->cmds), CMD);
}
commandList(CMD_LIST) ::= commandList(CMD_LIST_) PIPE command(CMD) . {
    CMD_LIST = CMD_LIST_;
    cmd_list_append_op(CMD_LIST, PIPE);
    ptrarr_append(&(CMD_LIST->cmds), CMD);
}
commandList(CMD_LIST) ::= commandList(CMD_LIST_) OR command(CMD) . {
    CMD_LIST = CMD_LIST_;
    cmd_list_append_op(CMD_LIST, OR);
    ptrarr_append(&(CMD_LIST->cmds), CMD);
}
commandList(CMD_LIST) ::= commandList(CMD_LIST_) AND command(CMD) . {
    CMD_LIST = CMD_LIST_;
    cmd_list_append_op(CMD_LIST, AND);
    ptrarr_append(&(CMD_LIST->cmds), CMD);
}
commandList(CMD_LIST) ::= commandList(CMD_LIST_) BACKGROUND command(CMD). {
    CMD_LIST = CMD_LIST_;
    cmd_list_append_op(CMD_LIST, BACKGROUND);
    ptrarr_append(&(CMD_LIST->cmds), CMD);
}
commandList(CMD_LIST) ::= commandList(CMD_LIST_) BACKGROUND . {
    CMD_LIST = CMD_LIST_;
    cmd_list_append_op(CMD_LIST, BACKGROUND);
}
commandList(CMD_LIST) ::= commandList(CMD_LIST_) SEMICOLON command(CMD). {
    CMD_LIST = CMD_LIST_;
    cmd_list_append_op(CMD_LIST, SEMICOLON);
    ptrarr_append(&(CMD_LIST->cmds), CMD);
}
commandList(CMD_LIST) ::= commandList(CMD_LIST_) SEMICOLON . {
    CMD_LIST = CMD_LIST_;
    cmd_list_append_op(CMD_LIST, SEMICOLON);
}

start(START) ::= commandList(CMD_LIST) . {
    START = NULL;
    // Save our AST from being freed by Lemon!
    *cmd_list_p = CMD_LIST;
}
