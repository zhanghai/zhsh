#include "line_lexer.h"
#include "util.h"

void parse_line(char *line, int **ops_p, char ****args_arr_p) {

    int *ops = NULL;
    size_t ops_len = 0;
    char ***args_arr = NULL;
    size_t args_arr_len = 0;
    size_t args_len = 0;

    yyscan_t scanner;
    yylex_init(&scanner);
    YY_BUFFER_STATE buf = yy_scan_string();

    int lex_code;
    do {
        lex_code = yylex(scanner);
        if (lex_code == ARGUMENT) {
            char *arg = yyget_text(scanner);
            args_arr[args_len] = realloc(args_arr[args_len], (args_len + 1) * sizeof(args_arr[0][0]));
            if (errno) {
                free(ops);
                strarrarr_free(args_arr);
                return;
            }
            args_arr[args_arr_len][args_len] = strdup(arg);
        }
        Parse(shellParser, lex_code, yyget_text(scanner));
        // XXX This line should not be necessary; EOL should automatically
        // terminate parsing. :-(
        if (lex_code == EOL) Parse(shellParser, 0, NULL);
    } while (lex_code > 0);

    if (lex_code == -1) {
        fprintf(stderr, "The scanner encountered an error.\n");
    }

    yy_delete_buffer(buf, scanner);
    yylex_destroy(scanner);
}
