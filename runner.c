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

int main(int argc, char * const argv[], char * const envp[])
{
	char _cwd[PATH_MAX], *cwd;
	char preload_path[PATH_MAX];
	char tracefile_path[PATH_MAX];
	int status;
	pid_t pid;
	
	if (argc < 2) {
		fprintf(stderr, "Usage: %s outfilename cmd [args]\n", argv[0]);
		exit(1);
	}
	cwd = getcwd(_cwd, PATH_MAX);

	snprintf(preload_path, PATH_MAX, "%s/preload.so", cwd);

	if (argv[1][0] != '/')
		snprintf(tracefile_path, PATH_MAX, "%s/%s", cwd, argv[1]);
	else
		strncpy(tracefile_path, argv[1], PATH_MAX);

	pid = fork();
	if (!pid) {
		char *av[argc];
		int i = 0;

		for (i = 2; i < argc; i++)
			av[i] = argv[i];
		av[i] = NULL;
		setenv("NVIDIA_TRACELOG_FILE", tracefile_path, 1);
		setenv("LD_PRELOAD", preload_path, 1);
		
		execvp(argv[2], av);
	} else {
		int i = waitpid(pid, &status, 0);

		if (i == -1 || !WIFEXITED(status)) {
			fprintf(stderr, "runner: child process exited abnormally: %d\n", status);
			return 1;
		}
		return 0;
	}
}
