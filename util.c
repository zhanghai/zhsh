//
// Copyright (c) 2015 zh
// All rights reserved.
//

#include "util.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

char **strarr_alloc() {
    char **strarr = NULL;
    strarr_realloc(&strarr, 0);
    return strarr;
}

void strarr_realloc(char ***strarr_p, size_t len) {
    *strarr_p = realloc(*strarr_p, (len + 1) * sizeof((*strarr_p)[0]));
    (*strarr_p)[len] = NULL;
}

size_t strarr_len(char **strarr) {
    char **str_i = strarr;
    for (; *str_i; ++str_i) {}
    return str_i - strarr;
}

void strarr_append(char ***strarr_p, char *str) {
    size_t len = strarr_len(*strarr_p);
    strarr_realloc(strarr_p, len + 1);
    (*strarr_p)[len] = str;
}

void strarr_free(char **strarr) {
    for (char **str_i = strarr, *str; (str = *str_i); ++str_i) {
        free(str);
    }
    free(strarr);
}

void **ptrarr_alloc() {
    void **ptrarr = NULL;
    ptrarr_realloc(&ptrarr, 0);
    return ptrarr;
}

void ptrarr_realloc(void ***ptrarr_p, size_t len) {
    *ptrarr_p = realloc(*ptrarr_p, (len + 1) * sizeof((*ptrarr_p)[0]));
    (*ptrarr_p)[len] = NULL;
}

size_t ptrarr_len(void **ptrarr) {
    void **ptr_i = ptrarr;
    for (; *ptr_i; ++ptr_i) {}
    return ptr_i - ptrarr;
}

void ptrarr_append(void ***ptrarr_p, void *ptr) {
    size_t len = ptrarr_len(*ptrarr_p);
    ptrarr_realloc(ptrarr_p, len + 1);
    (*ptrarr_p)[len] = ptr;
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
