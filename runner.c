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
	char cwd[PATH_MAX];
	char preload_path[PATH_MAX];
	char tracefile_path[PATH_MAX];
	int status;
	pid_t pid;
	
	if (argc < 2) {
		fprintf(stderr, "Usage: %s outfilename cmd [args]\n", argv[0]);
		exit(1);
	}
	(void)getcwd(cwd, PATH_MAX);

	sprintf(preload_path, "%s/preload.so", cwd);

	if (argv[1][0] != '/')
		sprintf(tracefile_path, "%s/%s", cwd, argv[1]);
	else
		strcpy(tracefile_path, argv[1]);

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
