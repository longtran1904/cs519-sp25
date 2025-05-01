#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

volatile sig_atomic_t stop = 0;

void handle_sigint(int sig) {
        stop = 1;
}

void* thread_function(void* arg) {
        while (!stop); // Loop until SIGINT is received
        return NULL;
}

int main(int argc, char* argv[]) {
        if (argc != 2) {
                fprintf(stderr, "Usage: %s <number_of_threads>\n", argv[0]);
                return EXIT_FAILURE;
        }

        int num_threads = atoi(argv[1]);
        if (num_threads <= 0) {
                fprintf(stderr, "Number of threads must be a positive integer.\n");
                return EXIT_FAILURE;
        }

        pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
        if (threads == NULL) {
                perror("Failed to allocate memory for threads");
                return EXIT_FAILURE;
        }

        for (int i = 0; i < num_threads; i++) {
                if (pthread_create(&threads[i], NULL, thread_function, NULL) != 0) {
                        perror("Failed to create thread");
                        free(threads);
                        return EXIT_FAILURE;
                }
        }

        printf("%d threads created. Press Ctrl+C to terminate.\n", num_threads);

        for (int i = 0; i < num_threads; i++) {
                pthread_join(threads[i], NULL);
        }

        free(threads);
        return EXIT_SUCCESS;
}