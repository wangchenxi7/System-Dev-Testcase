
# If we change the compilation path to /lib/modules/$(shell uname -r)/build,
# the compiler can find the module.sysvers and use the external symbol directly.
#
# But if we declare and implement external symbol ourselves, we need to import and import it explicitly.
# KSYMVERS= <path_to_module.sysvers>(space)<2nd_module.sysvers>(space)...
# make -C /lib/modules/$(shell uname -r)/build  KBUILD_EXTRA_SYMBOLS="$(KSYMVERS)"  M=$(PWD) modules
#  
#KSYMVERS=	/lib/modules/$(shell uname -r)/build/Module.symvers

obj-m += monotonic_clock.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
