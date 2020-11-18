
# Build a JVMTI agent to do bytecode instrument.
#	


# -fPIC is used to build a dynamic library.
CFLAGS= -fPIC

# Needed header files : jni.h, jni_md.h, jvmti.h
INCLUDE_PATH= -I "/mnt/ssd/haoran/jdk12u-dev/build/linux-x86_64-server-release/jdk/include" -I "/mnt/ssd/haoran/jdk12u-dev/build/linux-x86_64-server-release/jdk/include/linux"


all:
	cc ${CFLAGS}  ${INCLUDE_PATH} -c jvmti_class_instrument.cpp -o  jvmti_class_instrument.o
	cc -shared -o libjvmti_class_instrument.so jvmti_class_instrument.o
clean:
	rm jvmti_class_instrument.o
	rm libjvmti_class_instrument.so
