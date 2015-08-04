%{
    #include <stdbool.h>

    #define finish_last_token(last_token_type, token_length) \
        do { \
            if (yylength > token_length) { \
                yyless(token_length); \
                return last_token_type; \
            } \
        } while (false)

    #define finish_last_token_or_return(last_token_type, token_length, token_type) \
        do { \
            if (yylength > token_length) { \
                yyless(token_length); \
                return last_token_type; \
            } else { \
                return token_type; \
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

"[0-9]*<" { finish_last_token_or_return(ARGUMENT, 1, REDIRECT_INPUT_FROM_FILE); }
"[0-9]*<&" { finish_last_token_or_return(ARGUMENT, 2, REDIRECT_INPUT_FROM_FILE_DESCRIPTOR); }
"[0-9]*>" { finish_last_token_or_return(ARGUMENT, 1, REDIRECT_OUTPUT_TO_FILE); }
"[0-9]*>&" { finish_last_token_or_return(ARGUMENT, 2, REDIRECT_OUTPUT_TO_FILE_DESCRIPTOR); }
"[0-9]*>>" { finish_last_token_or_return(ARGUMENT, 2, REDIRECT_OUTPUT_APPEND_TO_FILE); }
"[0-9]*>>&" { finish_last_token_or_return(ARGUMENT, 3, REDIRECT_OUTPUT_APPEND_TO_FILE_DESCRIPTOR); }
"|" { finish_last_token_or_return(ARGUMENT, 1, PIPE); }
"&" { finish_last_token_or_return(ARGUMENT, 1, BACKGROUND); }
"||" { finish_last_token_or_return(ARGUMENT, 2, OR); }
"&&" { finish_last_token_or_return(ARGUMENT, 2, AND); }
";" { finish_last_token_or_return(ARGUMENT, 1, SEMICOLON); }

<INITIAL,DOUBLE_QUOTED>"\\" { yy_push_state(ESCAPED); yymore(); }
<ESCAPED>{
    . { yy_pop_state(); yymore(); }
    <<EOF>> { return -1; }
}

"'" { yy_push_state(SINGLE_QUOTED); return SINGLE_QUOTED_START; }
<SINGLE_QUOTED>{
    "\'" { yy_pop_state(SINGLE_QUOTED); yymore() }
    .+ { yymore() }
    <<EOF>> { return -1; }
}

"\"" { yy_push_state(DOUBLE_QUOTED); return DOUBLE_QUOTED_START; }
<DOUBLE_QUOTED>{
    "\"" { yy_pop_state(DOUBLE_QUOTED); yymore(); }
    .+ { yymore() }
    <<EOF>> { return -1; }
}

.+ { yymore(); }

%%
