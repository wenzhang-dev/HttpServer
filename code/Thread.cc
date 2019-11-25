#include "Thread.h"

#include <thread>
#include <cassert>

#include <sys/prctl.h>
#include <unistd.h>
#include <sys/syscall.h>

namespace webserver
{

std::atomic<int> Thread::threadNum_(0);

pid_t gettid()
{ 
	return static_cast<pid_t>(::syscall(SYS_gettid)); 
}

/* thread private data */
namespace CurrentThread
{
	
thread_local int t_cachedTid = 0;
/* maybe string is fine */
thread_local char t_tidString[32] = {0};
thread_local int t_tidStringLength = 0;
thread_local const char *t_threadName = nullptr;

}

void CurrentThread::cacheTid()
{
	if(likely(t_cachedTid == 0))
	{
		t_cachedTid = webserver::gettid();
		t_tidStringLength = 
			snprintf(t_tidString, sizeof(t_tidString), "%5d ", t_cachedTid);
	}
}

struct ThreadData
{
	typedef Thread::ThreadFunc ThreadFunc;
	ThreadFunc func_;
	std::string name_;
	pid_t *tid_;
	CountDownLatch *latch_;
	
	explicit ThreadData(const ThreadFunc &func, const std::string name, 
	                    pid_t *tid, CountDownLatch *latch)
		: func_(func),
		  name_(name),
		  tid_(tid),
		  latch_(latch)
	{}
	
	void runInThread()
	{
		*tid_ = CurrentThread::tid();
		/* wakeup main thread */
		latch_->countDown();
		
		/* set name of thread */
		CurrentThread::t_threadName = name_.data();
		prctl(PR_SET_NAME, CurrentThread::t_threadName);
		
		func_();
		CurrentThread::t_threadName = "finished";
	}
	
};

void *startThread(void *arg)
{
	ThreadData *data = static_cast<ThreadData *>(arg);
	data->runInThread();
	delete data;
	
	return nullptr;
}

Thread::Thread(const ThreadFunc &func, const std::string &name)
	: pthreadId_(0),
	  tid_(0),
	  func_(std::move(func)),
	  name_(std::move(name)),
	  started_(false),
	  joined_(false),
	  latch_(1)		/* each thread waits one second */
{
	setDefaultName();
}

Thread::~Thread()
{
	if(started_ && !joined_)
	{
		pthread_detach(pthreadId_);
	}
}

void Thread::start()
{
	assert(!started_);
	started_ = true;
	
	ThreadData *data = new ThreadData(func_, name_, &tid_, &latch_);
	if(pthread_create(&pthreadId_, nullptr, &startThread, data))
	{
		started_ = false;
		delete data;
	}
	else 
	{
		latch_.wait();
		assert(tid_ > 0);
	}
}	

int Thread::join()
{
	assert(started_);
	assert(!joined_);
	
	joined_ = true;
	
	return pthread_join(pthreadId_, NULL);
}

void Thread::setDefaultName()
{
	int num = threadNum_++;	/* atomic operation */
	if(name_.empty())
	{
		char buf[32];
		snprintf(buf, sizeof(buf), "Thread%d", num);
		name_ = buf;
	}
}

} 
