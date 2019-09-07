#! /bin/bash


#command line options

GC=$1

if [ -z "${GC}"  ]
then
	echo "Please choose the GC type, G1 or Shenandoah or ZGC:"
	read GC
fi

App=$2

if [ -z "${App}" ]
then
	echo "Pleasse enter the application"
	read App
fi



## GC options

if [ "${GC}" = "G1"  ]
then
	GC_type="-XX:+UseG1GC"
elif [ "${GC}" = "Shenandoah"  ]
then
	GC_type="-XX:+UnlockExperimentalVMOptions  -XX:+UseShenandoahGC"
elif	[ "${GC}" = "ZGC" ]
then
	GC_type="-XX:+UnlockExperimentalVMOptions  -XX:+UseZGC"
else
	echo "Error GC type choice."
	exit
fi

GC_thread="8"
#GC_log_opt="-Xlog:gc+stats"
GC_log_opt="-Xlog:gc"
Mem="4g"


## Log file
log_path="/mnt/ssd/wcx/Logs/${GC}.log"

time -p numactl --cpubind=0 java  ${GC_type}  -XX:ParallelGCThreads=${GC_thread} -XX:ConcGCThreads=${GC_thread} -Xms${Mem} -Xmx${Mem} ${GC_log_opt}  ${App} > ${log_path} 2>&1



