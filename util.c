//
// Copyright (c) 2015 zh
// All rights reserved.
//

#include "util.h"

#include <stdio.h>

void print_error(char *name) {
    fprintf(stderr, "zhsh: ");
    perror(name);
}

void print_error_message(char *name, char *message) {
    fprintf(stderr, "zhsh: %s: %s\n", name, message);
}
