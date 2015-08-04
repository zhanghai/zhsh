%include {
    #include <assert.h>

    #include "line_syntax.h"
}

%token_type { const char * }

%type redirection { redir_t * }
%destructor redirection {
    redir_free($$);
}
%type argument { char * }
%destructor redirection {
    free($$);
}
%type command { cmd_t * }
%destructor command {
    cmd_free($$);
}

%parse_failed {
    // TODO
}

%start_symbol commandList

redirection ::= REDIRECT_INPUT_FROM_FILE argument .
redirection ::= REDIRECT_INPUT_FROM_FILE_DESCRIPTOR argument .
redirection ::= REDIRECT_OUTPUT_TO_FILE argument .
redirection ::= REDIRECT_OUTPUT_TO_FILE_DESCRIPTOR argument .
redirection ::= REDIRECT_OUTPUT_APPEND_TO_FILE argument .
redirection ::= REDIRECT_OUTPUT_APPEND_TO_FILE_DESCRIPTOR argument .

command ::= argument . {

}
command ::= command argument .
command ::= command redirection .

commandList ::= command .
commandList ::= command PIPE command .
commandList ::= command OR command .
commandList ::= command AND command .
commandList ::= command BACKGROUND command.
commandList ::= command BACKGROUND .
