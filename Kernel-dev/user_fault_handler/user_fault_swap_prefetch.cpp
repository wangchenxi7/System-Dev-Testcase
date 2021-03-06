/**
 * Test the user space page fault handler function.
 * For Kernel 5.4
 * A Page Fault Notifications is raised and send the polling thread.
 * 
 * Use the kernel headers under /usr/include/linux/
 * Install the kernel headers to this path after chaing it.
 * 
 * 
 * 1) Page fault can be:
 *  1.1) The first time to touch the virtual memory. Need to map physical memory to the virtual memory range. 
 *  1.2) The mapped physical page is swapped out.
 * 
 * 2) ioctl ? 
 *    The virtual memory range is treated as a memory backed file ?
 * 
 * 3) The uffd is handled in an asynchronized way.
 *    Need to launch a daemon thread to pool events on specified io fd.
 *    When a page fault is triggerred, kernel will inform the daemon thread via the fd event.
 * 
 * 		[x] kernel return a "struct uffd_msg" back to the daemon thread.
 * 			  The uffd_msg contains the information about which page is on the page fault:
 * 
 * 			struct {
 *				__u64	flags;
					__u64	address;  // the virtual address, page aligned. However, the do_page_fault() know the extact virtual address.
					union {
						__u32 ptid;   // [?] the pthread who triggers the page fault
					} feat;
				} pagefault;
 * 
 * 
 */


#define _GNU_SOURCE  1  // Enable the macro defination of mremap prot flags
#define USE_USERFAULT 1 // [?] Enable the user page fault handler ?


#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/mman.h>  
#include <sys/errno.h>

#include <sys/ioctl.h>
#include <poll.h>
#include <fcntl.h>
#include <linux/userfaultfd.h>
#include <pthread.h>

#include "stdlib.h"
#include "stdint.h"
#include "stdio.h"
#include "stddef.h"

// c++ stl multithread random number generator
#include <random>

extern errno;

#ifndef PAGE_SIZE
	#define PAGE_SIZE 4096
#endif

#ifndef PAGE_SHIFT
  #define PAGE_SHIFT	(12UL)
#endif

#ifndef PAGE_MASK
  #define PAGE_MASK (~((1UL << PAGE_SHIFT) - 1))
#endif

#ifndef ONE_MB
	#define ONE_MB 1UL<<20
#endif


/**
 * Reserve memory at fixed address 
 */
static char* reserve_anon_memory(char* requested_addr, unsigned long bytes, bool fixed) {
	char * addr;
	int flags;

	flags = MAP_PRIVATE | MAP_NORESERVE | MAP_ANONYMOUS;   
	if (fixed == true) {
			printf("Request fixed addr 0x%lx ", (unsigned long)requested_addr);

		flags |= MAP_FIXED;
	}

	// Map reserved/uncommitted pages PROT_NONE so we fail early if we
	// touch an uncommitted page. Otherwise, the read/write might
	// succeed if we have enough swap space to back the physical page.
	addr = (char*)mmap(requested_addr, bytes, PROT_NONE,
											 flags, -1, 0);

	return addr == MAP_FAILED ? NULL : addr;
}



/**
 * Commit memory at reserved memory range.
 *  
 */
char* commit_anon_memory(char* start_addr, unsigned long size, bool exec) {
	int prot = (exec == true) ? PROT_READ|PROT_WRITE|PROT_EXEC : PROT_READ|PROT_WRITE;
	unsigned long res = (unsigned long)mmap(start_addr, size, prot,
																		 MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0);   // MAP_FIXED will override the old mapping
	
	// commit memory successfully.
	if (res == (unsigned long) MAP_FAILED) {

		// print errno here.
		return NULL;
	}

	return start_addr;
}



//
// User Fault FD structures
//

// parameters passed to pthread
struct pthread_args{
	int uffd;  // the registered ioctl fd


};


//
// Pthread handlers 


