

CFLAGS= -O0 -g


all:
	cc ${CFLAGS} swap_streaming.c -o swap_streaming.o
	cc ${CFLAGS} swap_mmap.c -o swap_mmap.o
	cc ${CFLAGS} add_page_to_inactive_list.c -o add_page_to_inactive_list.o
	cc ${CFLAGS} flush_page_to_swap_imm.c -o flush_page_to_swap_imm.o
	cc ${CFLAGS} swap_out_ratio_flush_based.c -o swap_out_ratio_flush_based.o
	cc ${CFLAGS} control_path.c -o control_path.o
	cc ${CFLAGS} flush_page_to_swap_imm.c -o flush_page_to_swap_imm.o
	cc ${CFLAGS} control_path_after_data_path_flush.c -o control_path_after_data_path_flush.o


clean:
	rm swap_streaming.o swap_mmap.o
