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
<<<<<<< HEAD
LogInfo="-Xmx128M -Xms128M -XX:PrefetchThreads=1 -XX:PrefetchNum=100 -XX:PrefetchSize=1000 -XX:PrefetchQueueThreshold=100 -XX:+PrintGCDetails"
#-Xlog:prefetch=debug
=======
#LogInfo="-Xlog:prefetch=debug"
>>>>>>> master

LD_LIBRARY_PATH=`pwd`:${LD_LIBRARY_PATH} gdb --args /mnt/ssd/haoran/jdk12u-dev/build/linux-x86_64-server-release/jdk/bin/java ${LogInfo} -agentlib:${JVMTILib} ${JavaApp}
# LD_LIBRARY_PATH=`pwd`:${LD_LIBRARY_PATH} /mnt/ssd/haoran/jdk12u-dev/build/linux-x86_64-server-slowdebug/jdk/bin/java -agentlib:${JVMTILib} -Xmx128M -Xms128M -XX:PrefetchThreads=1 -XX:PrefetchNum=100 -XX:PrefetchSize=1000 -XX:PrefetchQueueThreshold=100 -XX:+PrintGCDetails Simple