// pooling the fd events 
static void *handler(void *arg)
{
	struct pthread_args *p = (struct pthread_args*)arg;
	bool stop = false;
	char buf[PAGE_SIZE] = "test buffer";  // [?] What's the purpose of this buffer?
	int i;

	fprintf(stderr, "%s, User Fault Handler is on.\n", __func__);
	// [?] Infinite loop
	for (;;) {

		// PD's corresponding i/o message from kernel ?
		struct uffd_msg msg;

		struct pollfd pollfd[1];  // the function poll() is designed to monitor an array of pollfd.
		pollfd[0].fd = p->uffd;
		pollfd[0].events = POLLIN;  // [?] read data from i/o ? for user-fault handler, should write data back to kernel ?

		// wait for a userfaultfd event to occur
		// parameters: (struct pollfd *fds, nfds_t num_fds, const struct timespec *tmo_p, const sigset_t *sigmask)
		// 3rd, time out? if we don't receive any event within the timeout limits, return immediately.
		//	negative value means wait until get event.
		fprintf(stderr, "%s, poll on fd 0x%lx \n", __func__, (unsigned long)pollfd);
		int pollres = poll(pollfd, 1, -1);

		if (stop == true)
			return NULL;

		switch (pollres) {
		case -1:
			perror("poll/userfaultfd");
			continue;
		case 0:
			continue;
		case 1:
			break;  // goto handle this pd event
		default:
			fprintf(stderr, "unexpected poll result\n");
			goto exit_handler;
		}
		fprintf(stderr, "%s, Get uffd event %d \n", __func__, pollfd[0].revents);
		
		if (pollfd[0].revents & POLLERR) {
			fprintf(stderr, "POLLERR\n");
			goto exit_handler;
		}

		if (pollfd[0].revents & POLLNVAL) {
			fprintf(stderr, "POLLNVAL\n");
			goto exit_handler;
		}

		if (!pollfd[0].revents & POLLIN) {
			fprintf(stderr, "return %d, NOT POLLIN\n", pollfd[0].revents);
			continue;
		}

		// Read the message after the pd signal
		int readres = read(p->uffd, &msg, sizeof(msg));
		if (readres == -1) {
			if (errno == EAGAIN)
				continue;
			perror("read/userfaultfd");
			goto exit_handler;
		}

		if (readres != sizeof(msg)) {
			fprintf(stderr, "invalid msg size\n");
			goto exit_handler;
		}

		// handle the page fault by copying a page worth of bytes
		// [?] Copy the content of buf to the page who triggerred the page fault ?
		//     Don't we allocate physical page to the virtual page first ??
		//     Or the kernel will handle the physical page allocation directly ?
		if (msg.event & UFFD_EVENT_PAGEFAULT) {
			unsigned long addr = msg.arg.pagefault.address;
			unsigned pid = msg.arg.pagefault.feat.ptid;
			fprintf(stderr, "%s, Received page fault, Thread id %u at addr 0x%lx \n", __func__, pid, addr);


			// Pass information down to kernel
			struct uffdio_swap_prefetch swap_prefetch;
			swap_prefetch.vma_addr = msg.arg.reserved.reserved1;
			if(msg.arg.reserved.reserved1 == 0UL){
				fprintf(stderr, "%s, UFFDIO_SWAP_PREFETCH get a NULL vma field from uffd_msg message.", __func__);
			}
			swap_prefetch.prefetch_chunk_num = 1;	// 1 segment of pages
			swap_prefetch.prefetch_chunk_page_len[0]	= 1; // 1 page 
			swap_prefetch.prefetch_chunk_start[0] = (unsigned long)addr + PAGE_SIZE ; // prefetch next page
			
			fprintf(stderr, "%s, UFFDIO_SWAP_PREFETCH  uffd cmd 0x%lx , args 0x%lx \n", 
				__func__, (unsigned long)UFFDIO_SWAP_PREFETCH,  (unsigned long)&swap_prefetch);

			if(ioctl(p->uffd, UFFDIO_SWAP_PREFETCH, &swap_prefetch) == -1) {
				perror("ioctl/swap_prefetch");
				goto exit_handler;
			}

		}// get UFFD_EVENT_PAGEFAULT

	} // infinite for loop

exit_handler:
	// free resource
	free(arg);

	return NULL;
}





