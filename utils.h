#ifndef _UTILS_H_INCLUDE
#define _UTILS_H_INCLUDE

/* These are all of the library functions we're interposing for */
typedef int (*real_open_t)(const char *path, int flags, ...);
typedef int (*real_openat_t)(int fd, const char *path, int flags, ...);
typedef int (*real_creat_t)(const char *path, mode_t mode);
typedef size_t (*real_readlink_t)(const char *path, char *buf, size_t bufsiz);
typedef int (*real_execve_t)(const char *path, char *const argv[], char *const envp[]);
typedef int (*real_execl_t)(const char *path, const char *arg, ...);

void emit_traceline(char *func_name, ...);

#define DEFAULT_FILEMODE	0644

#endif
