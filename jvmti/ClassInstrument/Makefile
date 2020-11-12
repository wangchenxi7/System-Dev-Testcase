
# Build a JVMTI agent to do bytecode instrument.
#	


# -fPIC is used to build a dynamic library.
CFLAGS= -fPIC

# Needed header files : jni.h, jni_md.h, jvmti.h
INCLUDE_PATH= -I "/mnt/ssd/wcx/jdk12u-self-build/jvm/openjdk-12.0.2-internal/include" -I "/mnt/ssd/wcx/jdk12u-self-build/jvm/openjdk-12.0.2-internal/include/linux"


all:
	cc ${CFLAGS}  ${INCLUDE_PATH} -c jvmti_class_instrument.cpp -o  jvmti_class_instrument.o
	cc -shared -o libjvmti_class_instrument.so jvmti_class_instrument.o
clean:
	rm jvmti_class_instrument.o
	rm libjvmti_class_instrument.so
