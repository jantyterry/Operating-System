CC=gcc
CFLAGS=-Wall -g 

all: allocator

allocator: allocator.c listing.h
	$(CC) $(CFLAGS) -fPIC -shared allocator.c -o libmyalloc.so -ldl

clean:
	-rm libmyalloc.so
