/**
 * Test the user space page fault handler function.
 * For Kernel 4.11.
 * A Page Fault Notifications is raised and send the polling thread.
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
static char* reserve_anon_memory(char* requested_addr, uint64_t bytes, bool fixed) {
	char * addr;
	int flags;

	flags = MAP_PRIVATE | MAP_NORESERVE | MAP_ANONYMOUS;   
	if (fixed == true) {
			printf("Request fixed addr 0x%llx ", (uint64_t)requested_addr);

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
char* commit_anon_memory(char* start_addr, uint64_t size, bool exec) {
	int prot = (exec == true) ? PROT_READ|PROT_WRITE|PROT_EXEC : PROT_READ|PROT_WRITE;
	uint64_t res = (uint64_t)mmap(start_addr, size, prot,
																		 MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0);   // MAP_FIXED will override the old mapping
	
	// commit memory successfully.
	if (res == (uint64_t) MAP_FAILED) {

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
		// 3rd, time out? the poll() will occupy a core and busy wait on it ?
		// 4th, sigset can be omitted ?
		fprintf(stderr, "%s, poll on fd %d \n", __func__, pollfd);
		int pollres = poll(pollfd, 1, 2000);

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
			fprintf(stderr, "pollerr\n");
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
			uint64_t addr = msg.arg.pagefault.address;
			fprintf(stderr, "%s, Received page fault at 0x%llx \n", __func__, addr);

			struct uffdio_copy copy;
			copy.src = (uint64_t)buf;
			copy.dst = (uint64_t)addr;
			copy.len = PAGE_SIZE;
			copy.mode = 0;  // [?] copy mode ?

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





int main(int argc, char* argv[]){

  int fd = 0;
  
  char* user_buf;
  uint64_t user_buffer_addr = 0x20000000; // 512MB
  uint64_t user_buffer_size = 0x2000; // 8KB
	pthread_t uffd_thread;  // uffd handler daemon thread.

	int i;
	char *ptr;

   //
   // 1) Enable the user fault fd.

  // Get a fd from kernel for user fault usage.
  // We use this fd to manage a range of virual memory.
  // O_NONBLOCK : ? The user process is not blocked when the page fault is triggerred ?
  if ((fd = syscall(__NR_userfaultfd, O_NONBLOCK)) == -1) {
		printf("++ userfaultfd failed \n");
		goto out;
	}


  // Enable the user fualt fd API
  // [?] the fd will lead to ioctl function to userfaultfd_ioctl.
  struct uffdio_api api = { .api = UFFD_API };
  if (ioctl(fd, UFFDIO_API, &api)) {
		printf( "++ ioctl(fd, UFFDIO_API, ...) failed \n");
		goto out;
	}


  // 2) Prepare the user space memory
	fprintf(stderr, "Phase2, Register [0x%llx, 0x%llx) as range for user page fault handling. \n", 
																			user_buffer_addr, user_buffer_addr+user_buffer_size );
  // Reserve space at specified virtual memory.
  // Access it will lead to page fault.
  user_buf = commit_anon_memory((char*)user_buffer_addr, user_buffer_size, true);
  if(user_buf == NULL){
		printf("Commit user_buffer, 0x%llx failed. \n", user_buffer_addr);
	}else{
		printf("Commit user_buffer: 0x%llx, bytes_len: 0x%llx \n",user_buffer_addr, user_buffer_size);
	}

  
  // Register the virual memory range for the uffd
  struct uffdio_register reg = {
		.mode = UFFDIO_REGISTER_MODE_MISSING,
		.range = {
			.start = (uint64_t)user_buf,
			.len = user_buffer_size
		}
	};

  if (ioctl(fd, UFFDIO_REGISTER,  &reg)) {
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
	ptr = user_buf;
	size_t offset = 8; //bytes
	for(i=offset; i<user_buffer_size; i+=PAGE_SIZE ){
		fprintf(stderr, "Trigger fault page thread: Page[%d] (addr 0x%llx) : %s \n", i/4096, (uint64_t)(ptr+i), ptr+i );
	}



out:
  return 0;
}




