#include <iostream>

#include <sys/types.h>
#include <unistd.h>

#include "Thread.h"

void threadFunc1()
{
	printf("tid=%d\n", webserver::CurrentThread::tid());
}

void threadFunc2(int x)
{
	printf("tid=%d, x=%d\n", webserver::CurrentThread::tid(), x);
}

class Foo
{
public:
	explicit Foo(double x)
		: x_(x)
	{}
	
	void memberFunc1()
	{
		printf("tid=%d, Foo::x_=%f\n", webserver::CurrentThread::tid(), x_);
	}
	
	void memberFunc2(const std::string &text)
	{
		printf("tid=%d, Foo::x_=%f, text=%s\n", 
			   webserver::CurrentThread::tid(), x_, text.data());
	}
	
private:
	double x_;
};

int main(int argc, char *argv[])
{
	printf("pid=%d, tid=%d\n", ::getpid(), webserver::CurrentThread::tid());
	
	webserver::Thread t1(threadFunc1);
	t1.start();
	printf("t1.tid=%d\n", t1.tid());
	t1.join();
	
	webserver::Thread t2(std::bind(threadFunc2, 42), "function with arguments");
	t2.start();
	printf("t2.tid=%d\n", t2.tid());
	t2.join();
	
	Foo foo(87.53);
	webserver::Thread t3(std::bind(&Foo::memberFunc1, &foo), 
		                 "object function with arguments");
	t3.start();
	printf("t3.tid=%d\n", t3.tid());
	t3.join();
	
	webserver::Thread t4(std::bind(&Foo::memberFunc2, std::ref(foo),
	                     std::string("Hello, World")));
	t4.start();
	printf("t4.tid=%d\n", t4.tid());
	t4.join();
	
	{
		webserver::Thread t5(threadFunc1);
		t5.start();
		::sleep(1);
		/* t5 will destruct */
	}
	
	/* wait for t5 destruction */
	::sleep(2);
	
	printf("number of created threads %d\n", webserver::Thread::numCreated());
	
	return 0;
}
