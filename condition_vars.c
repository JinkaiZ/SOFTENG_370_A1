#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

#define NUM_THREADS 5

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static int count = 0;

void *cond_thread(void *id) {
    
    pthread_mutex_lock(&mutex);

    int thread_id = (int)(long)id; // Never do this in your code.

    // do some work - in this case sleep for 5 seconds
    for(int i = 0; i < 5; ++i) {
	sleep(1);
	printf("thread %d: working hard [%d/5]\n", thread_id, i + 1);
    }
    printf("thread %d: done!\n", thread_id);

    // we've done our 'work', so update the count and signal that we've changed it
    ++count;

    // will release the lock and signal
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, const char **argv) {

    pthread_mutex_lock(&mutex);

    // create 5 threads
    pthread_t threads[NUM_THREADS];
    for(int i = 0; i < NUM_THREADS; ++i) {
	pthread_create(&threads[i], NULL, cond_thread, (void *)(long)(i + 1));
    }

    printf("main thread: start\n");

    // wait until count == 5
    // always good to check condition variables in a while loop
    while(count != NUM_THREADS) {
        
        // will release the lock
        pthread_cond_wait(&cond, &mutex);
        printf("main thread: awoke, count is now %d\n", count);
    }
    pthread_mutex_unlock(&mutex); 

    printf("main thread: done\n");
}
