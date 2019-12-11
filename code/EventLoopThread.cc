#include "EventLoopThread.h"

#include <cassert>

#include "EventLoop.h"

namespace webserver
{
	
EventLoopThread::EventLoopThread(const ThreadInitCallback &cb)
	: loop_(nullptr),
	  exit_(false),
	  thread_(std::bind(&EventLoopThread::threadFunc, this)),
	  callback_(cb)
{}

EventLoopThread::~EventLoopThread()
{
	exit_ = true;
	loop_->quit();
	thread_.join();
}

EventLoop *EventLoopThread::startLoop()
{
	assert(!thread_.started());
	thread_.start();
	
	{
		std::unique_lock<std::mutex> lock(mutex_);
		while(loop_ == nullptr)
		{
			cond_.wait(lock);
		}
	}
	
	return loop_;
}

void EventLoopThread::threadFunc()
{
	EventLoop loop;
	if(callback_)
	{
		callback_(&loop);
	}
	
	{
		std::unique_lock<std::mutex> lock(mutex_);
		loop_ = &loop;
		cond_.notify_one();
	}
	
	loop.loop();
}

} //namespace webserver