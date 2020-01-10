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


STWParallelThread=1
concurrentThread=1

compressedOop="no"
#compressedOop="yes"


#logOpt="-Xlog:gc+heap=debug"
#logOpt="-Xlog:gc=info"

# Print GC, Concurrent Marking details
#logOpt="-Xlog:gc+marking=debug"
#logOpt="-Xlog:gc,gc+marking=debug"

# heap is a self defined Xlog tag.
logOpt="-Xlog:heap=debug,gc=debug,gc+marking=debug"


#
# Apply the options.
#

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
	gdb --args  java -XX:+UseG1GC  ${compressedOop}  ${logOpt}   -Xms${MemSize} -Xmx${MemSize}  -XX:ParallelGCThreads=${STWParallelThread}   -XX:ConcGCThreads=${concurrentThread}  ${bench}
elif [ "${mode}" = "execution" ]
then
	java -XX:+UseG1GC  ${compressedOop}  ${logOpt}   -Xms${MemSize} -Xmx${MemSize}  -XX:ParallelGCThreads=${STWParallelThread}   -XX:ConcGCThreads=${concurrentThread}  ${bench}

else
	echo "Wrong Mode."
fi



