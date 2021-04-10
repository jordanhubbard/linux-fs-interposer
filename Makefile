CFLAGS=	-g3

test:	runner sample preload.so
	rm -f tracefile
	runner tracefile sample

runner: runner.c
	cc runner.c ${CFLAGS} -o runner

sample: sample.c
	cc sample.c ${CFLAGS} -o sample

preload.so: preload.c utils.c utils.h 
	cc -shared -fPIC ${CFLAGS} -o preload.so preload.c utils.c -ldl

clean:
	rm -f preload.so sample runner tracefile
