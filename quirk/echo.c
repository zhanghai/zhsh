//
// Copyright (c) 2015 zh
// All rights reserved.
//

#include "echo.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int echo(char **argv) {
    bool first = true;
    for (char **arg_i = argv + 1, *arg; (arg = *arg_i); ++arg_i) {
        if (first) {
            first = false;
        } else {
            printf(" ");
        }
        printf(arg);
    }
    printf("\n");
    return EXIT_SUCCESS;
}
