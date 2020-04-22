#! /bin/bash



java -XX:+UseG1GC  -Xms4g -Xmx4g  -XX:ParallelGCThreads=12  -XX:ConcGCThreads=3

