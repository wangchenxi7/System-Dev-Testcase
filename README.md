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

After 1) and 2) CPU server will bind with Memory server. <br/>
The Memory server will be mount as a block device under /dev, <br/>
/dev/rmempool


