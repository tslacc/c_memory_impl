outdir = /tmp/build/
objects = $(outdir)libcustom_alloc.so

$(outdir)main.out: $(outdir) main.c $(objects)
	gcc -L$(outdir) -g -o2 -lcustom_alloc -Wl,-rpath=$(outdir) -Wall -Wextra -o $(outdir)main.out main.c

$(outdir)libcustom_alloc.so: main.c malloc_v2.c malloc.h
	mkdir -p $(outdir)
	gcc -Wall -Wextra -shared -fpic -o $(outdir)libcustom_alloc.so main.c malloc_v2.c


$(outdir):
	mkdir -p $(outdir)

.PHONY: clean
clean:
	rm main.out $(objects)
