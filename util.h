//
// Copyright (c) 2015 zh
// All rights reserved.
//

#ifndef ZHSH_UTIL_H
#define ZHSH_UTIL_H

#define ZHSH_ERROR_PREFIX "zhsh: "

/**
 * Print error message with perror(), prepended with {@link ZHSH_ERROR_PREFIX}. Also resets errno after perror().
 */
void print_err(char *name);

/**
 * Print custom error message prepended with {@link ZHSH_ERROR_PREFIX}.
 */
void print_err_msg(char *name, char *msg);

#endif //ZHSH_UTIL_H
