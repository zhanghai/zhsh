//
// Copyright (c) 2015 zh
// All rights reserved.
//

#ifndef ZHSH_UTIL_H
#define ZHSH_UTIL_H

#include <stddef.h>

/**
 * Global name used by {@link print_err} and {@print_err_msg}.
 */
#define ZHSH_NAME "zhsh"

/**
 * Allocate a NULL-terminated string array.
 *
 * @return Allocated string array.
 */
char ** strarr_alloc();

/**
 * Reallocate a string array.
 *
 * @param strarr String array.
 * @param length New length.
 *
 * @return Reallocated string array.
 */
char **strarr_realloc(char **strarr, size_t length);

/**
 * Free a NULL-terminated string array.
 *
 * @param strarr String array.
 */
void strarr_free(char **strarr);

/**
 * Allocate a NULL-terminated pointer array.
 *
 * @return Allocated pointer array.
 */
void ** ptrarr_alloc();

/**
 * Reallocate a pointer array.
 *
 * @param ptrarr Pointer array.
 * @param length New length.
 *
 * @return Reallocated pointer array.
 */
void **ptrarr_realloc(void **ptrarr, size_t length);

/**
 * Free a NULL-terminated pointer array.
 *
 * @param ptrarr Pointer array.
 */
void ptrarr_free(void **ptrarr);

/**
 * Tokenize a string. The return string array should be freed with {@link strarr_free}.
 *
 * @param str_p Pointer to string, also saves the next position for tokenization.
 * @param delims Delimiters that separate tokens, itself discarded.
 * @param puncts Punctuations that separate tokens, but itself can also be a token. The match for punctuation is greedy.
 * @return Tokens obtained from str.
 */
char *tokenize_str(char **str_p, char **delims, char **puncts);

/**
 * Print error message with perror(), prepended with {@link ZHSH_NAME}. Also resets errno to zero after perror().
 *
 * @param name Name to be prepended before the error message, used by perror().
 */
void print_err(char *name);

/**
 * Print custom error message prepended with {@link ZHSH_NAME} to stderr.
 *
 * @param name Name to be prepended before the error message.
 * @param msg Error message to be printed.
 */
void print_err_msg(char *name, char *msg);

#endif //ZHSH_UTIL_H
