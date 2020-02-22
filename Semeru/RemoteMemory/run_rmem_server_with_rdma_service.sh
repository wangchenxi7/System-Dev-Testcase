#! /bin/bash




### Testcase
if [ -z "${HOME}"  ]
then
	echo "HOME directory is NULL.	Please set it correctly."
	exit
fi

testcase_dir="${HOME}/testcase/Semeru/RemoteMemory"
bench=$1

if [ -z "${bench}"  ]
then
	echo "Inpute the bencmark name. e.g.  Case1"
	read bench
else
	echo "Run the benchmark ${testcase}${bench} "
fi



### Java verion
java_exe="${HOME}/jdk12u-self-build/jvm/openjdk-12.0.2-internal/bin/java"

### JVM configuration

## Semeru Configuration

EnableSemeruMemPool="true"

SemeruMemPoolSize="32G"

# Region size and Heap's allocation alignment.
SemeruMemPoolAlignment="1G"

SemeruConcurrentThread=2



## Original OpenJDK Configuration

STWParallelThread=1
concurrentThread=1
MemSize="128M"



## RDMA Buffer size + Memory server reserved size, i.e. 1GB.
original_gc_mode="-XX:+UseG1GC"

# Do not use compressed oop. Assume the Semeru heap is always larger than 32GB.
compressedOop="no"


# heap is a self defined Xlog tag.
#logOpt="-Xlog:heap=debug,gc=debug,gc+marking=debug,gc+remset=debug,gc+ergo+cset=debug,gc+bot=debug,gc+workgang=trace,workgang=debug,gc+task=debug,os+thread=debug
logOpt=""



### Apply the configuration

if [ -z ${EnableSemeruMemPool} ]
then
  SemeruMemPoolParameter=""
else
  SemeruMemPoolParameter="-XX:SemeruEnableMemPool -XX:SemeruMemPoolMaxSize=${SemeruMemPoolSize} -XX:SemeruMemPoolInitialSize=${SemeruMemPoolSize} -XX:SemeruMemPoolAlignment=${SemeruMemPoolAlignment} -XX:SemeruConcGCThreads=${SemeruConcurrentThread}"
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





## Do  the excution

 ${java_exe} ${original_gc_mode}  ${compressedOop}  ${logOpt}   -Xms${MemSize} -Xmx${MemSize} ${SemeruMemPoolParameter}  -XX:ParallelGCThreads=${STWParallelThread} -XX:-UseDynamicNumberOfGCThreads   -XX:ConcGCThreads=${concurrentThread}  ${bench}
