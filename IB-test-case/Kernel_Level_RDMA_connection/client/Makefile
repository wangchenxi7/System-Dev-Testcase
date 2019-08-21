# Module build
# 
#  The basic idea is to use the Makefile of Kernel Source to do the compilation.
#  This module needs some external variable defined in kernel, or other kernel module.
#	So we need to use the Module.SYSVERS
#


kernel_rdma_module_dir=/lib/modules/$(shell uname -r)/extra/kernel_rdma_module


#OFED_CFLAGS = -I/usr/src/ofa_kernel/default/include -include /usr/src/ofa_kernel/default/include/linux/compat-2.6.h

#ccflags-y := -v
#NOSTDINC_FLAGS += -I/usr/src/ofa_kernel/default/include

# The Modules.sysvers listed here will be copy to bulding directory
#KSYMVERS = /mnt/ssd/wcx/testcase/IB-test-case/RDMA_kernel_post_send_recv/Module.symvers /usr/src/ofa_kernel/default/Module.symvers



# the name for the kernel module client.ko
#obj-m += client.o

# obj-[m] means compile this module but not link it into kernel at startup.
obj-m += rdma_client.o

all:
	# -C change to kernel source code, to use its Makefile 
	#  M is a hythen, return to $(PWD) after rading the Make in /lib/modules/$(shell uname -r)/build
	# make  -C /lib/modules/$(shell uname -r)/build  SUBDIRS=$(PWD)  KBUILD_EXTRA_SYMBOLS="$(KSYMVERS)"  modules
	make  -C /lib/modules/$(shell uname -r)/build  M=$(PWD)  modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
	mkdir -p $(kernel_rdma_module_dir)
	/usr/bin/install -c -m 644 ./rdma_client.ko $(kernel_rdma_module_dir)

uninstall:
	modprobe -r rdma_client
	rm $(kernel_rdma_module_dir)/rdma_client.ko