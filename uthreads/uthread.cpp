#include "uthread.h"
#include<stdio.h>
#include<stdlib.h>
#include<queue>
#include<fstream>
#include<cassert>

using namespace std;

/*
SS_ONSTACK The process is currently executing on the alternate signal stack. Attempts to modify the alternate signal stack while the process is executing on it fail. This flag shall not be modified by processes.
SS_DISABLE The alternate signal stack is currently disabled.
*/

//decide on datastructure later. Ideally a structure with easy removal of a thread. Have to see about round robin 
//Todo: We shoukd use our own data structires and not use the library 

//runningq is not necessary as only one thread is running. But it is better to implement if the code has to be scaled to multiprocessor system. 
deque<TCB*> readyq,finishedq;
map<int,TCB*> blockedq;
TCB* running;
TCB* scheduler;
//std::ofstream f("stackContent.txt",std::ofstream::out);

//Sigaction for SIGVTALARM
struct sigaction act_timer;

//Timer Variable
struct itimerval timer,remainingtime;

//Scheduler has tid = 0
int numThreads=0;

//Test and Set function
int TAS(volatile int *addr, int newval){
    int result = newval;
    asm volatile("lock; xchg %0, %1"
                 : "+m" (*addr), "=r" (result)
                 : "1" (newval)
                 : "cc");
    return result;
                
}

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
	address_t ret;
	asm volatile("xor    %%fs:0x30,%0\n"
			"rol    $0x11,%0\n"
			: "=g" (ret)
			  : "0" (addr));
	return ret;
}

void enableInterrupts() {
    sigemptyset(&act_timer.sa_mask);
}

void disableInterrupts() {
    sigaddset(&act_timer.sa_mask, SIGVTALRM);
}

int uthread_create(void *(*start_routine)(void *), void *arg){


	if (numThreads == 0) {


            ///////////////////////////////////////////////////////////////////////////////
            //                         Create Main thread                                //
            ///////////////////////////////////////////////////////////////////////////////
	    
            std::cout << "creating main thread" << std::endl;

	    TCB* main_thread = (TCB*)malloc(sizeof(TCB));

            //Thread ID
	    main_thread->th_id = numThreads++;

	    //Stack
            main_thread->stack = (char*)malloc(STACK_SIZE);

	    //Initializing stack pointer and stack size
	    main_thread->stack_size = STACK_SIZE;
            main_thread->sp = (address_t)main_thread->stack + STACK_SIZE - sizeof(int); 

	    //Initializing thread function
	    main_thread->routine = (ThreadFunction*)malloc(sizeof(ThreadFunction));
	    main_thread->routine->func = NULL;
	    main_thread->routine->arg = NULL;
	    main_thread->routine->retval = NULL;

            //Initializing waiting_id
	    main_thread->waiting_id = -1;

	    //Thread State is ready and push to readyq
            running = main_thread;
            running->state = RUNNING;

	    //Set action for SIGVTALARM
            act_timer.sa_handler = &timer_handler;
            act_timer.sa_flags=0;
            sigemptyset(&act_timer.sa_mask);
            sigaddset(&act_timer.sa_mask, SIGVTALRM);
            sigaction(SIGVTALRM, &act_timer, NULL);
            if (setitimer(ITIMER_VIRTUAL, &timer, NULL) == -1) {
                perror("error calling setitimer()");
	        std::cout << "Unable to call setitimer for thread = " << main_thread->th_id << std::endl;
            }
	}

	TCB* thread = (TCB*)malloc(sizeof(TCB));

	//Thread ID
	thread->th_id = numThreads++;

        std::cout << "uthread_create: " << thread->th_id << std::endl;

	//Stack
        thread->stack = (char*)malloc(STACK_SIZE);

	//Initializing stack pointer and stack size
	thread->stack_size = STACK_SIZE;
        thread->sp = (address_t)thread->stack + STACK_SIZE - sizeof(int); 

	//PC
	thread->pc = (address_t)start_routine;

	//Initializing thread function
	thread->routine = (ThreadFunction*)malloc(sizeof(ThreadFunction));
	thread->routine->func = start_routine;
	thread->routine->arg = arg;
	thread->routine->retval = NULL;

        //Initializing waiting_id
	thread->waiting_id = -1;

	//Initialize jbuf
        int ret_val = sigsetjmp(thread->jbuf,1);
        (thread->jbuf->__jmpbuf)[JB_SP] = translate_address(thread->sp);
        (thread->jbuf->__jmpbuf)[JB_PC] = translate_address(thread->pc);
        sigemptyset(&thread->jbuf->__saved_mask);     

	//Thread State is ready and push to readyq
	thread->state=READY;
	readyq.push_front(thread);
       
	return thread->th_id;
}

