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

/* Set to 1 if you want the runner to fork(2) before exec(2)ing the process */
#define _FORK_CHILD	0

int main(int argc, char * const argv[], char * const envp[])
{
	char _cwd[PATH_MAX], *cwd;
	char preload_path[PATH_MAX];
	char tracefile_path[PATH_MAX];
	int status;
	char prefix;
	pid_t pid;
	
	if (argc < 3) {
		fprintf(stderr, "Usage: %s outfile command [args]\n", argv[0]);
		exit(1);
	}
	cwd = getcwd(_cwd, PATH_MAX);

	snprintf(preload_path, PATH_MAX, "%s/preload.so", cwd);
	prefix = argv[1][0];
	
	/* Check for existing absolute path or -, which means output to stderr */
	if (prefix != '/' && prefix != '-')
		snprintf(tracefile_path, PATH_MAX, "%s/%s", cwd, argv[1]);
	else
		strncpy(tracefile_path, argv[1], PATH_MAX);

#if _FORK_CHILD
	pid = fork();
	if (!pid)
#endif
	{
		setenv("NVIDIA_TRACELOG_FILE", tracefile_path, 1);
		setenv("LD_PRELOAD", preload_path, 1);
		execvp(argv[2], &argv[2]);
	}
#if _FORK_CHILD
	else {
		int i = waitpid(pid, &status, 0);

		if (i == -1 || !WIFEXITED(status)) {
			fprintf(stderr, "runner: child process exited abnormally: %d\n", status);
			return 1;
		}
	}
#endif
	return 0;
}
