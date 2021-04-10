#define _GNU_SOURCE
#include <dlfcn.h>

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "utils.h"

/*
 * Author:  Jordan K. Hubbard <jkh@nvidia.com>
 * Initial date: 2021/04/08
 *
 * preload: These are the interpositioning functions that we want to
 * register in order to trap key filesystem events.
 */

int open(const char *path, int flags, ...)
{
	va_list ap;
	if (__OPEN_NEEDS_MODE(flags)) {
		mode_t mode;
		int res;

		va_start(ap, flags);
		mode = va_arg(ap, mode_t);
		va_end(ap);
		res = ((real_open_t)dlsym(RTLD_NEXT, "open"))(path, flags, mode);
		if (res != -1)
			emit_traceline("open", "s d d", path, flags, mode);
		return res;
	} else {
		int res = ((real_open_t)dlsym(RTLD_NEXT, "open"))(path, flags);
		if (res != -1)
			emit_traceline("open", "s d", path, flags);
		return res;
	}
}

int openat(int fd, const char *path, int flags, ...)
{
	va_list ap;
	if (__OPEN_NEEDS_MODE(flags)) {
		mode_t mode;
		int res;
		
		va_start(ap, flags);
		mode = va_arg(ap, mode_t);
		va_end(ap);
		res = ((real_openat_t)dlsym(RTLD_NEXT, "openat"))(fd, path, flags, mode);
		if (res != -1)
			emit_traceline("openat", "s d d", path, flags, mode);
		return res;
	} else {
		int ret = ((real_openat_t)dlsym(RTLD_NEXT, "openat"))(fd, path, flags);
		if (ret != -1)
			emit_traceline("openat", "s d", path, flags);
	}
}

int creat(const char *path, mode_t mode)
{
	int ret = ((real_creat_t)dlsym(RTLD_NEXT, "creat"))(path, mode);
	if (ret != -1)
		emit_traceline("creat", "s o", path, mode);
	return ret;
}

ssize_t readlink(const char *path, char *buf, size_t bufsiz)
{
	ssize_t res = ((real_readlink_t)dlsym(RTLD_NEXT, "readlink"))(path, buf, bufsiz);

	if (res != -1)
		emit_traceline("readlink", "s", path);
	return res;
}

int execle(const char *path, const char *arg, ...)
{
	va_list ap;
	char *cp, *argv[255];	/* totally fudged limit */
	char *const *envp;
	int i = 0;
	
	va_start(ap, arg);
	argv[i++] = (char *)arg;
	while (cp = va_arg(ap, char *))
		argv[i++] = cp;
	argv[i] = NULL;
	envp = va_arg(ap, char **);
	va_end(ap);
	emit_traceline("execle", "s v v", path, argv, envp);
	return ((real_execve_t)dlsym(RTLD_NEXT, "execve"))(path, (char *const *)argv, envp);
}

int execl(const char *path, const char *arg, ...)
{
	va_list ap;
	char *cp, *argv[255];	/* totally fudged limit */
	int i = 0;
	
	va_start(ap, arg);
	argv[i++] = (char *)arg;
	while (cp = va_arg(ap, char *))
		argv[i++] = cp;
	argv[i] = NULL;
	va_end(ap);
	emit_traceline("execl", "s v", path, argv);
	return ((real_execvp_t)dlsym(RTLD_NEXT, "execvp"))(path, (char *const *)argv);
}

int execvp(const char *file, char *const argv[])
{
	emit_traceline("execvp", "s v", file, argv);
	return ((real_execvp_t)dlsym(RTLD_NEXT, "execvp"))(file, argv);
}

int execve(const char *path, char *const argv[], char *const envp[])
{
	emit_traceline("execve", "s v", path, argv);
	return ((real_execve_t)dlsym(RTLD_NEXT, "execve"))(path, argv, envp);
}
