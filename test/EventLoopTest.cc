#include <cassert>

#include <unistd.h>
#include <sys/types.h>

#include "EventLoop.h"
#include "Thread.h"
#include "CurrentThread.h"

void functor(std::string str)
{
	printf("%s\n", str.c_str());
}

void threadFunc()
{
	printf("threadFunc(): pid=%d, tid=%d\n", getpid(), webserver::CurrentThread::tid());
	
	assert(webserver::EventLoop::getEventLoopOfCurrentThread() == nullptr);
	webserver::EventLoop loop;
	assert(webserver::EventLoop::getEventLoopOfCurrentThread() == &loop);
	loop.runInLoop(std::bind(&functor, "Hello,World"));
	loop.loop();
}

int main(int argc, char *argv[])
{
	printf("main(): pid=%d, tid=%d\n", getpid(), webserver::CurrentThread::tid());
	
	assert(webserver::EventLoop::getEventLoopOfCurrentThread() == nullptr);
	webserver::EventLoop loop;
	assert(webserver::EventLoop::getEventLoopOfCurrentThread() == &loop);
	
	webserver::Thread t1(threadFunc);
	t1.start();
	
	loop.loop();
	
	return 0;
}