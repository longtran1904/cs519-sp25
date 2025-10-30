### Installation - Shmem
```
touch shmfile
<> Activate huge page for shared memory
sudo sh -c "echo 1000 > /proc/sys/vm/nr_hugepages" 
```

if get errors from semctl, please try:
`touch semfile`

### Compilation
```
make clean
make
```

### Run
```
<> Pipe version
./IPC-pipe 10000 {cores}

<> Shmem version
./IPC-shmem 10000 {cores}
```

Number of cores to use: `cores <= 16`
If another input size is desired, please make sure `{matrix size} > {cores}`

