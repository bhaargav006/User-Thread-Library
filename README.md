# User-Thread-Library

OVERVIEW

This project was implemented as a part of the OS course at the University of Minnesota.  

The project focuses on implementing basic thread library functionality, along with the relevant APIs for the control of the threads that are created/deleted by the self-declared APIs. With the help of the implemented APIs, basic Round-Robin scheduling is implemented in a preemptive fashion. Few test cases are written to showcase the multithreaded nature of the code that is written with the help of a few toy examples that showcase simple calculations.
Engineers in the industry are sometimes required to implement their own thread library due to:

1. The absence of a ​pthread​ library on some embedded systems;
2. The need for fine-grained control over thread behaviour; and 
3. Efficiency

The user-level thread library can replace the default pthread library for general use as it provides similar functionalities.

TESTS

A. test_all_APIs.cpp: 

	Description - tests all APIs. The test does the following:
  
		1. Spawns 10 threads
		2. Thread 5 is suspended
		3. Each thread increments a global variable after acquiring the lock
		4. All threads (except 5) join the main thread after terminating themselves.
		5. Thread 5 is resumed.
		6. Thread 5 joins the main thread after completion
		7. Print output

B. test_array_sum.cpp: 

	Description - Computes the sum of an array by spawning given number of threads. The test does the following:
  
		1.Accepts number of threads as an argument
		2.Generates an integer array with random numbers
		3.Spawns n threads
		4.Each thread reads one array element and updates the sum after acquiring the lock
		5.All threads join the main thread after terminating themselves 
		6.Print output
    
C. test_lock.cpp

	Description - Tests lock. The test does the following:
  
		1.Accepts number of threads as an argument
		2.Each thread increments a global variable after acquiring lock. (Thread releases lock after update)
		3.All threads join the main thread after terminating themselves 
		4.Print output
    
D. test_suspend_resume.cpp

	Description - Tests suspend and resume. The test does the following:
  
		1.Spawns 10 threads
		2.Each thread increments a global variable after acquiring lock. (Thread releases lock after update)
		3.All threads join the main thread after terminating themselves 
		4.Print output

E. Steps to build the tests:

	make all 			: builds all tests
	make test_name		: Builds the particular test
	make clean			: Deletes the object files for all tests

F. Steps to run the tests:

	./all_api			:Runs test_all_APIs.cpp (10 threads)
	./array_sum <num_threads>	:Runs test_array_sum.cpp
					(Accepts number of threads as argument (default 10))
	./lock_test <num_threads>	:Runs test_lock.cpp
						(Accepts number of threads as argument (default 10))
	./suspend_test			:Runs test_suspend_resume.cpp (10 threads)



