#! /bin/bash


bench=$1


if [ -z "${bench}" ]
then
	echo "Select the bench to run: e.g. stream, mmap, flush, swap_ratio"
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
	echo "${HOME}/System-Dev-Testcase/block_device/swap/flush_page_to_swap_imm.o"
	${HOME}/System-Dev-Testcase/block_device/swap/flush_page_to_swap_imm.o
elif [ "${bench}" =  "swap_ratio"  ]
then
	echo "${HOME}/System-Dev-Testcase/block_device/swap/swap_out_ratio_flush_based.o"
	${HOME}/System-Dev-Testcase/block_device/swap/swap_out_ratio_flush_based.o
else
	echo "!! Wrong choice : ${bench} !!"
	exit
fi
