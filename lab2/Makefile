CC = gcc
CFLAGS = -Wall -c -g -std=gnu99

MYOFILES = mythreads.o listing.o
MYHEADERS = listing.h mythreads.h

all:  libmythreads.a

libmythreads.a: mythreads.o listing.o
	ar -cvr $@ $(MYOFILES)
	
%.o: %.c $(MYHEADERS)
	$(CC) $(CFLAGS) $<

clean:
	rm -rf *.o *.a
