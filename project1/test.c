#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>       // for gettimeofday
#include <string.h>
#include <errno.h>

// Define the system call number if not present in your headers:
#ifndef SYS_app_helper
#define SYS_turn_on_extent 449  // <--- Adjust to match your setup
#define SYS_turn_off_extent 450
#endif

// Default values
#define DEFAULT_BUF_SIZE       409600
// #define NUM_ITERATIONS 100000
#define NUM_ITERATIONS 1

// A helper function to get current time in microseconds
static long get_time_us(void) {
    struct timeval tv;
    if (gettimeofday(&tv, NULL) < 0) {
        perror("gettimeofday");
        exit(EXIT_FAILURE);
    }
    return (tv.tv_sec * 1000000L) + tv.tv_usec;
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
        long start_time, end_time;
        long total_time = 0;
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

        start_time = get_time_us();
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
                int touch_list[size];
                for (int i = 0; i < size; i++) {
                        touch_list[i] = i;
                }
                touch_and_print_buffer(buffer, touch_list, size);

                free(buffer);

                // Test single call before loop to ensure correctness
                int number_of_extents = 0;
                if ((number_of_extents = syscall(SYS_turn_off_extent)) < 0) {
                        perror("syscall turn_off_extent");
                        return EXIT_FAILURE;
                }
                printf("[Total extents]: %d\n", number_of_extents);
        }
        end_time = get_time_us();
        total_time = end_time - start_time;

        // Compute average time in microseconds
        double avg_time_us = (double)total_time / (double)NUM_ITERATIONS;

        printf("Ran %d iterations.\n", NUM_ITERATIONS);
        printf("Total time: %ld microseconds.\n", total_time);
        printf("Average time per syscall: %.2f microseconds.\n", avg_time_us);

        return EXIT_SUCCESS;
}
