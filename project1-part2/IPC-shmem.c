/* CS 519, Spring 2025: Project 1 - Part 2
 * IPC using shared memory to perform matrix multiplication.
 * Feel free to extend or change any code or functions below.
 */
#define _GNU_SOURCE  // Enable GNU extensions for CPU affinity functions
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sched.h>
#include <wait.h>


#define RESET   "\033[0m"
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */

#ifndef DEBUG
    #define DEBUG 0
#endif

#ifndef SHM_HUGETLB
    #define SHM_HUGETLB 04000
#endif

//Add all your global variables and definitions here.
#define MATRIX_SIZE 10000
int cores;

/* Time function that calculates time between start and end */
double getdetlatimeofday(struct timeval *begin, struct timeval *end)
{
    return (end->tv_sec + end->tv_usec * 1.0 / 1000000) -
           (begin->tv_sec + begin->tv_usec * 1.0 / 1000000);
}

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

typedef struct {
  int A[MATRIX_SIZE][MATRIX_SIZE];
  int B[MATRIX_SIZE][MATRIX_SIZE];
  int C[MATRIX_SIZE][MATRIX_SIZE];
} Shmem;

void transpose_in_place(int a[MATRIX_SIZE][MATRIX_SIZE], int n) {
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            // Swap elements at (i, j) and (j, i)
            int temp = a[i][j];
            a[i][j] = a[j][i];
            a[j][i] = temp;
        }
    }
}

