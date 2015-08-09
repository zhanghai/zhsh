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
 * @param len New length, without the terminating NULL.
 *
 * @return Reallocated string array.
 */
char **strarr_realloc(char **strarr, size_t len);

/**
 * Get the length of a string array.
 *
 * @param strarr String array.
 *
 * @return Length of the string array.
 */
size_t strarr_len(char **strarr);

/**
 * Append a string to a string array.
 *
 * @param strarr String array.
 * @param str String.
 *
 * @return Reallocated string array.
 */
char ** strarr_append(char **strarr, char *str);

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
 * @param len New length, without the terminating NULL.
 *
 * @return Reallocated pointer array.
 */
void **ptrarr_realloc(void **ptrarr, size_t len);

/**
 * Get the length of a pointer array.
 *
 * @param ptrarr Pointer array.
 *
 * @return Length of the pointer array.
 */
size_t ptrarr_len(void **ptrarr);

/**
 * Append a pointer to a pointer array.
 *
 * @param ptrarr Pointer array.
 * @param ptr Pointer.
 *
 * @return Reallocated pointer array.
 */
void ** ptrarr_append(void **ptrarr, void *ptr);

typedef void (*free_ptr_func_t)(void *ptr);

/**
 * Free a NULL-terminated pointer array.
 *
 * @param free_ptr_func Function to free pointer.
 * @param ptrarr Pointer array.
 */
void ptrarr_free(void **ptrarr, free_ptr_func_t free_ptr_func);

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
