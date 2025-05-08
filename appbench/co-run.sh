#!/bin/bash

# Define arrays for parameters
MAT_SIZES=(10000)
CORES_NUMS=(16)
NORMAL_THREADS=(0)
COOP_THREADS=(32)
WAIT_TIMES=(10 20 40 60 80 100)
# Create the result directory if it doesn't exist
RESULT="resultdir"

# Iterate over the arrays
for MAT_SIZE in "${MAT_SIZES[@]}"; do
        for CORES_NUM in "${CORES_NUMS[@]}"; do
                for NORMAL_THREAD in "${NORMAL_THREADS[@]}"; do
                        for COOP_THREAD in "${COOP_THREADS[@]}"; do
                                for WAIT_TIME in "${WAIT_TIMES[@]}"; do
                                        echo "Running with the following configuration:"
                                        echo "  MAT_SIZE=$MAT_SIZE"
                                        echo "  CORES_NUM=$CORES_NUM"
                                        echo "  NORMAL_THREADS=$NORMAL_THREAD"
                                        echo "  COOP_THREADS=$COOP_THREAD"
                                        echo "  WAIT_TIME=$WAIT_TIME"
                                        # Determine the next RUN_{number} directory
                                        RUN_COUNT=$(find "$RESULT" -maxdepth 1 -type d -name 'RUN_*' | wc -l)
                                        RESULT_DIR="$RESULT/RUN_$((RUN_COUNT + 1))"

                                        mkdir -p "$RESULT_DIR"
                                        OUTPUT_MATMUL="$RESULT_DIR/matmul_${MAT_SIZE}_${CORES_NUM}.txt"
                                        OUTPUT_MULTIBENCH="$RESULT_DIR/multibench_${NORMAL_THREAD}_${COOP_THREAD}_${WAIT_TIME}.txt"

                                        # Redirect the output of the applications to the file
                                        ./IPC-shmem "$MAT_SIZE" "$CORES_NUM" > "$OUTPUT_MATMUL" 2>&1 &
                                        PID1=$!

                                        ./multi-bench "$NORMAL_THREAD" "$COOP_THREAD" "$WAIT_TIME" > "$OUTPUT_MULTIBENCH" 2>&1 &
                                        PID2=$!

                                        # Wait for both processes to finish
                                        wait $PID1 $PID2

                                        echo "Both applications have finished execution for the current configuration."
                                done
                        done
                done
        done
done