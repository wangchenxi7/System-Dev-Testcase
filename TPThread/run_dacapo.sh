#! /bin/bash

if [ -z "${HOME}" ]
then
  HOME="/mnt/ssd/wcx"
fi

JAVA_HOME="/mnt/ssd/wcx/jdk12u-self-build/jvm/openjdk-12.0.2-internal"
java_cmd=${JAVA_HOME}/bin/java



echo "Check Java version"
${java_cmd} -version



##
# Options 
confVar="on"
#youngRatio="8" 
gcMode="G1"
heapSize="512m" # -Xmx -Xmx 
#ParallelGCThread="32"  # CPU server GC threads 
ConcGCThread="2"
TPThreadNum="2"
logLevel="info"


##
# testcase && input data
Benchmark="${HOME}/dacapo-9.12-MR1-bach.jar"
testcase="h2"

### set Parallel GC, Concurrent GC parallelsim
if [ -n "${ParallelGCThread}"  ]
then
  ParallelGCThread="-XX:ParallelGCThreads=${ParallelGCThread}"  
else
  ParallelGCThread=""
fi

if [ -n "${ConcGCThread}"  ]
then
  ConcGCThread="-XX:ConcGCThreads=${ConcGCThread}"
else
  ConcGCThread=""
fi

if [ -n "${youngRatio}" ]
then
  youngRatio="-XX:NewRatio=${youngRatio}"
else
  youngRatio=""
fi




exe_mode=$1

if [ "${exe_mode}" = "gdb"  ]
then
  #GC_log="-XX:+PrintGCDetails"
  #JITOption=-XX:-TieredCompilation

  java_option=" ${JITOption} ${JITOption2}  -XX:+UseG1GC -Xnoclassgc -XX:-UseCompressedOops -XX:MetaspaceSize=0x10000000  ${ParallelGCThread} ${ConcGCThread}  -Xms${heapSize} -Xmx${heapSize}  ${youngRatio}   -XX:MarkStackSize=64M -XX:MarkStackSizeMax=64M  -XX:+TPThreadEnable -XX:PrefetchThreads=${TPThreadNum} -XX:PrefetchNum=4096 -XX:PrefetchSize=1000000  -XX:PrefetchQueueThreshold=64 -XX:G1PrefetchBufferSize=1024  -Xlog:tpthread=${logLevel}"  
 echo "gdb --args ${java_cmd} ${java_option} -jar ${Benchmark}  ${testcase} " 
 gdb --args ${java_cmd} ${java_option} -jar ${Benchmark}  ${testcase}

elif [ "${exe_mode}" = "exe" ]
then
  #GC_log="-XX:+PrintGCDetails"
  #JITOption=-XX:-TieredCompilation

  java_option=" ${JITOption} ${JITOption2}  -XX:+UseG1GC -Xnoclassgc -XX:-UseCompressedOops -XX:MetaspaceSize=0x10000000  ${ParallelGCThread} ${ConcGCThread}  -Xms${heapSize} -Xmx${heapSize}  ${youngRatio}   -XX:MarkStackSize=64M -XX:MarkStackSizeMax=64M  -XX:+TPThreadEnable -XX:PrefetchThreads=${TPThreadNum} -XX:PrefetchNum=4096 -XX:PrefetchSize=1000000  -XX:PrefetchQueueThreshold=64 -XX:G1PrefetchBufferSize=1024  -Xlog:tpthread=${logLevel}"  
 echo " ${java_cmd} ${java_option} -jar ${Benchmark}  ${testcase} " 
 ${java_cmd}   ${java_option} -jar ${Benchmark}   ${testcase}

else
  echo "Check Java version"
  ${java_cmd} -version

  echo " !! Wrong execution mode : ${exe_mode} !!"
  exit 1
fi


