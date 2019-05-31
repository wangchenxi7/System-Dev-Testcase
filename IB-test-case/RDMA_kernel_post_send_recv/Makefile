# Module build
# 
#  The basic idea is to use the Makefile of Kernel Source to do the compilation.

#OFED_CFLAGS = -I/usr/src/ofa_kernel/default/include -include /usr/src/ofa_kernel/default/include/linux/compat-2.6.h

#ccflags-y := -v
NOSTDINC_FLAGS += -I/usr/src/ofa_kernel/default/include

# The Modules.sysvers listed here will be copy to bulding directory
KSYMVERS = /mnt/ssd/wcx/testcase/IB-test-case/RDMA_kernel_post_send_recv/Module.symvers /usr/src/ofa_kernel/default/Module.symvers



# the name for the kernel module client.ko
#obj-m += client.o

obj-m += client.o

all:
	# -C change to kernel source code, to use its Makefile 
	#  M is a hythen, return to $(PWD) after rading the Make in /lib/modules/$(shell uname -r)/build
	make  -C /lib/modules/$(shell uname -r)/build  SUBDIRS=$(PWD)  KBUILD_EXTRA_SYMBOLS="$(KSYMVERS)"  modules
clean:
	make -C /lib/modules/$(shell uname -r)/build SUBDIRS=$(PWD) clean
