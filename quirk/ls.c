//
// Copyright (c) 2015 zh
// All rights reserved.
//

#include "ls.h"

#include <dirent.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "../shell.h"

int ls(int argc, char *argv[]) {

    time_t recent;
    time(&recent);
    /* From GNU coreutils ls implementation:
       Consider a time to be recent if it is within the past six
       months.  A Gregorian year has 365.2425 * 24 * 60 * 60 ==
       31556952 seconds on the average.  Write this value as an
       integer constant to avoid floating point hassles. */
    recent -= 31556952 / 2;

    char *dirname;
    if (argc == 1) {
        dirname = ".";
    } else if (argc == 2) {
        dirname = argv[1];
    } else {
        fprintf(stderr, "Usage: ls [directory]\n");
        return ZHSH_EXIT_BUILTIN_FAILURE;
    }

    DIR *dir = opendir(dirname);
    if (errno) {
        print_err(dirname);
        return ZHSH_EXIT_BUILTIN_FAILURE;
    }

    struct dirent *dirent;
    char mtime[256];
    while ((dirent = readdir(dir)) != NULL) {

        char *absname = malloc(strlen(dirname) + strlen(dirent->d_name) + 2);
        strcpy(absname, dirname);
        strcat(absname, "/");
        strcat(absname, dirent->d_name);

        struct stat fstat;
        stat(absname, &fstat);
        if (errno) {
            print_err(absname);
            free(absname);
            continue;
        }
        free(absname);

        char mode[11];
        if (S_ISREG(fstat.st_mode)) { mode[0] = '-'; }
        else if (S_ISBLK(fstat.st_mode)) { mode[0] = 'b'; }
        else if (S_ISCHR(fstat.st_mode)) { mode[0] = 'c'; }
        else if (S_ISDIR(fstat.st_mode)) { mode[0] = 'd'; }
        else if (S_ISLNK(fstat.st_mode)) { mode[0] = 'l'; }
        else if (S_ISFIFO(fstat.st_mode)) { mode[0] = 'p'; }
        else if (S_ISSOCK(fstat.st_mode)) { mode[0] = 's'; }
        mode[1] = fstat.st_mode & S_IRUSR ? 'r' : '-';
        mode[2] = fstat.st_mode & S_IWUSR ? 'w' : '-';
        mode[3] = fstat.st_mode & S_IXUSR ? 'x' : '-';
        mode[4] = fstat.st_mode & S_IRGRP ? 'r' : '-';
        mode[5] = fstat.st_mode & S_IWGRP ? 'w' : '-';
        mode[6] = fstat.st_mode & S_IXGRP ? 'x' : '-';
        mode[7] = fstat.st_mode & S_IROTH ? 'r' : '-';
        mode[8] = fstat.st_mode & S_IWOTH ? 'w' : '-';
        mode[9] = fstat.st_mode & S_IXOTH ? 'x' : '-';
        mode[10] = '\0';

        struct passwd *pwd = getpwuid(fstat.st_uid);
        if (errno) {
            print_err("getpwuid");
            continue;
        }

        struct group *grp = getgrgid(fstat.st_gid);
        if (errno) {
            print_err("getgrgid()");
            continue;
        }

        char *timefmt = fstat.st_mtim.tv_sec > recent ? "%b %d %H:%M" : "%b %d  %Y";
        strftime(mtime, sizeof(mtime), timefmt, localtime(&fstat.st_mtim.tv_sec));

        printf("%lu	%s	%lu	%s	%s	%ld	%s	%s\n", fstat.st_ino, mode, fstat.st_nlink, pwd->pw_name, grp->gr_name,
               fstat.st_size, mtime, dirent->d_name);
    }

    closedir(dir);

    return EXIT_SUCCESS;
}
