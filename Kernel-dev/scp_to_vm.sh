#! /bin/bash


forwarding_port="2222"
login_user="wcx"
dest_ip="localhost"
dest_dir="/testcase"

file="$1"


## file
if [ -z "${file}" ]
then
	echo "scp default file : a.out to destination"
	file="a.out"
else
	echo "scp specific file : ${file} to destination"
fi

echo "scp -P ${forwarding_port}  ${file}  ${login_user}@${dest_ip}:~/${dest_dir} "
scp -P ${forwarding_port} ${file}   ${login_user}@${dest_ip}:~/${dest_dir} 


