//
// Copyright (c) 2015 zh
// All rights reserved.
//

#include "util.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

char **strarr_realloc(char **strarr, size_t length) {
    return realloc(strarr, length * sizeof(strarr[0]));
}

void strarr_free(char **strarr) {
    for (char **str_i = strarr, *str; (str = *str_i); ++str_i) {
        free(str);
    }
    free(strarr);
}

char **strarrarr_realloc(char ***strarrarr, size_t length) {
    return realloc(strarrarr, length * sizeof(strarrarr[0]));
}

void strarrarr_free(char ***strarrarr) {
    for (char ***strarr_i = strarrarr, **strarr; (strarr = *strarr_i); ++strarr_i) {
        strarr_free(strarr);
    }
    free(strarrarr);
}

static bool is_char_escaped(char *str, char *ch, char escape) {
    char *escape_start = ch;
    while (escape_start > str && escape_start[-1] == escape) {
        --escape_start;
    }
    return (ch - escape_start) % 2 == 1;
}

static char *strnstr(const char *s1, const char *s2, size_t len) {
    size_t l2 = strlen(s2);
    if (!l2) {
        return (char *) s1;
    }
    while (len >= l2) {
        --len;
        if (!memcmp(s1, s2, l2))
            return (char *) s1;
        ++s1;
    }
    return NULL;
}

static char *find_unescaped_str(char *str, char *str_end, char *substr, char escape) {
    while (str < str_end) {
        char *substr_start = strnstr(str, substr, str_end - str);
        if (substr_start) {
            if (!is_char_escaped(str, substr_start, escape)) {
                return substr_start;
            } else {
                str = substr_start + 1;
            }
        } else {
            break;
        }
    }
    return NULL;
}

static bool is_str_quoted(char *str, char *str_end, char *substr, char *substr_end, char ***quote_pairs, char escape) {

}

// Escape is only targeted at tokenization delimiters and quotation marks.
// Should unquote before unescape, so tokenization should return the string escaped and quoted as it is since it cannot
// do unquote by itself.
// Escaping should be on only one character, because the multi-character case is handled by single quotation.
// Escape character should be only one character, because itself should be escapable.
// There should be only one escape character, because escape has one and only one function, that is to cancel the effect
// of the character following it.
//

char *tokenize_str(char **str_p, char **delims, char **puncts, char*** quote_pairs, char escape) {

    char *str = *str_p;
    char *str_end = strchr(str, '\0');

    // Skip all leading delimiters.
    char *token_start = *str_p, *token_start_o;
    do {
        token_start_o = token_start;
        for (char **delim_i = delims, *delim; (delim = *delim_i); ++delim_i) {
            size_t delim_len = strlen(delim);
            if (strncmp(token_start, delim, delim_len) == 0 && !is_char_escaped(str, token_start, escape)) {
                token_start += delim_len;
            }
        }
    } while (token_start != token_start_o);

    if (token_start == str_end) {
        // End of string reached, no token found.
        *str_p = str_end;
        return NULL;
    }

    // FIXME: Check whether token starts with left quotation mark, if so find right quotation mark and return the token,
    // otherwise go as usual and check for is_str_quoted.
    // Find the first left quotation mark.
    char *lquote_start = NULL;
    char **lquote_pair = NULL;
    for (char ***quote_pair_i = quote_pairs, **quote_pair; (quote_pair = (*quote_pair_i); ++quote_pair_i) {
        char *lquote_start_n = find_unescaped_str(token_start, str_end, quote_pair[0], escape);
        // Check and update the found lquote.
        if (lquote_start_n && (!lquote_start || lquote_start_n < lquote_start)) {
            lquote_start = lquote_start_n;
            lquote_pair = quote_pair;
        }
    }

    char *token_end;
    if (lquote_start) {
        // Deal with quotation mark.
        if (lquote_start == token_start) {
            // The token starts with a left quotation mark, find the right one and return the whole token.
            char *rquote = lquote_pair[1];
            char *rquote_start = find_unescaped_str(lquote_start + strlen(lquote_pair[0]), str_end, rquote, escape);
            if (rquote_start) {
                // Found right quotation mark.
                token_end = rquote_start + strlen(rquote);
                *str_p = token_end;
                return strndup(token_start, token_end - token_start);
            } else {
                // Unpaired quotation mark.
                errno = EINVAL;
                return NULL;
            }
        } else {
            // The token
        }
    }

    // Find the first delimiter.
    char *token_end = str_end;
    for (char **delim_i = delims, *delim; (delim = *delim_i); ++delim_i) {
        char *token_end_new = strstr(token_start, delim);
        if (token_end_new && token_end_new < token_end) {
            token_end = token_end_new;
        }
    }

    // Find the first punctuation, not starting from token.
    for (char **punct_i = puncts, *punct; (punct = *punct_i); ++punct_i) {
        char *token_end_new = strstr(token_start, punct);
        if (token_end_new && token_end_new != token_start && token_end_new < token_end) {
            token_end = token_end_new;
        }
    }

    // Take the longest sequence if this token is a punctuation.
    char *token_end_punct = NULL;
    for (char **punct_i = puncts, *punct; (punct = *punct_i); ++punct_i) {
        size_t punct_len = strlen(punct);
        if (strncmp(token_start, punct, punct_len) == 0) {
            char *token_end_new = token_start + punct_len;
            if (!token_end_punct || token_end_new > token_end_punct) {
                token_end_punct = token_end_new;
            }
        }
    }
    if (token_end_punct) {
        token_end = token_end_punct;
    }

    // Token found.
    *str_p = token_end;
    return strndup(token_start, token_end - token_start);
}

void print_err(char *name) {
    fflush(stdout);
    fprintf(stderr, "%s: ", ZHSH_NAME);
    perror(name);
    errno = 0;
}

void print_err_msg(char *name, char *msg) {
    fflush(stdout);
    fprintf(stderr, "%s: %s: %s\n", ZHSH_NAME, name, msg);
}
