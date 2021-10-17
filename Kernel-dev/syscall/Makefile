

CFLAGS= -O0 -g


all:
	cc ${CFLAGS} mremap.c -o mremap.o
	cc ${CFLAGS} rdma_test.c -o rdma_test.o
	cc ${CFLAGS} force_swap_out.c -o force_swap_out.o

clean:
	rm mremap.o
	rm rdma_test.o
	rm force_swap_out.o
