//
// Copyright (c) 2015 zh
// All rights reserved.
//

#ifndef ZHSH_UTIL_H
#define ZHSH_UTIL_H

#include <stddef.h>
#include <stdio.h>

#include <unistd.h>

/**
 * Reallocate memory.
 *
 * <p>Note: realloc() returns NULL on failure, however the normal usage is to set the pointer to be reallocated the
 * return value of realloc() (and check for errno), so we provide and use our own version here.</p>
 *
 * @param ptr Pointer.
 * @param size Size.
 *
 * @return Reallocated pointer, or ptr itself on failure.
 */
void *realloc_util(void *ptr, size_t size);

typedef struct {
    int *arr;
    size_t len;
} intarr_t;

/**
 * Initialize an int array.
 *
 * @param Uninitialized int array.
 */
void intarr_init(intarr_t *intarr);

/**
 * Reallocate an int array.
 *
 * @param intarr Int array.
 * @param len New length.
 */
void intarr_realloc(intarr_t *intarr, size_t len);

/**
 * Append an int to an int array.
 *
 * @param intarr Int array.
 * @param i Int.
 */
void intarr_append(intarr_t *intarr, int i);

/**
 * Remove an int from an int array.
 *
 * @param intarr Int array.
 * @param index Index of the int to be removed.
 */
void intarr_remove(intarr_t *intarr, size_t index);

/**
 * Finalize an int array.
 */
void intarr_fin(intarr_t *intarr);

typedef struct {
    pid_t *arr;
    size_t len;
} pidarr_t;

/**
 * Initialize a pid array.
 *
 * @param Uninitialized pid array.
 */
void pidarr_init(pidarr_t *pidarr);

/**
 * Reallocate a pid array.
 *
 * @param arr Pid array.
 * @param len New length.
 */
void pidarr_realloc(pidarr_t *pidarr, size_t len);

/**
 * Append a pid to a pid array.
 *
 * @param pidarr Pid array.
 * @param pid Pid.
 */
void pidarr_append(pidarr_t *pidarr, pid_t pid);

/**
 * Remove a pid from a pid array.
 *
 * @param pidarr Pid array.
 * @param index Index of the pid to be removed.
 */
void pidarr_remove(pidarr_t *pidarr, size_t index);

/**
 * Finalize a pid array.
 */
void pidarr_fin(pidarr_t *pidarr);

/**
 * Allocate a NULL-terminated string array.
 *
 * @return Allocated string array.
 */
char ** strarr_alloc();

/**
 * Reallocate a string array.
 *
 * @param strarr_p Pointer to string array.
 * @param len New length, without the terminating NULL.
 */
void strarr_realloc(char ***strarr_p, size_t len);

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
 * @param strarr_p Pointer to string array.
 * @param str String.
 */
void strarr_append(char ***strarr_p, char *str);

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
 * @param ptrarr_p Pointer to pointer array.
 * @param len New length, without the terminating NULL.
 */
void ptrarr_realloc(void ***ptrarr_p, size_t len);

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
 * @param ptrarr_p Pointer to pointer array.
 * @param ptr Pointer.
 */
void ptrarr_append(void ***ptrarr_p, void *ptr);

typedef void (*free_ptr_func_t)(void *ptr);

/**
 * Free a NULL-terminated pointer array.
 *
 * @param free_ptr_func Function to free pointer.
 * @param ptrarr Pointer array.
 */
void ptrarr_free(void **ptrarr, free_ptr_func_t free_ptr_func);

/**
 * Global name used by {@link print_err} and {@print_err_msg}.
 */
#define ZHSH_NAME "zhsh"

/**
 * Print error message with perror(), prepended with {@link ZHSH_NAME}. This also resets errno to 0.
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

/**
 * Get one line from a file.
 *
 * @param file File.
 *
 * @return One line from the file, or NULL if end is met.
 */
char *file_get_line(FILE *file);

/**
 * readlink() with malloc().
 *
 * @param path Path to the symbolic link.
 *
 * @return Content of the symbolic link.
 */
char *readlink_malloc(const char *path);

/**
 * Get the name of effective user.
 *
 * @return Name of effective user.
 */
char *geteuname();

/**
 * gethostname() with malloc().
 *
 * @return Host name.
 */
char *gethostname_malloc();

/**
 * getcwd() with malloc().
 *
 * @return Current working directory.
 */
char *getcwd_malloc();

/**
 * sprintf() with malloc().
 *
 * @param format Format.
 *
 * @return Printed string.
 */
char *sprintf_malloc(const char *format, ...);

#endif //ZHSH_UTIL_H
