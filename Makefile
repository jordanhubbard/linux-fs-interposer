CFLAGS=	-g3

all:	runner sample preload.so

runner: runner.c
	cc runner.c ${CFLAGS} -o runner

sample: sample.c
	cc sample.c ${CFLAGS} -o sample

preload.so: preload.c utils.c utils.h 
	cc -shared -fPIC ${CFLAGS} -o preload.so preload.c utils.c -ldl

test:	runner sample preload.so
	runner - sample

clean:
	rm -f preload.so sample runner
