#! /bin/bash
#
# Using Jon's c++ instrument tool, rescribo, to do the bytecode instrument.
#

JVMTILib="jvmti_class_instrument"

if [ -z "${1}" ]
then
  #JavaApp="HelloWorld"
  JavaApp="Simple"
  echo "Run the default application: ${JavaApp}"
else
  JavaApp="$1"
  echo "Run the enterred application: ${JavaAPp} "
fi


##
# Options

# Log 
#LogInfo="-Xlog:prefetch=debug"


LD_LIBRARY_PATH=`pwd`:${LD_LIBRARY_PATH} java ${LogInfo}  -agentlib:${JVMTILib} ${JavaApp}
