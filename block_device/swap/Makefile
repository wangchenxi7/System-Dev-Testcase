

CFLAGS= -O0 -g


all:
	cc ${CFLAGS} swap_streaming.c -o swap_streaming.o
	cc ${CFLAGS} swap_mmap.c -o swap_mmap.o
	cc ${CFLAGS} add_page_to_inactive_list.c -o add_page_to_inactive_list.o
	cc ${CFLAGS} flush_page_to_swap_imm.c -o flush_page_to_swap_imm.o

flush :
	cc ${CFLAGS} flush_page_to_swap_imm.c -o flush_page_to_swap_imm.o


clean:
	rm swap_streaming.o swap_mmap.o
