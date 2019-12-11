#include "EventLoopThreadPool.h"

#include <cassert>

#include "EventLoop.h"
#include "macros.h"
#include "EventLoopThread.h"

namespace webserver
{
	
EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, int numThreads)
	: baseLoop_(baseLoop),
	  started_(false),
	  numThreads_(numThreads),
	  next_(0)
{}

EventLoopThreadPool::~EventLoopThreadPool()
{
	// Don't delete loop, it's stack variable
}	

void EventLoopThreadPool::start(const ThreadInitCallback &cb)
{
	assert(unlikely(!started_));
	assert(unlikely(baseLoop_->isInLoopThread()));
	
	started_ = true;
	for(int i=0; i<numThreads_; ++i)
	{
		EventLoopThread *t =new EventLoopThread(cb);
		threads_.push_back(t);
		loops_.push_back(t->startLoop());
	}
	
	if(numThreads_ == 0 && cb)
	{
		cb(baseLoop_);
	}
}

//sub-reactor
EventLoop* EventLoopThreadPool::getNextLoop()
{
	assert(unlikely(baseLoop_->isInLoopThread()));
	EventLoop *loop = baseLoop_;
	
	if(!loops_.empty())
	{
		//round-robin
		loop = loops_[next_++];
		if(static_cast<size_t>(next_) >= loops_.size())
		{
			next_ = 0;
		}
	}
	
	return loop;
}

} //namespace webserver
