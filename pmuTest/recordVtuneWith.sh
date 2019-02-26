#! /bin/bash

echo "1st parameter is the applcaition"
echo "Specific the log file name by 2nd parameter,or print in screen"

#pmu event
#test IPC, MLP
#PMUEvent="INST_RETIRED.ANY,CPU_CLK_UNHALTED.THREAD,MEM_UOPS_RETIRED.ALL_LOADS,MEM_UOPS_RETIRED.ALL_STORES,UNC_M_RPQ_CYCLES_NE,UNC_M_WPQ_CYCLES_NE,UNC_M_WPQ_CYCLES_FULL,OFFCORE_REQUESTS_OUTSTANDING.DEMAND_DATA_RD,OFFCORE_REQUESTS_OUTSTANDING.DEMAND_CODE_RD,OFFCORE_REQUESTS_OUTSTANDING.DEMAND_RFO,OFFCORE_REQUESTS_OUTSTANDING.DEMAND_DATA_RD_GE_6"

#test bank level conflict
#PMUEvent="INST_RETIRED.ANY,CPU_CLK_UNHALTED.THREAD,UNC_M_PRE_COUNT.PAGE_MISS,UNC_M_CAS_COUNT.RD,UNC_M_CAS_COUNT.WR"

#test LLC cache miss ratio
#PMUEvent="INST_RETIRED.ANY,CPU_CLK_UNHALTED.THREAD,OFFCORE_REQUESTS.DEMAND_DATA_RD,OFFCORE_REQUESTS.DEMAND_CODE_RD,OFFCORE_REQUESTS.DEMAND_RFO,OFFCORE_RESPONSE:request=DEMAND_DATA_RD:response=LLC_MISS.ANY_RESPONSE,OFFCORE_RESPONSE:request=DEMAND_RFO:response=LLC_MISS.ANY_RESPONSE,OFFCORE_RESPONSE:request=DEMAND_CODE_RD:response=LLC_MISS.ANY_RESPONSE"

#test remote memory access
PMUEvent="INST_RETIRED.ANY,CPU_CLK_UNHALTED.THREAD,OFFCORE_REQUESTS.DEMAND_DATA_RD,OFFCORE_REQUESTS.DEMAND_CODE_RD,OFFCORE_REQUESTS.DEMAND_RFO,OFFCORE_RESPONSE:request=DEMAND_DATA_RD:response=LLC_MISS.ANY_RESPONSE,OFFCORE_RESPONSE:request=DEMAND_DATA_RD:response=LLC_MISS.LOCAL_DRAM,OFFCORE_RESPONSE:request=DEMAND_DATA_RD:response=LLC_MISS.REMOTE_DRAM,OFFCORE_RESPONSE:request=DEMAND_RFO:response=LLC_MISS.ANY_RESPONSE,OFFCORE_RESPONSE:request=DEMAND_RFO:response=LLC_MISS.LOCAL_DRAM,OFFCORE_RESPONSE:request=DEMAND_RFO:response=LLC_MISS.REMOTE_DRAM"


#get the CoarseGrainedExecutor Id
#executorId=""
#while [ -z "$executorId" ]
#do
#  executorId=` jps | grep CoarseGrainedExecutorBackend | sed -n "s/ CoarseGrainedExecutorBackend//p"`
#done
#echo "perf executor: $executorId"

cpubind=$1
membind=$2

if [ -z "${cpubind}" ]
then
  echo""
  echo""
  echo "please specify an node to bind cpu"
  echo""
  exit 1
fi


if [ -z "${membind}" ]
then
  echo""
  echo""
  echo "please specify an node to bind memory."
  echo "Or it will FOLLOW the cpu bind:${cpubind}"
  echo""
  membind=${cpubind}
fi

application=$3
logFile=$4

if [ -z "${application}" ]
then
  echo""
  echo""
  echo "please specify an application to perf"
  echo""
  exit 1
fi

if [ -z "$logFile" ]
then
  echo "Print in screen "
  echo " cpubind:${cpubind}, memory bind:${membind}, application ${application}" 
  numactl --cpunodebind=${cpubind} --membind=${membind}  amplxe-cl -data-limit=0   -collect-with runsa   -knob event-config=${PMUEvent} ${application}
else
  echo "dump in ${logFile}"
  echo -e "\n\n\n" >> ${logFile}
  echo " cpubind:${cpubind}, memory bind:${membind}, application ${application}" >> ${logFile} 
  echo " cpubind:${cpubind}, memory bind:${membind}, application ${application}" 
  numactl --cpunodebind=${cpubind} --membind=${membind}  amplxe-cl -data-limit=0   -collect-with runsa  -knob event-config=${PMUEvent} ${application}  >> ${logFile} 
fi









