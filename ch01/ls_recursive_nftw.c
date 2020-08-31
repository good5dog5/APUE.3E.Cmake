//
// Created by 黃柏瑀 on 2020/8/31.
//

#define _XOPEN_SOURCE 700

#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64

#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ftw.h>
#include <time.h>
#include <errno.h>

#ifndef USE_FDS
#define USE_FDS 15
#endif

int print_entry(const char *filepath, const struct stat * info,
                const int typeflag, struct FTW *pathinfo)
{
    const double bytes = (double )info->st_size;
    struct tm mtime;

    localtime_r(&(info->st_mtime), &mtime);

    printf("%04d-%02d-%02d %02d:%02d:%02d",
           mtime.tm_yday+1900, mtime.tm_mon+1, mtime.tm_mday,
           mtime.tm_hour, mtime.tm_min, mtime.tm_sec);

    if (bytes >= 1099511627776.0)
        printf(" %9.3f TiB", bytes / 1099511627776.0);

    else if (bytes >= 1073741824.0)
        printf(" %9.3f GiB", bytes / 1073741824.0);
    else if (bytes >= 1048576.0)
        printf(" %9.3f MiB", bytes / 1048576.0);
    else if (bytes >= 1024.0)
        printf(" %9.3f KiB", bytes / 1024.0);
    else
        printf(" %9.0f B  ", bytes);

    if (typeflag == FTW_SL) {
        char *target;
        size_t  maxlen = 1023;
        ssize_t len;

        while (1) {
            target = malloc(maxlen + 1);
            if (target == NULL)
                return ENOMEM;

            len = readlink(filepath, target, maxlen);
            if (len == (ssize_t) - 1) {
                const int saved_errno = errno;
                free(target);
                return saved_errno;
            }

            if (len >= (ssize_t)maxlen) {
                free(target);
                maxlen += 1024;
                continue;
            }

            target[len] = '\0';
            break;
        }

        printf(" %s -> %s\n", filepath, target);
        free(target);
    }
    else if (typeflag == FTW_SLN)
        printf(" %s (dangling symlink)\n", filepath);
    else if (typeflag == FTW_F)
        printf(" %s\n", filepath);
    else if (typeflag == FTW_D || typeflag == FTW_DP)
        printf(" %s/\n", filepath);
    else if (typeflag == FTW_DNR)
        printf(" %s/ (unreadable)\n", filepath);
    else
        printf( " %s (unknown\n", filepath);

    return 0;
}

int print_dir_tree(const char *const dirpath) {
    int result;

    if (dirpath == NULL || *dirpath == '\0')
        return errno = EINVAL;

    result = nftw(dirpath, print_entry, USE_FDS, FTW_PHYS);
    if (result >= 0)
        errno = result;

    return errno;
}

int main(int argc, char * argv[]) {
    int arg;

    if (argc < 2) {
        if (print_dir_tree(">")) {
            fprintf(stderr, "%s.\n", strerror(errno));
            return EXIT_FAILURE;
        }
    } else {
        for (arg = 1; arg < argc; arg++) {
            if (print_dir_tree(argv[arg])) {
                fprintf(stderr, "%s. \n", strerror(errno));
                return EXIT_FAILURE;
            }
        }
    }
    return 0;
}
