#include<stdlib.h>
#include "uthread.h"
#include<iostream>
#include <time.h>

int sum=0;
int *num_array;
lock_t lock;

void* array_sum(void* args){
    int tid = uthread_self();
    acquire(&lock);
    sum += num_array[tid-1];
    release(&lock);
    uthread_terminate(tid);
}

int main(int argc,char* argv[]){

    int thread_count;

    srand (time(NULL));

    if(argc==2)
        thread_count = atoi(argv[1]);
    else 
        thread_count=10;

    num_array = new int [thread_count];
    int *threads = new int [thread_count]; 

    uthread_init(1);

    for (int i=0; i<thread_count; i++) {
        num_array[i] = rand() % thread_count + 1;
    }

    lock_init(&lock);

    for(int i=0;i<thread_count;i++){
        threads[i]=uthread_create(array_sum,NULL);
    }

    for (int i = 0; i < thread_count; i++) {
        uthread_join(threads[i], NULL);
    }

    cout << "num_array = " ;
    for (int i=0; i<thread_count; i++)
        cout << num_array[i] << " " ;
    cout << endl;

    cout << endl << "///////////////////////////////////////////////////" << endl;
    cout << "Sum = " << sum << endl;
    cout << "///////////////////////////////////////////////////" << endl;

    delete[] num_array;
    delete[] threads;


}
