

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
	cc ${CFLAGS} on_demand_swapin.c -o on_demand_swapin.o 
	cc ${CFLAGS} on_demand_swapin_monitor.c -o on_demand_swapin_monitor.o
	cc ${CFLAGS} swap_info_count.c -o swap_info_count.o

clean:
	rm swap_streaming.o swap_mmap.o add_page_to_inactive_list.o  flush_page_to_swap_imm.o swap_out_ratio_flush_based.o  control_path.o \
			flush_page_to_swap_imm.o  control_path_after_data_path_flush.o   on_demand_swapin.o  on_demand_swapin_monitor.o swap_info_count.o
