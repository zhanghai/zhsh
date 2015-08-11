//
// Copyright (c) 2015 zh
// All rights reserved.
//

#ifndef ZHSH_QUIRK_H
#define ZHSH_QUIRK_H

#include "../shell.h"

extern const char *BUILTINS[];

void exec_builtin_quirk(char **argv, void **fdmaps, intarr_t *fds_to_close, bool wait);

#endif //ZHSH_QUIRK_H
