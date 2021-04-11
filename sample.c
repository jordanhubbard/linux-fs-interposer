#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
 * Author:  Jordan K. Hubbard <jkh@nvidia.com>
 * Initial date: 2021/04/08
 *
 * Simple test program to do LD_PRELOAD hack on - it expects to be run by the runner
 * utility which is also part of this package, though you can also run it directly if
 * you wish, just set the LD_PRELOAD environment variable to point to the
 * preload.so file as well as any environment variable(s) which are called out in
 * preload.c
 */

#define _OPEN_TEST_FILE		"/etc/passwd"
#define _CREAT_TEST_FILE	"/tmp/create_this_file"
#define _READLINK_TEST_FILE	"/etc/localtime"
#define _EXEC_TEST_FILE		"/bin/echo"

int main(int ac, const char *av[])
{
	int fd;
	ssize_t cnt;
	char buf[1024];
	pid_t pid;
	char *str = "this is a test";
	
	/* test the open(2) interceptor */
	fd = open(_OPEN_TEST_FILE, O_RDONLY);
	if (fd == -1) {
		fprintf(stderr, "Unable to open(%s) as test file\n", _OPEN_TEST_FILE);
		abort();
	} else {
		cnt = read(fd, buf, sizeof(buf) - 1);
		if (cnt == -1)
			fprintf(stderr, "Unable to read a line from %s\n", _OPEN_TEST_FILE);
		close(fd);
	}

	/* test the creat(2) interceptor */
	fd = creat(_CREAT_TEST_FILE, 0644);
	if (fd == -1) {
		fprintf(stderr, "Unable to creat(%s) as test file\n", _CREAT_TEST_FILE);
		abort();
	} else {
		cnt = write(fd, str, strlen(str));
		if (cnt != strlen(str))
			fprintf(stderr, "Unable to write a test string to %s\n", _CREAT_TEST_FILE);
		close(fd);
	}

	/* test the readlink(2) interceptor */
	cnt = readlink(_READLINK_TEST_FILE, buf, sizeof(buf));
	if (cnt == -1)
		fprintf(stderr, "Unable to readlink(%s) as a test link\n", _READLINK_TEST_FILE);

	/* test the execve() interceptor */
	if (!(pid = fork()))
		execl(_EXEC_TEST_FILE, _EXEC_TEST_FILE, "I like cats", NULL);
	else {
		int status;
		
		/* should check return value and status but who cares */
		waitpid(pid, &status, 0);
	}
	return 0;
}

