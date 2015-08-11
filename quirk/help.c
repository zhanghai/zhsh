//
// Copyright (c) 2015 zh
// All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#include "help.h"

static const char *USAGE =
"zhsh - Zhang Hai's shell\n"
"\n"
"Usage: zhsh [file]\n";

int help() {
    FILE *pipe = popen("more", "w");
    fprintf(pipe, USAGE);
    pclose(pipe);
    return EXIT_SUCCESS;
}
