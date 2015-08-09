//
// Copyright (c) 2015 zh
// All rights reserved.
//

#include "util.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

char **strarr_alloc() {
    char **strarr = strarr_realloc(NULL, 0);
    return strarr;
}

char **strarr_realloc(char **strarr, size_t len) {
    strarr = realloc(strarr, (len + 1) * sizeof(strarr[0]));
    strarr[len] = NULL;
    return strarr;
}

size_t strarr_len(char **strarr) {
    char **str_i = strarr;
    for (; *str_i; ++str_i) {}
    return str_i - strarr;
}

char **strarr_append(char **strarr, char *str) {
    size_t len = strarr_len(strarr);
    strarr_realloc(strarr, len + 1);
    strarr[len] = str;
    return strarr;
}

void strarr_free(char **strarr) {
    for (char **str_i = strarr, *str; (str = *str_i); ++str_i) {
        free(str);
    }
    free(strarr);
}

void **ptrarr_alloc() {
    void **ptrarr = ptrarr_realloc(NULL, 0);
    return ptrarr;
}

void **ptrarr_realloc(void **ptrarr, size_t len) {
    ptrarr = realloc(ptrarr, (len + 1) * sizeof(ptrarr[0]));
    ptrarr[len] = NULL;
    return ptrarr;
}

size_t ptrarr_len(void **ptrarr) {
    void **ptr_i = ptrarr;
    for (; *ptr_i; ++ptr_i) {}
    return ptr_i - ptrarr;
}

void **ptrarr_append(void **ptrarr, void *ptr) {
    size_t len = ptrarr_len(ptrarr);
    ptrarr_realloc(ptrarr, len + 1);
    ptrarr[len] = ptr;
    return ptrarr;
}

void ptrarr_free(void **ptrarr, free_ptr_func_t free_ptr_func) {
    for (void **ptr_i = ptrarr, *ptr; (ptr = *ptr_i); ++ptr_i) {
        free_ptr_func(ptr);
    }
    free(ptrarr);
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
