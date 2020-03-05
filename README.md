# testcase
Testcases for system development


#1 Testcase for Semeru InfiniBand.

Example:

Connect CPU Server and Memory Server

a. Start Memory server by running, <br/>
go to directory testcase/Semeru/RemoteMemory <br/>
./run_rmem_server_with_rdma_service.sh Case1 execution <br/>
 
b. Start CPU server, <br/>
go to directory : linux-4.11-rc8/semeru <br/>
make // compile the module <br/>
sudo insmod semeru_cpu.ko // insert the CPU server kernel module <br/>

After a. and b. CPU server will bind with Memory server. <br/>
The Memory server will be mounted as a block device under /dev, <br/>
e.g. /dev/rmempool <br/>
Until now, the control path can work.

c. To use the Control Path, invoke  the syscall explicitly <br\>
syscall id is 334.
The usage example is in Kernel-dev/syscall/user_kernel_copy.c  <br\>

d. To use the Data Path, need to mount the /dev/rmempool as the swap partition on CPU server. <br/>

swapon -s             // check current swap partition <br/>
swapoff /dev/current-swap-partition   // Close current swap partition  <br/>
mkswap /dev/rmempool  // format the fake block device <br/>
swapon /dev/rmempool  // mount the fake block device as swap partition <br/>

And then the Data-Path is controlle by swap mechanism now. <br/>

e. Use lxc or docker limit the applications memory usage to trigger swap.





