#include "ThreadPool.h"

#include <cassert>

#include "CurrentThread.h"

namespace webserver
{

ThreadPool::ThreadPool(int threadNum, int maxQueueSize)
	: maxQueueSize_(maxQueueSize), 
	  maxThreadSize_(threadNum),
	  running_(false)
{
	assert(maxQueueSize > 0);
	assert(threadNum > 0);
	threads_.reserve(threadNum);
	//queue_.reserve(maxQueueSize);
}

ThreadPool::~ThreadPool()
{
	if(running_)
	{
		stop();
	}
}

void ThreadPool::start()
{
	assert(!running_);
	running_ = true;
	
	for(int i=0; i<maxThreadSize_; ++i)
	{
		char id[32];
		snprintf(id, sizeof(id), "%d", i+1);
		threads_.emplace_back(new webserver::Thread(
		                     std::bind(&ThreadPool::runInThread, this), id));
		threads_[i]->start();
	}
}

void ThreadPool::stop()
{
	{
		std::unique_lock<std::mutex> lock(mutex_);
		running_ = false;
		condition_.notify_all(); /* wakeup all threads */
	}
	for(auto &thread : threads_)
	{
		thread->join();
	}
}

/* producer */
int ThreadPool::addTask(const ThreadPool::Task &task)
{
	if(static_cast<int>(queue_.size()) >= maxQueueSize_)
		return -1;	/* the task queue is filled */
	
	std::unique_lock<std::mutex> lock(mutex_);
	queue_.push_back(std::move(task));
	condition_.notify_one();
	
	return 0;
}

/* consumer */
void ThreadPool::runInThread()
{
	printf("new thread, tid=%d\n", webserver::CurrentThread::tid());
	
	while(running_)
	{
		Task task(take());
		if(task)
		{
			task();
		}
	}
}

ThreadPool::Task ThreadPool::take()
{
	std::unique_lock<std::mutex> lock(mutex_);
	/* spurious wakeup */
	while(queue_.empty() && running_)
	{
		condition_.wait(lock);
	}
	
	Task task;
	if(!queue_.empty())
	{
		task = queue_.front();
		queue_.pop_front();
	}
	
	return task;
}

} //webserver