Shmem *get_shared_mem(int *shmid){
    // Generate a unique key. Note: "shmfile" must exist in the filesystem.
    key_t key = ftok("shmfile", 66);
    if (key == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    // Create a shared memory segment large enough for c
    // TO-DO: Map into HUGE PAGE regions
    *shmid = shmget(key, sizeof(Shmem), SHM_HUGETLB | IPC_CREAT | SHM_R | SHM_W);
    if (shmid < 0) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    if (DEBUG) printf(YELLOW "Shmem key at creation: %d" RESET "\n", *shmid);

    // Attach the shared memory segment to our process
    Shmem *mat = (Shmem*) shmat(*shmid, NULL, 0);
    if (mat == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    return mat;
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


void matmul_IPC(int *a, int *b, int size){
    // Check system's number of cores
    if (!cores) cores = sysconf(_SC_NPROCESSORS_CONF);

    printf("Total cores: %d\n", cores);

    if (DEBUG) {
        printf("printing mat a\n");
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                printf("%d ", a[i*size + j]);
            }
            printf("\n");
        }

        printf("printing mat b\n");
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                printf("%d ", b[i*size + j]);
            }
            printf("\n");
        }
    }

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
    int key = ftok("semfile", 66);  // Generate a key
    int sem_id = semget(key, 1, 0666 | IPC_CREAT);  // Create a semaphore set

    if (DEBUG) printf(YELLOW "Created sem key %d " RESET "\n", sem_id);

    if (sem_id == -1) {
        perror("semget failed");
        exit(EXIT_FAILURE);
    }

    semaphore_init(sem_id, 0, 1);    

    // Allocate shared memory for matrices A, B, and C
    int shm_id = -1;
    Shmem *mat = get_shared_mem(&shm_id);
    if (DEBUG) printf(YELLOW "Shared mem ID: %d" RESET "\n", shm_id);

    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)  
        {
            mat->A[i][j] = a[i*size + j];
            mat->B[i][j] = b[j*size + i];
        }

    if (DEBUG) {
        printf("printing shared mem mat A\n");
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                printf("%d ", mat->A[i][j]);
            }
            printf("\n");
        }

        printf("printing shared mem mat B\n");
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                printf("%d ", mat->B[i][j]);
            }
            printf("\n");
        }
    }

    if (mat->A == NULL || mat->B == NULL || mat->C == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    // // transpose matrix B to fit data into cache lines
    // transpose_in_place(mat->B, size);

    // if (DEBUG) {
    //     printf("printing mat B - transposed\n");
    //     for (int i = 0; i < size; i++) {
    //         for (int j = 0; j < size; j++) {
    //             printf("%d ", mat->B[i][j]);
    //         }
    //         printf("\n");
    //     }
    // }

    // Fork cores child processes to perform matrix multiplication.
    int *pid = (int*) malloc(cores*sizeof(int));
    memset(pid, 0, cores*sizeof(int));
    for (int i = 1; i < cores; i++) {
        pid[i] = fork();
        if (pid[i] < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (pid[i] == 0) {
            // Child process: compute a portion of the result matrix C.
            pin_cpu(i, getpid());
            if (DEBUG) printf(GREEN"fork succeeded - pinned to cpu" RESET "\n");
            // if (DEBUG) printf(YELLOW "Child process %d - semaphore key_id %d" RESET "\n", i, sem_id);
            // split the work between processes
            int num_rows = size / (cores - 1); // exclude main process
            int start_row = (i-1) * num_rows;
            int end_row = (i == cores - 1) ? size : start_row + num_rows;

            // semaphore_reserve(sem_id, 0);
            for (int x = start_row; x < end_row; x++) {
                for (int y = 0; y < size; y++) {
                    int sum = 0;
                    for (int k = 0; k < size; k++) {
                        sum += mat->A[x][k] * mat->B[y][k];
                    }
                    mat->C[x][y] = sum;
                }
            }
            // semaphore_release(sem_id, 0);

            // Detach the shared memory segment and exit child process.
            if (shmdt(mat) == -1) {
                perror("shmdt in child");
            }
            if (DEBUG) printf(GREEN "Child process %d - exit success" RESET "\n", i);
            _exit(EXIT_SUCCESS);
        }
    } 

    // Parent waits for both child processes to complete.
    for (int i = 1; i < cores; i++)
        wait(NULL);

    // Print the result matrix.
    if (DEBUG) {
        printf("Result Matrix C (A x B):\n");
        for (int i = 0; i < size; i++){
            for (int j = 0; j < size; j++){
                printf("%d ", mat->C[i][j]);
            }
            printf("\n");
        }  
    }
  
    // Clean ups
    // Removed sephamore set
    semctl(sem_id, 0, IPC_RMID);

    // Detach the shared memory segment in the parent.
    if (shmdt(mat) == -1) {
        perror("shmdt in parent");
        exit(EXIT_FAILURE);
    }

    // Remove the shared memory segment
    if (shmctl(shm_id, IPC_RMID, 0) == -1) {
        perror("shmctl");
        exit(1);
    }
    
    free(pid);
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

    int *A = (int*) malloc(N*N*sizeof(int));
    int *B = (int*) malloc(N*N*sizeof(int));

    // Initialize matrices A and B (for example, fill with 1s)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i*N + j] = 1;
            B[i*N + j] = 1;
        }
    }

    // if (DEBUG) {
    //     printf("printing mat A\n");
    //     for (int i = 0; i < N; i++) {
    //         for (int j = 0; j < N; j++) {
    //             printf("%d ", A[i][j]);
    //         }
    //         printf("\n");
    //     }

    //     printf("printing mat B\n");
    //     for (int i = 0; i < N; i++) {
    //         for (int j = 0; j < N; j++) {
    //             printf("%d ", B[i][j]);
    //         }
    //         printf("\n");
    //     }
    // }

    if (DEBUG) printf("Initialized matrices A B C\n");
    if (DEBUG) printf("starts running matmul_IPC\n");
    // User input number of cores to run
    if (argv[2])
            cores = atoi(argv[2]);

    struct timeval start, end;
    gettimeofday(&start,NULL);
    matmul_IPC(A, B, N);
    // matmul_IPC(A, B, C, N);
    gettimeofday(&end, NULL);

    // Warning: Printing 100 million numbers to the console can be extremely slow.
    // It is recommended to redirect the output to a file.
    // Example: ./program > output.txt

    // for (int i = 0; i < N; i++) {
    //     for (int j = 0; j < N; j++) {
    //         printf("%d ", mat->C[i][j]);
    //     }
    //     printf("\n");
    // }
    printf("Total runtime: %f seconds\n", getdetlatimeofday(&start, &end));

   return 0;
}


