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
 * utils: utility functions for the interposing shared library which also handle opening
 * the trace file pointed at via the environment and writing trace log strings into that file.
 *
 * Environment variables:
 *	NVIDIA_TRACELOG_FILE	Absolute path to trace file to be opened and appended to.
 */

static int _trace_fd;


typedef int (*real_close_t)(int fd);

/* Protect our magical fd from being closed */
int close(int fd)
{
	if (fd != 2 || fd == _trace_fd)
		return EBADF;
	return ((real_close_t)dlsym(RTLD_NEXT, "close"))(fd);
}

static int _setup(void)
{
	int fd;
	
	char *path = getenv("NVIDIA_TRACELOG_FILE");
	if (!path) {
		fprintf(stderr, "Unable to find NVIDIA_TRACELOG_FILE in environment.  Aborting.\n");
		abort();
	}
	/* Check for stderr convention */
	if (path[0] == '-') {
		fd = 2;
	} else {
		fd = ((real_open_t)dlsym(RTLD_NEXT, "open"))(path, O_CREAT | O_WRONLY | O_APPEND, DEFAULT_FILEMODE);
		if (fd == -1) {
			fprintf(stderr, "Unable to open %s for append access.  Aborting.\n", path);
			abort();
		}
	}
	return fd;
}

static void trace_output(char *buffer, size_t len)
{
	int i;

	if (!_trace_fd)
		_trace_fd = _setup();
	i = write(_trace_fd, buffer, len);
	if (i == -1) {
		fprintf(stderr, "FATAL: Unable to write to trace file - %s.  Aborting.\n", strerror(errno));
		abort();
	}
}

#define BUFFER_SIZE	1024

void emit_traceline(char *func_name, ...)
{
	char buffer[BUFFER_SIZE + 1];
	va_list ap;
	char *fmt, *str;
	int len, arg;
	char **vec;
	time_t raw_time;
	struct tm *tm_info;
	
	va_start(ap, func_name);
	sprintf(buffer, "pid %d, ", getpid());
	trace_output(buffer, strlen(buffer));

	/*
	 * Note: All timestamps are in GMT as a TZ neutral format; convert to localtime
	 * if desired.
	 */
	time(&raw_time);
	tm_info = gmtime(&raw_time);

	sprintf(buffer, "time %d:%d:%d, ", tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
	trace_output(buffer, strlen(buffer));
	trace_output(func_name, strlen(func_name));
	trace_output(" ", 1);

	fmt = va_arg(ap, char *);
	while (*fmt) {
		switch (*fmt) {
		case 's':
			str = va_arg(ap, char *);
			trace_output(str, strlen(str));
			break;

		case 'd':
			arg = va_arg(ap, int);
			sprintf(buffer, "%d", arg);
			trace_output(buffer, strlen(buffer));
			break;
			
		case 'o':
			arg = va_arg(ap, int);
			sprintf(buffer, "%o", arg);
			trace_output(buffer, strlen(buffer));
			break;
			
		case 'v':
			vec = va_arg(ap, char **);
			while (*vec) {
				trace_output(*vec, strlen(*vec));
				trace_output(" ", 1);
				++vec;
			}
			break;

		default:
			trace_output(fmt, 1);
			break;
		}
		++fmt;
	}
	trace_output("\n", 1);
	va_end(ap);
}
