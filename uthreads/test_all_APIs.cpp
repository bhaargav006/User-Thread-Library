#include<stdlib.h>
#include "uthread.h"
#include<iostream>

int count=0;
lock_t lock;

void* func(void* args){
    int tid = uthread_self();

    uthread_yield();
    for(int i=0;i<10000000;i++) {};
    for(int i=0;i<10000000;i++) {};
    for(int i=0;i<10000000;i++) {};

    acquire(&lock);
    count++;
    cout<<"Count after thread "<<" is "<<count<<endl;
    release(&lock);

    uthread_yield();
    for(int i=0;i<10000000;i++) {};
    for(int i=0;i<10000000;i++) {};
    for(int i=0;i<10000000;i++) {};
    uthread_terminate(tid);
}

int main(int argc,char* argv[]){

    //Num of threads
    int thread_count=10;

    //Initialize time slice
    uthread_init(1);

    //Initilaize lock
    lock_init(&lock);

    //Array of thread indices
    int threads[thread_count];

    //Create threads
    for(int i=0;i<thread_count;i++){
        threads[i] = uthread_create(func,NULL);
    }

    //Suspend Thread 5
    uthread_suspend(threads[5]);

    //Wait till all threads join except 5
    for(int i=0;i<thread_count;i++){
        if(i != 5) {
            uthread_join(threads[i],NULL);
        }
    }

    cout << endl << "Count without thread 5 = " << count << endl << endl;

    uthread_resume(threads[5]);
    uthread_join(threads[5],NULL);

    //Printing output
    cout << endl << "///////////////////////////////////////////////////" << endl;
    cout << "Count at the end of process = " << count << endl;
    cout << "///////////////////////////////////////////////////" << endl;
}

