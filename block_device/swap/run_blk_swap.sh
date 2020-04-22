#! /bin/bash


bench=$1


if [ -z "${bench}" ]
then
	echo "Select the bench to run: e.g. stream, mmap, flush"
	read bench
fi



# Do the action
if [ "${bench}" = "mmap"  ]
then

	echo "${HOME}/System-Dev-Testcase/block_device/swap/swap_mmap.o"
	${HOME}/System-Dev-Testcase/block_device/swap/swap_mmap.o

elif [ "${bench}" = "stream" ]
then

	echo "${HOME}/System-Dev-Testcase/block_device/swap/swap_stream.o"
	${HOME}/System-Dev-Testcase/block_device/swap/swap_stream.o

elif [ "${bench}" =  "flush"  ]
then
	echo "${HOME}/System-Dev-Testcase/block_device/swapflush_page_to_swap_imm.o"
	${HOME}/System-Dev-Testcase/block_device/swap/flush_page_to_swap_imm.o
else
	echo "!! Wrong choice : ${bench} !!"
	exit
fi
