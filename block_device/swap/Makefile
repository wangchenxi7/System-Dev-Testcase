

PTHREAD= -lpthread

CFLAGS= -O0 -g
CXXFLAGS= -std=c++11

all:
	cc ${CFLAGS} swap_streaming.c -o swap_streaming.o
	cc ${CFLAGS} swap_mmap.c -o swap_mmap.o
	cc ${CFLAGS} swap_mmap_random.c -o swap_mmap_random.o
	cc ${CFLAGS} swap_mmap_dual_path.c ${PTHREAD} -o swap_mmap_dual_path.o
	c++ ${CFLAGS} ${CXXFLAGS} swap_mmap_multi_thread.cpp ${PTHREAD} -o swap_mmap_multi_thread.o
	cc ${CFLAGS} add_page_to_inactive_list.c -o add_page_to_inactive_list.o
	cc ${CFLAGS} flush_page_to_swap_imm.c -o flush_page_to_swap_imm.o
	cc ${CFLAGS} swap_out_ratio_flush_based.c -o swap_out_ratio_flush_based.o
	cc ${CFLAGS} control_path.c -o control_path.o
	cc ${CFLAGS} flush_page_to_swap_imm.c -o flush_page_to_swap_imm.o
	cc ${CFLAGS} control_path_after_data_path_flush.c -o control_path_after_data_path_flush.o
	cc ${CFLAGS} on_demand_swapin.c -o on_demand_swapin.o 
	cc ${CFLAGS} on_demand_swapin_monitor.c -o on_demand_swapin_monitor.o
	cc ${CFLAGS} swap_info_count.c -o swap_info_count.o
	cc ${CFLAGS} swap_reset_counter.c -o swap_reset_counter.o
	cc ${CFLAGS} swap_counter_read.c -o swap_counter_read.o
	cc ${CFLAGS} remoteswap_reset_counter.c -o remoteswap_reset_counter.o
	cc ${CFLAGS} remoteswap_read_counter.c -o remoteswap_read_counter.o

clean:
	rm swap_streaming.o swap_mmap.o add_page_to_inactive_list.o  flush_page_to_swap_imm.o swap_out_ratio_flush_based.o  control_path.o \
			control_path_after_data_path_flush.o   on_demand_swapin.o  on_demand_swapin_monitor.o swap_info_count.o swap_reset_counter.o \
			swap_counter_read.o swap_mmap_dual_path.o swap_mmap_random.o  swap_mmap_multi_thread.o remoteswap_reset_counter.o remoteswap_read_counter.o
