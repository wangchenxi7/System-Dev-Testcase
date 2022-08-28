#
#


CC=gcc
CFLAGS= -lrdmacm -libverbs

# Default rules 
all : client.c
	$(CC)   client.c -o client.o $(CFLAGS)

clean: 
	rm client.o
