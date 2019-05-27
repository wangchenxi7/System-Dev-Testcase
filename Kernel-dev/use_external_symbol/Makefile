



KSYMVERS=	/mnt/ssd/wcx/testcase/Kernel-dev/external_symbol_provider/Module.sysvers


obj-m += use_external_symbol.o

all:
	# -C change to kernel source code, to use its Makefile 
	#  	#  M is a hythen, return to $(PWD) after rading the Make in /lib/modules/$(shell uname -r)/build
	make -C /lib/modules/$(shell uname -r)/build KBUILD_EXTRA_SYMBOLS="$(KSYMVERS)"   M=$(PWD)  modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
