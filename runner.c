#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
 * Author:  Jordan K. Hubbard <jkh@nvidia.com>
 * Initial date: 2021/04/08
 *
 * This is a simple runner for executing things under trace.  It is very simple (too simple) as
 * it should probably fork() and clean up fds before execing the passed-in command and args,
 * but it works fine as a basic POC.
 */

int main(int ac, char * const av[])
{
	char path[PATH_MAX + 1];
	char trace[PATH_MAX + 1];
	int status;
	pid_t pid;
	
	if (ac < 2) {
		fprintf(stderr, "Usage: %s outfilename cmd [args]\n", av[0]);
		exit(1);
	}
	(void)getcwd(path, PATH_MAX);
	strncat(path, "/preload.so", PATH_MAX);
	if (av[1][0] != '/') {
		(void)getcwd(trace, PATH_MAX);
		strncpy(trace, "/", PATH_MAX);
		strncpy(trace, av[1], PATH_MAX);
	} else {
		strncpy(trace, av[1], PATH_MAX);
	}
	pid = fork();
	if (!pid) {
		setenv("NVIDIA_TRACELOG_FILE", trace, 1);
		setenv("LD_PRELOAD", path, 1);
		execv(av[2], av + 2);
	} else {
		int i = waitpid(pid, &status, 0);

		if (i == -1 || !WIFEXITED(status)) {
			fprintf(stderr, "runner: child process exited abnormally\n");
			return 1;
		}
		return 0;
	}
}
