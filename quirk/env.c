//
// Copyright (c) 2015 zh
// All rights reserved.
//

#include "env.h"

#include <stdlib.h>
#include <stdio.h>

extern char **environ;

int env() {
    for (char **env_i = environ, *env; (env = *env_i); ++env_i) {
        printf("%s\n", env);
    }
    return EXIT_SUCCESS;
}
