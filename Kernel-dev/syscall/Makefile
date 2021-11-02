

CFLAGS= -O0 -g
CXXFLAGS= -std=c++11

PTHREAD= -lpthread

all:
	cc ${CFLAGS} mremap.c -o mremap.o
	cc ${CFLAGS} rdma_test.c -o rdma_test.o
	cc ${CFLAGS} force_swap_out.c -o force_swap_out.o
	c++ ${CFLAGS} ${CXXFLAGS} cp_flush_and_send_flag.cpp ${PTHREAD} -o cp_flush_and_send_flag.o

clean:
	rm mremap.o
	rm rdma_test.o
	rm force_swap_out.o
	rm cp_flush_and_send_flag.o