//void printStack(TCB* tcb){
//   f<<"STACK CONTENT FOR "<< tcb->th_id<<"\n";
//   for(int i=0;i<=STACK_SIZE;i++){
//      f<< (int)tcb->stack[i]<<std::endl;
//   }
//   f<<std::endl; 
//}

void swapcontext(TCB* new_thread, TCB* old_thread)
{
    if (setitimer(ITIMER_VIRTUAL, &timer, &remainingtime) == -1) {
           perror("error calling setitimer()");
           std::cout << "Unable to call setitimer for thread = " << old_thread->th_id << std::endl;
    }
    int ret_val = sigsetjmp(old_thread->jbuf,1);
    if (ret_val == 1) {
	enableInterrupts();
        return;
    }
    //The following is needed for the first call of the worker thread
    running = new_thread;
    running->state = RUNNING;
    enableInterrupts();

    //Call siglongjmp
    siglongjmp(new_thread->jbuf,1);
}

////It is a single thread process, so there is always just a thread running at a time : Running
////Context switch will switch running with ready
////Todo: signal masking 
int uthread_yield(){
	//implement signal masking
	disableInterrupts();

	//Push to readq 
	running->state = READY;
	readyq.push_front(running);

        TCB* chosenTCB = readyq.back();

        //Pop readyq
        readyq.pop_back();

        //Context Switch
        std::cout << "uthread_yield:: Thread " << running->th_id <<" yielding to thread "<< chosenTCB->th_id<< std::endl;
        swapcontext(chosenTCB, running);
}

int uthread_join(int tid, void **retvalue) {
    //Disable interrupts
    disableInterrupts();

    TCB* child = getTCB(tid);

    if(child == NULL) {
        std::cout << "uthread_join:: Tid entered is invalid" << std::endl;
        return 0;
    }

    if (child->state != FINISHED) {
        child->waiting_id = running->th_id;
        running->state = BLOCKED;
        blockedq.insert(make_pair(running->th_id,running));
        std::cout << "uthread_join:: Thread " << running->th_id <<" waiting on thread "<<child->th_id<< std::endl;
        if (readyq.empty()) {
            std::cout<<"Nothing to run. Readyq is empty. Child state = " << child->state << std::endl;
            std::cout<<"Continuing running the thread "<<running->th_id<<" instead."<<std::endl;
            return 0;  
        }

        TCB* chosenTCB = readyq.back();

        //Pop readyq
        readyq.pop_back();

        //Context Switch
        std::cout << "uthread_join:: switching to thread " << chosenTCB->th_id << std::endl;
        swapcontext(chosenTCB, running);
    }
    retvalue = child->routine->retval ;
    return 1;
}

int uthread_self() {
    return running->th_id;
}

int uthread_init(int time_slice) {
    
    std::cout << "setting timer = " << time_slice << "ms" << std::endl;
    timer.it_value.tv_sec = time_slice / 1000;
    timer.it_value.tv_usec = (time_slice * 1000) % 1000000;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    return 0;
}

int uthread_suspend(int tid) {
    //Disable interrupts
    disableInterrupts();
    
    //Get TCB
    TCB* suspendedTCB = getTCB(tid);
    if(suspendedTCB == NULL) {
        std::cout << "uthread_suspend:: Tid entered is invalid" << std::endl;
        return 0;
    }

    //Remove from readyq if it is present
    int i = 0;
    for (std::deque<TCB*>::iterator it = readyq.begin(); it!=readyq.end(); ++it,++i) {
        if (*it == suspendedTCB) {
	    assert((*it)->th_id == suspendedTCB->th_id);
            readyq.erase(it);
            break;
        }
    }

    //Push to blockedq 
    suspendedTCB->state = SUSPENDED;
    blockedq.insert(make_pair(tid,suspendedTCB));
    
    //Swap only if thread suspends itself
    if (running->th_id != tid) {
        std::cout << "uthread_suspend:: suspended thread " << tid << std::endl;
        return 1;
    }    

    //Switch to next ready thread
    TCB* chosenTCB = readyq.back();

    //Pop readyq
    readyq.pop_back();

    //Context Switch
    std::cout << "uthread_suspend:: suspended thread " << tid << " and switching to thread " << chosenTCB->th_id << std::endl;
    swapcontext(chosenTCB, running);
    
    return 1;
}


