# testcase
Testcases for system development


#1 Testcase for Semeru InfiniBand.

Example:

Connect CPU Server and Memory Server
a. Start Memory server by running,
go to directory testcase/Semeru/RemoteMemory：
./run_rmem_server_with_rdma_service.sh Case1 execution

b. Start CPU server, 
go to directory : linux-4.11-rc8/semeru
make // compile the module
sudo insmod semeru_cpu.ko // insert the CPU server kernel module

After 1) and 2) CPU server will bind with Memory server.
The Memory server will be mount as a block device under /dev,
/dev/rmempool


