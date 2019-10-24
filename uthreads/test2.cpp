#include "uthread.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

unsigned long g_cnt = 0;
int points_per_thread;
int quantum_usecs = 1000;
unsigned int total_count = 0;

void* worker(void* arg) {
    int my_tid = uthread_self();
    unsigned int count = 0;

	while(1){
        count += 1;
        if(count == 100000000){
            printf("thread %d\n", my_tid);
            count = 0;
            total_count += 1;
        }
    }
	    uthread_terminate(my_tid);

}

int main(int argc, char *argv[]) {
    if (argc < 2){
		fprintf(stderr, "Usage: ./pi <threads> [quantum_usecs]\n");
        exit(1);
    }else if (argc == 3){
        quantum_usecs = atoi(argv[2]);
    }

	int i, j, ret;
	//unsigned long totalpoints = atol(argv[1]);
	int thread_count = atoi(argv[1]);
	int *threads = new int[thread_count];
	//points_per_thread = totalpoints / thread_count;

	ret = uthread_init(quantum_usecs);
	if (ret != 0) {
		fprintf(stderr, "uthread_init FAIL!\n");
		exit(1);
	}

	srand(time(NULL));

	for (i = 0; i < thread_count; i++) {
		int tid = uthread_create(worker, NULL);
		printf("tid=%d\n", tid);
		threads[i] = tid;
	}
    
    uthread_suspend(threads[0]);
    
    while(total_count < 10);
    
    printf("resume first thread\n");
    uthread_resume(threads[0]);

	for (i = 0; i < thread_count; i++) {
		uthread_join(threads[i], NULL);
	}

	delete[] threads;

	return 0;
}

/*
typedef enum Priority { RED, ORANGE, GREEN } Priority;
int uthread_init(int quantum_usecs);
int uthread_create(void *(*start_routine)(void), void *arg);
int uthread_terminate(int tid);
int uthread_suspend(int tid);
int uthread_resume(int tid);
int uthread_get_tid();
int uthread_get_total_quantums();
int uthread_get_quantums(int tid);
*/
