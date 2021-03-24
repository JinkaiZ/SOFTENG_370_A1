/*
    The Hybrid Merge Sort to use for Operating Systems Assignment 1 2021
    written by Robert Sheehan

    Modified by: jinkai Zhang   
    UPI: put your Jzha541

    By submitting a program you are claiming that you and only you have made
    adjustments and additions to this code.
 */

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
#include <semaphore.h>

#define SIZE 4
#define MAX 1000
#define SPLIT 16
#define THREAD_NUM 7

pthread_mutex_t merge_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t main_cond = PTHREAD_COND_INITIALIZER;
//static pthread_cond_t thread_cond = PTHREAD_COND_INITIALIZER;

static int thread_count = 0;

sem_t semaphore;

struct block
{
    int size;
    int *data;
};

void print_data(struct block *block)
{
    for (int i = 0; i < block->size; ++i)
        printf("%d ", block->data[i]);
    printf("\n");
}

/* The insertion sort for smaller halves. */
void insertion_sort(struct block *block)
{
    for (int i = 1; i < block->size; ++i)
    {
        for (int j = i; j > 0; --j)
        {
            if (block->data[j - 1] > block->data[j])
            {
                int temp;
                temp = block->data[j - 1];
                block->data[j - 1] = block->data[j];
                block->data[j] = temp;
            }
        }
    }
}

/* Combine the two halves back together. */
void merge(struct block *left, struct block *right)
{

    int *combined = calloc(left->size + right->size, sizeof(int));
    if (combined == NULL)
    {
        perror("Allocating space for merge.\n");
        exit(EXIT_FAILURE);
    }
    int dest = 0, l = 0, r = 0;
    while (l < left->size && r < right->size)
    {
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

/* Merge sort the data. */
void *merge_sort(void *block)
{
    //active the child threads
    sem_post(&semaphore);

    struct block *merge_block = (struct block *)block;

    if (merge_block->size > SPLIT)
    {
        struct block left_block;
        struct block right_block;
        left_block.size = merge_block->size / 2;
        left_block.data = merge_block->data;
        right_block.size = merge_block->size - left_block.size; // left_block.size + (block->size % 2);
        right_block.data = merge_block->data + left_block.size;
        int task_index = assign_task(&left_block);

        if (task_index < 0)
        {
            merge_sort(&left_block);
            sem_post(&semaphore);
        };

        merge_sort(&right_block);

        //merge
        merge(&left_block, &right_block);
    }
    else
    {
        insertion_sort(block);
        sem_wait(&semaphore);
    }
}

/* Check to see if the data is sorted. */
bool is_sorted(struct block *block)
{
    bool sorted = true;
    for (int i = 0; i < block->size - 1; i++)
    {
        if (block->data[i] > block->data[i + 1])
            sorted = false;
    }
    return sorted;
}

/* Fill the array with random data. */
void produce_random_data(struct block *block)
{
    srand(1); // the same random data seed every time
    for (int i = 0; i < block->size; i++)
    {
        block->data[i] = rand() % MAX;
    }
}
void *init_thread(void *args)
{
    thread_count++;
    pthread_cond_signal(&main_cond);
    printf("active main");
    sem_wait(&semaphore);
}

int assign_task(struct block *block)
{
    int task_index = -1;
    if (thread_count < 7)
    {
        merge_sort(&block);
    }
    return task_index;
}

int main(int argc, char *argv[])
{
    long size;
    if (argc < 2)
    {
        size = SIZE;
    }
    else
    {
        size = atol(argv[1]);
    }

    sem_init(&semaphore, 0, 7);
    struct block block;
    block.size = (int)pow(2, size);
    block.data = (int *)calloc(block.size, sizeof(int));
    if (block.data == NULL)
    {
        perror("Unable to allocate space for data.\n");
        exit(EXIT_FAILURE);
    }

    produce_random_data(&block);

    struct timeval start_wall_time, finish_wall_time, wall_time;
    struct tms start_times, finish_times;
    gettimeofday(&start_wall_time, NULL);
    times(&start_times);

    //In the main thread.

    pthread_mutex_lock(&merge_mutex);

    //Create 7 threads
    pthread_t thread[THREAD_NUM];

    for (int i = 0; i < THREAD_NUM; i++)
    {
        //the size is depends on the work_count.
        pthread_create(&thread[i], NULL, init_thread, NULL);
        printf("Thread %d created \n", i);
    }

    printf("The main thread: start \n");
    printf("The thread_count is %d \n", thread_count);

    while (thread_count != THREAD_NUM)
    {
        pthread_cond_wait(&main_cond, &merge_mutex);
        printf("main thread: awoke, count is now %d \n", thread_count);
    }

    merge_sort(&block);

    printf("main thread: done \n");
    pthread_mutex_unlock(&merge_mutex);

    gettimeofday(&finish_wall_time, NULL);
    times(&finish_times);
    timersub(&finish_wall_time, &start_wall_time, &wall_time);
    printf("start time in clock ticks: %ld\n", start_times.tms_utime);
    printf("finish time in clock ticks: %ld\n", finish_times.tms_utime);
    printf("wall time %ld secs and %ld microseconds\n", wall_time.tv_sec, wall_time.tv_usec);

    if (block.size < 1025)
        print_data(&block);

    printf(is_sorted(&block) ? "sorted\n" : "not sorted\n");
    free(block.data);

    exit(EXIT_SUCCESS);
}
