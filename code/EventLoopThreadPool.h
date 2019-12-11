#ifndef code_EventLoopThreadPool_h
#define code_EventLoopThreadPool_h

#include <functional>
#include <vector>

#include "noncopyable.h"

namespace webserver
{

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable
{
public:
	typedef std::function<void (EventLoop*)> ThreadInitCallback;
	
	EventLoopThreadPool(EventLoop* baseLoop, int numThreads = 0);
	~EventLoopThreadPool();
	
	void start(const ThreadInitCallback &cb = ThreadInitCallback());
	EventLoop* getNextLoop();
	
private:
	EventLoop* baseLoop_;	/* main loop */
	bool started_;
	int numThreads_;
	int next_;
	std::vector<EventLoopThread *> threads_;
	std::vector<EventLoop *> loops_;	
};

}//namespace webserver

#endif
