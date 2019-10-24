#include<stdlib.h>
#include "uthread.h"
#include<iostream>

int count=0;
lock_t lock;

void* func(void* args){
    int tid = uthread_self();
    acquire(&lock);
    count++;
    cout<<"Count after thread "<<" is "<<count<<endl;
    release(&lock);
    uthread_terminate(tid);
}

int main(int argc,char* argv[]){

    int thread_count;
    if(argc==2)
        thread_count = atoi(argv[1]);
    else thread_count=10;
    int rc;
    lock_init(&lock);
    int threads[thread_count];
    for(int i=0;i<thread_count;i++){
        threads[i] = uthread_create(func,NULL);
    }
    for(int i=0;i<thread_count;i++){
        uthread_join(threads[i],NULL);
    }

    cout << endl << "///////////////////////////////////////////////////" << endl;
    cout << "Count at the end of process = " << count << endl;
    cout << "///////////////////////////////////////////////////" << endl;

}

