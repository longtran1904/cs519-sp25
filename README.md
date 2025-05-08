#### Coding and other resources for CS 519

Cooperative Scheduler
------------------

### 1. Apply patch
If you've not installed custom kernel
```
cd linux-5.15.0
./patch/apply_patch.sh

../project1/install_packages.sh
../project1/compile_os_nopackages.sh

sudo reboot
```

If you've already installed custom kernel
```
cd linux-5.15.0
./patch/apply_patch.sh

../project1/compile_os_quick.sh
sudo reboot
```

The following steps help reproduce the results in the report:
- https://docs.google.com/document/d/1omEWKq6jcyHEBj425BckKszd7eH5rvyHklaviXzCv5Y/edit?usp=sharing

### 2. Set up matrix multiplication
```
cd project1-part2
make clean
make

touch shmfile
<> Activate huge page for shared memory
sudo sh -c "echo 1000 > /proc/sys/vm/nr_hugepages" 

<> if get errors from semctl, please try:
`touch semfile`

cp IPC-shmem ../appbench/IPC-shmem

cd appbench
touch shmfile
```

### 3. Set up benchmark
```
cd project3
make clean
make

cp multi-bench ../appbench/multi-bench
```

### 4. Run the experiment
Configure the parameters and run `./co-run.sh`