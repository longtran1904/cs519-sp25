#! /bin/bash
declare -a cores=("2" "4" "6" "8" "10" "12" "14" "16")
# declare -a cores=("16")

rm -f output-shmem.txt

# for core in "${cores[@]}"; do
#     ./IPC-pipe 10000 $core >> output-pipe.txt
#     echo -e '\n' >> output-pipe.txt
# done

for core in "${cores[@]}"; do
    ./IPC-shmem 10000 $core >> output-shmem.txt
    echo -e '\n' >> output-shmem.txt
done