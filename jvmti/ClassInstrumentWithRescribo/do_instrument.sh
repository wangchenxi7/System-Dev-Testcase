#! /bin/bash
#
# Using Jon's c++ instrument tool, rescribo, to do the bytecode instrument.
#

JVMTILib="jvmti_class_instrument"

JavaApp="HelloWorld"




LD_LIBRARY_PATH=`pwd`:${LD_LIBRARY_PATH} java -agentlib:${JVMTILib} ${JavaApp}
