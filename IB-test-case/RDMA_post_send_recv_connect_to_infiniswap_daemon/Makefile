#
#


CC=gcc
CFLAGS= -lrdmacm -libverbs

# Default rules 
all : client.c
	$(CC) $(CFLAGS)   client.c -o client 

clean: 
	rm client
