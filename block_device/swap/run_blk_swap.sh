#! /bin/bash


bench=$1


if [ -z "${bench}" ]
then
	echo "Select the bench to run: e.g. stream, mmap, flush, swap_ratio, control_path, control_path_after_data"
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
elif [ "${bench}" =  "control_path"  ]
then
	echo "${HOME}/System-Dev-Testcase/block_device/swap/control_path.o"
	${HOME}/System-Dev-Testcase/block_device/swap/control_path.o
elif [ "${bench}" =  "control_path_after_data"  ]
then
	echo "${HOME}/System-Dev-Testcase/block_device/swap/control_path_after_data_path_flush.o"
	${HOME}/System-Dev-Testcase/block_device/swap/control_path_after_data_path_flush.o
elif [ "${bench}" =  "on_demand_swapin"  ]
then
	echo "${HOME}/System-Dev-Testcase/block_device/swap/on_demand_swapin.o"
	${HOME}/System-Dev-Testcase/block_device/swap/on_demand_swapin.o
else
	echo "!! Wrong choice : ${bench} !!"
	exit
fi
