
# Build a JVMTI agent to do bytecode instrument.
#	


# -fPIC is used to build a dynamic library.
CFLAGS= -fPIC

# Needed header files : jni.h, jni_md.h, jvmti.h
INCLUDE_PATH= -I "/mnt/ssd/wcx/jdk12u-self-build/jvm/openjdk-12.0.2-internal/include" -I "/mnt/ssd/wcx/jdk12u-self-build/jvm/openjdk-12.0.2-internal/include/linux"


all:
	cc ${CFLAGS}  ${INCLUDE_PATH} -c jvmti_hello_world.cpp -o  jvmti_hello_world.o
	cc -shared -o libjvmti_hello_world.so jvmti_hello_world.o
clean:
	rm jvmti_hello_world.o
	rm libjvmti_hello_world.so
