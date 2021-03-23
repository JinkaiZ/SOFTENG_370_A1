/*
    The Hybrid Merge Sort to use for Operating Systems Assignment 1 2021
    written by Robert Sheehan

    Modified by: put your name here
    UPI: put your login here

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
#include <unistd.h>
#include <pthread.h>

#define THREAD_NUM 4


typedef struct  Task {
int a,b;
} Task;

Task taskQueue[256];
int taskCount = 0;

pthread_mutex_t  mutexQueue;
pthread_cond_t condQueue;

void executeTask(struct Task* task){
    int result = task->a + task->b;
    printf("The sum of %d and %d is %d\n", task->a, task->b,result );
}

void submitTask(Task task){
    pthread_mutex_lock(&mutexQueue);
    taskQueue[taskCount] = task;
    taskCount++;
    pthread_mutex_unlock(&mutexQueue);
    pthread_cond_signal(&condQueue);
}

void* startThread(void* args){
    while(1) {
        Task task;

        pthread_mutex_lock(&mutexQueue);

        while(taskCount == 0){
            pthread_cond_wait(&condQueue,&mutexQueue);
        }

            found = 1;
            task = taskQueue[0];
            int i;
            for (i = 0; i < taskCount - 1; i++) {
                taskQueue[i] = taskQueue[i + 1];
            }
            taskCount--;

        pthread_mutex_unlock(&mutexQueue);

            executeTask(&task);

    }
}

int main(int argc, char* argv[]){

    pthread_t th[THREAD_NUM];
    pthread_mutex_init(&mutexQueue,NULL);
    pthread_cond_init(&condQueue,Null);
    int i;
    for(i=0;i<THREAD_NUM;i++){

        if( pthread_create(&th[i],NULL,&startThread,NULL) != 0 ){

        perror("Failed to create the thread");
}
    }

    srand(time(NULL));

for(i = 0;i < 10; i++){
    Task t = {
            .a=rand() % 100,
            .b = rand() % 100
    };
submitTask(t);
}

        for(i=0;i<THREAD_NUM;i++){
            if(pthread_join(&th[i],NULL) != 0){
                perror("Failed to join the thread");
            }
    }

    pthread_mutex_destroy(&mutexQueue);
    pthread_cond_destroy(&condQueue);
    return 0;
}