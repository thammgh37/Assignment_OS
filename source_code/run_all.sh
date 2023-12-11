#!/bin/bash
make clean
make all
./os os_0_mlq_paging > our_output/os_0_mlq_paing.output
./os os_1_mlq_paging > our_output/os_1_mlq_paing.output
./os os_1_mlq_paging_small_1K  > our_output/os_1_mlq_paing_small_1K.output 
./os os_1_mlq_paging_small_4K  > our_output/os_1_mlq_paing_small_4K.output
./os os_1_mlq_paging_small_4K  > our_output/os_1_singleCPU_mlq.output     
./os os_1_singleCPU_mlq  > our_output/os_1_singleCPU_mlq.output
./os os_1_singleCPU_mlq_paging  > our_output/os_1_singleCPU_mlq_paging.output 
# ./os sched  >> our_output/sched.output 
# ./os sched_0  >> our_output/sched_0.output 
# ./os sched_1  >> our_output/sched_1.output 
./os testmem  > our_output/testmem.output 
./os testmemswp  > our_output/testmemswp.output 

echo "DONE"