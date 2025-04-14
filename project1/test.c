#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>       // for gettimeofday
#include <string.h>
#include <errno.h>
#include <pthread.h>

// Define the system call number if not present in your headers:
#ifndef SYS_app_helper
#define SYS_turn_on_extent 449  // <--- Adjust to match your setup
#define SYS_turn_off_extent 450
#endif

// Default values
#define DEFAULT_BUF_SIZE       4096000
// #define NUM_ITERATIONS 100000
#define NUM_ITERATIONS 1
#define NUM_THREADS  10


// A helper function to get current time in microseconds
static long get_time_us(void) {
    struct timeval tv;
    if (gettimeofday(&tv, NULL) < 0) {
        perror("gettimeofday");
        exit(EXIT_FAILURE);
    }
    return (tv.tv_sec * 1000000L) + tv.tv_usec;
}

typedef struct {
        char *buffer;
        int *touch_list;
        int start;
        int end;
} thread_arg_t;

static double get_time_ms(void) {
        struct timeval tv;
        if (gettimeofday(&tv, NULL) < 0) {
            perror("gettimeofday");
            exit(EXIT_FAILURE);
        }
        return ((double) tv.tv_sec * 1000) + ((double) tv.tv_usec / 1000);
    }

void* thread_touch(void *arg) {
        thread_arg_t *args = (thread_arg_t *)arg;
        int page_size = 4096;

        // Write pattern
        for (int i = args->start; i < args->end; i++) {
                args->buffer[args->touch_list[i] * page_size] = 1;
        }

        // Multiply pattern
        for (int r = 0; r < 5; r++) {
                for (int i = args->start; i < args->end; i++) {
                args->buffer[args->touch_list[i] * page_size] *= 2;
                }
        }

        return NULL;
}
void touch_and_print_buffer(char *buffer, int touch_list[], int size) {
        int page_size = 4096; // Assuming page size is 4096 bytes
        for (int i = 0; i < size; i++) {
                buffer[touch_list[i]*page_size] = 1;
        }

        for (int i = 0; i < 5; i++){
                for (int j = 0; j < size; j++)
                        buffer[touch_list[j]*page_size] *= 2;
                }   

        // for (int i = 0; i < size; i++) {
        //         printf("Buffer[%d] = %d\n", touch_list[i]*page_size, buffer[touch_list[i]*page_size]);
        // }
}

int main(int argc, char **argv) {
        char *buffer;
        int i;
        double start_time, end_time;
        double total_time = 0;
        size_t buf_size = DEFAULT_BUF_SIZE;

        // If a command-line argument is provided, use it as the buffer size.
        if (argc > 1) {
                buf_size = (size_t)atoi(argv[1]);
                if (buf_size == 0) {
                        fprintf(stderr, "Invalid buffer size provided. Using default %d bytes.\n", DEFAULT_BUF_SIZE);
                        buf_size = DEFAULT_BUF_SIZE;
                }
        }
        printf("Using buffer size: %zu bytes\n", buf_size);

        start_time = get_time_ms();
        for (i = 0; i < NUM_ITERATIONS; i++) {
                // Test single call before loop to ensure correctness
                if (syscall(SYS_turn_on_extent) < 0) {
                        perror("syscall rb_extent (test)");
                        return EXIT_FAILURE;
                }
                printf("Single syscall test completed successfully.\n");

                // Allocate user-level buffer
                buffer = (char *)malloc(buf_size);
                if (!buffer) {
                        perror("malloc");
                        return EXIT_FAILURE;
                }

                int size = (int) (buf_size / 4096); // number of pages
                int *touch_list = (int *)malloc(sizeof(int) * size);
                for (int i = 0; i < size; i++) {
                        touch_list[i] = i;
                }
                pthread_t threads[NUM_THREADS];
                thread_arg_t args[NUM_THREADS];
                int chunk_size = size / NUM_THREADS;

                for (int t = 0; t < NUM_THREADS; t++) {
                        args[t].buffer = buffer;
                        args[t].touch_list = touch_list;
                        args[t].start = t * chunk_size;
                        args[t].end = (t == NUM_THREADS - 1) ? size : (t + 1) * chunk_size;
            
                        if (pthread_create(&threads[t], NULL, thread_touch, &args[t]) != 0) {
                            perror("pthread_create");
                            free(buffer);
                            free(touch_list);
                            return EXIT_FAILURE;
                        }
                }
                for (int t = 0; t < NUM_THREADS; t++) {
                        pthread_join(threads[t], NULL);
                }

                free(buffer);
                free(touch_list);

                // Test single call before loop to ensure correctness
                int number_of_extents = 0;
                if ((number_of_extents = syscall(SYS_turn_off_extent)) < 0) {
                        perror("syscall turn_off_extent");
                        return EXIT_FAILURE;
                }
                printf("[Total extents]: %d\n", number_of_extents);
        }
        end_time = get_time_ms();
        total_time = end_time - start_time;

        // Compute average time in microseconds
        double avg_time_ms = (double)total_time / (double)NUM_ITERATIONS;

        printf("Ran %d iterations.\n", NUM_ITERATIONS);
        printf("Total time: %.3f miliseconds.\n", total_time);
        printf("Average time per syscall: %.3f miliseconds.\n", avg_time_ms);

        return EXIT_SUCCESS;
}
