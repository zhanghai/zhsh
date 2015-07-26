//
// Copyright (c) 2015 zh
// All rights reserved.
//

#include "util.h"

#include <errno.h>
#include <stdio.h>

void print_err(char *name) {
    fprintf(stderr, ZHSH_ERROR_PREFIX);
    perror(name);
    errno = 0;
}

void print_err_msg(char *name, char *msg) {
    fprintf(stderr, "%s%s: %s\n", ZHSH_ERROR_PREFIX, name, msg);
}
