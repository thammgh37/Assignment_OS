#!/bin/bash
./os os_0_mlq_paging >> os_0_mlq_paing.output
./os os_1_mlq_paging >> our_output/os_1_mlq_paing.output
./os os_1_mlq_paging_small_1K  >> our_output/os_1_mlq_paing_small_1K.output 
./os os_1_mlq_paging_small_4K  >> our_output/os_1_mlq_paing_small_4K.output
./os os_1_mlq_paging_small_4K  >> our_output/os_1_singleCPU_mlq.output     
./os os_1_singleCPU_mlq  >> our_output/os_1_singleCPU_mlq.output
./os os_1_singleCPU_mlq_paging  >> our_output/os_1_singleCPU_mlq_paging.output 
echo "DONE"