//Return 1 if successful; 0 if not successful
int uthread_resume(int tid) {
    //Disable interrupts
    disableInterrupts();
    
    TCB* resumeTCB ;
    if (blockedq.find(tid) != blockedq.end() && blockedq.find(tid)->second->state == SUSPENDED) {
        resumeTCB = blockedq.find(tid)->second;

        //Push to readyq 
        resumeTCB->state = READY;
        readyq.push_front(resumeTCB);
         
        std::cout << "uthread_resume:: pushed thread " << tid << " to readyq" << std::endl;
	enableInterrupts();
	return 1;
    }

    enableInterrupts();
    return 0;

};



int uthread_terminate(int tid) {
    //Disable interrupts
    disableInterrupts();

    //Get the terminated thread's TCB
    TCB* finishedTCB = getTCB(tid);
    if(finishedTCB == NULL) {
        std::cout << "uthread_terminate:: Tid entered is invalid" << std::endl;
        return 0;
    }
    
    std::cout << "uthread_terminate: Terminated thread " << tid << std::endl; 

    //Checking if there is a waiting thread
    if (finishedTCB->waiting_id != -1 ) {
        //Push parent thread to readq 
        TCB* parent = getTCB(finishedTCB->waiting_id);
        parent->state = READY;
        std::cout << "uthread_terminate:: Pushing waiting thread " << parent->th_id << " to readyq" << std::endl;
        readyq.push_front(parent);
	blockedq.erase(parent->th_id);
    }

    //Push finished thread to finishedq
    finishedTCB->state = FINISHED;
    finishedq.push_front(finishedTCB);
   
    if (readyq.empty()) {
	    std::cout << "ready queue empty while trying to terminate tid=" << tid << std::endl;
	    return 0;
    }

    TCB* chosenTCB = readyq.back();

    //Pop readyq
    readyq.pop_back();

    //Context Switch
    std::cout << "uthread_terminate:: switching to thread " << chosenTCB->th_id << std::endl;
    swapcontext(chosenTCB, running);

    return 1;
 
}

TCB* getTCB(int tid) {

    if (running->th_id == tid) 
        return running;

    //Search in blockedq
    if (blockedq.find(tid) != blockedq.end()) {
        return blockedq.find(tid)->second;
    }

    //Search in readyq
    for (deque<TCB*>::iterator it = readyq.begin(); it != readyq.end(); it++) {
        if ((*it)->th_id == tid) {
            return *it;
        }
    }

    //Search in finishedq
    for (deque<TCB*>::iterator it = finishedq.begin(); it != finishedq.end(); it++) {
        if ((*it)->th_id == tid) {
            return *it;
        }
    }

    return NULL;
}

void timer_handler(int sig) {

    //Disable Interrupts
    disableInterrupts();

    //Push to readq 
    running->state = READY;
    readyq.push_front(running);

    //get thread from top of readq
    if (readyq.empty()) {
        if (setitimer(ITIMER_VIRTUAL, &timer, NULL) == -1) {
            perror("error calling setitimer()");
            std::cout << "Unable to call setitimer for running tid:" << running->th_id << std::endl;
        }
        return;
    }

    TCB* chosenTCB = readyq.back();

    //Pop readyq
    readyq.pop_back();

    std::cout << "timer_handler:: Interrupted thread " << running->th_id << " and switching to thread " << chosenTCB->th_id << std::endl;
    swapcontext(chosenTCB, running);

    return;
}

int lock_init(lock_t *lock) {
    lock->value = 0;
    return 0;
}

int acquire(lock_t *lock) {
    while (TAS(&(lock->value),1)) {
        ;
    }
    return 1;
}

int release(lock_t *lock) {
    lock->value = 0;
    return 1;
}

//int main() {
//    return 0;
//}
