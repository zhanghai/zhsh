%{
    #include "parser.h"
%}

%option reentrant
%option stack
%option noyywrap

%x ESCAPE
%x VARIABLE_SUBSTITUTION
%s COMMAND_SUBSTITUTION
%x SINGLE_QUOTED_STRING
%x DOUBLE_QUOTED_STRING

%%

[ \t\n\v\f\r] { return SPACE; }

"<" { return REDIRECT_INPUT_FROM_FILE; }
"<&" { return REDIRECT_INPUT_FROM_FILE_DESCRIPTOR; }
">" { return REDIRECT_OUTPUT_TO_FILE; }
">&" { return REDIRECT_OUTPUT_TO_FILE_DESCRIPTOR; }
">>" { return REDIRECT_OUTPUT_APPEND_TO_FILE; }
">>&" { return REDIRECT_OUTPUT_APPEND_TO_FILE_DESCRIPTOR; }
"|" { return PIPE; }
"&" { return BACKGROUND; }
"||" { return OR; }
"&&" { return AND; }

";" { return SEMICOLON; }
.+ { return FRAGMENT; }

<*> "\\" { yy_push_state(ESCAPE); }
<ESCAPE> {
    [.\n] { yy_pop_state(); return ESCAPED_CHAR; }
    <<EOF>> { return -1; }
}

<INITIAL, COMMAND_SUBSTITUTION, DOUBLE_QUOTED_STRING> "${" { yy_push_state(VARIABLE_SUBSTITUTION); return VARIABLE_SUBSTITUTION_START; }
<VARIABLE_SUBSTITUTION> {
    [a-zA-Z][a-zA-Z0-9_]* { return VARIABLE_SUBSTITUTION_NAME; }
    "}" { yy_pop_state(); return VARIABLE_SUBSTITUTION_END; }
    . { return -2; }
    <<EOF>> { return -1; }
}

<INITIAL, COMMAND_SUBSTITUTION, DOUBLE_QUOTED_STRING> "$(" { yy_push_state(COMMAND_SUBSTITUTION); return COMMAND_SUBSTITUTION_START; }
<COMMAND_SUBSTITUTION> {
    "}" { yy_pop_state(); return COMMAND_SUBSTITUTION_END; }
    <<EOF>> { return -1; }
}

<INITIAL, COMMAND_SUBSTITUTION> "'" { yy_push_state(SINGLE_QUOTED_STRING); return SINGLE_QUOTED_STRING_START; }
<SINGLE_QUOTED_STRING> {
    [^\\\']+ { return SINGLE_QUOTED_STRING_FRAGMENT; }
    "\'" { yy_pop_state(SINGLE_QUOTED_STRING); return SINGLE_QUOTED_STRING_END; }
    <<EOF>> { return -1; }
}

<INITIAL, COMMAND_SUBSTITUTION> "\"" { yy_push_state(DOUBLE_QUOTED_STRING); return DOUBLE_QUOTED_STRING_START; }
<DOUBLE_QUOTED_STRING> {
    [^\\\"\$]+ { return DOUBLE_QUOTED_STRING_FRAGMENT; }
    "\'" { yy_pop_state(DOUBLE_QUOTED_STRING); return DOUBLE_QUOTED_STRING_END; }
    <<EOF>> { return -1; }
}

%%
