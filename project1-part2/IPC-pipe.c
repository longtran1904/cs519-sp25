/* CS 519, Spring 2025: Project 1 - Part 2
 * IPC using pipes to perform matrix multiplication.
 * Feel free to extend or change any code or functions below.
 */
 #define _GNU_SOURCE  // Enable GNU extensions for CPU affinity functions
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/time.h>
#include "queue.h"
#include <assert.h>
#include <sys/wait.h>


#define RESET   "\033[0m"
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */

#ifndef DEBUG
    #define DEBUG 0
#endif

//Add all your global variables and definitions here.
#define MATRIX_SIZE 1000
#define PIPE_LIMIT 65536
int cores;

// Define union semun before using it in semctl()
union semun {
    int val;                  // Used for SETVAL
    struct semid_ds *buf;      // Used for IPC_STAT, IPC_SET
    unsigned short *array;     // Used for GETALL, SETALL
};

void semaphore_init(int sem_id, int sem_num, int init_value)
{
    union semun sem_union;
    sem_union.val = init_value;
   //Use semctl to initialize a semaphore
    if (semctl(sem_id, sem_num, SETVAL, sem_union) == -1) {
        perror("semctl");
        exit(1);
    }

    if (DEBUG) printf(RED "Semaphore created with ID: %d" RESET "\n", sem_id);
}

void semaphore_release(int sem_id, int sem_num)
{
    //Use semop to release a semaphore
    struct sembuf sop;
    sop.sem_num = sem_num;
    sop.sem_op = 1;
    sop.sem_flg = 0;
    if (semop(sem_id, &sop, 1) == -1){ // Perform operation
        perror("semop failed");
        _exit(EXIT_FAILURE);
    }
}

void semaphore_reserve(int sem_id, int sem_num)
{
    //Use semop to acquire a semaphore
    struct sembuf sop;
    sop.sem_num = sem_num;
    sop.sem_op = -1;
    sop.sem_flg = 0;
    if (semop(sem_id, &sop, 1) == -1){ // Perform operation
        perror("semop failed");
        _exit(EXIT_FAILURE);
    }
}

/* Time function that calculates time between start and end */
double getdetlatimeofday(struct timeval *begin, struct timeval *end)
{
    return (end->tv_sec + end->tv_usec * 1.0 / 1000000) -
           (begin->tv_sec + begin->tv_usec * 1.0 / 1000000);
}


/* Stats function that prints the time taken and other statistics that you wish
 * to provide.
 */
void print_stats() {

}



void matmul_single_threaded(int *a, int *b, int *c, int size){
    // matrix multiplication: C = A * B
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            int sum = 0;
            for (int k = 0; k < size; k++) {
                sum += a[i * size + k] * b[k * size + j];
            }
            c[i * size + j] = sum;
        }
    }
}

void pin_cpu(int core, int pid){
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(core, &mask);  // Assign pid to core

    if (sched_setaffinity(0, sizeof(mask), &mask) == -1) {
        perror("sched_setaffinity");
        exit(EXIT_FAILURE);
    }
    if (DEBUG) printf(GREEN "pinned child pid %d to core %d" RESET "\n", pid, core);
}

void transpose(int *b, int size){
    for (int i = 0; i < size; i++)
        for (int j = i + 1; j < size; j++){
            int tmp = b[i*size + j];
            b[i*size + j] = b[j*size + i];
            b[j*size + i] = tmp;
        }
}

ssize_t write_all(int fd, const void *buf, size_t count) {
    size_t total_written = 0;
    const char *ptr = buf;
    while (total_written < count) {
        ssize_t written = write(fd, ptr + total_written, count - total_written);
        if (written <= 0) return written; // Handle error
        total_written += written;
        // printf("writting ... %ld bytes ...\n", total_written);
    }
    // printf("total written: %ld bytes\n", total_written);
    return total_written;
}

ssize_t read_all(int fd, void *buf, size_t count) {
    // if (DEBUG) printf("reading from pipe %d\n", fd);
    size_t total_read = 0;
    char *ptr = buf;
    while (total_read < count) {
        ssize_t n = read(fd, ptr + total_read, count - total_read);
        if (n <= 0) return n; // Handle EOF or error
        total_read += n;
        // printf("reading ... %ld bytes ...\n", total_read);

    }
    // printf("total read: %ld bytes\n", total_read);
    return total_read;
}

