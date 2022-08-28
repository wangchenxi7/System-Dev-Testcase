#
#


CC=gcc
CFLAGS=
LDLIBS= -lrdmacm -libverbs

# Default rules 
all : client.c
	$(CC) $(CFLAGS)   client.c -o client $(LDLIBS)

clean: 
	rm client
