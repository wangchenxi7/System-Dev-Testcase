#! /bin/bash
#
# Using Jon's c++ instrument tool, rescribo, to do the bytecode instrument.
#

JVMTILib="jvmti_class_instrument"


TPThreadNum=1



# execution mode, gdb, production.
if [ "${1}" = "exe"  ]
then
  # execution mode
  exeMode=""
elif [ "${1}" = "gdb" ]
then
  # gdb mode
  exeMode="${1} --args "
else
  echo "Wrong exeMode: ${exeMode}"
  exit 1
fi

# Application name
if [ -z "${2}" ]
then
  #JavaApp="HelloWorld"
  JavaApp="Simple"
  echo "Run the default application: ${JavaApp}"
else
  JavaApp="$2"
  echo "Run the enterred application: ${JavaAPp} "
fi


##
# Options

# Log 
#LogInfo="-Xlog:prefetch=debug,instrument=trace,tpthread=trace"
LogInfo="-Xlog:prefetch=debug,instrument=debug,tpthread=trace"


##
# TP thread options
TPThreadOption="-XX:+TPThreadEnable -XX:PrefetchThreads=${TPThreadNum} -XX:PrefetchNum=50 -XX:PrefetchSize=500 -XX:PrefetchQueueThreshold=10"

LD_LIBRARY_PATH=`pwd`:${LD_LIBRARY_PATH} ${exeMode}  java ${LogInfo} ${TPThreadOption}   -agentlib:${JVMTILib} ${JavaApp}
