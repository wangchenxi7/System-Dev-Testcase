#! /bin/bash



choice="${1}"

if [ -z "${choice}" ]
then
	echo "Please select the operation:"
	echo""
	echo "1 : build the fat jar"
	echo "2 : Copy to master's execution destination."
	read choice
fi



if [ "${choice}" = "1" ]
then
	echo "Build the fat Jar"
	echo "Enter suffix of the jar : SparkApp-assembly-<JarSuffix>.jar  "
	read JarSuffix

	echo "The suffix of the fat jar  : ${JarSuffix} "
	echo "The name of the fat jar    : SparkApp-assembly-${JarSuffix}.jar "



	#1, build the jar
	echo "#1, Build the fat jar ...."
	echo "sbt assembly"
	sbt assembly


	#2, change the name to SparkApp-assembly-${choice}
	echo "#2, change the name to SparkApp-assembly-${JarSuffix}.jar"

	JarPath=`pwd`
	mv ${JarPath}/target/scala-2.11/SparkApp-assembly-1.0.jar  ${JarPath}/target/scala-2.11/SparkApp-assembly-${JarSuffix}.jar


elif [ ${choice} = "2"  ]
then
	echo "Copy to master's execution destination"
	echo "enter the suffix of the fat jar to be sent"
	read JarSuffix	

	JarPath=`pwd`
	scp ${JarPath}/target/scala-2.11/SparkApp-assembly-${JarSuffix}.jar wcx@python.cs.ucla.edu:~/Spark-app-jars/

fi




