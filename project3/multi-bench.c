#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/syscall.h>
#include <time.h>
#include <linux/time.h>

#define SYS_enable_coop_sched 449
#define SYS_set_thread_coop 450

#define MAX_THREADS 1000

enum thread_type {
        NORMAL_THREAD = 0,
        COOP_THREAD = 1
};

struct thread_info {
        int id;
        int tid;
        struct timespec start_time;
        struct timespec end_time;
        double work_result;
        enum thread_type type;
};

pthread_t threads[MAX_THREADS];
struct thread_info infos[MAX_THREADS];

int thread_done[MAX_THREADS] = {0};
int threads_joined = 0;
int total_threads = 0;
int coop_threads = 0;
int sleep_time = 0;

pthread_mutex_t done_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t done_cond = PTHREAD_COND_INITIALIZER;

void record_time(struct timespec* t) {
        clock_gettime(CLOCK_MONOTONIC, t);
}

void notify_completion(int id) {
        pthread_mutex_lock(&done_mutex);
        thread_done[id] = 1;
        record_time(&infos[id].end_time);
        pthread_cond_signal(&done_cond);
        pthread_mutex_unlock(&done_mutex);
}

int any_thread_done(int* done, int n) {
        for (int i = 0; i < n; ++i)
            if (done[i]) return 1;
        return 0;
}

void wait_and_join_all(int n) {
        while (threads_joined < n) {
            pthread_mutex_lock(&done_mutex);
    
            while (!any_thread_done(thread_done, n)) {
                pthread_cond_wait(&done_cond, &done_mutex);
            }
    
            for (int i = 0; i < n; ++i) {
                if (thread_done[i]) {
                    pthread_join(threads[i], NULL);
                    thread_done[i] = 0;
                    threads_joined++;
                    printf("Main: Thread %d joined\n", i);
                }
            }
    
            pthread_mutex_unlock(&done_mutex);
        }
}

void* coop_thread_function(void* arg) {
        struct thread_info* info = (struct thread_info*)arg;
        info->tid = syscall(SYS_gettid);
        record_time(&info->start_time);

        if (syscall(SYS_set_thread_coop, info->tid, 1) < 0) {
            perror("set_thread_coop");
            return NULL;
        }

        volatile double count = 1;
        for (int i = 0; i < 2000000000; i++) {
                count += (i%3) * 0.1;
        }

        info->work_result = count;

        notify_completion(info->id);
        return NULL;
}

void* thread_function(void* arg) {
        struct thread_info* info = (struct thread_info*)arg;
        info->tid = syscall(SYS_gettid);
        record_time(&info->start_time);

        volatile double count = 1;
        for (int i = 0; i < 2000000000; i++) {
                count += (i%3) * 0.1;
        }

        info->work_result = count;

        notify_completion(info->id);
        return NULL;
}

int main(int argc, char* argv[]) {
        if (argc != 4) {
                fprintf(stderr, "Usage: %s <number_of_normal_threads> <number_of_inactive_threads> <sleep_time>\n", argv[0]);
                return EXIT_FAILURE;
        }
        total_threads = atoi(argv[1]);
        coop_threads = atoi(argv[2]);

        if (total_threads < 0 || coop_threads < 0) {
                fprintf(stderr, "Invalid thread counts. Please enter non-negative integers for thread counts.\n");
                return EXIT_FAILURE;
        }

        sleep_time = atoi(argv[3]);
        if (sleep_time <= 0) {
                fprintf(stderr, "Invalid sleep time. Please enter integer > 0.\n");
                return EXIT_FAILURE;
        }

        // Enable cooperative scheduling
        if (syscall(SYS_enable_coop_sched, 1) < 0) {
            perror("enable_coop_sched = 1");
            return EXIT_FAILURE;
        }

        if (threads == NULL) {
                perror("Failed to allocate memory for threads");
                return EXIT_FAILURE;
        }

        for (int i = 0; i < total_threads + coop_threads; i++) {
                infos[i].id = i;
                infos[i].type = (i < total_threads) ? NORMAL_THREAD : COOP_THREAD;
                void* (*thread_func)(void*) = (infos[i].type == COOP_THREAD) ? coop_thread_function : thread_function;

                if (pthread_create(&threads[i], NULL, thread_func, &infos[i]) != 0) {
                        perror("Failed to create non-cooperative thread");
                        return EXIT_FAILURE;
                }
        }

        printf("Created %d normal threads.\nCreated %d coop threads\n", total_threads, coop_threads);

        printf("Waiting %d seconds before turning off cooperative scheduling...\n", sleep_time);
        sleep(sleep_time);

        for (int i = total_threads; i < total_threads + coop_threads; i++) {
                if (infos[i].type == COOP_THREAD) {
                        if (syscall(SYS_set_thread_coop, infos[i].tid, 0) < 0) {
                                perror("set_thread_coop");
                                return EXIT_FAILURE;
                        }
                }
                infos[i].type = NORMAL_THREAD;
        }
        printf("Cooperative threads set to normal.\n");

        wait_and_join_all(total_threads + coop_threads);

        printf("All threads terminated.\n");

        // disable cooperative scheduling
        if (syscall(SYS_enable_coop_sched, 0) < 0) {
            perror("enable_coop_sched = 0");
            return EXIT_FAILURE;
        }
        
        printf("Cooperative scheduling disabled.\n");

        double normal_avg_turnaround = 0.0;
        double coop_avg_turnaround = 0.0;

        printf("Total:\n Normal threads: %d.\nCoop threads: %d\n", total_threads, coop_threads);
        for (int i = 0; i < total_threads + coop_threads; i++) {
                double turnaround_time = (infos[i].end_time.tv_sec - infos[i].start_time.tv_sec) +
                                         (infos[i].end_time.tv_nsec - infos[i].start_time.tv_nsec) / 1e9;

                if (i < total_threads) {
                        normal_avg_turnaround += turnaround_time;
                        printf("Normal Thread %2d: Turnaround Time: %.9f seconds\n", i, turnaround_time);
                } else {
                        coop_avg_turnaround += turnaround_time;
                        printf("Coop Thread %2d: Turnaround Time: %.9f seconds\n", i, turnaround_time);
                }
        }

        if (total_threads > 0) {
                normal_avg_turnaround /= total_threads;
        }
        if (coop_threads > 0) {
                coop_avg_turnaround /= coop_threads;
        }

        printf("Average Turnaround Time:\n");
        printf(" Normal threads: %.9f seconds\n", normal_avg_turnaround);
        printf(" Coop threads: %.9f seconds\n", coop_avg_turnaround);
        printf("Total:\nNormal threads: %d\nCoop threads: %d\n", total_threads, coop_threads);

        return EXIT_SUCCESS;
}