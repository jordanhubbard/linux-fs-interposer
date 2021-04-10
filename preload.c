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

int execve(const char *path, char *const argv[], char *const envp[])
{
	/* execve() is special in that we need to log first, not afterwards, for obvious reasons */
	emit_traceline("execve", "s v", path, argv);
	return ((real_execve_t)dlsym(RTLD_NEXT, "execve"))(path, argv, envp);
}

int execl(const char *path, const char *arg, ...)
{
	/* execl() is special in that we need to log first, not afterwards, for obvious reasons */
	emit_traceline("execl", "s s", path, arg);
	return ((real_execl_t)dlsym(RTLD_NEXT, "execl"))(path, arg, NULL);
}
