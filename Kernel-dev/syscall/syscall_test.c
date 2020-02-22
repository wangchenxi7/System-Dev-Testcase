#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>


// Define a name for syscall do_semeru_rdma_ops
#define SYS_do_semeru_rdma_ops  333 

int main()
{
         //long int amma = syscall(333,0x1,0x2,0x3);
         long int amma = syscall(SYS_do_semeru_rdma_ops,0x1,0x2,0x3);
         printf("System call id 333 returned %ld\n", amma);
         return 0;
}
