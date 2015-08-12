//
// Copyright (c) 2015 zh
// All rights reserved.
//

#include "util.h"

#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pwd.h>
#include <unistd.h>
#include <sys/utsname.h>

void *realloc_util(void *ptr, size_t size) {
    void *ptr_tmp = realloc(ptr, size);
    return ptr_tmp != NULL ? ptr_tmp : ptr;
}

void intarr_init(intarr_t *intarr) {
    intarr->arr = NULL;
    intarr->len = 0;
}

void intarr_realloc(intarr_t *intarr, size_t len) {
    intarr->arr = realloc_util(intarr->arr, len * sizeof((intarr->arr)[0]));
    if (errno) {
        return;
    }
    intarr->len = len;
}

void intarr_append(intarr_t *intarr, int i) {
    size_t len = intarr->len;
    intarr_realloc(intarr, len + 1);
    if (errno) {
        return;
    }
    intarr->arr[len] = i;
}

void intarr_remove(intarr_t *intarr, size_t index) {
    --(intarr->len);
    for (; index < intarr->len; ++index) {
        intarr->arr[index] = intarr->arr[index + 1];
    }
}

void intarr_fin(intarr_t *intarr) {
    free(intarr->arr);
    intarr->len = 0;
}

void pidarr_init(pidarr_t *pidarr) {
    pidarr->arr = NULL;
    pidarr->len = 0;
}

void pidarr_realloc(pidarr_t *pidarr, size_t len) {
    pidarr->arr = realloc_util(pidarr->arr, len * sizeof((pidarr->arr)[0]));
    if (errno) {
        return;
    }
    pidarr->len = len;
}

void pidarr_append(pidarr_t *pidarr, pid_t pid) {
    size_t len = pidarr->len;
    pidarr_realloc(pidarr, len + 1);
    if (errno) {
        return;
    }
    pidarr->arr[len] = pid;
}

void pidarr_remove(pidarr_t *pidarr, size_t index) {
    --(pidarr->len);
    for (; index < pidarr->len; ++index) {
        pidarr->arr[index] = pidarr->arr[index + 1];
    }
}

void pidarr_fin(pidarr_t *pidarr) {
    free(pidarr->arr);
    pidarr->len = 0;
}

char **strarr_alloc() {
    char **strarr = NULL;
    strarr_realloc(&strarr, 0);
    return strarr;
}

void strarr_realloc(char ***strarr_p, size_t len) {
    *strarr_p = realloc_util(*strarr_p, (len + 1) * sizeof((*strarr_p)[0]));
    if (errno) {
        return;
    }
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
    if (errno) {
        return;
    }
    (*strarr_p)[len] = str;
}

void strarr_free(char **strarr) {
    if (strarr == NULL) {
        return;
    }
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
    *ptrarr_p = realloc_util(*ptrarr_p, (len + 1) * sizeof((*ptrarr_p)[0]));
    if (errno) {
        return;
    }
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
    if (errno) {
        return;
    }
    (*ptrarr_p)[len] = ptr;
}

void ptrarr_free(void **ptrarr, free_ptr_func_t free_ptr_func) {
    if (ptrarr == NULL) {
        return;
    }
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

char *file_get_line(FILE *file) {
    char *line = NULL;
    size_t buf_len = 0;
    ssize_t len = getline(&line, &buf_len, file);
    if (errno || len == -1) {
        // Should free line even if getline() failed, according to manual.
        free(line);
        return NULL;
    }
    // Strip trailing newline.
    --len;
    if (line[len] == '\n') {
        line[len] = '\0';
    }
    return line;
}

char *readlink_malloc(const char *path) {

    size_t size = 128;
    char *buffer = NULL;

    while (true) {
        buffer = realloc_util(buffer, size);
        if (errno) {
            free(buffer);
            return NULL;
        }
        ssize_t num_chars = readlink(path, buffer, size);
        if (errno) {
            free(buffer);
            return NULL;
        }
        if (num_chars < size) {
            return buffer;
        }
        size *= 2;
    }
}

char *geteuname() {
    uid_t uid = geteuid();
    struct passwd *pw = getpwuid(uid);
    if (errno) {
        return NULL;
    }
    return pw->pw_name;
}

// From `man 2 gethostname`:
// The GNU C library  does  not  employ  the  gethostname()  system  call;
// instead,  it  implements gethostname() as a library function that calls
// uname(2) and copies up to len bytes from the  returned  nodename  field
// into  name.
// This is also the way systemd employed.
char *gethostname_malloc() {
    struct utsname u;
    uname(&u);
    if (errno) {
        return NULL;
    }
    return strdup(u.nodename);
}

char *getcwd_malloc() {

    size_t size = 128;
    char *buffer = NULL;

    while (true) {
        buffer = realloc_util(buffer, size);
        if (errno) {
            free(buffer);
            return NULL;
        }
        getcwd(buffer, size);
        if (errno) {
            if (errno == ERANGE) {
                size *= 2;
                errno = 0;
            } else {
                free(buffer);
                return NULL;
            }
        } else {
            return buffer;
        }
    }
}

char *sprintf_malloc(const char *format, ...) {
    va_list vl;
    va_start(vl, format);
    va_list vl2;
    va_copy(vl2, vl);
    size_t size = (size_t) vsnprintf(NULL, 0, format, vl2) + 1;
    va_end(vl2);
    char *buffer = malloc(size);
    vsnprintf(buffer, size, format, vl);
    va_end(vl);
    return buffer;
}
