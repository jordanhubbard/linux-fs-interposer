# linux-fs-interposer
Demonstrate how to interpose specific linux filesystem calls in user mode to implement tracing.

To execute it on a simple program, just "make test" on a Linux machine and enjoy the output.

Known issues:

1. fork() and posix_spawn() need to be properly interposed to keep the parent/child relationships straight (the trace code should also print the parent pid too, I suppose) and also to properly re-open the tracefile across fork() because the behavior of close-on-exec is undefined as you go down the process hierarchy and a lot of processes like to "clean up" all of their file descriptors on fork() and will stomp the fd, though I catch some of these by trapping close(2).

2. Linux likes to implement the actual syscalls I'm interposing via hidden internal symbols and then violate the abstraction layer inside glibc by cross-calling these internal symbols from other code.  Ulrich, what have you done?
