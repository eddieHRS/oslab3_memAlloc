myprogram:mymain.c libmem.so
	gcc mymain.c -lmem -L. -o myprogram
libmem.so:mem.c mem.h
	gcc -shared -fPIC mem.c -o libmem.so

	