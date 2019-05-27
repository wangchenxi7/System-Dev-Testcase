

obj-m += extra_functions.o

all:
	# -C change to kernel source code, to use its Makefile 
	#  	#  M is a hythen, return to $(PWD) after rading the Make in /lib/modules/$(shell uname -r)/build
	make -C /lib/modules/$(shell uname -r)/build    M=$(PWD)  modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
