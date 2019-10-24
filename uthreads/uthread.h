#ifndef UTHREAD_H
#define UTHREAD_H
//#define _XOPEN_SOURCE 600
#include<stdio.h>
#include<stdlib.h>
#include<queue>
#include <setjmp.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <map>
#include <iostream>


using namespace std;

#define STACK_SIZE 4096

/* code for 64 bit Intel arch */

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7



//Lock structure
typedef struct {
   int value;
} lock_t;



enum mypthread_state {
	INIT,
	READY,
	BLOCKED,
	SUSPENDED,
	RUNNING,
	FINISHED
};

typedef struct {

        void *(*func)(void *);
	void *arg;
	void **retval;

} ThreadFunction;

typedef struct{
        //Stack pointer and stack size 
	char* stack;
        int stack_size;
	address_t sp;

        //PC
	address_t pc;

	//Context
        sigjmp_buf jbuf;
	
	//Thread ID
	int th_id;

	//Thread State
	enum mypthread_state state;

        //Thread Function
	ThreadFunction* routine;

	//Thread id of the thread waiting on this thread
	int waiting_id;

	void TCB(){

            th_id=0;
            state=INIT;
	    waiting_id = -1;

	}
} TCB;


void enableInterrupts();
void disableInterrupts();
int uthread_create(void *(*start_routine)(void *), void *arg);
void swapcontext(TCB* new_thread, TCB* old_thread);
int uthread_yield(void);
int uthread_self(void);
int uthread_join(int tid, void **retval);
int uthread_init(int time_slice);
int uthread_suspend(int tid);
int uthread_resume(int tid);
int uthread_terminate(int tid);
void timer_handler(int sig);
TCB* getTCB(int tid);
int lock_init(lock_t *lock);
int acquire(lock_t *lock);
int release(lock_t *lock);
#endif
