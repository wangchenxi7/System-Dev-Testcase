#! /bin/bash



JVMTILib="jvmti_class_instrument"

JavaApp="HelloWorld"




LD_LIBRARY_PATH=`pwd` java -agentlib:${JVMTILib} ${JavaApp}
