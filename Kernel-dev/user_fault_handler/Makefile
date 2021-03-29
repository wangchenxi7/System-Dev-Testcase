

CFLAGS= -O0 -g 
LIBS= -lpthread

all:
	cc ${CFLAGS} user_fault_handler_poll.c -o user_fault_handler_poll.o ${LIBS}
	cc ${CFLAGS} user_fault_swap_prefetch.c -o user_fault_swap_prefetch.o ${LIBS}
	cc ${CFLAGS} user_fault_swap_prefetch_multi_commit.c -o user_fault_swap_prefetch_multi_commit.o ${LIBS}
	cc ${CFLAGS} user_fault_vma_merge.c -o user_fault_vma_merge.o ${LIBS}

clean:
	rm user_fault_handler_poll.o user_fault_swap_prefetch.o user_fault_swap_prefetch_multi_commit.o user_fault_vma_merge.o
