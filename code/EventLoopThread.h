#ifndef code_EventLoopThread_h
#define code_EventLoopThread_h

#include <mutex>
#include <condition_variable>
#include <functional>

#include "Thread.h"
#include "noncopyable.h"

namespace webserver
{

class EventLoop;

class EventLoopThread : noncopyable
{
public:
	typedef std::function<void (EventLoop*)> ThreadInitCallback;
	
	EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback());
	~EventLoopThread();
	EventLoop* startLoop();

private:
	void threadFunc();
	
	EventLoop* loop_;
	bool exit_;
	Thread thread_;
	std::mutex mutex_;
	std::condition_variable cond_;
	ThreadInitCallback callback_;
};
	
} //namespace webserver


#endif
