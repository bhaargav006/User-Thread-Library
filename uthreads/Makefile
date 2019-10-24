
all: clean array_sum lock_test suspend_test all_api test1 test2

array_sum:
	g++  uthread.cpp test_array_sum.cpp -o array_sum

lock_test:
	g++  uthread.cpp test_lock.cpp -o lock_test

suspend_test:
	g++  uthread.cpp test_suspend_resume.cpp -o suspend_test

all_api:
	g++  uthread.cpp test_all_APIs.cpp -o all_api

test1:
	g++ uthread.cpp test1.cpp -o test1

test2:
	g++ uthread.cpp test2.cpp -o test2

clean:
	rm -f array_sum
	rm -f lock_test
	rm -f suspend_test
	rm -f all_api
	rm -f test1
	rm -f test2
