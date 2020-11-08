#! /bin/bash



JVMTILib="jvmti_method_entry"

JavaApp="HelloWorld"




LD_LIBRARY_PATH=`pwd` java -agentlib:${JVMTILib} ${JavaApp}
