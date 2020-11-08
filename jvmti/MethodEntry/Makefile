
# Build a JVMTI agent to do bytecode instrument.
#	


# -fPIC is used to build a dynamic library.
CFLAGS= -fPIC

# Needed header files : jni.h, jni_md.h, jvmti.h
INCLUDE_PATH= -I "/mnt/ssd/wcx/jdk12u-self-build/jvm/openjdk-12.0.2-internal/include" -I "/mnt/ssd/wcx/jdk12u-self-build/jvm/openjdk-12.0.2-internal/include/linux"


all:
	cc ${CFLAGS}  ${INCLUDE_PATH} -c jvmti_method_entry.cpp -o  jvmti_method_entry.o
	cc -shared -o libjvmti_method_entry.so jvmti_method_entry.o
clean:
	rm jvmti_method_entry.o
	rm libjvmti_method_entry.so
