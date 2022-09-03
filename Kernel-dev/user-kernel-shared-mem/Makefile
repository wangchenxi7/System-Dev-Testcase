

CFLAGS= -O0 -g 
LIBS= -lpthread

all:
	cc ${CFLAGS} read_page_swap_status.c -o read_page_swap_status.o ${LIBS}
	cc ${CFLAGS} register_user_kernel_shared_buf.c -o register_user_kernel_shared_buf.o ${LIBS}


clean:
	rm read_page_swap_status.o register_user_kernel_shared_buf.o
