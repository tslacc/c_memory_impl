outdir = /tmp/build

$(outdir)main.out: main.c malloc_v2.c malloc.h
	mkdir -p $(outdir)
	gcc -Wall -Wextra -o $(outdir)main.out main.c malloc_v2.c