int main(int argc, char* argv[]){

  int fd = 0;
  
  char* user_buf;
  unsigned long user_buffer_addr = 0x400000000000UL; // start addr
  unsigned long user_buffer_size = 0x10000000; // 128MB size 

	pthread_t uffd_thread;  // uffd handler daemon thread.
	int i;
	unsigned long *ptr;

   //
   // 1) Enable the user fault fd.

  // Get a fd from kernel for user fault usage.
  // We use this fd to manage a range of virual memory.
  // O_NONBLOCK : ? The user process is not blocked when the page fault is triggerred ?
  if ((fd = syscall(__NR_userfaultfd, O_NONBLOCK)) == -1) {
		printf("++ userfaultfd failed \n");
		return 0;
	}


  // Enable the user fualt fd API
  // [?] the fd will lead to ioctl function to userfaultfd_ioctl.
  struct uffdio_api api = { .api = UFFD_API };
  if (ioctl(fd, UFFDIO_API, &api)) {
		printf( "++ ioctl(fd, UFFDIO_API, ...) failed \n");
		return 0;
	}


  // 2) Prepare the user space memory
	fprintf(stderr, "Phase2, Register [0x%lx, 0x%lx) as range for user page fault handling. \n", 
																			user_buffer_addr, user_buffer_addr+user_buffer_size );
  // 2.1 Reserve space at specified virtual memory.
	// We can commit the range dirrectly without reserving.
	user_buf = reserve_anon_memory((char*)user_buffer_addr, user_buffer_size, true);
	if(user_buf == NULL){
		printf("Reserve user_buffer, 0x%lx failed. \n", user_buffer_addr);
	}else{
		printf("Reserve user_buffer: 0x%lx, bytes_len: 0x%lx \n",user_buffer_addr, user_buffer_size);
	}
	
	// 2.2 Commit the range 
	// Access it will lead to page fault.
	user_buf = commit_anon_memory((char*)user_buffer_addr, user_buffer_size, true);
  if(user_buf == NULL){
		printf("Commit user_buffer, 0x%lx failed. \n", user_buffer_addr);
	}else{
		printf("Commit user_buffer: 0x%lx, bytes_len: 0x%lx \n",user_buffer_addr, user_buffer_size);
	}

  
  // UFFDIO_REGISTER_MODE_WP : Register the virual memory range for the uffd, swap fault event.
	// Pass the page addr triggered the major swap fault.
	// In some design, only pass the page fault info to user when the fault pattern can't be detected by kernle prefetcher.
	struct uffdio_register reg;
	reg.mode = UFFDIO_REGISTER_MODE_WP;
	reg.range.start = (unsigned long)user_buf;
	reg.range.len = user_buffer_size;


  if (ioctl(fd, UFFDIO_REGISTER,  &reg)) {
		fprintf(stderr, "++ ioctl(fd, UFFDIO_REGISTER, ...) failed\n");
		return 0;
	}

	// launch a daemon thread to handle the page fault.
	struct pthread_args* p = (struct pthread_args*)malloc(sizeof(struct pthread_args));
	p->uffd = fd; // pass the uffd to the daemon thread
	pthread_create(&uffd_thread, NULL, handler, p);

	sleep(1); // wait the launching of the uffd daemon thread.

	// 3) Trigger the page fault
	// 		Touch each page to trigger a page fault.
	//      
	fprintf(stderr, "Phase3, trigger page fault.\n");
	unsigned long sum = 0;

	// 3.1 uffd range in sequential
	fprintf(stderr, " \n\n access uffd range in sequential pattern \n");
	ptr = (unsigned long*)user_buf;
	int offset = 0; //bytes
	int end = user_buffer_size/sizeof(unsigned long);
	int step = 1;
	for(i=offset; i<end; i+= step ){
		//fprintf(stderr, "Trigger fault on non-uffd range thread: Page[%d] (addr 0x%lx) : %s (value) \n", i/4096, (unsigned long)(ptr+i), ptr+i );
		*(ptr+i) = (unsigned long)i;
	}

	// 3.2 uffd range in random
	fprintf(stderr, " \n\n access uffd range in random pattern \n");

	// MT random number generator
	thread_local std::mt19937 engine(std::random_device{}());
	std::uniform_int_distribution<int> dist(1, user_buffer_size);


	ptr = (unsigned long*)user_buf;
	offset = 0; //bytes
	end = user_buffer_size/sizeof(unsigned long);

	for(i=0; i<128; i++ ){
		//fprintf(stderr, "Trigger fault on uffd range thread: Page[%d] (addr 0x%lx) : %s (value) \n", i/4096, (unsigned long)(ptr+i), ptr+i );
		unsigned long array_index = (unsigned long)dist(engine)/sizeof(unsigned long) % end;
		sum += (unsigned long)*(ptr + array_index);
	}

	fprintf(stderr, "%s, sum %lu \n", __func__, sum);

  return 0;
}




