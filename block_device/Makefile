

remote_mempool_module_dir=/lib/modules/$(shell uname -r)/extra/remote_mempool


obj-m += register_disk.o

all:
	# -C change to kernel source code, to use its Makefile 
	#  	#  M is a hythen, return to $(PWD) after rading the Make in /lib/modules/$(shell uname -r)/build
	make -C /lib/modules/$(shell uname -r)/build   M=$(PWD)  modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
	mkdir -p $(remote_mempool_module_dir)
	/usr/bin/install -c -m 644 ./register_disk.ko $(remote_mempool_module_dir)

uninstall:
	modprobe -r register_disk
	rm $(remote_mempool_module_dir)/register_disk.ko
