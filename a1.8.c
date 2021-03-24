/*
    The Hybrid Merge Sort to use for Operating Systems Assignment 1 2021
    written by Robert Sheehan

    Modified by: Jinkai Zhang
    UPI: Jzha541

    By submitting a program you are claiming that you and only you have made
    adjustments and additions to this code.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/times.h>
#include <math.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define SIZE    4
#define MAX     1000
#define SPLIT   16

static pthread_mutex_t merge_mutex;
struct combine {
        struct block {
            int size;
            int *data;
        }block;
    int depth;
};


void print_data(struct block *block) {
    for (int i = 0; i < block->size; ++i)
        printf("%d ", block->data[i]);
    printf("\n");
}

/* The insertion sort for smaller halves. */
void insertion_sort(struct block *block) {
    for (int i = 1; i < block->size; ++i) {
        for (int j = i; j > 0; --j) {
            if (block->data[j-1] > block->data[j]) {
                int temp;
                temp = block->data[j-1];
                block->data[j-1] = block->data[j];
                block->data[j] = temp;
            }
        }
    }
}

/* Combine the two halves back together. */
void merge(struct block *left, struct block *right) {
    int *combined = calloc(left->size + right->size, sizeof(int));
    if (combined == NULL) {
        perror("Allocating space for merge.\n");
        exit(EXIT_FAILURE);
    }
        int dest = 0, l = 0, r = 0;
        while (l < left->size && r < right->size) {
                if (left->data[l] < right->data[r])
                        combined[dest++] = left->data[l++];
                else
                        combined[dest++] = right->data[r++];
        }
        while (l < left->size)
                combined[dest++] = left->data[l++];
        while (r < right->size)
                combined[dest++] = right->data[r++];
    memmove(left->data, combined, (left->size + right->size) * sizeof(int));
    free(combined);
}

/* Check to see if the data is sorted. */
bool is_sorted(struct block *block) {
    bool sorted = true;
    for (int i = 0; i < block->size - 1; i++) {
        if (block->data[i] > block->data[i + 1])
            sorted = false;
    }
    return sorted;
}

/* Merge sort the data. */
void *merge_sort(void *combine) {
    struct combine *merge_combine = (struct combine *)combine;

    if (merge_combine->block.size > SPLIT) {
        struct combine left_block;
        struct combine right_block;
        left_block.block.size = merge_combine->block.size / 2;
        left_block.block.data = merge_combine->block.data;
        right_block.block.size = merge_combine->block.size - left_block.block.size; // left_block.size + (block->size % 2);
        right_block.block.data = merge_combine->block.data + left_block.block.size;
       left_block.depth = merge_combine->depth +1;
       right_block.depth = merge_combine->depth + 1;

        pthread_mutex_lock(&merge_mutex);
        if (left_block.depth < 3) {

            pid_t pid = fork();

            if(pid <0) {
                fprintf(stderr, "Fork failed");
                exit(EXIT_FAILURE);
            }

            else if (pid > 0){

                merge_sort(&right_block);
                //Wait for the child process.
                wait(NULL);
                merge(&left_block.block, &right_block.block);
                munmap(NULL,merge_combine->block.size * sizeof(int));

            }
            else{
                printf("ChildProcess created ,the depth is %d \n",left_block.depth);
                merge_sort(&left_block);
                exit(EXIT_SUCCESS);
            }

        }
        else{
            merge_sort(&right_block);
            merge_sort(&left_block);
            merge(&left_block.block, &right_block.block);
        }
        pthread_mutex_unlock(&merge_mutex);
    } else {

        insertion_sort(&merge_combine->block);
    }
}

/* Fill the array with random data. */
void produce_random_data(struct block *block) {
    srand(1); // the same random data seed every time

    for (int i = 0; i < block->size; i++) {
        block->data[i] = rand() % MAX;
    }
}

int main(int argc, char *argv[]) {
        long size;

        if (argc < 2) {
                size = SIZE;
        } else {
                size = atol(argv[1]);
        }

        struct combine combine;
    combine.block.size = (int)pow(2, size);
    combine.depth = -1;
    combine.block.data = mmap(NULL, combine.block.size * sizeof(int), PROT_READ |PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1,0);


    if (combine.block.data == NULL) {
        perror("Unable to allocate space for data.\n");
        exit(EXIT_FAILURE);
    }

    produce_random_data(&combine.block);

    struct timeval start_wall_time, finish_wall_time, wall_time;
    struct tms start_times, finish_times;
    gettimeofday(&start_wall_time, NULL);
    times(&start_times);

    merge_sort(&combine);

    gettimeofday(&finish_wall_time, NULL);
    times(&finish_times);
    timersub(&finish_wall_time, &start_wall_time, &wall_time);
    printf("start time in clock ticks: %ld\n", start_times.tms_utime);
    printf("finish time in clock ticks: %ld\n", finish_times.tms_utime);
    printf("wall time %ld secs and %ld microseconds\n", wall_time.tv_sec, wall_time.tv_usec);

    if (combine.block.size < 1025)
        print_data(&combine.block);

    printf(is_sorted(&combine.block) ? "sorted\n" : "not sorted\n");

    exit(EXIT_SUCCESS);
}
