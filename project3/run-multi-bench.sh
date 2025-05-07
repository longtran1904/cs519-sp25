#!/bin/bash

# Declare arrays for configurations
declare -a normal_threads=(48)
declare -a coop_threads=(0)
declare -a wait_times=(1)

FlushDisk()
{
        sudo sh -c "echo 3 > /proc/sys/vm/drop_caches"
        sudo sh -c "sync"
        sudo sh -c "echo 3 > /proc/sys/vm/drop_caches"
        sudo sh -c "sync"
        #sudo dmesg --clear
        sleep 5
}

# Iterate over all combinations of parameters
for normal in "${normal_threads[@]}"; do
        for coop in "${coop_threads[@]}"; do
                for wait in "${wait_times[@]}"; do
                        FlushDisk
                        
                        LOG_FILE="multi-bench_normal${normal}_coop${coop}_wait${wait}.log"
                        echo "Running with normal_threads=$normal, coop_threads=$coop, time_to_wait=$wait"
                        ./multi-bench "$normal" "$coop" "$wait" > "$LOG_FILE" 2>&1
                        echo "Log saved to $LOG_FILE"
                done
        done
done

echo "All configurations have been executed. Logs are saved in individual files."
