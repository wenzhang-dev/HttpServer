#include "EventLoop.h"

#include <cassert>

#include <sys/eventfd.h>
#include <unistd.h>

#include "Epoll.h"
#include "Channel.h"
#include "CurrentThread.h"
#include "HttpHandler.h"

namespace webserver
{

thread_local EventLoop *t_loopInThisThread = nullptr;
static const int kEPollTimeMs = 10000;	//10s

int createEventFd()
{
	int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if(evtfd < 0)
	{
		fprintf(stderr, "failed in eventfd\n");
		abort();
	}
	return evtfd;
}

EventLoop::EventLoop()
	: looping_(false),
	  quit_(false),
	  threadId_(CurrentThread::tid()),
	  poller_(new Epoll()),
	  wakeupFd_(createEventFd()),
	  wakeupChannel_(new Channel(wakeupFd_, this)),
	  callingPendingFucntors_(false)
{
	if(unlikely(t_loopInThisThread))
	{
		/* one threads only owns one eventloop */
		fprintf(stderr, "one threads only owns one eventloop\n");
		abort();
	}
	t_loopInThisThread = this;
	
	/* may be wakeuped from other thread */
	wakeupChannel_->setReadCallback(
		std::bind(&EventLoop::wakeupRead, this));
	wakeupChannel_->enableReading();	/* core dump, why??? */
}

EventLoop::~EventLoop()
{
	::close(wakeupFd_);
	t_loopInThisThread = nullptr;
}

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
	return t_loopInThisThread;
}

void EventLoop::loop()
{
	assert(!looping_);
	assert(isInLoopThread());
	
	looping_ = true;
	ChannelVector activeChannels;
	while(!quit_)
	{
		activeChannels.clear();
		/* acquire activate events */
		activeChannels = poller_->poll(kEPollTimeMs);
		
		/* handle activate events */
		/* ?? may have a rece condition ?? */
		for(auto &it : activeChannels)
		{
			/* 处理读写，并更新状态 */
			it->handleEvent();

			/* 根据状态反馈Http */
			/* acceptFd_, wakeupfd是没有插入到httpMap */
			/* 上诉fd，不进入该分支 */
			if(it->isReading() && httpMap.count(it))
			{
				SP_HttpHandler &handler = httpMap[it];
				handler->handleHttpReq();
			}
		}
		
		/* handle extra functors */
		doPendingFunctors();
	}
	looping_ = false;
}

void EventLoop::quit()
{
	quit_ = true;
	/* non-eventloop thread quits the eventloop */
	if(!isInLoopThread())
	{
		wakeup();
	}
}

void EventLoop::wakeup()
{
	uint64_t one = 1;
	ssize_t nbytes = ::write(wakeupFd_, &one, sizeof(one));
	if(unlikely(nbytes != sizeof(one))){
		fprintf(stderr, "wakeup failed\n");
	}
}

void EventLoop::wakeupRead()
{
	uint64_t one; 
	ssize_t nbytes = ::read(wakeupFd_, &one, sizeof(one));
	if(unlikely(nbytes != sizeof(one)))
	{
		fprintf(stderr, "wakeupRead failed\n");
	}
}

/* be used in eventloop */
void EventLoop::runInLoop(Functor &&cb)
{
	if(likely(isInLoopThread()))
	{
		cb();
	}
	else
	{
		queueInLoop(std::move(cb));
	}
}

/* be used in other threads */
void EventLoop::queueInLoop(Functor &&cb)
{
	{
		std::unique_lock<std::mutex> lock(mutex_);
		pendingFunctors_.push_back(cb);
	}
	if(!isInLoopThread() || callingPendingFucntors_)
	{
		wakeup();
	}
}

void EventLoop::doPendingFunctors()
{
	std::vector<Functor> functors;
	callingPendingFucntors_ = true;
	
	/* use a local variable to reduce critical region */
	{
		std::unique_lock<std::mutex> lock(mutex_);
		functors.swap(pendingFunctors_);
	}
	for(auto &functor : functors)
	{
		functor();
	}
	
	callingPendingFucntors_ = false;
}

/* void EventLoop::updateChannel(SP_Channel &&channel) */
/* shared_from_this() return a temporary variable(rvalue) */
void EventLoop::updateChannel(SP_Channel channel)
{
	assert(isInLoopThread());
	assert(channel->ownerLoop() == this);
	
	poller_->updateChannel(channel);
}

void EventLoop::removeChannel(SP_Channel channel)
{ 
	poller_->removeChannel(channel);
	httpMap.erase(channel);
}

void EventLoop::addHttpConnection(SP_HttpHandler handler, SP_Channel channel)
{
	httpMap.insert(make_pair(channel, handler));
}

} //namespace webserver
