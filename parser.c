//
// Copyright (c) 2015 zh
// All rights reserved.
//

#include "parser.h"

#include "line_lexer.h"

void *LineParserAlloc(void *(*mallocProc)(size_t));
void LineParser(void *yyp, int yymajor, const char *yyminor, cmd_list_t **cmd_list_p);
// Debug
void LineParserTrace(FILE *TraceFILE, char *zTracePrompt);
void LineParserFree(void *p, void (*freeProc)(void*));

cmd_list_t *parse_line(const char *line) {

    yyscan_t scanner;
    yylex_init(&scanner);
    // Debug
    //yyset_debug(1, scanner);
    YY_BUFFER_STATE buf = yy_scan_string(line, scanner);

    void* shellParser = LineParserAlloc(malloc);
    // Debug
    LineParserTrace(stderr, "Parser: ");

    cmd_list_t *cmd_list = NULL;
    int lex_code;
    do {
        lex_code = yylex(scanner);
        // Debug
        fprintf(stderr, "Lexer: %s\n", yyget_text(scanner));
        LineParser(shellParser, lex_code, yyget_text(scanner), &cmd_list);
        if (errno) {
            break;
        }
    } while (lex_code > 0);

    if (lex_code == -1) {
        // Lexer failed.
        errno = EINVAL;
    }

    yy_delete_buffer(buf, scanner);
    yylex_destroy(scanner);

    LineParserFree(shellParser, free);

    return cmd_list;
}
