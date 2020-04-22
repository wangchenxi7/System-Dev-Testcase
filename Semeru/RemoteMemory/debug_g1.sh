#! /bin/bash

bench=$1

if [ -z "${bench}"  ]
then
	echo "Please input the bench : e.g.  Simple"
	read bench
fi


mode=$2

if [ -z "${mode}"  ]
then
	echo "Please input the execution mode: gdb, execution"
	read mode
fi

#
# Parameters
#

MemSize="128M"


# Enable the Semeru Memory pool
EnableSemeruMemPool="true"
#EnableSemeruMemPool="false"

SemeruMemPoolSize="32G"
#SemeruMemPool=""

# Region size and Heap's allocation alignment.
SemeruMemPoolAlignment="1G"


STWParallelThread=1
concurrentThread=1
SemeruConcurrentThread=1

compressedOop="no"
#compressedOop="yes"


#logOpt="-Xlog:gc+heap=debug"
#logOpt="-Xlog:gc=info"

# Print GC, Concurrent Marking details
#logOpt="-Xlog:gc+marking=debug"
#logOpt="-Xlog:gc,gc+marking=debug"

# heap is a self defined Xlog tag.

# Full Debug
#logOpt="-Xlog:semeru=debug,heap=debug,gc=debug,gc+marking=debug,gc+remset=debug,gc+ergo+cset=debug,gc+bot=debug,gc+workgang=trace,workgang=debug,gc+task=debug,gc+thread=debug,os+thread=debug"

# Thread debug
logOpt="-Xlog:semeru=debug,heap=debug,gc=debug,gc+marking=debug,semeru+compact=debug,semeru+alloc=debug,semeru+thread=debug"

#logOpt="-Xlog:semeru+heap=debug,heap=debug,gc=debug,gc+marking=debug,gc+remset=debug,gc+ergo+cset=debug,gc+bot=debug"


#
# Apply the options.
#


if [ ${EnableSemeruMemPool} = "false" ]
then
	SemeruMemPoolParameter=""
elif  [ ${EnableSemeruMemPool} = "true" ]
then
	SemeruMemPoolParameter="-XX:SemeruEnableMemPool -XX:SemeruMemPoolMaxSize=${SemeruMemPoolSize} -XX:SemeruMemPoolInitialSize=${SemeruMemPoolSize} -XX:SemeruMemPoolAlignment=${SemeruMemPoolAlignment}  -XX:SemeruConcGCThreads=${SemeruConcurrentThread}"
else
	echo "Wrong vlaue for 'EnableSemeruMemPool'"
	exit
fi


if [ ${compressedOop} = "no"  ]
then
	compressedOop="-XX:-UseCompressedOops"	

elif [ ${compressedOop} = "yes"  ]
then
	# Open the compressed oop, no matter what's  the size of the Java heap
	compressedOop="-XX:+UseCompressedOops"

else
	# Used the default policy
	# If Heap size <= 32GB, use Compressed oop, 32 bits addresso. 
	compressedOop=""
fi





#
# Execute the  Commandlines 
#

if [ "${mode}" = "gdb"  ]
then
	gdb --args  java -XX:+UseG1GC  ${compressedOop}  ${logOpt}   -Xms${MemSize} -Xmx${MemSize}   ${SemeruMemPoolParameter}  -XX:ParallelGCThreads=${STWParallelThread}   -XX:ConcGCThreads=${concurrentThread}  ${bench}
elif [ "${mode}" = "execution" ]
then
	java -XX:+UseG1GC  ${compressedOop}  ${logOpt}   -Xms${MemSize} -Xmx${MemSize} ${SemeruMemPoolParameter}  -XX:ParallelGCThreads=${STWParallelThread}   -XX:ConcGCThreads=${concurrentThread}  ${bench}

else
	echo "Wrong Mode."
fi



