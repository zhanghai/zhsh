%include {
    #include <assert.h>
}

%token_type { const char * }

start ::= commandList .

command ::= argument .
command ::= command argument .
command ::= command REDIRECT_INPUT_FROM_FILE argument .
command ::= command REDIRECT_INPUT_FROM_FILE_DESCRIPTOR argument .
command ::= command REDIRECT_OUTPUT_TO_FILE argument .
command ::= command REDIRECT_OUTPUT_TO_FILE_DESCRIPTOR argument .
command ::= command REDIRECT_OUTPUT_APPEND_TO_FILE argument .
command ::= command REDIRECT_OUTPUT_APPEND_TO_FILE_DESCRIPTOR argument .

commandList ::= command .
commandList ::= command PIPE command .
commandList ::= command OR command .
commandList ::= command AND command .
commandList ::= command BACKGROUND command.
commandList ::= command BACKGROUND.
