%include {
    #include <assert.h>
}

%token_type { const char * }

singleQuotedStringStart ::= SINGLE_QUOTED_STRING_START SINGLE_QUOTED_STRING_FRAGMENT .
singleQuotedStringFragment ::= singleQuotedStringStart .
singleQuotedStringFragment ::= singleQuotedStringStart SINGLE_QUOTED_STRING_FRAGMENT .
singleQuotedStringFragment ::= singleQuotedStringStart ESCAPED_CHAR .
singleQuotedString ::= singleQuotedStringFragment SINGLE_QUOTED_STRING_END .

doubleQuotedStringStart ::= DOUBLE_QUOTED_STRING_START DOUBLE_QUOTED_STRING_FRAGMENT .
doubleQuotedStringFragment ::= doubleQuotedStringStart .
doubleQuotedStringFragment ::= doubleQuotedStringStart DOUBLE_QUOTED_STRING_FRAGMENT .
doubleQuotedStringFragment ::= doubleQuotedStringStart ESCAPED_CHAR .
doubleQuotedString ::= doubleQuotedStringFragment DOUBLE_QUOTED_STRING_END .

argument ::= FRAGMENT .
argument ::= ESCAPED_CHAR .
argument ::= singleQuotedString .
argument ::= doubleQuotedString .
argument ::= argument FRAGMENT .
argument ::= argument ESCAPED_CHAR .
argument ::= argument singleQuotedString .
argument ::= argument doubleQuotedString .

command ::= argument .
command ::= command SPACE argument .
command ::= command REDIRECT_INPUT_FROM_FILE argument .
command ::= command SPACE REDIRECT_INPUT_FROM_FILE argument .
command ::= command REDIRECT_INPUT_FROM_FILE SPACE argument .
// FIXME: This fails LALR(1).
command ::= command SPACE REDIRECT_INPUT_FROM_FILE SPACE argument .
// FIXME: Space is not handled here.
command ::= command REDIRECT_INPUT_FROM_FILE_DESCRIPTOR argument .
command ::= command REDIRECT_OUTPUT_TO_FILE argument .
command ::= command REDIRECT_OUTPUT_TO_FILE_DESCRIPTOR argument .
command ::= command REDIRECT_OUTPUT_APPEND_TO_FILE argument .
command ::= command REDIRECT_OUTPUT_APPEND_TO_FILE_DESCRIPTOR argument .
command ::= command BACKGROUND .

commandList ::= command .
commandList ::= command PIPE command .
commandList ::= command OR command .
commandList ::= command AND command .

start ::= commandList .
