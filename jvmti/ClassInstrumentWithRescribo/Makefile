
# Build a JVMTI agent to do bytecode instrument.
#	


# -fPIC is used to build a dynamic library.
CXXFLAGS= -fPIC -std=c++17

# Needed header files : jni.h, jni_md.h, jvmti.h
JDK_INCLUDE_PATH= -I "${JAVA_HOME}/include" -I "${JAVA_HOME}/include/linux" 
# Configure the path of project-rescribo
RESCRIBO_INCLUDE_PATH= -I "${RESCRIBO_HOME}/include/project-rescribo"

# Library path
# Add the path of libproject-rescribo.so to the LD_LIBRARY_PATH for execution loading.
# -L (LIBRARY_PATH) is used for compilation.
RESCRIBO_LIB_PATH=-L${RESCRIBO_HOME}/build/src
LDLIBS=-lproject-rescribo

all:
	g++ ${CXXFLAGS}  ${JDK_INCLUDE_PATH} ${RESCRIBO_INCLUDE_PATH} ${RESCRIBO_LIB_PATH} ${LDLIBS} -c jvmti_class_instrument.cpp -o  jvmti_class_instrument.o
	g++ -shared ${RESCRIBO_LIB_PATH}  ${LDLIBS}   jvmti_class_instrument.o  -o libjvmti_class_instrument.so

clean:
	rm jvmti_class_instrument.o
	rm libjvmti_class_instrument.so
