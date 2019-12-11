#include "ThreadPool.h"
#include "CountDownLatch.h"

void print()
{
	printf("%s\n", "Hello,World");
}

void printString(const std::string &str)
{
	printf("%s\n", str.c_str());
}

int main()
{
	webserver::ThreadPool pool(6, 100);
	pool.start();
	
	/* no arguments function*/
	pool.addTask(print);
	
	/* function with arguments */
	for(int i=0; i<100; ++i)
	{
		char buf[32];
		snprintf(buf, sizeof(buf), "task %d", i);
		pool.addTask(std::bind(&printString, std::string(buf)));
	}
	
	/* member functions with no argument */
	webserver::CountDownLatch latch(1);
	pool.addTask(std::bind(&webserver::CountDownLatch::countDown, &latch));
	latch.wait();
	
	pool.stop();
	
	return 0;
}
