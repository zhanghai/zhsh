%{
    #include <stdbool.h>

    #include "line_parser.h"

    #define finish_last_token(last_token_type, token_length) \
        do { \
            if (yyleng > token_length) { \
                yyless(yyleng - token_length); \
                return last_token_type; \
            } \
        } while (false)

    #define finish_last_token_or_return(last_token_type, token_length, token_type) \
        do { \
            if (yyleng > token_length) { \
                yyless(yyleng - token_length); \
                return last_token_type; \
            } else { \
                return token_type; \
            } \
        } while (false)
    #define finish_last_token_or_terminate(last_token_type, token_length) \
        do { \
            if (yyleng > token_length) { \
                yyless(yyleng - token_length); \
                return last_token_type; \
            } else { \
                yyterminate(); \
            } \
        } while (false)
%}

%option reentrant
%option stack
%option noyywrap

%x ESCAPED
%x SINGLE_QUOTED
%x DOUBLE_QUOTED

%%

[ \t\v\f] { finish_last_token(ARGUMENT, 1); }

[0-9]*"<" { finish_last_token_or_return(ARGUMENT, 1, REDIRECT_INPUT_FROM_FILE); }
[0-9]*"<&" { finish_last_token_or_return(ARGUMENT, 2, REDIRECT_INPUT_FROM_FILE_DESCRIPTOR); }
[0-9]*">" { finish_last_token_or_return(ARGUMENT, 1, REDIRECT_OUTPUT_TO_FILE); }
[0-9]*">&" { finish_last_token_or_return(ARGUMENT, 2, REDIRECT_OUTPUT_TO_FILE_DESCRIPTOR); }
[0-9]*">>" { finish_last_token_or_return(ARGUMENT, 2, REDIRECT_OUTPUT_APPEND_TO_FILE); }
"|" { finish_last_token_or_return(ARGUMENT, 1, PIPE); }
"&" { finish_last_token_or_return(ARGUMENT, 1, BACKGROUND); }
"||" { finish_last_token_or_return(ARGUMENT, 2, OR); }
"&&" { finish_last_token_or_return(ARGUMENT, 2, AND); }
";" { finish_last_token_or_return(ARGUMENT, 1, SEMICOLON); }

<INITIAL,DOUBLE_QUOTED>"\\" { yy_push_state(ESCAPED, yyscanner); yymore(); }
<ESCAPED>{
    . { yy_pop_state(yyscanner); yymore(); }
    <<EOF>> { return -1; }
}

"'" { yy_push_state(SINGLE_QUOTED, yyscanner); yymore(); }
<SINGLE_QUOTED>{
    "\'" { yy_pop_state(yyscanner); yymore(); }
    . { yymore(); }
    <<EOF>> { return -1; }
}

"\"" { yy_push_state(DOUBLE_QUOTED, yyscanner); yymore(); }
<DOUBLE_QUOTED>{
    "\"" { yy_pop_state(yyscanner); yymore(); }
    . { yymore(); }
    <<EOF>> { return -1; }
}

. { yymore(); }

<<EOF>> {
    finish_last_token_or_terminate(ARGUMENT, 1);
}

%%
