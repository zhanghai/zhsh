%{
    #include "parser.h"
%}

%option reentrant
%option stack
%option noyywrap

%x ESCAPE
%x SINGLE_QUOTED_STRING
%x DOUBLE_QUOTED_STRING

%%

[ \t\n\v\f\r]+ { return SPACE; }

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

"'" { yy_push_state(SINGLE_QUOTED_STRING); return SINGLE_QUOTED_STRING_START; }
<SINGLE_QUOTED_STRING> {
    [^\\\']+ { return SINGLE_QUOTED_STRING_FRAGMENT; }
    "\'" { yy_pop_state(SINGLE_QUOTED_STRING); return SINGLE_QUOTED_STRING_END; }
    <<EOF>> { return -1; }
}

<INITIAL> "\"" { yy_push_state(DOUBLE_QUOTED_STRING); return DOUBLE_QUOTED_STRING_START; }
<DOUBLE_QUOTED_STRING> {
    [^\\\"\$]+ { return DOUBLE_QUOTED_STRING_FRAGMENT; }
    "\'" { yy_pop_state(DOUBLE_QUOTED_STRING); return DOUBLE_QUOTED_STRING_END; }
    <<EOF>> { return -1; }
}

%%
