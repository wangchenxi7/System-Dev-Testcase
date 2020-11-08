#! /bin/bash



JVMTILib="jvmti_hello_world"

JavaApp="HelloWorld"




LD_LIBRARY_PATH=`pwd` java -agentlib:${JVMTILib} ${JavaApp}