ssize_t read_large_files(int fd_read, int fd_write, void* buf, size_t count, int sem_id){

    int total_fragments;
    if (read(fd_read, &total_fragments, sizeof(int)) < 0){
        perror("Can't read total fragments number");
        exit(EXIT_FAILURE);
    }
    char ack = '1';
    if (write(fd_write, &ack, sizeof(char)) == -1){ // Confirm number of fragments
        perror("IPC acknowledgement error");
        exit(EXIT_FAILURE);
    }

    int frag[PIPE_LIMIT];
    int to_read;
    ssize_t readBytes;
    ssize_t totalReadBytes = 0;
    while (total_fragments){
        to_read = (count < PIPE_LIMIT ? count : PIPE_LIMIT);
        readBytes = read_all(fd_read, frag, to_read);
        if (readBytes < 0) return readBytes; // Handle Error
        if (DEBUG) printf("[PARENT]: Received number of frags to retrieve\n");

        count -= to_read;   
        // copy fragments to buffer
        semaphore_reserve(sem_id, 0);
        memcpy(buf + totalReadBytes, frag, readBytes);
        semaphore_release(sem_id, 0);

        totalReadBytes += readBytes;
        total_fragments--;

        if (DEBUG) printf("[PARENT]: Sending ack - received frag\n");
        // send ack to writer
        if (total_fragments) 
            if (write(fd_write, &ack, sizeof(char)) == -1){ // Received fragment
                perror("IPC acknowledgement error");
                exit(EXIT_FAILURE);
            }
    }
    return totalReadBytes;
}

ssize_t write_large_files(int fd_read, int fd_write, void* buf, size_t count){

    int total_fragments = (int) ((count-1) / PIPE_LIMIT) + 1;
    if (write(fd_write, &total_fragments, sizeof(int)) < 0){
        perror("Can't write total fragments number");
        exit(EXIT_FAILURE);
    }
    char ack;
    if (DEBUG) printf("[CHILD]: Waiting ack for number of frags...\n");
    if (read(fd_read, &ack, sizeof(char)) == -1){
        perror("IPC acknowledgement error");
        exit(EXIT_FAILURE);
    }
    if (ack == '1'){
        int frag[PIPE_LIMIT];
        int to_write;
        ssize_t writtenBytes;
        ssize_t totalWrittenBytes = 0;

        while (total_fragments){
            to_write = (count < PIPE_LIMIT ? count : PIPE_LIMIT); // Write maximum of PIPE LIMIT
            memcpy(frag, buf + totalWrittenBytes, to_write); // copy content to frag buffer
            writtenBytes = write_all(fd_write, frag, to_write);
            if (writtenBytes < 0) return writtenBytes; // Handle Error

            totalWrittenBytes += writtenBytes;
            count -= to_write;
            total_fragments--;

            if (DEBUG) printf("[CHILD] Waiting ack for received frag before proceeding to next frag...\n");
            if (total_fragments) 
                if (read(fd_read, &ack, sizeof(char)) == -1 && ack != '1'){
                    perror("Can't get acknowledgement from reader for fragments");
                    exit(EXIT_FAILURE);
                }
            
        }
        if (DEBUG) printf(GREEN "[CHILD] FINISHED MESSAGE" RESET "\n");
        return totalWrittenBytes;
    }
    return -1; // No acknowledgement from reader
    // Ack Success
    
}

