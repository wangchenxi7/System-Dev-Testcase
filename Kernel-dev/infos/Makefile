# kernel information

#CFLAGS= -O0 -g


obj-m += kernel_version_utsname.o

all:
	cc ${CFLAGS} kernel_version_macro.c -o kernel_version_macro.o
	# -C change to kernel source code, to use its Makefile 
	#  	#  M is a hythen, return to $(PWD) after rading the Make in /lib/modules/$(shell uname -r)/build
	make -C /lib/modules/$(shell uname -r)/build    M=$(PWD)  modules

clean:
	rm kernel_version_macro.o
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean