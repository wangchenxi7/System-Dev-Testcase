/**
 * ### Basic Information
 * Test the user space page fault handler function.
 * For Kernel 5.4
 * A Page Fault Notifications is raised and send the polling thread.
 * 
 * Use the kernel headers under /usr/include/linux/
 * Install the kernel headers to this path after chaing it.
 * 
 * ### Current Testcase
 * 
 * Commit several contiguous virtual ranges and try to register them to user-fault fd together.
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


#include "stdint.h"
#include "stdio.h"
#include "stddef.h"


typedef enum {true, false} bool;

extern errno;

#ifndef PAGE_SIZE
	#define PAGE_SIZE 4096
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
	struct pthread_args *p = arg;
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
			fprintf(stderr, "%s, Received page fault at 0x%lx \n", __func__, addr);


			// Pass information down to kernel
			struct uffdio_swap_prefetch swap_prefetch;
			swap_prefetch.prefetch_len	= 1; // page size
			for(i=0; i< swap_prefetch.prefetch_len; i++){
				swap_prefetch.prefetch_virt_page[i] = (unsigned long)addr + PAGE_SIZE * i ; // prefetch next page
			}

			fprintf(stderr, "%s, uffd cmd 0x%lx , args 0x%lx \n", 
				__func__, (unsigned long)UFFDIO_SWAP_PREFETCH,  (unsigned long)&swap_prefetch);

			if(ioctl(p->uffd, UFFDIO_SWAP_PREFETCH, &swap_prefetch) == -1) {
				perror("ioctl/swap_prefetch");
				goto exit_handler;
			}


			struct uffdio_copy copy;
			copy.src = (unsigned long)buf;
			copy.dst = (unsigned long)addr; // the address triggered the uffd
			copy.len = PAGE_SIZE;
			copy.mode = 0;  // [?] copy mode ?

			fprintf(stderr, "%s, uffd cmd 0x%lx , args 0x%lx \n", 
					__func__, (unsigned long)UFFDIO_COPY,  (unsigned long)&copy);

			// Apply the UFFDIO_COPY operation
			if (ioctl(p->uffd, UFFDIO_COPY, &copy) == -1) {
				perror("ioctl/copy");
				goto exit_handler;
			}
		}

	}

exit_handler:
	return NULL;
}

int main(int argc, char *argv[])
{

	int fd = 0;

	char *user_buf;
	unsigned long user_buffer_size = 0x2000; // 8KB

	unsigned long user_buffer_addr_1 = 0x20000000; // 512MB
	unsigned long user_buffer_addr_2 =
	    user_buffer_addr_1 + user_buffer_size;
	unsigned long user_buffer_addr_3 =
	    user_buffer_addr_2 + user_buffer_size;

	pthread_t uffd_thread; // uffd handler daemon thread.

	int i;
	char *ptr;

	//
	// 1) Enable the user fault fd.

	// Get a fd from kernel for user fault usage.
	// We use this fd to manage a range of virual memory.
	// O_NONBLOCK : ? The user process is not blocked when the page fault is
	// triggerred ?
	if ((fd = syscall(__NR_userfaultfd, O_NONBLOCK)) == -1) {
		printf("++ userfaultfd failed \n");
		goto out;
	}

	// Enable the user fualt fd API
	// [?] the fd will lead to ioctl function to userfaultfd_ioctl.
	struct uffdio_api api = {.api = UFFD_API};
	if (ioctl(fd, UFFDIO_API, &api)) {
		printf("++ ioctl(fd, UFFDIO_API, ...) failed \n");
		goto out;
	}

	// 2) Prepare the user space memory
	fprintf(stderr,
	        "Phase2, Register [0x%lx, 0x%lx) as range for user page fault "
	        "handling. \n",
	        user_buffer_addr_1, user_buffer_addr_1 + 3 * user_buffer_size);

	// 2.1 Commit the ranges separetely
	// Access it will lead to page fault.
	user_buf = commit_anon_memory((char *)user_buffer_addr_1,
	                              user_buffer_size, true);
	if (user_buf == NULL) {
		printf("Commit user_buffer_addr_1, 0x%lx failed. \n",
		       user_buffer_addr_1);
	} else {
		printf("Commit user_buffer_addr_1: 0x%lx, bytes_len: 0x%lx \n",
		       user_buffer_addr_1, user_buffer_size);
	}

	// buffer 2
	user_buf = commit_anon_memory((char *)user_buffer_addr_2,
	                              user_buffer_size, true);
	if (user_buf == NULL) {
		printf("Commit user_buffer_addr_2, 0x%lx failed. \n",
		       user_buffer_addr_2);
	} else {
		printf("Commit user_buffer_addr_2: 0x%lx, bytes_len: 0x%lx \n",
		       user_buffer_addr_2, user_buffer_size);
	}

	user_buf = commit_anon_memory((char *)user_buffer_addr_3,
	                              user_buffer_size, true);
	if (user_buf == NULL) {
		printf("Commit user_buffer_addr_3, 0x%lx failed. \n",
		       user_buffer_addr_3);
	} else {
		printf("Commit user_buffer_addr_3: 0x%lx, bytes_len: 0x%lx \n",
		       user_buffer_addr_3, user_buffer_size);
	}

	// 2.2 Register these separate virual memory ranges for the uffd
	struct uffdio_register reg = {
	    .mode =
	        UFFDIO_REGISTER_MODE_MISSING, /*both minor and major faults*/
	    .range = {.start = (unsigned long)user_buffer_addr_1,
	              .len = user_buffer_size * 3}};

	if (ioctl(fd, UFFDIO_REGISTER, &reg)) {
		fprintf(stderr, "++ ioctl(fd, UFFDIO_REGISTER, ...) failed\n");
		goto out;
	}

	// launch a daemon thread to handle the page fault.
	struct pthread_args p;
	p.uffd = fd; // pass the uffd to the daemon thread
	pthread_create(&uffd_thread, NULL, handler, &p);

	sleep(1); // wait the launching of the uffd daemon thread.

	// 3) Trigger the page fault
	// 		Touch each page to trigger a page fault.
	//
	fprintf(stderr, "Phase3, trigger page fault.\n");

	// 3.1 uffd range#1
	ptr = (char *)user_buffer_addr_1;
	size_t offset = 8; // bytes
	for (i = offset; i < user_buffer_size; i += PAGE_SIZE) {
		fprintf(stderr,
		        "Try to trigger fault page thread: Page[%d] (addr "
		        "0x%lx) : %s (value) \n",
		        i / 4096, (unsigned long)(ptr + i), ptr + i);
	}

	fprintf(stderr, " \n\n access uffd range\n");

	// 3.2 uffd range #2
	ptr = (char *)user_buffer_addr_2;
	;
	offset = 8; // bytes
	for (i = offset; i < user_buffer_size; i += PAGE_SIZE) {
		fprintf(stderr,
		        "Try to trigger fault page thread: Page[%d] (addr "
		        "0x%lx) : %s (value) \n",
		        i / 4096, (unsigned long)(ptr + i), ptr + i);
	}

	// 3.2 uffd range #3
	ptr = (char *)user_buffer_addr_3;
	;
	offset = 8; // bytes
	for (i = offset; i < user_buffer_size; i += PAGE_SIZE) {
		fprintf(stderr,
		        "Try to trigger fault page thread: Page[%d] (addr "
		        "0x%lx) : %s (value) \n",
		        i / 4096, (unsigned long)(ptr + i), ptr + i);
	}

out:
	return 0;
}