void matmul_IPC(int *a, int *b, int *c, int size){
    // Check system's number of cores
    // int cores = sysconf(_SC_NPROCESSORS_CONF);
    if (!cores) cores = sysconf(_SC_NPROCESSORS_CONF);
    printf("Total cores: %d\n", cores);

    pid_t main_pid = getpid();
    cpu_set_t mask;

    // Get the current CPU affinity
    if (sched_getaffinity(main_pid, sizeof(cpu_set_t), &mask) == -1) {
        perror("sched_getaffinity");
        exit(EXIT_FAILURE);
    }

    // Print initial CPU affinity
    if (DEBUG){
        printf(YELLOW "Initial CPU affinity for process %d: ", main_pid);
        for (int i = 0; i < CPU_SETSIZE; i++) {
            if (CPU_ISSET(i, &mask)) {
                printf("%d ", i);
            }
        }
        printf(RESET "\n");
    }
    
    CPU_ZERO(&mask);
    CPU_SET(0, &mask);  // Assign to CPU 0

    if (sched_setaffinity(0, sizeof(mask), &mask) == -1) {
        perror("sched_setaffinity");
        exit(EXIT_FAILURE);
    }

    // Print initial CPU affinity
    if (DEBUG) {
            printf(GREEN "Pinned main to process CPU: ");
        for (int i = 0; i < CPU_SETSIZE; i++) {
            if (CPU_ISSET(i, &mask)) {
                printf("%d ", i);
            }
        }
        printf(RESET "\n");
    }

    // Create semaphore for IPC sync
    key_t key = ftok("semfile", 65);  // Generate a key
    int sem_id = semget(key, 1, 0666 | IPC_CREAT);  // Create a semaphore set with two semaphore

    if (sem_id == -1) {
        perror("semget failed");
        exit(EXIT_FAILURE);
    }

    semaphore_init(sem_id, 0, 1);    

    transpose(b, size); // Optimization for fitting data into cache line
    
    if (DEBUG) {
        printf("Printing transposed b: \n");
        for (int i = 0; i < size; i++)
        {
            for (int j = 0; j < size; j++)
                printf("%d ", b[i * size + j]);
            printf("\n");
        }
    }
    int *pd = (int*) malloc(cores * 4 * sizeof(int));
    for (int i = 1; i < cores; i++)
        if ((pipe(pd+(i*4)) == -1) || (pipe(pd+(i*4)+2) == -1)){
            perror("pipe initilization failed");
            return;
        }
    
    // Create process pool 
    int *pid = (int*) malloc(cores * sizeof(int));
    for (int i = 1; i < cores; i++)
    {
        if (DEBUG) printf(YELLOW"forking child process ... pinning to cpu %d" RESET "\n", i);
        fflush(stdout);
        pid[i] = fork();
        if (pid[i] < 0){
            perror("fork failed");
            _exit(EXIT_FAILURE);
        }
        else if (pid[i] == 0){ // Child process
            pin_cpu(i, getpid());
            if (DEBUG) printf(GREEN"fork succeeded - pinned to cpu" RESET "\n");
            // if (DEBUG) printf("Hello world from child pid: %d\n", getpid());
            int rows;
            int readBytes = read_all(pd[i*4], &rows, sizeof(int));
            // if (DEBUG) printf("Finished reading rows\n");
            if (readBytes <= 0)
            {
                perror("read from pipe failed");
                exit(EXIT_FAILURE);
            }
            else {
                if (DEBUG) printf("child process %d - takes %d rows\n", i, rows);
                int *partial_res = (int *)malloc(rows * size * sizeof(int));
                // Do work
                for (int x = 0; x < rows; x++)
                    for (int y = 0; y < size; y++)
                    {
                        int sum = 0;
                        for (int k = 0; k < size; k++) {
                            sum += a[x * size + k] * b[y * size + k];
                        }
                        partial_res[x * size + y] = sum;
                    }
                if (DEBUG) { // print partial_res to debug
                    printf("check partial_res before writing to pipe, process %d, %d rows\n", i, rows);}
                //     for (int x = 0; x < rows; x++)
                //     {
                //         printf("row %d: ", x + 1);
                //         for (int y = 0; y < size; y++)
                //             printf("%d ", partial_res[x * size + y]);
                //         printf("\n");
                //     }
                // }
                // Write back to main process
                // semaphore_reserve(sem_id, 1);
                ssize_t writeBytes = write_large_files(pd[i*4], pd[i*4+3], partial_res, rows * size * sizeof(int));
                if (DEBUG) printf(YELLOW"[CHILD] Finished write_large_file... child process %d, transferred %d bytes" RESET "\n", i, writeBytes);
                // ssize_t writeBytes = write_all(pd[i*4+3], partial_res, rows * size * sizeof(int));
                // semaphore_release(sem_id, 1);
                if (writeBytes <= 0){
                    perror("write computed data back to main process failed");
                    free(partial_res);
                    _exit(EXIT_FAILURE);
                }
                // if (DEBUG) printf(YELLOW "Child process %d - pid %d - %d rows - wrote %zd bytes to pipe" RESET "\n", i, getpid(), writeBytes);
                free(partial_res);
            }
            if (DEBUG) printf(YELLOW "Child %d exiting process ..." RESET "\n", i);
            _exit(0);
        }
    }
    
    // split the work between processes
    int num_rows = size / (cores - 1); // exclude main process
    int last_rows = num_rows + (size % (cores - 1));

    assert((num_rows * (cores - 2) + last_rows) == size);

    // Assign tasks
    for (int i = 1; i < cores; i++){
        if (DEBUG) printf(YELLOW "Writing task to children process %d - pid %d" RESET "\n", i, pid[i]);
        if (write_all(pd[i*4+1], (i == cores - 1) ? &last_rows : &num_rows, sizeof(int)) <= 0){
            perror("Assign tasks to children failed");
            exit(EXIT_FAILURE);
        }
    }

    // Synchronize results from child workers
    int status;
    pid_t wpid;
    int pid_count = cores-1;
    int done[cores];
    memset(done, 0, cores * sizeof(int));
    while (pid_count){
        if (DEBUG) printf(RED "PID_COUNT LEFT: %d" RESET "\n", pid_count);
        for (int i = 1; i < cores; i++){
            if (DEBUG){
                printf("num of cores: %ld - current iteration: %d\n", cores, i);
                printf("Waiting for child process %d ", i);
                printf("- pid %d \n", pid[i]);
            }
            
            if ((wpid = waitpid(pid[i], &status, WNOHANG)==-1))
            {
                perror("waitpid failed");
                exit(EXIT_FAILURE);
            }
        
            if (wpid == 0 && !done[i]){ // WNOHANG returns 0 if child not terminated
                if (DEBUG) printf("Child process %d - pid %d blocking ... Reading from child process\n", i, pid[i]);
                int buffer_size = size;
                if (i == cores - 1) buffer_size *= last_rows;
                else buffer_size *= num_rows;
                int c_pos = (i-1)*num_rows*size;
                ssize_t readBytes = read_large_files(pd[i*4+2], pd[i*4+1], (int*)(c+c_pos), buffer_size * sizeof(int), sem_id);
                
                if (readBytes <= 0){
                    perror("Read from worker failed");
                    exit(EXIT_FAILURE);
                }
                if (DEBUG) printf(YELLOW"[PARENT] Finished write_large_file... child process %d, transferred %d bytes" RESET "\n", i, readBytes);
    
                if (DEBUG) printf(GREEN "shared array synchronized" RESET "\n");
                // if (DEBUG){
                //     printf("Print C after every synchronization\n");    
                //     for (int x = 0; x < size; x++)
                //     {
                //         for (int y = 0; y < size; y++)
                //             printf("%d ", c[x * size + y]);    
                //         printf("\n");
                //     }
                // }
                
                // free(buffer);
                done[i] = 1;
                pid_count--;
                close(pd[i*4+3]);
                close(pd[i*4+2]);
                close(pd[i*4+1]);
                close(pd[i*4]);
                // printf("num of cores: %d - current iteration: %d\n", cores, i);
            } else if (wpid == pid[i]) {
                // WNHOHANG returns pid when child terminated
                if (WIFEXITED(status)) {
                    printf("Child exited with status %d\n", WEXITSTATUS(status));
                } else if (WIFSIGNALED(status)) {
                    printf("Child was terminated by signal %d\n", WTERMSIG(status));
                }
            } else {
                // An error occurred
                perror("waitpid");
                break;
            }
        }
    }
    

    // Cleanup: Remove the semaphore
    free(pid);
    free(pd);
    semctl(sem_id, 0, IPC_RMID);
    if (DEBUG) printf("Semaphore removed.\n");
}


