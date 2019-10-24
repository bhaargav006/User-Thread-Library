#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "uthread.h"
#include <sys/time.h>

unsigned long g_cnt = 0;
int points_per_thread;
int quantum_usecs = 1000;

void* worker(void* arg) {
    int my_tid = uthread_self();
	printf("Inside tid %d\n", my_tid);

	unsigned long local_cnt = 0;
	unsigned int rand_state = rand();
	for (int i = 0; i < points_per_thread; i++) {
		double x = rand_r(&rand_state) / ((double)RAND_MAX + 1) * 2.0 - 1.0;
		double y = rand_r(&rand_state) / ((double)RAND_MAX + 1) * 2.0 - 1.0;
		if (x * x + y * y < 1)
			local_cnt++;
	}
        uthread_yield();
        std::cout << "after yield " << my_tid << std::endl;
        for(int i=0;i<10000000;i++) {};
        for(int i=0;i<10000000;i++) {};
        for(int i=0;i<10000000;i++) {};
        for(int i=0;i<10000000;i++) {};
        for(int i=0;i<10000000;i++) {};
        for(int i=0;i<10000000;i++) {};
	g_cnt += local_cnt;
	uthread_terminate(my_tid);
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		fprintf(stderr, "Usage: ./pi <total points> <threads> [quantum_usecs]\n");
		exit(1);
	} else if (argc == 4) {
		quantum_usecs = atoi(argv[3]);
	}

	int i, j, ret;
	unsigned long totalpoints = atol(argv[1]);
	int thread_count = atoi(argv[2]);
	int *threads = new int[thread_count];
	points_per_thread = totalpoints / thread_count;


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

	//Calling scheduler_routine here is not correct
	//Scheduler routine exits and main becomes the thread 0
	//We need scheduler to be run independant of the other threads
	//including main 
	//To create a scheduler thread, we can make the user call
	//a thread_create for the scheduler
	//Or, we can make use of another signal to switch context to
	//the scheduler
        //scheduler_routine(NULL);

	for (i = 0; i < thread_count; i++) {
		uthread_join(threads[i], NULL);
	}

	delete[] threads;

	printf("Pi: %f\n", (4. * (double)g_cnt) / ((double)points_per_thread * thread_count));

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
