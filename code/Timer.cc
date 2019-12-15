#include "Timer.h"

#include <cassert>

#include <sys/timerfd.h>
#include <unistd.h>
#include <strings.h>

#include "macros.h"
#include "Channel.h"
#include "EventLoop.h"

namespace webserver
{

Timer::Timer(EventLoop *loop)
	: loop_(loop),
	  timerFd_(::timerfd_create(CLOCK_MONOTONIC,
                                TFD_NONBLOCK | TFD_CLOEXEC)),
	  timerChannel_(new Channel(timerFd_, loop_)),
	  timeout_(0)
{
	assert(timerFd_ > 0);
	timerChannel_->setReadCallback(std::bind(&Timer::readCallback, this));
}

Timer::~Timer()
{
	::close(timerFd_);
}

void Timer::addTimer(int nS)
{
	timeout_ = nS;
	
	struct itimerspec newValue;
	bzero(&newValue, sizeof(newValue));
	newValue.it_value.tv_sec = nS;
	
	int ret = ::timerfd_settime(timerFd_, 0, &newValue, nullptr);
	if(unlikely(ret<0))
	{
		perror("timerfd_settime");
	}
	
	if(! timerChannel_->isEnableReading())
	{
		timerChannel_->enableReading();
	}
}

/* 在addTimer后调用 */
void Timer::reset(void)
{
	assert(timeout_ > 0);
	this->addTimer(timeout_);
}

void Timer::addPeriodicTimer(int nS)
{
	timeout_ = nS;
	
	struct itimerspec newValue;
	bzero(&newValue, sizeof(newValue));
	newValue.it_value.tv_sec = nS;		/* 第一次溢出时间 */
	newValue.it_interval.tv_sec = nS;	/* 此后周期性溢出时间 */
		
	int ret = ::timerfd_settime(timerFd_, 0, &newValue, nullptr);
	if(unlikely(ret<0))
	{
		perror("timerfd_settime");
	}
	
	if(! timerChannel_->isEnableReading()) 
	{
		timerChannel_->enableReading();
	}
}

void Timer::readCallback()
{
	uint64_t num = 0;
	int ret = ::read(timerFd_, &num, sizeof(num));
	if(unlikely(ret!=sizeof(num)))
	{
		perror("timerfd read error");
	}
	
	if(expireCallback_)
	{
		expireCallback_();
	}
}

}