int main(int argc, char const *argv[])
{
    // Check if the user provided at least one argument
    if (argc < 3) {
        printf("Usage: %s <input>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    printf("Input size: %d rows, %d columns\n", N, N);

    // Allocate memory for matrices A, B, and C
    int *A = (int *)malloc(N * N * sizeof(int));
    int *B = (int *)malloc(N * N * sizeof(int));
    int *C = (int *)malloc(N * N * sizeof(int));

    if (A == NULL || B == NULL || C == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    // Initialize matrices A and B (for example, fill with 1s)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i * N + j] = 1;
            B[i * N + j] = 1;
            C[i * N + j] = 0;  // initialize C to zero
        }
        B[i*N] = 3;
    }

    if (DEBUG) printf("Initialized matrices A B C\n");
    if (DEBUG) printf("starts running matmul_IPC\n");
    if (argv[2])
        cores = atoi(argv[2]);

    struct timeval start, end;
    gettimeofday(&start,NULL);
    if (strcmp(argv[2],"single") == 0)
        matmul_single_threaded(A, B, C, N);
    else matmul_IPC(A, B, C, N);
    // matmul_IPC(A, B, C, N);
    gettimeofday(&end, NULL);
    
    // Warning: Printing 100 million numbers to the console can be extremely slow.
    // It is recommended to redirect the output to a file.
    // Example: ./program > output.txt

    if (DEBUG)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                printf("%d ", C[i * N + j]);
            }
            printf("\n");
        }
    printf("Time for matmul: %f seconds\n", getdetlatimeofday(&start, &end));

    // Free allocated memory
    free(A);
    free(B);
    free(C);
            

    print_stats(); 

    return 0;
}


