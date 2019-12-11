#include <iostream>
#include <cassert>
#include <functional>

#include <sys/types.h>
#include <unistd.h>

#include "EventLoopThreadPool.h"
#include "CurrentThread.h"
#include "Thread.h"
#include "EventLoop.h"

void print(webserver::EventLoop *p = nullptr)
{
	printf("main(): pid=%d, tid=%d, loop=%p\n",
	       getpid(), webserver::CurrentThread::tid(), p);
}

void init(webserver::EventLoop *p)
{
	printf("init(): pid=%d, tid=%d, loop=%p\n",
	       getpid(), webserver::CurrentThread::tid(), p);
}

int main(int argc, char *argv[])
{
	print();

	webserver::EventLoop loop;
	
	{
		printf("Single thread %p:\n", &loop);
		webserver::EventLoopThreadPool model(&loop, 0);
		model.start(init);
		assert(model.getNextLoop() == &loop);
		assert(model.getNextLoop() == &loop);
		assert(model.getNextLoop() == &loop);
	}
	
	{
		printf("Another thread:\n");
		webserver::EventLoopThreadPool model(&loop, 1);
		model.start(init);
		webserver::EventLoop* nextLoop = model.getNextLoop();
		assert(nextLoop != &loop);
		assert(nextLoop == model.getNextLoop());
	}
	
	{
		printf("Three threads:\n");
		webserver::EventLoopThreadPool model(&loop, 3);
		model.start(init);
		webserver::EventLoop* nextLoop = model.getNextLoop();
		nextLoop->runInLoop(std::bind(print, nextLoop));
		assert(nextLoop != &loop);
		assert(nextLoop != model.getNextLoop());
		assert(nextLoop != model.getNextLoop());
		assert(nextLoop == model.getNextLoop());
	}
	
	loop.loop();
	return 0;
}